#include "Arduino.h"
// RGB-лента 12 В + Arduino Nano + MOSFET-модули
// Короткое нажатие - следующий цвет по кругу:
//   белый -> жёлтый -> оранжевый -> красный -> зелёный -> (снова белый)
// Удержание 1 секунду - включение / выключение ленты

// ================== НАСТРОЙКИ ==================
// Закомментируйте строку ниже, чтобы отключить включение/выключение удержанием.
// Тогда лента загорается белым сразу после подачи питания, а кнопка только меняет цвета.
#define onOff

#define PIN_R      9      // MOSFET красного канала
#define PIN_G      10     // MOSFET зелёного канала
#define PIN_B      11     // MOSFET синего канала
#define PIN_BTN    2      // кнопка между D2 и +5V, резистор 10 кОм между D2 и GND
#define BTN_PRESSED HIGH  // нажата = 1, отпущена = 0
#define HOLD_TIME  1000   // время удержания для вкл/выкл, мс
#define DEBOUNCE   50     // защита от дребезга, мс
// ===============================================

// ---- Цвета (R, G, B: 0..255) ----
struct Color { byte r, g, b; };
const Color colors[] = {
  // {255, 150,  50},   // тёплый белый
  {255, 135,  35},   // очень тёплый (как лампа накаливания)
  // {255, 120,  25},   // свеча, почти янтарный
  {255, 130,   0},   // жёлтый (если зеленит - уменьшите второе число)
  {255,  60,   0},   // оранжевый (если желтит - уменьшите второе число)
  {255,   0,   0},   // красный
  {  0, 255,   0},   // зелёный
};
const byte NUM = sizeof(colors) / sizeof(colors[0]);

void handleButton();
void show();
void ledOff();

byte idx = 0;             // текущий цвет

#ifdef onOff
bool enabled = false;     // при старте выключено, включается удержанием 1 сек
#else
bool enabled = true;      // вкл/выкл отключено - лента горит сразу
#endif

// Состояние кнопки
bool lastReading = !BTN_PRESSED;
bool stableState = !BTN_PRESSED;
unsigned long lastChange = 0;
unsigned long pressStart = 0;
bool holdDone = false;

void setup()
{
  Serial.begin(9600);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP );        // внешний стягивающий резистор к GND

  if (enabled) show();    // без onOff - сразу белый
  else         ledOff();
}

void loop()
{
  handleButton();
}

// ---------- Обработка кнопки ----------
void handleButton()
{
  bool reading = digitalRead(PIN_BTN);
  unsigned long now = millis();

  if (reading != lastReading)
  {
    lastChange = now;
    lastReading = reading;
  }

  // Состояние стабильно дольше DEBOUNCE - принимаем его
  if (now - lastChange > DEBOUNCE && reading != stableState)
  {
    stableState = reading;

    if (stableState == BTN_PRESSED)
    { // кнопку нажали
      pressStart = now;
      holdDone = false;
    }
    else
    { // кнопку отпустили
      if (!holdDone && enabled)
      { // короткое нажатие - следующий цвет
        idx = (idx + 1) % NUM;
        show();
        Serial.print("Цвет: ");
        Serial.println(idx + 1);
      }
    }
  }

#ifdef onOff
  // Удержание 1 секунду - вкл/выкл (срабатывает, не дожидаясь отпускания)
  if (stableState == BTN_PRESSED && !holdDone && now - pressStart >= HOLD_TIME)
  {
    enabled = !enabled;
    holdDone = true;
    if (enabled) show();
    else         ledOff();
    Serial.println(enabled ? "Включено" : "Выключено");
  }
#endif
}

// ---------- Вывод цвета ----------
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
