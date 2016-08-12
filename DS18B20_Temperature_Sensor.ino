/*****************************************************************************
* Copyright (C) 2016 by Vasco Ferraz. All Rights Reserved.                   *
*                                                                            *
* This program is free software: you can redistribute it and/or modify       *
* it under the terms of the GNU General Public License as published by       *
* the Free Software Foundation, either version 3 of the License, or          *
* (at your option) any later version.                                        *
*                                                                            *
* This program is distributed in the hope that it will be useful,            *
* but WITHOUT ANY WARRANTY; without even the implied warranty of             *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               *
* GNU General Public License for more details.                               *
*                                                                            *
* You should have received a copy of the GNU General Public License          *
* along with this program. If not, see <http://www.gnu.org/licenses/>.       *
*                                                                            *
*  Author:      Vasco Ferraz                                                 *
*  Contact:     http://vascoferraz.com/contact/                              *
*  Description: http://vascoferraz.com/tutorials/ds18b20-temperature-sensor/ *
*****************************************************************************/

// Define constants
#define ONE_WIRE_BUS 2 // Define the input/output data pin where the DS18B20 temperature sensor is connected.

// Declare global variables
const char tempPin = ONE_WIRE_BUS; // Define the input/output data pin where the DS18B20 temperature sensor is connected. Change it in the #define zone.
const char resolution = 0x7F; // Set resolution. 9-bit:0x1F | 10-bit:0x3F | 11-bit:0x5F | 12-bit:0x7F
float tempC=0; // Temperature value.
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
const long interval = 2000;


void setup()
{
 Serial.begin(9600);
 readROM(tempPin);
 setResolution(resolution, tempPin); // Resolution is hardcoded. This means that you cannot change it while the software is running.
 readScratchpad(tempPin);
}


void loop()
{
 currentMillis = millis();

 if (currentMillis - previousMillis >= interval)
  {
   previousMillis = currentMillis; // Save the last time you entered here
   tempC=tempGet(tempPin); // The returned value is 10000 times higher...
   tempC=tempC/10000; // ...so you need to divide it by 10000.
   Serial.print("Temperature: ");
   Serial.print(tempC,4); // Print the result up to 4 decimal digits (resolution for 12-bit mode is 0.0625ºC).
   Serial.println(" degrees Celsius");
  }
}


long int tempGet(int tempPin)
{
 // The temperature is organized in two bytes (Byte0 or Least Significant Byte and Byte1 or Most Significant Byte).
 // Least Significant Byte is organized like this: (Bit7|2^3)   (Bit6|2^2)   (Bit5|2^1)   (Bit4|2^0)     (Bit3|2^-1)    (Bit2|2^-2) (Bit1|2^-3) (Bit0|2^-4)
 // Most  Significant Byte is organized like this: (Bit15|Sign) (Bit14|Sign) (Bit13|Sign) (Bit12|2^Sign) (Bit11|2^Sign) (Bit10|2^6) (Bit9|2^5)  (Bit8|2^4)
 byte HighByte, LowByte, OK;
 long int TReading;

 // Request a new reading
 OK = OneWireReset(tempPin);
 if(!OK) return -27400;
 OneWireWriteByte(tempPin, 0xCC); // Skip Rom [CCh]
 OneWireWriteByte(tempPin, 0x44); // Convert T [44h]

 // Read the new reading
 OK = OneWireReset(tempPin);
 if(!OK) return -27400;
 OneWireWriteByte(tempPin, 0xCC); // Skip Rom [CCh]
 OneWireWriteByte(tempPin, 0xBE); // Read Scratchpad [BEh]

 LowByte = OneWireReadByte(tempPin);  // Temperature LSB
 HighByte = OneWireReadByte(tempPin); // Temperature MSB
 TReading = (HighByte << 8) + LowByte; // Shift HighByte to the most significant part of the TReading and then add the LowByte to the least significant part of TReading.
 TReading = (625*TReading); // TReading (in 10000ths of degrees C) is obtained by multiplying it by 625.

 return(TReading);
}


