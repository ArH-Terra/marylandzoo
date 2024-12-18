// -------------- DO NOT MODIFY FOLLOWINGS ----------------
#include <SoftwareSerial.h>
const byte Xbee_rxPin = 7;
const byte Xbee_txPin = 6;
SoftwareSerial XBee(Xbee_rxPin, Xbee_txPin);

// setting
const int detector_ID_list_num_max = 100;

//general
int success_detector_num;
bool ackReceived = false;
unsigned long startTime;
int ii;
String temp = "";

// msg
bool started = false;  //True: Message is strated
bool ended = false;    //True: Message is finished
char incomingByte;     //Variable to store the incoming byte
char msg[100];         //Message - array
int msg_num[100];      //Message - array of number
bool is_received = false;
byte index;  //Index of array
// user msg
int feeder_ID = 1;
int detector_seq_num = 1;
int detector_IDs[detector_ID_list_num_max] = {};
int detector_Timeouts[detector_ID_list_num_max] = {};
int detector_Distances[detector_ID_list_num_max] = {};

// XBee msg
int node_ID;  // 3: 3, 4, 5
// XBee receive msg
bool is_success = false;  // 1: 6
// XBee send msg
int cmd = 0;               // 3
int planned_timeout = 0;   // 4
int planned_distance = 0;  // 4

// real code
void setup() {
  Serial.begin(9600);
  XBee.begin(9600);  // Set up XBee communication at 9600 baud
  drain_XBee();

  while (!Serial)
    ;
  //drain_XBee();
}

void loop() {
  receive_UserInput();
  if (!is_received) {
    return;
  } else {
    is_received = 0;  // reset
  }

  drain_XBee();

  // initialization
  is_success = 0;
  success_detector_num = 0;

  // added this part because the XBee at detectors always ignore the first message, for no good reason
  for (ii = 0; ii < detector_seq_num; ii++) {
    node_ID = detector_IDs[ii];
    cmd = 000;  //just a no-meaning one
    send_XBee();
  }

  // trace
  while (success_detector_num < detector_seq_num) {
    // still those detectors
    node_ID = detector_IDs[success_detector_num];
    cmd = 100;
    planned_timeout = detector_Timeouts[success_detector_num];
    planned_distance = detector_Distances[success_detector_num];
    drain_XBee();
    send_XBee();

    // Now wait for acknowledgment
    ackReceived = false;
    startTime = millis();
    while (!ackReceived && (millis() - startTime) < planned_timeout * 1000 + 1000) {  // give 1s delay
      if (XBee.available()) {
        receive_XBee();
        ackReceived = true;
      }
    }

    // if did not receive any response, maybe the detector is bad
    if (!ackReceived) {
      is_success = false;
      Serial.print("Detector fail: ");
      Serial.println(node_ID);
      break;
    }

    if (!is_success) {
      Serial.print("Trace fail: ");
      Serial.println(node_ID);
      break;
    } else {
      success_detector_num++;
      Serial.print("Trace succeed: ");
      Serial.println(node_ID);
      delay(1000);
    }
  }

  // feed
  if (is_success) {
    node_ID = feeder_ID;
    cmd = 101;
    send_XBee();
  }

  delay(1000);
}

// receive message from PC
void receive_UserInput() {
  Serial.println("Please input command:");
  while (!Serial.available())
    ;
  delay(200);  // so that the giver gives the info fully
  while (Serial.available() > 0) {
    //Read the incoming byte
    incomingByte = Serial.read();
    //Start the message when the '<' symbol is received
    if (incomingByte == '<') {
      started = true;
      index = 0;
    }
    //End the message when the '>' symbol is received
    else if (incomingByte == '>') {
      if (temp.length() > 0) {
        msg_num[index] = temp.toInt();  // Convert the string to an integer
      } else {
        msg_num[index] = 0;
      }
      index++;
      temp = "";  // Reset temp string

      ended = true;
      break;  // Done reading - exit from while loop!
    }
    //Read the message!
    else if (incomingByte == ' ') {
      if (temp.length() > 0) {
        msg_num[index] = temp.toInt();  // Convert the string to an integer
      } else {
        msg_num[index] = 0;
      }
      index++;
      temp = "";  // Reset temp string
    } else {
      temp += incomingByte;  // Append the character to temp string
    }
  }

  if (started && ended) {
    // display it
    Serial.println("PC received number:");
    for (ii = 0; ii < index; ii++) {
      Serial.println(msg_num[ii]);
    }

    // load data
    feeder_ID = msg_num[0];
    detector_seq_num = msg_num[1];
    for (ii = 0; ii < detector_seq_num; ii++) {
      detector_IDs[ii] = msg_num[3 * ii + 2];
      Serial.println(detector_IDs[ii]);

      detector_Timeouts[ii] = msg_num[3 * ii + 3];
      detector_Distances[ii] = msg_num[3 * ii + 4];
    }

    is_received = 1;

    // reset receiver
    index = 0;
    msg[index] = '\0';
    started = false;
    ended = false;
  }
}

// receive message from XBee
// <[center_ID = 000, 3][detector_ID, 3][is_success, 1]>
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
    node_ID = receivedMsg.substring(3, 6).toInt();
    is_success = receivedMsg.substring(6, 7).toInt();

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
// <[node_ID, 3][cmd, 3][planned_timeout, 4][planned_distance, 4]>
void send_XBee() {
  sprintf(msg, "<%03d%03d%04d%04d>", node_ID, cmd, planned_timeout, planned_distance);
  XBee.print(msg);
  Serial.println("XBee sent:");
  Serial.println(msg);
  delay(200);
}