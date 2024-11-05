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

// timer
#include <arduino-timer.h>
auto timer = timer_create_default();
const int blink_interval_ms = 400;
bool timer_toggle = false;
int timer_num = 0;
void timer_function() {
  timer_num++;
  if (timer_num > 2) {
    timer_num = 0;
  }
  if (timer_num == 2) {
    timer_toggle = false;
  } else {
    timer_toggle = true;
  }
  delay(1);
}

// variables
const int digit_num = 3;
char num_c[digit_num];
int num = 0;
const int low_lim = 0;
const int high_lim = 999;

// functions
void num2char(int n, char* n_c, int digit_n) {  // calculate number char
  for (int i = digit_n - 1; i >= 0; i--) {
    n_c[i] = n % 10 + '0';
    n = n / 10;
  }
}

// main
void setup() {
  // number initiation
  num2char(num, num_c, digit_num);

  // LCD initiation
  lcd.init();
  lcd.backlight();
  lcd.print(F("SET TEMPERATURE"));
  lcd.setCursor(8, 1);
  lcd.print(F("F"));

  // button initiation
  re_b.begin();

  // timer initiation
  timer.every(blink_interval_ms, timer_function);

  // serial?
  Serial.begin(9600);
  while (!Serial) { ; };
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
  static int input_mode = 0;
  static int digit_at = 0;
  const int digit_c = 4;
  const int digit_r = 1;
  static bool is_timer_on = 0;
  static int loop_num = 0;
  char kp_input = '0';
  int re_input = 0;

  // monitor changes
  kp_input = kp.getKey();
  re.tick();
  re_input = (int)(re.getDirection());

  /// button pressed
  if (re_b.pressed()) {
    Serial.println("Detect button.");
    if (input_mode == 0) {  // -> c
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
      if (input_mode == 0) {       // -> b
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
    num2char(num, num_c, digit_num);
  }

  // check low and high boundary
  if (num < low_lim) {
    num = low_lim;
    num2char(num, num_c, digit_num);
  } else if (num > high_lim) {
    num = high_lim;
    num2char(num, num_c, digit_num);
  }

  // decide blinking according to the input_mode
  if (input_mode == 0) {
    // disable blinking
    timer_toggle = true;
    timer_num = 0;
  } else {
    // initiate blinking
    timer.tick();  // tick the timer
  }

  // show number
  for (int i = 0; i < digit_num; i++) {
    lcd.setCursor(digit_c + i, digit_r);
    if (!timer_toggle && (digit_at == -1 || i == digit_at))  // shine
      lcd.print(' ');
    else
      lcd.print(num_c[i]);
  }
}
