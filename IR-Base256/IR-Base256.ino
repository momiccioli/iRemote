/* The IR sensor
// This is the production version that uses the 256kb chip for more memory.
   10/28/2015 It's built on the long white Bread Board
   It is associated with version 3 in Eagle.
      Pin 1 to Arduino IDE Pin 8 / Chip Pin 14
      Pin 2 to GND
      Pin 3 to Vcc (+5v from Arduino)
   
*/
//----------------------------------------------------------------------------------------
// Includes and Declarations:
//----------------------------------------------------------------------------------------
  #include <IRremote.h>
  #include <Wire.h>
  #include "eepromio.h"
  //#include <LCD.h>
  //#include <LiquidCrystal_I2C.h>
  //#include <LiquidCrystal.h>
  
  
   //void SerDisplay();
   void ModeCheck(); 
   void LearnButton();
   void ModeButton();
   void UpButton();
   void DownButton();
   void LcdPrint();
   void LearnIt();
   void SendIt(unsigned int RmtCmd[], int Length);
   void Learning(int Butn); 
   void BlinkIt(int On, int Off, int Cycl);
   
//----------------------------------------------------------------------------------------
// Constants:
//----------------------------------------------------------------------------------------
   #define DISK1 0x50				//Address of 24LC256 eeprom chip
   //char Prot [11] [11] = { "UNKNOWN", "NEC", "SONY", "RC5", "RC6", "DISH", "SHARP", "PANASONIC", "JVC", "SANYO", "MITSUBISH" };
	//#define LCD 0x27
	//#define BACKLIGHT_PIN 3
	//#define En_pin 2
	//#define Rw_pin 1
	//#define Rs_pin 0
	//#define D4_pin 4
	//#define D5_pin 5
	//#define D6_pin 6
	//#define D7_pin 7
	//LiquidCrystal_I2C lcd(LCD,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin); 
	  
   // Lights -----------------------------------------------
   const int IRpin  = 8;					// This is the IR Receiver
   const int PwrLed = 13;					// Power Indicator
   const int LrnLed = 12;					// Learn Indicator
   const int ModLed[3] = {9,10,11};	// Mode Indicator Grn, Yel, Red

   // Buttons ----------------------------------------------
   const int UppBut = 7;          		// Up
   const int DwnBut = 6;         		// Down
   const int ModBut = 5;         		// Mode
   const int LrnBut = 4;         		// Learn

   // IR Information ---------------------------------------------------------------------
   IRsend irsend;
   IRrecv irrecv(IRpin);
   decode_results results;

//----------------------------------------------------------------------------------------
// Variables:
//----------------------------------------------------------------------------------------
   int LrnButState = 0;     // variable for reading the Learn button status
   int ModButState = 0;     // variable for reading the Mode button status
   int UppButState = 0;     // variable for reading the Up button status
   int DwnButState = 0;     // variable for reading the Down button status

   int WaitToLearn = false;
   int Mode = 0;
   
   // Timer variables --------------------------------------------------------------------
   unsigned long LrnHold = 3000;				// Number of millis the learn button is heald to enter learn mode.
   unsigned long ModeReturnStart = 0;		// Used as the starting count for switching back to Green (Mode 1)
   unsigned long ModeReturnStop = 20000;	// Milliseconds count before switching back to Green (Mode 1)
   unsigned long ModeDelayStart = 0;		// Millisconds start for user to swicth modes
   unsigned long ModeDelayStop = 150;  	// Time given to Switch the modes (fast sip and puf)

   struct IrCode {
      int  Len;               
      unsigned int Raw[75];   
   };
   IrCode IrCodes[3][2];         // This is the biggest this can can be to store in flash
   
//****************************************************************************************
// Setup Section (runs once)
//****************************************************************************************
void setup() {
   Serial.begin(57600);          // Start the serial session
	Wire.begin();
	//lcd.begin (20,4,LCD_5x8DOTS);
	//lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
	//lcd.setBacklight(HIGH);

   pinMode(UppBut, INPUT);       // Button Up    IrCodes 1
   pinMode(DwnBut, INPUT);       // Button Down  IrCodes 2
   pinMode(ModBut, INPUT);       // Button Mode  
   pinMode(LrnBut, INPUT);       // Button Learn 
   
   pinMode(PwrLed, OUTPUT);      
   pinMode(LrnLed, OUTPUT); 
   
   pinMode(ModLed[0], OUTPUT);	// Green Light
   pinMode(ModLed[1], OUTPUT);	// Yellow Light
   pinMode(ModLed[2], OUTPUT);	// Red Light

	EEPROM_readAnything(DISK1, 0, IrCodes);
	//LcdPrint();

   for (int i = 0; i < 3; i++) { 
   	digitalWrite(PwrLed, HIGH);
   	delay (300);
   	digitalWrite(PwrLed, LOW);
   	delay (300);
   }      

   digitalWrite(PwrLed, HIGH);
   ModeCheck();   
}

