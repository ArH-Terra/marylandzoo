#include <RGB_led.h>
RGB_led led(3, 4, 5);

void setup() {
  Serial.begin(9600);
}
void loop() {
  if (Serial.available()) {
    char inChar = (char)Serial.read();
    led.change_color(inChar);
  }
}

