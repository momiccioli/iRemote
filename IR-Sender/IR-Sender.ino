/*IR-Sender April 2021
This is the remote sender for the IR-Base channle changer.  This module is
not required and only needed if you don't want to connect directly to the
the base changer with wires.
*/

//*****************************************************************************
// Import needed libraries
//*****************************************************************************
#include <SPI.h>   // Comes with Arduino IDE
#include "RF24.h"  // Download and Install (See above)

//*****************************************************************************
// Declare Constants and Pin Numbers
//*****************************************************************************
void SendIt(long dataTransmitted);

const int DwnBut = 6;           // Down - Also has external port via jack
const int UppBut = 7;           // Up   - Also has external port via jack
const int IndLed = 8;

//*****************************************************************************
// Declare objects
//*****************************************************************************
RF24 myRadio (9, 10); // "myRadio" an instance of a radio, specifying the CE and CS pins.

//*****************************************************************************
// Variables
//*****************************************************************************
byte addresses[][6] = {"1Node"};    // Create address for 1 pipe.
//long dataTransmitted;             // Data that will be Transmitted.
int UppButState = 0;                // variable for reading the Up button status
int DwnButState = 0;                // variable for reading the Down button status
int dataReceived;                   // Data that will be received from the transmitter

// *****************************************************************************
// Setup routine runs once when you press reset:
// *****************************************************************************
void setup() {
   pinMode(UppBut, INPUT);          // Button Up    IrCodes 1
   pinMode(DwnBut, INPUT);          // Button Down  IrCodes 2
   pinMode(IndLed, OUTPUT);
   //Serial.begin(115200);
   //delay(1000);
   randomSeed(analogRead(0));              // Seed for random nuber generator
   myRadio.begin();                        // Start up the physical nRF24L01 Radio
   myRadio.setChannel(108);                // Above most Wifi Channels
   myRadio.setPALevel(RF24_PA_MAX);        // PA Power level
   myRadio.openWritingPipe( addresses[0]); // Use the first entry in array 'addresses' (Only 1 right now)
   delay(50);
}

// *****************************************************************************
// Setup routine runs once when you press reset:
// *****************************************************************************
void loop() {
   if ( digitalRead(UppBut) ) { SendIt(2); }
   if ( digitalRead(DwnBut) ) { SendIt(1); }
}

// *****************************************************************************
// *****************************************************************************
void SendIt(long dataTransmitted) {

   unsigned long RecWait = millis();
   myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
   myRadio.openReadingPipe(1, addresses[0]);   // Use the first entry in array 'addresses' (Only 1 right now)
   myRadio.startListening();                   // Radio is listening

   while ( millis() - RecWait < 3000 and !myRadio.available()) {
      digitalWrite(IndLed, HIGH);
      delay(25);
      if ( myRadio.available()) {               // Check for incoming data from transmitter
         while (myRadio.available()) {          // While there is data ready
            myRadio.read( &dataReceived, sizeof(dataReceived) ); // Get the data payload (You must have defined that already!)
         }
         break;
      }
   }
   dataReceived="";
   myRadio.stopListening();
   digitalWrite(IndLed, LOW);
}
