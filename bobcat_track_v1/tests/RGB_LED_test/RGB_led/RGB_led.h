
#include "Arduino.h"

#ifndef RGB_LED_H
    #define RGB_LED_H

class RGB_led {
public:
  RGB_led(int pin_R, int pin_G, int pin_B);
  void change_color(char color);
private:
  int _pin_R;
  int _pin_G;
  int _pin_B;
};

#endif
