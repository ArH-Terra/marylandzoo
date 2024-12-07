// MODIFY ACCORDINGLY
const int this_node_ID = 1;       // Unique ID, positive integer
const int relay_trigger = LOW;    // "HIGH" or "LOW"
const char feeder_type = 'A';     // 'A': Auger, means that a motor only needs to move forward
                                  // 'L': Linear actuator, means a motor needs to move back-and-forth
const int actuation_time = 3000;  // Time period (millisecond) of each auger motion or linear actuator motion.
// -------------- DO NOT MODIFY FOLrelay_triggerINGS ----------------
#include <SoftwareSerial.h>

// pin settings
const int actuator_pos_relay_pin = 8;
const int actuator_neg_relay_pin = 9;
const byte Xbee_rxPin = A6;
const byte Xbee_txPin = A7;

SoftwareSerial XBee(Xbee_rxPin, Xbee_txPin);

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
int feeder_ID;  // 3: 0, 1, 2
int cmd;        // 3: 3, 4, 5

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

  // Initialize linear actuator
  pinMode(actuator_pos_relay_pin, OUTPUT);
  pinMode(actuator_neg_relay_pin, OUTPUT);
  digitalWrite(actuator_pos_relay_pin, 1 - relay_trigger);
  digitalWrite(actuator_neg_relay_pin, 1 - relay_trigger);
  delay(1000);

  if (feeder_type == 'A') {
    ball_reset();
  }

  drain_XBee();
  Serial.println("Ready!");
}

void loop() {
  XBee.listen();
  if (XBee.available()) {
    // read message and store
    receive_XBee();

    // skip other node information
    if (feeder_ID != this_node_ID)
      return;
    Serial.println("This node!");

    // Feed cmd
    if (cmd == 101) {
      if (feeder_type == 'A') {
        Serial.println("Start ball release!");
        ball_release();
        Serial.println("End ball release!");
      } else if (feeder_type == 'L') {
        ball_reset();
        Serial.println("Auger release!");
      }
    }
    // other cmd
    else {
      Serial.println("Unknown cmd...");
    }
    delay(200);
  }
}

// receive message from XBee
// <[feeder_ID, 3][cmd, 3][planned_timeout_sec, 4][planned_distance, 4]>
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
    feeder_ID = receivedMsg.substring(0, 3).toInt();
    // only process more when this is the ID
    if (feeder_ID == this_node_ID) {
      cmd = receivedMsg.substring(3, 6).toInt();
      Serial.println("CMD:");
      Serial.println(cmd);
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
// <[center_ID = 000, 3][feeder_ID, 3][is_success, 1]>
void send_XBee(bool is_success) {
  XBee.listen();
  sprintf(msg, "<%03d%03d%1d>", 0, feeder_ID, is_success);
  XBee.println(msg);
  Serial.println("XBee sent:");
  Serial.println(msg);
}

// Function ball_release: relay_triggerers linear actuator to release the ball and then raises it back up to original position ready to be loaded again.
void ball_release() {
  digitalWrite(actuator_neg_relay_pin, relay_trigger);
  delay(actuation_time);
  digitalWrite(actuator_neg_relay_pin, 1 - relay_trigger);
  delay(100);
  // Resets actuator to fully extended position
  digitalWrite(actuator_pos_relay_pin, relay_trigger);
  delay(actuation_time + 1000);
  digitalWrite(actuator_pos_relay_pin, 1 - relay_trigger);
}

void ball_reset() { // also for auger motion
  // Raise linear actuator if not raised already
  digitalWrite(actuator_pos_relay_pin, relay_trigger);
  delay(actuation_time + 1000);
  digitalWrite(actuator_pos_relay_pin, 1 - relay_trigger);
}
