/*
Ir Receiver (IrReceiver.ino)  August 2014
This circuit will receive IR signals and display them on a 4x20 LCD display.
You can also see the encoded string by connecting to the serial port (57,600) 
while running an FTDI programmer.
 
	The IR sensor
		Pin 1 to 		D11 (Chip Pin 17)
		Pin 2 to 		GND
		Pin 3 to 		Vcc (+5v from Arduino)
	
	The LCD
		Yellow Cable	A5 (scl Chip Pin 28)
		Green Cable		A4 (sda Chip Pin 27)
		Red Cable		Vcc (+5v from Arduino)
		Black Cable		GND
		
	The LED Indicator Light
		Anode			D13 (Chip PIN 13 - the default light pin)
						There is 100 Ohm Resistor between the Cathode and Gnd
*/
//****************************************************************************************
// Includes and Declarations:
//****************************************************************************************
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>
#include <IRremote.h>
void LcdDisplay();
void SerDisplay();
	
//****************************************************************************************
// Constants:
//****************************************************************************************
	char Prot [11] [11] = { "UNKNOWN", "NEC", "SONY", "RC5", "RC6", "DISH", "SHARP", "PANASONIC", "JVC", "SANYO", "MITSUBISH" };
	#define I2C_ADDR 0x27
	#define BACKLIGHT_PIN 3
	#define En_pin 2
	#define Rw_pin 1
	#define Rs_pin 0
	#define D4_pin 4
	#define D5_pin 5
	#define D6_pin 6
	#define D7_pin 7

	const int IRpin  = 11;		  // (Chip Pin 17)
	const int LedPin = 13;		  // (Chip Pin 19)

	IRsend irsend;
	IRrecv irrecv(IRpin);
	decode_results results;
	LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

//****************************************************************************************
// Variables:
//****************************************************************************************

	 int Ctr;
	 long Tmr;

//****************************************************************************************
// Setup Section (runs once)
//****************************************************************************************
void setup()
{
	digitalWrite(LedPin, HIGH);
	delay(500);
	digitalWrite(LedPin, LOW);
	

	lcd.begin (20,4,LCD_5x8DOTS);
	lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
	lcd.setBacklight(HIGH);
	
	pinMode(LedPin, OUTPUT);

	Serial.begin(57600);
	irrecv.enableIRIn(); // Start the receiver
	lcd.clear();
	lcd.setCursor(4, 1); 
	lcd.print("Let's Rock!!");
	lcd.setCursor(2, 2); 
	lcd.print("Ready to Receive");
	Tmr = millis();
	Ctr = 0;
	lcd.setCursor(14, 3); 
	lcd.print(Ctr);

}

//****************************************************************************************
//****************************************************************************************
// Main Section (continuous loop)
//****************************************************************************************
//****************************************************************************************
void loop() {
	if (irrecv.decode(&results)) {
		digitalWrite(LedPin, HIGH);

		LcdDisplay();
		SerDisplay();
		
		digitalWrite(LedPin, LOW);
		irrecv.resume();	 // Receive the next value		  
	} 
	if (millis() > Tmr + 1000) {
		Ctr++;
		lcd.setCursor(14, 3); 
		lcd.print(Ctr);
		//Serial.print(Ctr, DEC);	//Commented out unless testing.
		Tmr = millis();
	}	
}

//****************************************************************************************
// Display Information on the LCD
//****************************************************************************************
void LcdDisplay() {
	lcd.clear();
 
	lcd.setCursor(0, 0); 
	lcd.print("Raw    : ");	lcd.print(results.value, HEX);
	
	lcd.setCursor(0, 1); 
	lcd.print("Bits   : " ); lcd.print(results.bits, DEC);
 
	lcd.setCursor(0, 2);	 
	lcd.print("Length : ");	lcd.print(results.rawlen, DEC);
	
	lcd.setCursor(0, 3); 
	lcd.print("Delay  : ");  lcd.print(results.rawbuf[1]*USECPERTICK, DEC);
}

//****************************************************************************************
// Display Information to Serial
//****************************************************************************************
void SerDisplay() {
	
	Serial.print("Raw: ");
	Serial.print(results.value, HEX);
	
	Serial.print(" Bits: ");	  Serial.print(results.bits, DEC);
	Serial.print(" Length: ");	  Serial.print(results.rawlen, DEC);
	  
	Serial.print("	 ");
	for (int i = 1; i < results.rawlen; i++) {  
		Serial.print(results.rawbuf[i]*USECPERTICK, DEC);
		Serial.print(",");
	}
 Serial.println("");
	switch (results.decode_type) {
		case NEC:
			Serial.println("NEC");
			break;
		case SONY:
			Serial.println("SONY");
			break;
		case RC5:
			Serial.println("RC5");
			break;
		case RC6:
			Serial.println("RC6");
			break;
		case DISH:
			Serial.println("DISH");
			break;
		case SHARP:
			Serial.println("SHARP");
			break;
		case PANASONIC:
			Serial.println("PANASONIC");
			break;
		case JVC:
			Serial.println("JVC");
			break;
		case SANYO:
			Serial.println("SANYO");
			break;
		case MITSUBISHI:
			Serial.println("MITSUBISH");
			break;
		default:
			Serial.println("UNKNOWN");
			break;
	}
	Serial.println(" ");
	Serial.println(" ");
}
