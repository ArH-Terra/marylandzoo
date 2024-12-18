#include <SoftwareSerial.h>

SoftwareSerial XBee(2, 3);  // RX, TX

// setting
//const int node_ID_list_num = 4;
//int node_ID_list[node_ID_list_num] = { 1, 2, 3, 4 };
const int node_ID_list_num = 1;
int node_ID_list[node_ID_list_num] = { 1 };
int food_node_ID = 1;

//general
int success_node_num;
bool ackReceived = false;
unsigned long startTime;
int ii;

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
int node_ID;  // 3: 3, 4, 5
// XBee receive msg
bool is_success = false;  // 1: 6
// XBee send msg
int cmd;  // 3

// real code
void setup() {
  Serial.begin(9600);
  XBee.begin(9600);  // Set up XBee communication at 9600 baud
  drain_XBee();

  // added this part because the XBee at nodes always ignore the first message, for no good reason
  // ----------
  for (ii = 0; ii < node_ID_list_num; ii++) {
    node_ID = node_ID_list[ii];  // always select node 1 for now
    cmd = 000;                   //just a no-meaning one
    send_XBee();
  }
  // ----------

  while (!Serial)
    ;
  drain_XBee();
}

void loop() {
  receive_UserInput();
  drain_XBee();

  // initialization
  is_success = 0;
  success_node_num = 0;

  // decide the sequence now
  int node_ID_seq[planned_node_num];
  for (ii = planned_node_num - 1; ii >= 0; ii--) {
    if (ii == planned_node_num - 1) {
      node_ID = food_node_ID;
    } else {
      node_ID = node_ID_list[random(0, node_ID_list_num)];
      while (node_ID == node_ID_seq[ii + 1]) {
        node_ID = node_ID_list[random(0, node_ID_list_num)];
      }
    }
    node_ID_seq[ii] = node_ID;
  }
  Serial.print("Node sequence:");
  for (ii = 0; ii < planned_node_num; ii++) {
    Serial.print(" ");
    Serial.print(node_ID_seq[ii]);
  }
  Serial.println(".");

  // trace
  while (success_node_num < planned_node_num) {
    //node_ID = food_node_ID;
    node_ID = node_ID_seq[success_node_num];
    cmd = 100;
    drain_XBee();
    send_XBee();

    // Now wait for acknowledgment
    ackReceived = false;
    startTime = millis();
    while (!ackReceived && (millis() - startTime) < planned_timeout + 1000) {  // give 1s delay
      if (XBee.available()) {
        receive_XBee();
        ackReceived = true;
      }
    }

    // if did not receive any response, maybe the node is bad
    if (!ackReceived) {
      is_success = false;
      Serial.print("Node fail: ");
      Serial.println(node_ID);
      break;
    }

    if (!is_success) {
      Serial.print("Trace fail: ");
      Serial.println(node_ID);
      break;
    } else {
      success_node_num++;
      Serial.print("Trace succeed: ");
      Serial.println(node_ID);
      delay(1000);
    }
  }

  // feed
  if (is_success) {
    node_ID = food_node_ID;
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

  Serial.println("Please input planned timeout (sec) for each node: (try 20)");
  while (!Serial.available())
    ;
  delay(200);
  planned_timeout_sec = receive_number();
  planned_timeout = planned_timeout_sec * 1000;

  Serial.println("Please input planned distance (mm): (try 300)");
  while (!Serial.available())
    ;
  delay(200);
  planned_distance = receive_number();
}

// receive message from XBee
// <[center_ID = 000, 3][node_ID, 3][is_success, 1]>
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
// <[node_ID, 3][cmd, 3][planned_timeout_sec, 4][planned_distance, 4]>
void send_XBee() {
  sprintf(msg, "<%03d%03d%04d%04d>", node_ID, cmd, planned_timeout_sec, planned_distance);
  XBee.print(msg);
  Serial.println("XBee sent:");
  Serial.println(msg);
  delay(200);
}