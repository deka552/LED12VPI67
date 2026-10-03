#include "Arduino.h"
// RGB-лента 12 В + Arduino Nano + MOSFET-модули
// После подачи питания лента выключена.
// Каждое нажатие кнопки - следующий цвет на полной яркости на ON_TIME (10 с),
// затем лента гаснет. Нажатие во время свечения - сразу следующий цвет,
// и отсчёт 10 секунд начинается заново.
// Порядок: белый -> жёлтый -> оранжевый -> красный -> зелёный -> (снова белый)

// ================== НАСТРОЙКИ ==================
// Как подключена кнопка (оставьте ОДНУ строку):
#define BTN_TO_GND        // кнопка между D2 и GND, без резистора (INPUT_PULLUP)
// #define BTN_TO_5V      // кнопка между D2 и +5V, резистор 10 кОм от D2 на GND

// Пины ШИМ. Если D9, D10, D11 сгорели - используем D3, D5, D6.
// На новой Nano можно вернуть 9, 10, 11.
#define PIN_R      9      // MOSFET красного канала
#define PIN_G      10      // MOSFET зелёного канала
#define PIN_B      11     // MOSFET синего канала
#define PIN_BTN    2      // кнопка

#define ON_TIME    10000  // сколько горит цвет после нажатия, мс (10 секунд)
#define FILTER_MS  20     // фильтр помех кнопки: сколько мс "перевеса" нужно для срабатывания
// ===============================================

#ifdef BTN_TO_GND
  #define BTN_PRESSED LOW
  #define BTN_MODE    INPUT_PULLUP
#else
  #define BTN_PRESSED HIGH
  #define BTN_MODE    INPUT
#endif

// ---- Цвета (R, G, B: 0..255) ----
struct Color { byte r, g, b; };
const Color colors[] = {
  {255, 135,  35},   // белый, очень тёплый
  {255, 130,   0},   // жёлтый (если зеленит - уменьшите второе число)
  {255,  60,   0},   // оранжевый (если желтит - уменьшите второе число)
  {255,   0,   0},   // красный
  {  0, 255,   0},   // зелёный
};
const byte NUM = sizeof(colors) / sizeof(colors[0]);

void handleButton();
void handleTimer();
void show();
void ledOff();

int  idx = -1;              // -1: ещё ни один цвет не включали (первое нажатие - белый)
bool isOn = false;          // горит ли сейчас лента
unsigned long onStart = 0;  // когда включили текущий цвет

// Состояние кнопки (фильтр-накопитель)
int  btnCount = 0;                  // 0 = точно отпущена, FILTER_MS = точно нажата
bool btnState = false;              // принятое состояние: true = нажата
unsigned long lastSample = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_BTN, BTN_MODE);
  ledOff();                 // при старте лента выключена
}

void loop()
{
  handleButton();
  handleTimer();
}

// ---------- Кнопка: каждое нажатие - следующий цвет ----------
// Фильтр-накопитель: раз в 1 мс читаем кнопку. "Нажата" - счётчик +1, "отпущена" - счётчик -1.
// Состояние меняется, только когда счётчик дошёл до края.
// Отдельные всплески помех от включённой ленты лишь немного сдвигают счётчик
// и не мешают распознать настоящее нажатие.
void handleButton()
{
  unsigned long now = millis();
  if (now == lastSample) return;    // опрос раз в 1 мс
  lastSample = now;

  bool pressedNow = (digitalRead(PIN_BTN) == BTN_PRESSED);

  if (pressedNow) { if (btnCount < FILTER_MS) btnCount++; }
  else            { if (btnCount > 0)         btnCount--; }

  if (!btnState && btnCount >= FILTER_MS)   // уверенно нажата
  {
    btnState = true;
    idx = (idx + 1) % NUM;
    show();
    isOn = true;
    onStart = now;                  // отсчёт 10 секунд заново
    Serial.print("Цвет: ");
    Serial.println(idx + 1);
  }
  else if (btnState && btnCount == 0)       // уверенно отпущена
  {
    btnState = false;
  }
}

// ---------- Таймер: через ON_TIME гасим ленту ----------
void handleTimer()
{
  if (isOn && millis() - onStart >= ON_TIME)
  {
    ledOff();
    isOn = false;
    Serial.println("Выключено (прошло 10 секунд)");
  }
}

// ---------- Вывод цвета (полная яркость из таблицы) ----------
void show()
{
  analogWrite(PIN_R, colors[idx].r);
  analogWrite(PIN_G, colors[idx].g);
  analogWrite(PIN_B, colors[idx].b);
}

void ledOff()
{
  analogWrite(PIN_R, 0);
  analogWrite(PIN_G, 0);
  analogWrite(PIN_B, 0);
}
