#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd( 0x27, 16, 2 );

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.print( F( "SET TEMPERATURE" ) );
  lcd.setCursor( 8, 1 );
  lcd.print( F( "F" ) );
}


void loop()
{
  const  uint32_t now         = millis();
  static uint32_t blink_timer = 0;
  static bool     blink_state = false;
  static uint8_t  temperature = 22;
    
  if ( blink_timer == 0 || now - blink_timer >= 500 )
  {
    lcd.setCursor( 5, 1 );

    if ( blink_state == false )
    {
      lcd.print( temperature );
    }
    else
    {
      lcd.print( F( "  " ) );
    }

    blink_timer = now;
    blink_state = !blink_state;
  }
}