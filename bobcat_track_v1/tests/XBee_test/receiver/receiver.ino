// for arduino mega only
// XBee is on Serial3

//Variables
bool started = false;  //True: Message is strated
bool ended = false;    //True: Message is finished
char incomingByte;     //Variable to store the incoming byte
char msg[3];           //Message - array from 0 to 2 (3 values - PWM - e.g. 240)
byte index;            //Index of array

void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);  //Baud rate must be the same as is on xBee module
}

void loop() {

  while (Serial3.available() > 0) {
    //Read the incoming byte
    incomingByte = Serial3.read();
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

  if (started && ended) {
    int value = atoi(msg);
    Serial.println(value);
    //Serial.println(value); //Only for debugging
    index = 0;
    msg[index] = '\0';
    started = false;
    ended = false;
  }
}
