#include "Arduino.h"
// ================== НАСТРОЙКИ ==================
// Закомментируйте строку ниже, чтобы отключить включение/выключение удержанием.
// Тогда светодиод работает сразу после подачи питания, а кнопка только переключает режимы.
// #define onOff

#define BUTTON_PIN 2   // кнопка между пином 2 и GND
#define LED_PIN 13     // светодиод через резистор 220 Ом (пин с ШИМ для режима "дыхание")
#define HOLD_TIME 1000 // время удержания для вкл/выкл, мс
#define DEBOUNCE 50    // защита от дребезга, мс
#define MODES 5        // количество режимов
// ===============================================
void handleButton();
void runMode();
void blink(unsigned long now, unsigned int onTime, unsigned int offTime);
byte mode = 0;

#ifdef onOff
bool enabled = false; // при старте выключено, включается удержанием 1 сек
#else
bool enabled = true; // режим вкл/выкл отключён — светодиод работает сразу
#endif

// Состояние кнопки
bool lastReading = HIGH;
bool stableState = HIGH;
unsigned long lastChange = 0;
unsigned long pressStart = 0;
bool holdDone = false;

void setup()
{
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
}

void loop()
{
  handleButton();

  if (enabled)
  {
    runMode();
  }
  else
  {
    analogWrite(LED_PIN, 0);
  }
}

// ---------- Обработка кнопки ----------
void handleButton()
{
  bool reading = digitalRead(BUTTON_PIN);
  unsigned long now = millis();

  if (reading != lastReading)
  {
    lastChange = now;
    lastReading = reading;
  }

  // Состояние стабильно дольше DEBOUNCE — принимаем его
  if (now - lastChange > DEBOUNCE && reading != stableState)
  {
    stableState = reading;

    if (stableState == LOW)
    { // кнопку нажали
      pressStart = now;
      holdDone = false;
    }
    else
    { // кнопку отпустили
      if (!holdDone && enabled)
      { // короткое нажатие — следующий режим
        mode = (mode + 1) % MODES;
        Serial.print("Режим: ");
        Serial.println(mode + 1);
      }
    }
  }

#ifdef onOff
  // Удержание 1 секунду — вкл/выкл (срабатывает, не дожидаясь отпускания)
  if (stableState == LOW && !holdDone && now - pressStart >= HOLD_TIME)
  {
    enabled = !enabled;
    holdDone = true;
    Serial.println(enabled ? "Включено" : "Выключено");
  }
#endif
}

// ---------- Режимы ----------
void runMode()
{
  unsigned long now = millis();

  switch (mode)
  {
  case 0: // 1. Горит постоянно
    analogWrite(LED_PIN, 255);
    break;

  case 1: // 2. Медленное мигание
    blink(now, 500, 500);
    break;

  case 2: // 3. Быстрое мигание
    blink(now, 100, 100);
    break;

  case 3:
  { // 4. Двойная вспышка (стробоскоп)
    unsigned long t = now % 1000;
    bool on = (t < 80) || (t >= 160 && t < 240);
    analogWrite(LED_PIN, on ? 255 : 0);
    break;
  }

  case 4:
  { // 5. Плавное "дыхание"
    unsigned long t = now % 2000;
    int value = (t < 1000) ? t * 255 / 1000 : (2000 - t) * 255 / 1000;
    analogWrite(LED_PIN, value);
    break;
  }
  }
}

// Мигание без delay(): onTime мс горит, offTime мс не горит
void blink(unsigned long now, unsigned int onTime, unsigned int offTime)
{
  bool on = (now % (onTime + offTime)) < onTime;
  analogWrite(LED_PIN, on ? 255 : 0);
}