//****************************************************************************************
// Main Section (continuous loop)
//****************************************************************************************
void loop() {
   
	ModeDelayStop = analogRead(0);				// Repeat delay based on the trim pot setting.
	//ModeDelayStop = 0;				            // Repeat delay override this is needed on bld1 due to circuit design issue.
	
	if ( digitalRead(LrnBut) ) LearnButton();
	if ( digitalRead(ModBut) ) ModeButton();
	if ( digitalRead(UppBut) ) UpButton();
	if ( digitalRead(DwnBut) ) DownButton();

  	Serial.print("Down: ");	Serial.print(digitalRead(DwnBut), DEC); Serial.print(DwnButState, DEC);
   //Serial.print(" Up:   ");	Serial.print(digitalRead(UppBut), DEC); Serial.print(UppButState, DEC);
   //Serial.print(" Mode: ");	Serial.print(digitalRead(ModBut), DEC); Serial.print(ModButState, DEC);
   //Serial.print(" Learn:");	Serial.print(digitalRead(LrnBut), DEC); Serial.print(LrnButState, DEC);
   //Serial.println(" ");
   
   if (Mode != 0 and millis() - ModeReturnStart > ModeReturnStop) {	//Check to see if mode should switch back to green
      Mode = 0;
      ModeCheck();
   }
}

//----------------------------------------------------------------------------------------
// Learn Button
//----------------------------------------------------------------------------------------
void LearnButton() {
	unsigned long TmrStart = millis();
	Serial.print(" Learn Timer:");	Serial.print(TmrStart, DEC); Serial.print("  "); Serial.println(millis(), DEC);

	while ( digitalRead(LrnBut) ) {         				// Checking the state of the Learn Button
		if (millis() - TmrStart >= LrnHold) {			// Must hold the learn button for 3 Seconds
			WaitToLearn = true;									// Set learning to true
			digitalWrite(LrnLed, HIGH);
		}
		delay (25);
	}
	if (WaitToLearn) {  LearnIt(); }							// Run the learning routine
   return;
}

//----------------------------------------------------------------------------------------
// Mode Button
//----------------------------------------------------------------------------------------
void ModeButton() {
	Mode = Mode + 1;
   ModeCheck();
   while ( digitalRead(ModBut) ) {  }					// Wait until the Mode button is released
   return;
}

//----------------------------------------------------------------------------------------
// Up Button Push
//----------------------------------------------------------------------------------------
void UpButton() {
   unsigned long TmrStart = millis();

   while ( millis() - TmrStart < ModeDelayStop and digitalRead(DwnBut) == LOW) {
   	DwnButState = digitalRead(DwnBut);
   }

   //Serial.print("Down2: "); Serial.print(digitalRead(DwnBut), DEC); Serial.println(DwnButState, DEC);
   delay(25);
   
   if (DwnButState == HIGH) {
      Mode = Mode - 1;
      ModeCheck();
   } else { 
      SendIt(IrCodes[Mode][0].Raw, IrCodes[Mode][0].Len);
      //lcd.setCursor(0, 4); lcd.print(Mode); lcd.print(" "); lcd.print(IrCodes[Mode][1].Raw[0], DEC); 
      
   }      
   return;
}

//----------------------------------------------------------------------------------------
// Down Button Push
//----------------------------------------------------------------------------------------
void DownButton() {
	unsigned long TmrStart = millis();

   while ( millis() - TmrStart < ModeDelayStop and digitalRead(UppBut)== LOW) {
   	UppButState = digitalRead(UppBut);
   }

   //Serial.print("Up2: ");	Serial.print(digitalRead(UppBut), DEC); Serial.println(UppButState, DEC);
   delay(25);

   if (UppButState == HIGH) {
   	Mode = Mode + 1;
      ModeCheck();
   } else { 
   	SendIt(IrCodes[Mode][1].Raw, IrCodes[Mode][1].Len); 
   }
   return;
}

//----------------------------------------------------------------------------------------
// Send the IR Signal
//----------------------------------------------------------------------------------------
void SendIt(unsigned int RmtCmd[], int Length) {
   irsend.sendRaw(RmtCmd,Length,38);       // This works don't mess with it
   digitalWrite(ModLed[Mode], LOW);
   delay(200);
   digitalWrite(ModLed[Mode], HIGH);
   return;
}

