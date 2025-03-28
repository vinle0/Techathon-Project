// Wire Slave Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI slave device
// Refer to the "Wire Master Writer" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
  Serial.println("Hello World!");
}
String msg = "";
volatile bool newData = false;   // flag to indicate new data received
volatile uint8_t receivedX = 0;  // store the received value
void loop()
{
  Serial.print(".");
  delay(100);
  // if (msg.length() > 1) {
  //   Serial.println(msg);
  //   msg = "";
  // }
  if (newData) {
    Serial.print("Received x = ");
    Serial.println(receivedX);
    Serial.print(" | (hex): 0x");
    Serial.println(receivedX, HEX);
    newData = false;  // reset flag
  }
  
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()

void receiveEvent(int howMany)
{
  // Serial.println("Msg:");
//   msg = "Msg: ";
//   while(1 < Wire.available()) // loop through all but the last
//   {
//     char c = Wire.read(); // receive byte as a character
//     msg += c;
//     // Serial.print(c);         // print the character
//   }
//   while (Wire.available()) {
//     uint8_t x = Wire.read();    // receive byte as an integer
//     msg += String(x);
//   }
//   // Serial.println(x);         // print the integer
//   //Serial.println("EndMsg.");
  // if (Wire.available()) {
  //   uint8_t x = Wire.read();    // read the byte
  //   Serial.print("Received x = ");
  //   Serial.println(x);
  // }
  if (Wire.available()) {
    receivedX = Wire.read();
    newData = true;
  }
}
