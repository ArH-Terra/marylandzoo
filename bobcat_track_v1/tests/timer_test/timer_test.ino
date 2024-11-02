// These define's must be placed at the beginning before #include "TimerInterrupt.h"
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0
#define USE_TIMER_1     true
#define USE_TIMER_2     false
#define USE_TIMER_3     false
#include "TimerInterrupt.h"

unsigned int outputPin1 = LED_BUILTIN;
unsigned int outputPin  = A0;

#define TIMER1_INTERVAL_MS    200

void TimerHandler1(unsigned int outputPin = LED_BUILTIN)
{
  static bool toggle1 = false;
  static bool started = false;

  if (!started)
  {
    started = true;
    pinMode(outputPin, OUTPUT);
  }
  
  digitalWrite(outputPin, toggle1);
  toggle1 = !toggle1;
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  Serial.print(F("\nStarting Argument_Simple on "));
  Serial.println(BOARD_TYPE);
  Serial.println(TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

  ITimer1.init();

  // Using ATmega328 used in UNO => 16MHz CPU clock ,
  // For 16-bit timer 1, 3, 4 and 5, set frequency from 0.2385 to some KHz
  // For 8-bit timer 2 (prescaler up to 1024, set frequency from 61.5Hz to some KHz

  if (ITimer1.attachInterruptInterval(TIMER1_INTERVAL_MS, TimerHandler1, outputPin1))
  {
    Serial.print(F("Starting  ITimer1 OK, millis() = ")); Serial.println(millis());  
  }
  else
    Serial.println(F("Can't set ITimer1. Select another freq. or timer"));
}

void loop()
{
}
