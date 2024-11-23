// library and objects
/// LCD
#include <LiquidCrystal_I2C.h>
const int LCD_row_max = 4;
const int LCD_col_max = 20;
LiquidCrystal_I2C lcd(0x27, LCD_col_max, LCD_row_max);

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

// settings
const int LCD_row_num = 4;
const int list_row_num = 12;
int LCD_pointer = 0;
int list_pointer = 0;
int LCD_1st_row_in_list = 0;

// define animal sound types
const int animal_sound_type_num = 4;
String animal_sound_types[animal_sound_type_num] = {
  "Bird",
  "Unknown#1",
  "Unknown#2",
  "Unknown#3"
};

// define setting page lists
/// variables on the left
String var_list[list_row_num] = {
  "Confirm settings!  ",
  "# of nodes:        ",
  "# of seq:          ",
  "Seq:               ",
  "Randomize seq!     ",
  "Broadcast #1!      ",
  "Node #:            ",
  " Timeout (sec):    ",
  " Distance (mm):    ",
  " AD val (<=30):    ",
  " AD time (sec):    ",
  " AD animal:        "
};

/// records on the right
String rec_list[list_row_num] = {
  "",
  "4",
  "5",
  "12341",
  "",
  "",
  "1",
  "020",
  "300",
  "30",
  "05",
  "Bird"
};

// recorded data
int num_of_nodes = 4;
const int num_of_nodes_max = 10;
int num_of_seq = 5;
const int num_of_seq_max = 10;
char seq_long[num_of_seq_max] = { '1', '2', '3', '4', '1' };
int node_data[num_of_nodes_max + 1][5];

// some small variables
int i;
int re_input = 0;



void setup() {
  // initialize node_data
  for (i = 1; i <= num_of_nodes_max; i++) {
    node_data[i][0] = 20;
    node_data[i][1] = 300;
    node_data[i][2] = 30;
    node_data[i][3] = 5;
    node_data[i][4] = 0;
  }

  // LCD initiation
  lcd.init();
  lcd.backlight();
  for (int LCD_row_i = 0; LCD_row_i < LCD_row_num; LCD_row_i++) {
    lcd_print_string(var_list[LCD_1st_row_in_list + LCD_row_i], LCD_row_i, 1, 1);
    lcd_print_string(rec_list[LCD_1st_row_in_list + LCD_row_i], LCD_row_i, LCD_col_max - 1, 0);
  }
  lcd_print_string(">", LCD_pointer, 0, 1);

  // button initiation
  re_b.begin();

  // timer initiation
  timer.every(blink_interval_ms, timer_function);

  // initialize serial
  Serial.begin(9600);
  while (Serial.available()) { ; }
}

void loop() {
  /*
  There are three modes:
  a. Number secured
  b. Number edit: Type from keypad
  c. Number edit: Rotate from rotary encoder
  Keypad keys:
  A: confirm
  B: ignore
  C: up
  D: down
  *: left
  #: right

  when a
    not blinking
  when b
    blinking one character for input
  when c
    blinking the whole number

  when Key A:
    a: -> b
    b, c: -> a
  when Key C/D / rotate:
    a: -> line goes that direction
    b: -> c, move that direction
    c: -> move that direction
  when Key #:
    a: -> b
    b: -> next digit, if last -> a
    c: -> b, first digit
  when Key *:
    a: -> ignore
    b: -> previous digit, if first -> a
    c: -> a
  when number Key:
    a: -> b, write the first character and go next
    b: -> write this character and go next
    c: -> b, write the first character and go next
  when button:
    a: -> c
    b, c: -> a
  any other keys: ignore
  */

  // variables
  static char input_mode = 'a';
  static int digit_at = 0;
  static bool is_timer_on = 0;
  static int loop_num = 0;
  char kp_input = '0';
  int re_input = 0;

  // monitor rotatory changes
  kp_input = kp.getKey();
  re.tick();
  re_input = (int)(re.getDirection());

  //--------------------------------------------------------


  /// rotatory encoder
  if (re_input != 0) {
    Serial.println("Detect rotatory encoder.");
    if (list_pointer == list_row_num - 1 && re_input == 1) {  // list last one +1
      list_pointer = 0;
      LCD_pointer = 0;
      LCD_1st_row_in_list = 0;
    } else if (list_pointer == 0 && re_input == -1) {  // list first one -1
      list_pointer = list_row_num - 1;
      LCD_pointer = LCD_row_num - 1 < list_row_num - 1 ? LCD_row_num - 1 : list_row_num - 1;
      LCD_1st_row_in_list = list_row_num - LCD_row_num > 0 ? list_row_num - LCD_row_num : 0;
    } else if (LCD_pointer == LCD_row_num - 1 && re_input == 1) {  // LCD last one +1
      list_pointer++;
      LCD_1st_row_in_list++;
    } else if (LCD_pointer == 0 && re_input == -1) {  // LCD first one -1
      list_pointer--;
      LCD_1st_row_in_list--;
    } else {  // default: just commonly go to the next one
      LCD_pointer = LCD_pointer + re_input;
      list_pointer = list_pointer + re_input;
    }

    // refresh LCD screen
    //lcd.clear();
    for (int LCD_row_i = 0; LCD_row_i < LCD_row_num; LCD_row_i++) {
      lcd_print_string(var_list[LCD_1st_row_in_list + LCD_row_i], LCD_row_i, 1, 1);
      lcd_print_string(rec_list[LCD_1st_row_in_list + LCD_row_i], LCD_row_i, LCD_col_max - 1, 0);
      if (LCD_row_i == LCD_pointer) {
        lcd_print_string(">", LCD_row_i, 0, 1);
      } else {
        lcd_print_string(" ", LCD_row_i, 0, 1);
      }
    }
  }

  // print all the result
  static bool should_print_all = false;
  if (should_print_all) {
    should_print_all = false;
    // Print
    /// general
    Serial.println("Settings:");
    Serial.print(var_list[1]);
    Serial.println(num_of_nodes);
    Serial.print(var_list[2]);
    Serial.println(num_of_seq);
    Serial.print(var_list[3]);
    Serial.println(seq_long);
    // every node
    for (i = 1; i <= num_of_nodes; i++) {
      Serial.println("");
      Serial.print(var_list[6]);
      Serial.println(i);
      Serial.print(var_list[7]);
      Serial.println(node_data[i][0]);
      Serial.print(var_list[8]);
      Serial.println(node_data[i][1]);
      Serial.print(var_list[9]);
      Serial.println(node_data[i][2]);
      Serial.print(var_list[10]);
      Serial.println(node_data[i][3]);
      Serial.print(var_list[11]);
      Serial.println(animal_sound_types[node_data[i][4]]);
    }
  }
}

// functions
void lcd_print_string(String words, byte curser_row, byte curser_col, bool dir) {  // dir = 0: inverse; dir = 1: directly
  if (!dir) {
    curser_col = curser_col - words.length() + 1;
    lcd.setCursor(curser_col, curser_row);
  }
  lcd.setCursor(curser_col, curser_row);
  lcd.print(words);
}

String num2str(int n, int digit_n) {  // calculate number string
  char* format = "%01d";
  format[2] = digit_n + '0';
  char* str;
  sprintf(str, format, n);
  return String(str);
}

int str2num(String str) {  // calculate number string
  return atoi(str.c_str());
}
