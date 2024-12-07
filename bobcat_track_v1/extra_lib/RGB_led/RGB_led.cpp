#include "RGB_led.h"
RGB_led::RGB_led(int pin_R, int pin_G, int pin_B) {
  _pin_R = pin_R;
  _pin_G = pin_G;
  _pin_B = pin_B;
  pinMode(_pin_R, OUTPUT);
  pinMode(_pin_G, OUTPUT);
  pinMode(_pin_B, OUTPUT);
  digitalWrite(_pin_R, LOW);
  digitalWrite(_pin_G, LOW);
  digitalWrite(_pin_B, LOW);
}
void RGB_led::change_color(char color) {
  switch (color) {
    case 'r':
      digitalWrite(_pin_R, HIGH);
      digitalWrite(_pin_G, LOW);
      digitalWrite(_pin_B, LOW);
      break;
    case 'g':
      digitalWrite(_pin_R, LOW);
      digitalWrite(_pin_G, HIGH);
      digitalWrite(_pin_B, LOW);
      break;
    case 'b':
      digitalWrite(_pin_R, LOW);
      digitalWrite(_pin_G, LOW);
      digitalWrite(_pin_B, HIGH);
      break;
    case 'c':
      digitalWrite(_pin_R, LOW);
      digitalWrite(_pin_G, HIGH);
      digitalWrite(_pin_B, HIGH);
      break;
    case 'm':
      digitalWrite(_pin_R, HIGH);
      digitalWrite(_pin_G, LOW);
      digitalWrite(_pin_B, HIGH);
      break;
    case 'y':
      digitalWrite(_pin_R, HIGH);
      digitalWrite(_pin_G, HIGH);
      digitalWrite(_pin_B, LOW);
      break;
    case 'k':
      digitalWrite(_pin_R, LOW);
      digitalWrite(_pin_G, LOW);
      digitalWrite(_pin_B, LOW);
      break;
    case 'w':
      digitalWrite(_pin_R, HIGH);
      digitalWrite(_pin_G, HIGH);
      digitalWrite(_pin_B, HIGH);
      break;
    default:
      break;
  }
}
