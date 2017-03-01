#include "RotaryEncoder.h" // Load RotaryEncoder library

RotaryEncoder Encoder = RotaryEncoder(PIND6, PIND7, PIND5); // 
Create an instance of the encoder object


void setup() {
	
	Serial.begin (115200); // Init serial comm port
	
	Serial.println("Ready"); // Print readyness message

}



void loop() {
	
	static int lastCount = 0;
	
	if (Encoder.buttonDown() == 0x00) // Read push button status (Ground true => 0=down)
		
	Encoder.setCounter(0); // Reset counter
	
	int counts = Encoder.getCounter(); // Get counter value
	
	if (lastCount != counts) { // If counter has changed since last display
		
	Serial.print(counts, DEC); 	// Print counter value
			Serial.println();
		
	lastCount = counts; // Record current counter value
	}
	delay(10);

}