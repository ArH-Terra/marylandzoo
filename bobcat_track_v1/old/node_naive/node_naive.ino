#include <SoftwareSerial.h>
SoftwareSerial XBee(12, 13);  // RX, TX

/// variables
// general
const int this_node_ID = 1;  // Unique ID for this node
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
int feeder_ID;            // 3: 0, 1, 2
int cmd;                  // 3: 3, 4, 5
int planned_timeout_sec;  // 4: 6, 7, 8, 9
int planned_distance;     // 4: 10, 11, 12, 13

unsigned long planned_timeout;

// setup
void setup() {
  // to computer
  Serial.begin(9600);

  // Initialize XBee
  XBee.begin(9600);  // Initialize XBee communication at 9600 baud
  drain_XBee();
  Serial.println("Ready!");
}

void loop() {
  if (XBee.available()) {
    // read message and store
    receive_XBee();
    Serial.println("Receive once!");

    // skip other node information
    if (feeder_ID != this_node_ID)
      return;
    Serial.println("This node!");

    // tracing cmd
    if (cmd == 100) {
      Serial.println("Start tracing");
      delay(5000);
      send_XBee(true);
      Serial.println("End tracing.");
    } else if (cmd == 101) {
      Serial.println("Feed!");
    }

    delay(200);
  }
}

// receive message from XBee
// <[feeder_ID, 3][cmd, 3][planned_timeout_sec, 4][planned_distance, 4]>
void receive_XBee() {
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
      planned_timeout_sec = receivedMsg.substring(6, 10).toInt();
      planned_distance = receivedMsg.substring(10, 14).toInt();  //mm
      planned_timeout = planned_timeout_sec * 1000;              // sec convert to msec
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
  while (XBee.available() > 0) {
    //Read the incoming byte
    incomingByte = XBee.read();
  }
}

// send message to XBee
// <[center_ID = 000, 3][feeder_ID, 3][is_success, 1]>
void send_XBee(bool is_success) {
  sprintf(msg, "<%03d%03d%1d>", 0, feeder_ID, is_success);
  XBee.print(msg);
  Serial.println("XBee sent:");
  Serial.println(msg);
}