//----------------------------------------------------------------------------------------
// Waiting To Learn Mode
//----------------------------------------------------------------------------------------
void LearnIt() {
   unsigned long Blinker;
   Blinker = millis();

   while ( WaitToLearn ) {

      if (millis() - Blinker < 200) { digitalWrite(LrnLed, LOW); }
      if (millis() - Blinker > 200) { digitalWrite(LrnLed, HIGH); }
      if (millis() - Blinker > 400) { Blinker = millis(); }
      
      LrnButState = digitalRead(LrnBut);
      UppButState = digitalRead(UppBut);
      DwnButState = digitalRead(DwnBut);
      
      if (LrnButState || DwnButState || UppButState ) { 
         WaitToLearn = false; 					// A button was pushed so the while loop ends
         }
   }

   if (UppButState) {                        // User chose to assign Up button
      UppButState = LOW;
      Learning(0);                           // Up position in the array
   }
   if (DwnButState) {                        // User chose to assign Down button 
      DwnButState = LOW;                              
      Learning(1);                           // Down position in the array
   }
   
   digitalWrite(LrnLed, LOW);						// Turn off the learn LED because learning is done
   digitalWrite(ModLed[Mode], HIGH);
   return;
}

//----------------------------------------------------------------------------------------
// Learning: This process waits for the IR signal to be learned
//----------------------------------------------------------------------------------------
void Learning(int Butn) {
   int Loop = true;
   digitalWrite(LrnLed, HIGH);
   irrecv.enableIRIn();          // Start the receiver
   
   while ( Loop && digitalRead(LrnBut) == LOW) {
      if (irrecv.decode(&results)) {
         BlinkIt(45, 30, 3); 

   		digitalWrite(ModLed[Mode], LOW);
   		delay(100);
   		//SerDisplay();
   		digitalWrite(ModLed[Mode], HIGH);
   		
         for (int i = 1; i < results.rawlen; i++) {  
            IrCodes[Mode][Butn].Raw[i-1] = results.rawbuf[i]*USECPERTICK;		//Fills the raw array
         }
         IrCodes[Mode][Butn].Len = results.rawlen;										//Gets the Len  Then issues a write
         EEPROM_writeAnything(DISK1, 0, IrCodes);                             //Writes to 256K Chip
         //LcdPrint();
         Loop = false;
         irrecv.resume();                                      					// Receive the next value
      }
   }
   return;
}

//----------------------------------------------------------------------------------------
// IR Data Reception.
//----------------------------------------------------------------------------------------
void BlinkIt(int On, int Off, int Cycl) {
   for (int i = 0; i < Cycl; i++) {
      digitalWrite(LrnLed, HIGH);
      delay(On);
      digitalWrite(LrnLed, LOW);
      delay(Off);
   }
   return;
}

//----------------------------------------------------------------------------------------
// Display Information to Serial
//----------------------------------------------------------------------------------------
//void SerDisplay() {
//   Serial.print("Raw: ");        Serial.print(results.value, HEX);
//   Serial.print(" Bits: ");      Serial.print(results.bits, DEC);
//   Serial.print(" Length: ");    Serial.print(results.rawlen, DEC);  
//   Serial.print(" Mode: ");    	Serial.print(Mode, DEC); 
   //Serial.print(" Button: ");    	Serial.print(Butn, DEC); 
//   Serial.println(" ");
//   for (int i = 1; i < results.rawlen; i++) {  
//      Serial.print(results.rawbuf[i]*USECPERTICK, DEC);
//      Serial.print(",");
//   }
//   Serial.println(" ");
//   return;
//}

//----------------------------------------------------------------------------------------
// Confirm the mode
//----------------------------------------------------------------------------------------
void ModeCheck() {  

   ModeReturnStart = millis();
   if (Mode > 2) { Mode = 0; }
   if (Mode < 0) { Mode = 2; }
   for (int i = 0; i < 3; i++) {
      if ( i == Mode) {
         digitalWrite(ModLed[i], HIGH);
      } else {
         digitalWrite(ModLed[i], LOW);
      }
   }
   //Serial.print("Mode: ");
   //Serial.println(Mode, DEC);  

   delay(300);
   return;
}

//----------------------------------------------------------------------------------------
// Display Information to Serial
//----------------------------------------------------------------------------------------
//void LcdPrint() {
//	lcd.clear(); 
//	lcd.setCursor(0, 0); lcd.print("Sz: ");	lcd.print(sizeof(IrCodes), DEC);
//   lcd.setCursor(0, 1); lcd.print("Len: ");	lcd.print(sizeof(IrCodes[0][1].Len), DEC);
//   lcd.setCursor(7, 1); lcd.print("Raw: ");   lcd.print(sizeof(IrCodes[0][1].Raw), DEC);
//   lcd.setCursor(0,2); lcd.print("Len: "); lcd.print(IrCodes[0][0].Len);lcd.print(" ");
//   lcd.print("Raw: "); lcd.print(IrCodes[0][0].Raw[0]);
//   return;
//}