boolean OneWireReset(int Pin) // Reset
{
 // All communication with the DS18B20 begins with an initialization sequence that consists of a reset pulse from the master followed by a presence pulse from the DS18B20.
 // When the DS18B20 sends the presence pulse in response to the reset, it is indicating to the master that it is on the bus and ready to operate.

 char errors;
 digitalWrite(Pin, LOW); // During the initialization sequence...
 pinMode(Pin, OUTPUT);   // ... the bus master transmits (TX) the reset pulse...
 delayMicroseconds(500); // ... by pulling the 1-Wire bus low for a minimum of 480μs.
 pinMode(Pin, INPUT);    // The bus master then releases the bus and goes into receive mode (RX). When the bus is released, the 5kΩ pullup resistor pulls the 1-Wire bus high.
 delayMicroseconds(65);  // ... When the DS18B20 detects this rising edge, it waits 15μs to 60μs...
 byte presence = digitalRead(Pin); //... and then transmits a presence pulse by pulling the 1-Wire bus low for 60μs to 240μs.
 delayMicroseconds(450); // When the bus master goes into receive mode (RX) it has to wait for at least 480μs to check for that presence pulse.
                         // Keep in mind that bus master has already waited for the rising edge (15μs to 60μs) which in this case I used 65μs (450+65=515μs).
 if (presence)
 {
  errors|=B01000000;
  //Serial.println("Presence pulse: error");
  return false;
 }
  //Serial.println("Presence pulse: OK");
  return true;
}


void OneWireWriteByte(int Pin, byte d) // Write time slot (least significant bit first).
{
 byte n;
 for(n=8; n!=0; n--)
 {
  if ((d & 0x01) == 1) // Test if least significant bit is 1.
  {
   digitalWrite(Pin, LOW); // To generate a Write 1 time slot,...
   pinMode(Pin, OUTPUT);   // ... after pulling the 1-Wire bus low,...
   delayMicroseconds(3);   // ... the bus master must release the 1-Wire bus within 15μs.
   pinMode(Pin, INPUT);    // When the bus is released, the 5kΩ pullup resistor will pull the bus high.
   delayMicroseconds(60);
  }

  else // Test if least significant bit is anything else, that is, 0.
  {
   digitalWrite(Pin, LOW); // To generate a Write 0 time slot,...
   pinMode(Pin, OUTPUT);   // ... after pulling the 1-Wire bus low,...
   delayMicroseconds(60);  // ... the bus master must continue to hold the bus low for the duration of the time slot (at least 60μs).
   pinMode(Pin, INPUT);
  }
   d=d>>1; // Now the next bit is in the least significant bit position.
 }
}


byte OneWireReadByte(int Pin) // Read time slot (least significant bit first). Needs interrupts disabled to be reliable.
{
 byte d=0, n=0, b=0;
 for (n=0; n<8; n++)
 {
  digitalWrite(Pin, LOW); // A read time slot is initiated by the master device...
  cli(); // Disable global interrupts
  pinMode(Pin, OUTPUT);   //... pulling the 1-Wire bus low...
  delayMicroseconds(4);   //... for a minimum of 1μs...
  pinMode(Pin, INPUT);    //... and then releasing the bus.
  delayMicroseconds(3);   // Output data from the DS18B20 is valid for 15μs after the falling edge that initiated the read time slot.
  b = digitalRead(Pin);   // Read the 1-Wire bus and store its current data (0 or 1) in byte b.
  sei(); // Enable interrupts
  delayMicroseconds(55); // All read time slots must be a minimum of 60μs in duration with a minimum of a 1μs recovery time between slots.
  d = (d >> 1) | (b << 7); // Shift d to right and insert b in most significant bit position.
 }
  return(d);
}


