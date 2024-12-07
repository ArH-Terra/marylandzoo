// MODIFY ACCORDINGLY
const int this_node_ID = 1;     // Unique ID, positive integer
const int relay_trigger = LOW;  // "HIGH" or "LOW"

// -------------- DO NOT MODIFY FOLLOWINGS ----------------
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// pin settings
const byte mp3_rxPin = 2;
const byte mp3_txPin = 3;
const byte ultrasonic_rxPin = 10;
const byte ultrasonic_txPin = 11;
const int ultrasonic_relay_pin = 8;
const byte Xbee_rxPin = A6;
const byte Xbee_txPin = A7;

SoftwareSerial XBee(Xbee_rxPin, Xbee_txPin);
SoftwareSerial mp3_serial(mp3_rxPin, mp3_txPin);
SoftwareSerial ultrasonic_serial(ultrasonic_rxPin, ultrasonic_txPin);

// class objects
DFRobotDFPlayerMini myDFPlayer;

/// variables
// genera
unsigned long start_time;
bool is_success;  // Bool used to determine success or failure in node
int consist_pos_counter;

// Xbee
bool started = false;  //True: Message is strated
bool ended = false;    //True: Message is finished
char incomingByte;     //Variable to store the incoming byte
char msg[100];         //Message - array
byte index;            //Index of array
// message context
int detector_ID;            // 3: 0, 1, 2
int cmd;                  // 3: 3, 4, 5
int planned_timeout_sec;  // 4: 6, 7, 8, 9
int planned_distance;     // 4: 10, 11, 12, 13

unsigned long planned_timeout;

// audio
int audio_volume = 30;                 // volume of audio played from speaker. Must be between 0-30 (Default is 20)
unsigned long audio_loop_time = 5000;  // ms
int audio_file = 1;                    // Determines which audio file is played from SD card -- corresponds to order of library on sd card (default is first file; currently gopher scream)
unsigned long audio_next_time;

// ultrasonic
unsigned int distance;  // this is the good one
byte hdr, data_h, data_l, chksum;
unsigned int dist;  // for the realing function

// setup
void setup() {
  delay(1000);

  // to computer
  Serial.begin(9600);

  // Initialize XBee
  XBee.begin(9600);  // Initialize XBee communication at 9600 baud

  // Initialize ultrasonic module
  ultrasonic_serial.begin(9600);
  pinMode(ultrasonic_relay_pin, OUTPUT);
  digitalWrite(ultrasonic_relay_pin, LOW);

  // Initialize mp3 module
  mp3_serial.begin(9600);
  myDFPlayer.begin(mp3_serial);
  myDFPlayer.volume(audio_volume);

  drain_XBee();
  Serial.println("Ready!");
}

void loop() {
  XBee.listen();
  if (XBee.available()) {
    // read message and store
    receive_XBee();

    // skip other node information
    if (detector_ID != this_node_ID)
      return;
    Serial.println("This node!");

    // detector cmd
    if (cmd == 100) {
      Serial.println("Start tracing");

      // initializatoin
      enable_ultrasonic();
      start_time = millis();
      is_success = false;
      consist_pos_counter = 0;
      audio_next_time = 0;

      // Active node detect_dision loop; Remains true while nothing is within detect_dision radius and node has not reached timeout limit to be considered failure
      while (millis() - start_time < planned_timeout) {
        // check if it is the time to play audio again
        if (millis() - start_time > audio_next_time) {
          audio_next_time = audio_next_time + audio_loop_time;
          play_audio();
          Serial.println("play_audio.");
        }

        // check distance
        distance = detect_dis();
        //Serial.println(distance);
        if (distance < planned_distance) {
          consist_pos_counter++;            // Increment false positives by 1
          if (consist_pos_counter >= 15) {  // If the object has been consistently detect_dised
            is_success = true;              // Set success to true
            break;                          // Break out of the loop
          }
        } else {
          consist_pos_counter = 0;
        }
      }

      // finish up
      disable_ultrasonic();
      pause_audio();
      send_XBee(is_success);
    }
    // other cmd
    else {
      Serial.println("Unknown cmd...");
    }
    delay(200);
  }
}

