/* The IR Remote Base Station April 2021
*/

//------------------------------------------------------------------------------
// Includes and Declarations:
//------------------------------------------------------------------------------
   #include <IRremote.h>
   #include <Wire.h>
   #include <EEPROM.h>
   #include "EEPROMWriteAnything.h"
   #include <SPI.h>                         // Needed for the radio lib
   #include "RF24.h"                        // Radio librabry

   void ModeCheck();
   void LearnButton();
   void ModeButton(char Direction);
   void UpButton();
   void DownButton();
   void LcdPrint();
   void LearnIt();
   void SendIt(unsigned int RmtCmd[], int Length);
   void Learning(int Butn);
   void BlinkIt(int On, int Off, int Cycl);

//------------------------------------------------------------------------------
// Constants:
//------------------------------------------------------------------------------
   //#define DISK1 0x50               //Address of 24LC256 eeprom chip (256KB)

   const int IRpin  = 2;				// This is the IR Receiver
   const int irLed  = 3;				// This is the IR Sender

   const int LrnBut = 4;            // Learn button
   const int ModBut = 5;            // Mode button
   const int DwnBut = 6;            // Down - Also has external port via jack
   const int UppBut = 7;            // Up   - Also has external port via jack
   const int LrnLed = 8;            // Learn Indicator Blu

   const int RfCE   = 9;            // RF24 Radio CE pin 3 (Can move)
   const int RfCSN  = 10;           // RF24 Radio CSN pin 4 (Can Move)
   const int RfMOSI = 11;           // RF24 Radio MOSI pin 6
   const int RfMISO = 12;           // RF24 Radio MISO pin 7
   const int RfSCK  = 13;           // RF24 Radio SCK pin 5 (GND 1, V3.3 2, Unused 8)
   const int RepRes = 0;            // Repeat Resistor Analog 0

   const int ModLed[3] = {15,16,17};	// Mode Indicator Grn, Yel, Red

   RF24 myRadio (RfCE, RfCSN);      // "myRadio"
   // IR Information -----------------------------------------------------------
   IRsend irsend;
   IRrecv irrecv(IRpin);
   decode_results results;

   const long sCount = 65536 - (16000000/(256*(1/.2)));
   ISR(TIMER1_OVF_vect) {
      TCNT1 = sCount;
      digitalWrite(LrnLed, !digitalRead(LrnLed));
   }
//------------------------------------------------------------------------------
// Variables:
//------------------------------------------------------------------------------
   int LrnButState = 0;             // variable for reading the Learn button status
   int ModButState = 0;             // variable for reading the Mode button status
   int UppButState = 0;             // variable for reading the Up button status
   int DwnButState = 0;             // variable for reading the Down button status

   int WaitToLearn = false;
   int Mode = 0;
   int z =0;

   // Timer variables ----------------------------------------------------------
   unsigned long LrnHold = 3000;				  // Number of millis the learn button is heald to enter learn mode.
   unsigned long ModeReturnStart = 0;     // Used as the starting count for switching back to Green (Mode 1)
   unsigned long ModeReturnStop = 20000;	// Milliseconds count before switching back to Green (Mode 1)
   unsigned long ModeDelayStart = 0;      // Millisconds start for user to swicth modes
   unsigned long ModeDelayStop = 150;     // Time given to Switch the modes (fast sip and puf)

   struct IrCode {
      int  Len;
      unsigned int Raw[75];
   };
   IrCode IrCodes[3][2]; // IR signal array for 6 commands. This is the biggest taht will fit in flash

   byte addresses[][6] = {"1Node"};          // Create address for 1 pipe.
   int dataReceived;                         // Data that will be received from the transmitter