void readROM(int tempPin) // Read ROM: "Family Code", "Unique Serial Number" and "Cyclic Redundancy Check (CRC)".
{
 OneWireReset(tempPin);
 OneWireWriteByte(tempPin, 0x33); // Read Rom [33h]
 Serial.println("-- 64-Bit Lasered ROM Code --");

 for (unsigned char x=1 ; x<=8 ; x++)
 {
  byte Byte = OneWireReadByte(tempPin);

  // Print Family Code
  if (x==1) {Serial.print("Family Code: ");}
  // Add an extra "0" if Byte is <=15 (0xF).
  if (Byte<=15 && x==1) {Serial.print(0,HEX); Serial.println(Byte,HEX);}
  // Do not add an extra "0" if Byte is >=16 (0x10).
  if (Byte>=16 && x==1) {Serial.println(Byte,HEX);}

  // Print Unique Serial Number
  if (x==2) {Serial.print("Unique Serial Number: ");}
  // Add an extra "0" if Byte is <=15 (0xF). Add a separation symbol ":" at the end if that byte is not the last one.
  if (Byte<=15 && x>=2 && x<=6) {Serial.print(0,HEX); Serial.print(Byte,HEX); Serial.print(":");}
  // Add an extra "0" if Byte is <=15 (0xF). Do not add a separation symbol ":" at the end if that byte is the last one.
  if (Byte<=15 && x>=2 && x==7) {Serial.print(0,HEX); Serial.println(Byte,HEX);}
  // Do not add an extra "0" if Byte is >=16 (0x10). Add a separation symbol ":" at the end if that byte is not the last one.
  if (Byte>=16 && x>=2 && x<=6) {Serial.print(Byte,HEX); Serial.print(":");}
  // Do not add an extra "0" if Byte is >=16 (0x10). Do not add a separation symbol ":" at the end if that byte is the last one.
  if (Byte>=16 && x>=2 && x==7) {Serial.println(Byte,HEX);}

  // Print Cyclic Redundancy Check (CRC)
  if (x==8) {Serial.print("CRC: ");}
  // Add an extra "0" if Byte is <=15 (0xF).
  if (Byte<=15 && x==8) {Serial.print(0,HEX); Serial.println(Byte,HEX);}
  // Do not add an extra "0" if Byte is >=16 (0x10).
  if (Byte>=16 && x==8) {Serial.println(Byte,HEX);}
 }
}