// receive message from XBee
// <[detector_ID, 3][cmd, 3][planned_timeout_sec, 4][planned_distance, 4]>
void receive_XBee() {
  XBee.listen();
  delay(200);  // so that the giver gives the info fully
  while (XBee.available() > 0) {
    //Read the incoming byte
    incomingByte = XBee.read();
    //Start the message when the '<' symbol is received
    if (incomingByte == '<') {
      started = true;
      index = 0;
      msg[index] = '\0';  // Throw away any incomplete packet
    }
    //End the message when the '>' symbol is received
    else if (incomingByte == '>') {
      ended = true;
      break;  // Done reading - exit from while loop!
    }
    //Read the message!
    else {
      if (index < 100 - 1)  // Make sure there is room
      {
        msg[index] = incomingByte;  // Add char to array
        index++;
        msg[index] = '\0';  // Add NULL to end
      }
    }
  }

  Serial.println("XBee received:");
  Serial.println(msg);  // display it

  if (started && ended) {
    String receivedMsg(msg);
    detector_ID = receivedMsg.substring(0, 3).toInt();
    // only process more when this is the ID
    if (detector_ID == this_node_ID) {
      cmd = receivedMsg.substring(3, 6).toInt();
      planned_timeout_sec = receivedMsg.substring(6, 10).toInt();
      planned_distance = receivedMsg.substring(10, 14).toInt();  //mm
      planned_timeout = planned_timeout_sec * 1000;              // sec convert to msec
      Serial.println("CMD, timeout, distance:");
      Serial.println(cmd);
      Serial.println(planned_timeout_sec);
      Serial.println(planned_distance);
    }
    // reset receiver
    index = 0;
    msg[index] = '\0';
    started = false;
    ended = false;
  }
}

// drain old message from XBee
void drain_XBee() {
  XBee.listen();
  while (XBee.available() > 0) {
    //Read the incoming byte
    incomingByte = XBee.read();
  }
}

// send message to XBee
// <[center_ID = 000, 3][detector_ID, 3][is_success, 1]>
void send_XBee(bool is_success) {
  XBee.listen();
  sprintf(msg, "<%03d%03d%1d>", 0, detector_ID, is_success);
  XBee.println(msg);
  Serial.println("XBee sent:");
  Serial.println(msg);
}

// Function play_audio: Plays specified audio file at given volume defined
void play_audio() {
  mp3_serial.listen();
  delay(200);
  myDFPlayer.play(audio_file);
  delay(200);
}
void pause_audio() {
  mp3_serial.listen();
  delay(200);
  myDFPlayer.stop();
  delay(200);
}

// Function enable_untrasonic, disable_ultrasonic
void enable_ultrasonic() {
  digitalWrite(ultrasonic_relay_pin, relay_trigger);
  delay(200);
}
void disable_ultrasonic() {
  digitalWrite(ultrasonic_relay_pin, 1 - relay_trigger);
}

// Function detect_dis: Returns distance away an object is from node in mm; ultrasonic sensor requires ~1/10s delay between each reading for better accuracy
int detect_dis() {
  ultrasonic_serial.listen();
  while (1) {
    if (ultrasonic_serial.available()) {
      hdr = (byte)ultrasonic_serial.read();
      if (hdr == 255) {
        data_h = (byte)ultrasonic_serial.read();
        data_l = (byte)ultrasonic_serial.read();
        chksum = (byte)ultrasonic_serial.read();

        if (chksum == ((hdr + data_h + data_l) & 0x00FF)) {
          dist = data_h * 256 + data_l;
          Serial.println(dist);
          return dist;
        }
      }
    } else
      delay(100);
  }
}
