#include <SoftwareSerial.h>
#include <math.h>  // For pow function in extractDigit
#include <DFRobotDFPlayerMini.h>
DFRobotDFPlayerMini myDFPlayer;

// pin settings
#define mp3_serial Serial1

// audio
int audio_volume = 20;  // volume of audio played from speaker. Must be between 0-30 (Default is 20)
int audio_loop = 1;     // Number of times audio sound is played while node is active (default is 10 times)
int audio_file = 1;     // Determines which audio file is played from SD card -- corresponds to order of library on sd card (default is first file; currently gopher scream)
int audio_timer;

void setup() {
  Serial.begin(9600);
  mp3_serial.begin(9600);

  // Initialize mp3 module
  if (!myDFPlayer.begin(mp3_serial)) {  // Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true) {
      delay(0);  // Code to compatible with ESP8266 watch dog.
    }
  }
  myDFPlayer.begin(mp3_serial);
  myDFPlayer.volume(audio_volume);


}

void loop() {
  myDFPlayer.play(audio_file);
  delay(10000);
  myDFPlayer.pause();
  delay(10000);

}
