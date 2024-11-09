// library and objects
/// LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

/// RotaryEncoder
#include <Arduino.h>
#include <RotaryEncoder.h>
RotaryEncoder re(A2, A3, RotaryEncoder::LatchMode::FOUR0);

// settings
const int LCD_row_num = 4;
const int list_row_num = 10;
int LCD_pointer = 0;
int list_pointer = 0;
int LCD_1st_row_in_list = 0;

// make a list
String list[list_row_num] = { "1st line ", "2nd line ", "3ed line ", "4th line ", "5th line ",
                              "6th line ", "7th line ", "8th line ", "9th line ", "10th line" };

void setup() {
  // LCD initiation
  lcd.init();
  lcd.backlight();
  for (int LCD_row_i = 0; LCD_row_i < LCD_row_num; LCD_row_i++) {
    lcd.setCursor(2, LCD_row_i);
    lcd.print(list[LCD_1st_row_in_list + LCD_row_i]);
  }
  lcd.setCursor(0, LCD_pointer);
  lcd.print("->");

  // serial?
  Serial.begin(9600);
}

void loop() {
  int re_input = 0;

  // monitor rotatory encoder
  re.tick();
  re_input = (int)(re.getDirection());

  /// rotatory encoder
  if (re_input != 0) {  // -> c, move that direction
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
    for (int LCD_row_i = 0; LCD_row_i < LCD_row_num; LCD_row_i++) {
      lcd.setCursor(2, LCD_row_i);  // write lines
      lcd.print(list[LCD_1st_row_in_list + LCD_row_i]);
      lcd.setCursor(0, LCD_row_i);  // write pointer
      if (LCD_row_i == LCD_pointer) {
        lcd.print("->");
      } else {
        lcd.print("  ");
      }
    }

    Serial.println(list[list_pointer]);
  }
}
