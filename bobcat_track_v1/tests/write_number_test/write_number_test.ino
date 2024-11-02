
// library and objects
/// LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

/// button
#include <Button.h>
Button re_b(12);

/// keypad
#include <Keypad.h>
const byte ROWS = 4;
const byte COLS = 4;
// this is for membrane keypad
// transpose this for telephone style keypad
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 9, 8, 7, 6 };
byte colPins[COLS] = { 5, 4, 3, 2 };
Keypad kp = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

/// RotaryEncoder
#include <Arduino.h>
#include <RotaryEncoder.h>
RotaryEncoder re(A2, A3, RotaryEncoder::LatchMode::FOUR0);

// variables
const int blink_interval_ms = 100;

// main
void setup() {
  // LCD initiation
  lcd.init();
  lcd.backlight();
  lcd.print(F("SET TEMPERATURE"));
  lcd.setCursor(8, 1);
  lcd.print(F("F"));

  // button initiation
  re_b.begin();

  // serial?
  Serial.begin(9600);
  while (!Serial) {};
}

void loop() {
  /*
  There are three modes:
  a. Set
  b. Type from keypad
  c. Rotate from rotary encoder
  when a
    not blinking
    number keys: -> b, write the first character and go next
    Key A: -> b
    Button: -> c
    any other keys: ignore
    rotation: -> c, move that direction
  when b
    blinking one character for input
    number keys: write this character and go next
    Key A, button: -> a
    any other keys: ignore
    rotation: -> c, move that direction
    check digit full: -> a
  when c
    blinking the whole number
    number keys: -> b, write the first character and go next
    Key A, button: -> a
    any other keys -> b
    rotation: -> move that direction
  */

  // variables
  const int digit_num = 2;
  static char num_c[2] = { '0', '0' };
  static int num = 0;
  static int input_mode = 0;
  const int low_lim = 0;
  const int high_lim = 99;
  static int digit_at = 0;

  char kp_input = '0';
  int re_input = 0;

  // monitor changes
  kp_input = kp.getKey();
  re.tick();
  re_input = (int)(re.getDirection());

  /// button pressed
  if (re_b.pressed()) {
    Serial.println("Detect button.");
    if (input_mode = 0) {  // -> c
      input_mode = 2;
      digit_at = -1;
    } else {  // -> a
      input_mode = 0;
      digit_at = 0;
    }
  }

  /// keypad pressed
  else if (kp_input) {
    Serial.println("Detect keypad.");
    if (kp_input >= '0' && kp_input <= '9') {    // number key
      if (input_mode == 0 || input_mode == 2) {  // -> b, write the first character and go next
        input_mode = 1;
        num_c[0] = kp_input;
        digit_at = 1;
      } else {
        num_c[digit_at] = kp_input;
        digit_at = digit_at + 1;
      }
    } else if (kp_input >= 'A') {  // key 'A'
      if (input_mode = 0) {        // -> b
        input_mode = 1;
        digit_at = 0;
      } else {  // -> a
        input_mode = 0;
        digit_at = 0;
      }
    } else {  // other keys
      // do nothing for other keys for now
    }

    // calculate number
    num = 0;
    for (int i = 0; i < digit_num; i++) {
      num = num * 10 + num_c[i] - '0';
    }

    // check digit full
    if (digit_at >= digit_num) {  // -> a
      input_mode = 0;
      digit_at = 0;
    }
  }

  /// rotatory encoder
  else if (re_input != 0) {  // -> c, move that direction
    Serial.println("Detect rotatory encoder.");

    input_mode = 2;
    digit_at = -1;
    num = num + re_input;

    // calculate number char
    int num_ = num;
    for (int i = digit_num - 1; i >= 0; i--) {
      num_c[i] = num_ % 10 + '0';
      num_ = num_ / 10;
    }
  }

  // check low and high boundary
  if (num < low_lim) {
    num = low_lim;
    // calculate number char
    int num_ = num;
    for (int i = digit_num - 1; i >= 0; i--) {
      num_c[i] = num_ % 10 + '0';
      num_ = num_ / 10;
    }
  }
  if (num > high_lim) {
    num = high_lim;
    // calculate number char
    int num_ = num;
    for (int i = digit_num - 1; i >= 0; i--) {
      num_c[i] = num_ % 10 + '0';
      num_ = num_ / 10;
    }
  }

  // decide blinking according to the input_mode
  if (input_mode == 0) {
    // disable blinking
    // show number
    lcd.setCursor(5, 1);
    lcd.print(num);
  } else {
    // initiate blinking
    // show number for now
    lcd.setCursor(5, 1);
    lcd.print(num);
  }
}
