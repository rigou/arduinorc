/* all_inputs_test.ino
 * 2021-09-14
 */
 

void setup() {
	Serial.begin(9600);
	while (!Serial) ;
	for (byte idx_byt=2; idx_byt <= 12; idx_byt++) {
	  	pinMode(idx_byt, INPUT_PULLUP);
	}
	pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
	// read the state of the digital inputs
	for (byte idx_byt=2; idx_byt <= 12; idx_byt++) {
		// Serial.print("DIN");
		// Serial.print(idx_byt);
		// Serial.print(" = ");
		// Serial.println(digitalRead(idx_byt));
		Serial.print(digitalRead(idx_byt));
		Serial.print(" ");
	}
	// Serial.println("");

	// read the state of the analog inputs
	for (byte idx_byt=0; idx_byt <= 7; idx_byt++) {
		//Serial.print("AIN");
		//Serial.print(idx_byt);
		//Serial.print(" = ");
		//Serial.println(analogRead(idx_byt));
		Serial.print(analogRead(idx_byt));
		Serial.print(" ");
	}
	Serial.println("");
	
	digitalWrite(LED_BUILTIN, HIGH);
	delay(250);
	digitalWrite(LED_BUILTIN, LOW);
	delay(250);
}