//******************************************************************************
// Setup Section (runs once)
//******************************************************************************
void setup() {
   Serial.begin(115200);           // Start the serial session
   //Wire.begin();                  // Initiate the Wire library and join the I2C bus as a master or slave.
   TCCR1A = 0;             //"Timer/Counter Control Register 1 A" set all bits to 0
   TCCR1B = 0;             //"Timer/Counter Control Register 1 B" set all bits to 0
   TCNT1 = sCount;         //Set the counter start point (16bit counter 2^16 or 0 - 65535)
   TIMSK1 |= (1<<TOIE1);   // Enable overflow so that an interupt will occurr at the set time.

   pinMode(UppBut, INPUT);        // Button Up    IrCodes 1
   pinMode(DwnBut, INPUT);        // Button Down  IrCodes 2
   pinMode(ModBut, INPUT);        // Button Mode
   pinMode(LrnBut, INPUT);        // Button Learn

   pinMode(LrnLed, OUTPUT);

   pinMode(ModLed[0], OUTPUT);	 // Green Light
   pinMode(ModLed[1], OUTPUT);	 // Yellow Light
   pinMode(ModLed[2], OUTPUT);	 // Red Light

   EEPROM_readAnything(0, IrCodes);

   myRadio.begin();                             // Start up the physical nRF24L01 Radio
   myRadio.setChannel(108);                     // Above most Wifi Channels
   myRadio.setPALevel(RF24_PA_MAX);             // Set the PA Level high more power must have clean 3.3v power
   myRadio.openReadingPipe(1, addresses[0]);    // Use the first entry in array 'addresses' (Only 1 right now)
   myRadio.startListening();                    // Radio is listening

   TCCR1B |= (1<<CS12);                         //Set Prescaler to 256 for Timer 1
   delay(2000);
   TCCR1B &= ~(1<<CS12);                        //Set Prescaler to 256 for Timer 1
   ModeCheck();                                 // Make sure this is the last line in setup.
   Serial.println("Setup");
  
}

//******************************************************************************
// Main Section (continuous loop)
//******************************************************************************
void loop() {
   if ( myRadio.available()) {               // Check for incoming data from transmitter
      //while (myRadio.available()) {          // While there is data ready
         myRadio.read( &dataReceived, sizeof(dataReceived) ); // Get the data payload (You must have defined that already!)
      //}
      if ( dataReceived == 1 ) { DownButton(); }
      if ( dataReceived == 2 ) { UpButton(); }
      if ( dataReceived == 3 ) { ModeButton('D'); } // D is a down mode change direction
      if ( dataReceived == 4 ) { ModeButton('U'); } // U is a up mode change direction
      myRadio.stopListening();
      myRadio.openWritingPipe( addresses[0]);
      myRadio.write( &dataReceived, sizeof(dataReceived) );
      myRadio.startListening();
   }
   //---------- This section process button pushes and radio commands ----------

   //ModeDelayStop = analogRead(RepRes);              // Repeat delay based on the trim pot setting.
   //Serial.println(ModeDelayStop, DEC);
   ModeDelayStop = 75;
	if ( digitalRead(LrnBut) ) LearnButton();
	if ( digitalRead(ModBut) ) {
      Serial.println("Mode button pushed");
      ModeButton('U');      // Base mode button only goes up.
   }
	if ( digitalRead(UppBut) ) {
   //   UpButton();
      SendIt(IrCodes[Mode][1].Raw, IrCodes[Mode][1].Len);
   }
	if ( digitalRead(DwnBut) ) {
      //DownButton();
      SendIt(IrCodes[Mode][0].Raw, IrCodes[Mode][0].Len);
   }
   //if (Mode != 0 and millis() - ModeReturnStart > ModeReturnStop) {	//Check to see if mode should switch back to green
   //   Mode = 0;
   //   ModeCheck();
   //}
   z++;
   //Serial.println(z, DEC);
}

