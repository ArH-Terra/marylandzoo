int value = 0;

void setup() {
  Serial.begin(9600); //Baud rate must be the same as is on xBee module
}

void loop() {
  value = (value + 1) % 10;
  Serial.print('<'); 	//Starting symbol
 	Serial.print(value);//Value from 0 to 255
	Serial.println('>');//Ending symbol
  delay(200);
}
