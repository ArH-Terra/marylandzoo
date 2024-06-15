#include <SoftwareSerial.h>

SoftwareSerial XBee(2, 3);  // RX, TX

// setting
int food_node = 1;

//general
int success_node_num;
bool ackReceived = false;
unsigned long startTime;

// msg
bool started = false;  //True: Message is strated
bool ended = false;    //True: Message is finished
char incomingByte;     //Variable to store the incoming byte
char msg[100];         //Message - array
byte index;            //Index of array
// user msg
int planned_node_num;     // 2: 0, 1
int planned_timeout_sec;  // 4: 2, 3, 4, 5
int planned_distance;     // 4: 6, 7, 8, 9
unsigned long planned_timeout;
// XBee msg
int feeder_ID;  // 3: 3, 4, 5
// XBee receive msg
bool is_success = false;  // 1: 6
// XBee send msg
int cmd;  // 3

// real code
void setup() {
  Serial.begin(9600);
  XBee.begin(9600);  // Set up XBee communication at 9600 baud
  while (!Serial)
    ;
}

void loop() {
  receive_UserInput();

  // initialization
  is_success = 0;
  success_node_num = 0;

  // trace
  while (success_node_num < planned_node_num) {
    // select a new node
    feeder_ID = 1;  // always select node 1 for now
    cmd = 100;
    send_XBee();

    // Now wait for acknowledgment
    ackReceived = false;
    startTime = millis();
    while (!ackReceived && (millis() - startTime) < planned_timeout + 100) {  // give 0.1s delay
      if (XBee.available()) {
        receive_XBee();
        ackReceived = true;
      }
    }

    // if did not receive any response, maybe the node is bad
    if (!ackReceived) {
      Serial.print("Node fail: ");
      Serial.println(feeder_ID);
      break;
    }

    if (!is_success) {
      Serial.print("Trace fail: ");
      Serial.println(feeder_ID);
      break;
    } else {
      Serial.print("Trace succeed: ");
      Serial.println(feeder_ID);
    }
  }

  // feed
  if (is_success) {
    feeder_ID = food_node;
    cmd = 101;
    send_XBee();
  }

  delay(1000);
}

// receive message from user
int receive_number() {
  int number = 0;
  while (Serial.available() > 0) {
    incomingByte = Serial.read();
    if (incomingByte >= '0' && incomingByte <= '9')
      number = number * 10 + incomingByte - '0';
  }
  Serial.println(number);
  return number;
}

void receive_UserInput() {
  Serial.println("Please input planned node num:");
  while (!Serial.available())
    ;
  delay(200);
  planned_node_num = receive_number();

  Serial.println("Please input planned timeout (sec) for each node:");
  while (!Serial.available())
    ;
  delay(200);
  planned_timeout_sec = receive_number();
  planned_timeout = planned_timeout_sec * 1000;

  Serial.println("Please input planned distance (mm):");
  while (!Serial.available())
    ;
  delay(200);
  planned_distance = receive_number();
}

// receive message from XBee
// <[center_ID = 000, 3][feeder_ID, 3][is_success, 1]>
void receive_XBee() {
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
      if (index < 4)  // Make sure there is room
      {
        msg[index] = incomingByte;  // Add char to array
        index++;
        msg[index] = '\0';  // Add NULL to end
      }
    }
  }

  Serial.println(msg);  // display it
  if (started && ended) {
    String receivedMsg(msg);
    feeder_ID = receivedMsg.substring(3, 3).toInt();
    is_success = receivedMsg.substring(6, 1).toInt();

    // reset receiver
    index = 0;
    msg[index] = '\0';
    started = false;
    ended = false;
  }
}

// send message to XBee
// <[feeder_ID, 3][cmd, 3][planned_timeout_sec, 4][planned_distance, 4]>
void send_XBee() {
  sprintf(msg, "<%03d%03d%04d%04d>", feeder_ID, cmd, planned_timeout_sec, planned_distance);
  XBee.println(msg);
  Serial.println(msg);
  delay(200);
}