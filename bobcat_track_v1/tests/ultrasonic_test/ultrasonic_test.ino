#include <SoftwareSerial.h>
// pin settings
#define ultrasonic_serial Serial2

byte hdr, data_h, data_l, chksum;
unsigned int dist;

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  ultrasonic_serial.begin(9600);
}

void loop() {
  dist = ultrasonic_read_dist();
  Serial.print(dist, DEC);
  Serial.println(" mm");
  delay(100);
}


unsigned int ultrasonic_read_dist() {
  while (1) {
    if (ultrasonic_serial.available()) {
      hdr = (byte)ultrasonic_serial.read();
      if (hdr == 255) {
        data_h = (byte)ultrasonic_serial.read();
        data_l = (byte)ultrasonic_serial.read();
        chksum = (byte)ultrasonic_serial.read();

        if (chksum == ((hdr + data_h + data_l) & 0x00FF)) {
          dist = data_h * 256 + data_l;
          return dist;
        }
      }
    } else
      delay(100);
  }
}