void LearnButton() {
   //------------------------------------------------------------------------------
   // Learn Button
   //------------------------------------------------------------------------------
   unsigned long TmrStart = millis();
	Serial.print(" Learn Timer:");	Serial.print(TmrStart, DEC); Serial.print("  "); Serial.println(millis(), DEC);

	while ( digitalRead(LrnBut) ) {             // Checking the state of the Learn Button
		if (millis() - TmrStart >= LrnHold) {    // Must hold the learn button for 3 Seconds
			WaitToLearn = true;                   // Set learning to true
			digitalWrite(LrnLed, HIGH);
		}
		delay (25);
	}
	if (WaitToLearn) {  LearnIt(); }					// Run the learning routine
   return;
}
void ModeButton(char Direction)  {
   //------------------------------------------------------------------------------
   // Mode Button
   //------------------------------------------------------------------------------
   Serial.println("Mode Button");
   if ( Direction == 'D') {
      Mode = Mode - 1;
   }else{
      Mode = Mode + 1; // This means anything else is an up direction (default)
   }
   ModeCheck();
   //while ( digitalRead(ModBut) ) {  }				// Wait until the Mode button is released
   return;
}
void DownButton() {
   //------------------------------------------------------------------------------
   // Down Button Push
   //------------------------------------------------------------------------------
   //unsigned long TmrStart = millis();

   //while ( millis() - TmrStart < ModeDelayStop and digitalRead(DwnBut) == HIGH) {
   //   DwnButState = digitalRead(DwnBut);
   //}

   //while ( digitalRead(DwnBut) == HIGH) { }

   //delay(25);

   //if (DwnButState == HIGH) {
      //Mode = Mode - 1;
      //ModeCheck();
   //} else {
      SendIt(IrCodes[Mode][0].Raw, IrCodes[Mode][0].Len);
   //}
   return;
}
void UpButton() {
   //------------------------------------------------------------------------------
   // Up Button Push
   //------------------------------------------------------------------------------
   //unsigned long TmrStart = millis();

   //while ( millis() - TmrStart < ModeDelayStop and digitalRead(UppBut)== HIGH) {
   //	UppButState = digitalRead(UppBut);
   //}
   //while ( digitalRead(UppBut) == HIGH) { }

   //delay(25);

   //if (UppButState == HIGH) {
   	//Mode = Mode + 1;
   //   ModeCheck();
   //} else {
      SendIt(IrCodes[Mode][1].Raw, IrCodes[Mode][1].Len);
   //}
   return;
}
void SendIt(unsigned int RmtCmd[], int Length) {
   //------------------------------------------------------------------------------
   // Send the IR Signal
   //------------------------------------------------------------------------------
   Serial.println("Sending a command");
   Serial.println(Mode);
   Serial.println(RmtCmd[Mode]);
   irsend.sendRaw(RmtCmd,Length,38);       // This works don't mess with it
   digitalWrite(ModLed[Mode], LOW);
   delay(200);
   digitalWrite(ModLed[Mode], HIGH);
   Serial.println("Returning from sending a command");
   return;
}
void LearnIt() {
   //------------------------------------------------------------------------------
   // Waiting To Learn Mode
   //------------------------------------------------------------------------------
   unsigned long Blinker;
   Blinker = millis();

   while ( WaitToLearn ) {
      Serial.println("Waiting to learn");
      TCCR1B |= (1<<CS12);    //Set Prescaler to 256 for Timer 1

      LrnButState = digitalRead(LrnBut);
      UppButState = digitalRead(UppBut);
      DwnButState = digitalRead(DwnBut);

      if (LrnButState || DwnButState || UppButState ) {
         WaitToLearn = false; 					// A button was pushed so the while loop ends
         TCCR1B &= ~(1<<CS12);               // Disable Timer1 interrupt by setting prescaler bit to 0
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
void Learning(int Butn) {
   //------------------------------------------------------------------------------
   // Learning: This process waits for the IR signal to be learned
   //------------------------------------------------------------------------------
   int Loop = true;
   digitalWrite(LrnLed, HIGH);
   irrecv.enableIRIn();          // Start the receiver

   while ( Loop && digitalRead(LrnBut) == LOW) {
      if (irrecv.decode(&results)) {
         BlinkIt(45, 30, 3);

   		digitalWrite(ModLed[Mode], LOW);
   		delay(100);
   		digitalWrite(ModLed[Mode], HIGH);

         for (int i = 1; i < results.rawlen; i++) {
            IrCodes[Mode][Butn].Raw[i-1] = results.rawbuf[i]*USECPERTICK;		//Fills the raw array
            Serial.println(USECPERTICK,HEX);
            Serial.println(results.rawbuf[i],HEX);
         }
         IrCodes[Mode][Butn].Len = results.rawlen;
         EEPROM_writeAnything(0, IrCodes);	//Gets the Len  Then issues a write
         Loop = false;
         irrecv.resume();                   // Receive the next value
      }
   }
   return;
}
void BlinkIt(int On, int Off, int Cycl) {
  //------------------------------------------------------------------------------
   // IR Data Reception.
   //------------------------------------------------------------------------------
   for (int i = 0; i < Cycl; i++) {
      digitalWrite(LrnLed, HIGH);
      delay(On);
      digitalWrite(LrnLed, LOW);
      delay(Off);
   }
   return;
}
void ModeCheck() {
   //------------------------------------------------------------------------------
   // Confirm the mode
   //------------------------------------------------------------------------------
   Serial.println("Mode Check");
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
   delay(300);
   return;
}
