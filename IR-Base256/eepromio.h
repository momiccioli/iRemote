/*
This module will read or write anything to/from the 24LC256-IP memory chip.
It is based on the function used for the Arduino's internal EEPROM.

It will accept a the following information;
	Chip Device address as int
	Starting address as int
	Pointer reference to the data where the data is retrieved or written to. (See comments below).
	
These routines work by reading / writing bytes.  Although the easiest way to use the chip 
It's not the fastest.  Performing Page Reads / Writes is faster but more complicated.
*/

#include <Arduino.h>  // for type definitions

//----------------------------------------------------------------------------------------
// Write data passed to function to the 24LC256-I/P Chip:
//		deviceaddress:	The I2C address of the chip
//		eeaddress:		The starting memory address to store the data on the chip.
//		data:			Pointer constant to the data to written to the chip (data source can't be changed).
//----------------------------------------------------------------------------------------
template <class T> int EEPROM_writeAnything(int deviceaddress, unsigned int eeaddress, const T& data){
	const byte* p = (const byte*)(const void*)&data;	// Get the pointer address of the source data as byte data type regardless of source data
   unsigned int i;
	digitalWrite(13, LOW);
   for (i = 0; i < sizeof(data); i++) {				// Get data length so we know how many bytes to write
		Wire.beginTransmission(deviceaddress);			// Open transmission
  		Wire.write((int)(eeaddress >> 8));   // MSB
  		Wire.write((int)(eeaddress & 0xFF)); // LSB
  		Wire.write(*p);									// One byte of data to write using a pointer reference
  		Wire.endTransmission();							// Close the transmission or commit the write
  		delay(5);										// Need to wait while chip stores the data
  		p++;											// Increment the pointer to the next byte
  		eeaddress++;									// Increment to the next memory location on the chip
  	}  
	digitalWrite(13, HIGH);
   return i;
}

//----------------------------------------------------------------------------------------
// Read data from the 24LC256-I/P Chip and return to the data structure:
//		deviceaddress:	The I2C address of the chip
//		eeaddress:		The starting memory address to read the data on the chip from.
//		data:			Pointer to variable where the data stored (not a const as is in write).
//----------------------------------------------------------------------------------------
template <class T> int EEPROM_readAnything(int deviceaddress, unsigned int eeaddress, T& data){
    byte* p = (byte*)(void*)&data;					// Get the pointer address of the variable to store the data.
    unsigned int i;
    for (i = 0; i < sizeof(data); i++) {			// Get data length so we know how many bytes to write
 		Wire.beginTransmission(deviceaddress);		// Open transmission
  		Wire.write((int)(eeaddress >> 8));   // MSB
  		Wire.write((int)(eeaddress & 0xFF)); // LSB
  		Wire.endTransmission();						// Close the transmission to tell the chip to execute the read
   		Wire.requestFrom(deviceaddress,1);			// Get the byte that was read.
   		if (Wire.available()) *p = Wire.read();		// If there was data read store it vio pointer ref to variable
  	  	p++;										// Increment the pointer to the next byte
  		eeaddress++;								// Increment to the next memory location on the chip
	}
    return i;
}