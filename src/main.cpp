#include "Arduino.h"
// RGB-лента 12 В + Arduino Nano + MOSFET-модули
// При включении питания - белый
// Каждое нажатие кнопки - следующий цвет по кругу:
//   белый -> жёлтый -> оранжевый -> красный -> зелёный -> (снова белый)

// ---- Пины ----
const byte PIN_R   = 9;
const byte PIN_G   = 10;
const byte PIN_B   = 11;
const byte PIN_BTN = 2;    // кнопка между D2 и GND

// ---- Цвета (R, G, B: 0..255) ----
struct Color { byte r, g, b; };
const Color colors[] = {
  {255, 255, 255},   // белый (при включении)
  {255, 130,   0},   // жёлтый (если зеленит - уменьшите второе число)
  {255,  60,   0},   // оранжевый (если желтит - уменьшите второе число)
  {255,   0,   0},   // красный
  {  0, 255,   0},   // зелёный
};
const byte NUM = sizeof(colors) / sizeof(colors[0]);

const unsigned long DEBOUNCE = 30;  // мс, защита от дребезга кнопки

byte idx = 0;

void show() {
  analogWrite(PIN_R, colors[idx].r);
  analogWrite(PIN_G, colors[idx].g);
  analogWrite(PIN_B, colors[idx].b);
}

void setup() {
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_BTN, INPUT_PULLUP);
  show();                            // сразу белый
}

void loop() {
  static bool pressed = false;
  static unsigned long lastChange = 0;

  bool btn = (digitalRead(PIN_BTN) == LOW);
  unsigned long now = millis();

  if (btn != pressed && now - lastChange > DEBOUNCE) {
    pressed = btn;
    lastChange = now;
    if (pressed) {                   // момент нажатия
      idx = (idx + 1) % NUM;
      show();
    }
  }
}