void readScratchpad (int tempPin) // Read Scratchpad (Memory Map).
{
 OneWireReset(tempPin);
 OneWireWriteByte(tempPin, 0xCC); // Skip Rom [CCh]
 OneWireWriteByte(tempPin, 0xBE); // Read Scratchpad [BEh]
 Serial.println();
 Serial.println("-- Scratchpad (Memory Map) --");

 for (unsigned char x=1 ; x<=9 ; x++)
 {
  byte Byte = OneWireReadByte(tempPin);

  // Print Temperature LSB and MSB (Byte 0,1)
  if (x==1) {Serial.print("Temperature LSB|MSB: ");}
  // Add an extra "0" if Byte is <=15 (0xF).
  if (Byte<=15 && x==1) {Serial.print(0,HEX); Serial.print(Byte,HEX);}
  // Do not add an extra "0" if Byte is >=16 (0x10).
  if (Byte>=16 && x==1) {Serial.print(Byte,HEX);} 
  if (x==2) {Serial.print(":");}
  // Add an extra "0" if Byte is <=15 (0xF).
  if (Byte<=15 && x==2) {Serial.print(0,HEX); Serial.println(Byte,HEX);}
  // Do not add an extra "0" if Byte is >=16 (0x10).
  if (Byte>=16 && x==2) {Serial.println(Byte,HEX);}

  // Print Temperature TH or User Byte 1 and Temperature TL or User Byte 2 (Byte 2,3)
  if (x==3) {Serial.print("Temperature TH/UB1|TL/UB2: ");}
  // Add an extra "0" if Byte is <=15 (0xF).
  if (Byte<=15 && x==3) {Serial.print(0,HEX); Serial.print(Byte,HEX);}
  // Do not add an extra "0" if Byte is >=16 (0x10).
  if (Byte>=16 && x==3) {Serial.print(Byte,HEX);} 
  if (x==4) {Serial.print(":");}
  // Add an extra "0" if Byte is <=15 (0xF).
  if (Byte<=15 && x==4) {Serial.print(0,HEX); Serial.println(Byte,HEX);}
  // Do not add an extra "0" if Byte is >=16 (0x10).
  if (Byte>=16 && x==4) {Serial.println(Byte,HEX);}

  // Print Configuration Register - Resolution (Byte 4)
  if (x==5) {Serial.print("Resolution: ");}
  // Add an extra "0" if Byte is <=15 (0xF).
  if (Byte<=15 && x==5) {Serial.print(0,HEX); Serial.println(Byte,HEX);}
  // Do not add an extra "0" if Byte is >=16 (0x10).
  if (Byte>=16 && x==5) {Serial.println(Byte,HEX);} 

  // Print Reserved (Byte 5,6,7)
  if (x==6) {Serial.print("Reserved: ");}
  // Add an extra "0" if Byte is <=15 (0xF).
  if (Byte<=15 && x==6) {Serial.print(0,HEX); Serial.print(Byte,HEX);}
  // Do not add an extra "0" if Byte is >=16 (0x10).
  if (Byte>=16 && x==6) {Serial.print(Byte,HEX);} 
  if (x==7) {Serial.print(":");}
  // Add an extra "0" if Byte is <=15 (0xF).
  if (Byte<=15 && x==7) {Serial.print(0,HEX); Serial.print(Byte,HEX);}
  // Do not add an extra "0" if Byte is >=16 (0x10).
  if (Byte>=16 && x==7) {Serial.print(Byte,HEX);}
  if (x==8) {Serial.print(":");}
  // Add an extra "0" if Byte is <=15 (0xF).
  if (Byte<=15 && x==8) {Serial.print(0,HEX); Serial.println(Byte,HEX);}
  // Do not add an extra "0" if Byte is >=16 (0x10).
  if (Byte>=16 && x==8) {Serial.println(Byte,HEX);}

  // Print CRC (Byte 8)
  if (x==9) {Serial.print("CRC: ");}
  // Add an extra "0" if Byte is <=15 (0xF).
  if (Byte<=15 && x==9) {Serial.print(0,HEX); Serial.println(Byte,HEX);}
  // Do not add an extra "0" if Byte is >=16 (0x10).
  if (Byte>=16 && x==9) {Serial.println(Byte,HEX); Serial.println();}
 }
}


void setResolution(int resolution, int tempPin)
{
 byte TH, TL; // Upper (TH) and lower (TL) alarm trigger registers.
 // The Write Scratchpad [4Eh] command will rewrite all the three bytes stored in the following registers: TH, TL and Config.
 // So, to keep TH and TL bytes unchanged it's necessary to read them both and then write them back.
 OneWireReset(tempPin);
 OneWireWriteByte(tempPin, 0xCC); // Skip Rom [CCh]
 OneWireWriteByte(tempPin, 0xBE); // Read Scratchpad [BEh]
 for (unsigned char x=1 ; x<=4 ; x++) // The first two readings are the temperature LSB and MSB. They should the ignored.
 {
  byte Byte = OneWireReadByte(tempPin);
   if (x==3){TH=Byte;} // Third reading is the TH register.
   if (x==4){TL=Byte;} // Fourth reading is the TL register.
 }

 // Set the resolution and keep the values of the upper (TH) and lower (TL) alarm trigger registers.
 OneWireReset(tempPin);
 OneWireWriteByte(tempPin, 0xCC); // Skip Rom [CCh]
 OneWireWriteByte(tempPin, 0x4E); // Write Scratchpad [4Eh]
 OneWireWriteByte(tempPin, TH);   // Set the TH register.
 OneWireWriteByte(tempPin, TL);   // Set the TL register.
 OneWireWriteByte(tempPin, resolution); // Set the configuration register (resolution).
}
