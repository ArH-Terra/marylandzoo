#include <Button.h>

Button button1(2); // Connect your button between pin 2 and GND
Button button2(3); // Connect your button between pin 3 and GND
Button button3(4); // Connect your button between pin 4 and GND

void setup() {
	button1.begin();
	button2.begin();
	button3.begin();
	
	while (!Serial) { }; // for Leos
	Serial.begin(9600);
}

void loop() {
	if (button2.pressed())
		Serial.println("Button 2 pressed");
	
	if (button2.released())
		Serial.println("Button 2 released");
	
	if (button1.toggled()) {
		if (button1.read() == Button::PRESSED)
			Serial.println("Button 1 has been pressed");
		else
			Serial.println("Button 1 has been released");
	}
}