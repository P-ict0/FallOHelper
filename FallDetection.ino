/*

Author: Rodrigo Martín Núñez
Date: 28 October 2021

Automatic fall detection device using:
-Arduino Nano microcontroller
-Gyrosensor/Accelerometer MPU6050
-Micro SD card reader
-2 Buttons
-1 LED
-SIM800L Module (NOT IN USE)
*/

#include <Wire.h>
#include <MPU6050_light.h>
#include <SPI.h>
#include "SdFat.h"

// SD card
SdFat SD;

MPU6050 mpu(Wire);

//values


/*
  Pins setup
*/
const int ledPin = 7;                 //Pin of the led indicator
const int resetButtonPin = 6;         // the number of the reset button pin
const int helpButtonPin = 5;          // the number of the help button pin
#define SD_CS_PIN SS                  // Defines the SD card module pin

/*
  State setup
*/
int resetButtonState = 0;             // variable for reading the reset button status
int helpButtonState = 0;              // variable for reading the help button status
int ledState = LOW;                   // ledState used to set the LED

/*
  Time variables
*/
unsigned long gyroMillis = 0;         // Stores previous time that gyro is used
unsigned long lastBlinkMillis = 0;    // Stores last time LED was updated
unsigned long buttonMillis = 0;       // Stores time just before the help button is pressed

/*
  Intervals (ms)
*/
const int interval = 2000;            // Gyro interval to check falling
const int rapidLedInterval = 100;     // LED interval for when person is falling
const int detectingLedInterval = 450; // LED interval when starting to detect the fall (will go faster)
int ledInterval;                      // Sets the interval of the led, can vary
const int buttonInterval = 5000;      // Detects if person pushes button for more than 1s

/*
  Relevant variables
*/
bool falling = false;                 // If true then the person is falling
double angleX;                        // Value of angle in the X-axis
double angleY;                        // Value of angle in the Y-axis
double accX;                          // Value of acceleration in the X-axis
double accY;                          // Value of acceleration in the Y-axis
double accZ;                          // Value of acceleration in the Z-axis
bool accelerating = false;            // If any of the 3 acceleration values exceed the acceleration limit or not
const double accelerationLimit = 2;   // Sets the  limit for the acceleration (to differentiate falling from laying down)
int strike = 0;                       // Strikes that determine if the person if falling
int strikeLimit = 6;                  // Sets the strike limit
const int alpha = 60;                 // Sets the x and y axis fall angle limit
bool calibrating = false;             // true if device is calibrating

/*
  SD card relevant variables
*/
char telephoneNumber[15];             // Phone number to call
char input;                           // Used to read the SD card content
int stringIndex;                      // Used to read the SD card content
File myFile;                          // Initialized File instance


void setup() {

  pinMode(ledPin, OUTPUT); // Initializes led pin

  //initialize the button pins as an inputs
  pinMode(resetButtonPin, INPUT);
  pinMode(helpButtonPin, INPUT);
  
  Serial.begin(9600); //(uncomment if using native USB)

  Wire.begin();
  byte status = mpu.begin();

  digitalWrite(ledPin, HIGH); // Turns on the led while setting up

  Serial.print(F("MPU6050 status: ")); //(uncomment if using native USB)
  Serial.println(status);

  while ((status!=0) || (!SD.begin(SD_CS_PIN))) { // If cannot connect to gyrosensor or sd card module
    
  } 

  Serial.println(F("Calculating offsets, do not move MPU6050")); //(uncomment if using native USB)


  delay(1000);


  mpu.calcOffsets(true,true); // gyro and accelerometer calibration

   Serial.println("Done!"); //(uncomment if using native USB)

  //(uncomment if using native USB)
  
  Serial.print("This took ");
  Serial.print(millis()); 
  Serial.print(" ms...");
  
  
  delay(3000); // Have some time to read output

  digitalWrite(ledPin, LOW); // Turns on the led while setting up

  myFile = SD.open("setup.txt"); //Open the file called "setup.txt"
  
  // Loop over characters until we reach the end of file
  for (stringIndex = 0; stringIndex < 15; stringIndex++) {
    input = myFile.read();
    if (input != EOF) {
      telephoneNumber[stringIndex] = input; // Store it in a variable
    }
  }
  
}

void loop() {
  mpu.update();

  // read the state of the button values:
  resetButtonState = digitalRead(resetButtonPin);
  helpButtonState = digitalRead(helpButtonPin);

  //Get acceleration of all axis
  accX = mpu.getAccX();
  accY = mpu.getAccY();
  accZ = mpu.getAccZ();

  // Enable fall detection with gyroscope when sudden increase in speed happens
  if ((abs(accX) >= accelerationLimit) || (abs(accY) >= accelerationLimit) || (abs(accZ) >= accelerationLimit)) {
    accelerating = true;
  }
  

  /*
    Fall detection
  */
  if( ((( millis() - gyroMillis ) > interval) & !falling & accelerating)) { // Start detection every interval
    
    //Get current values of angles
    angleX = mpu.getAngleX();
    angleY = mpu.getAngleY();

    if ((abs(angleX) > alpha) || (abs(angleY) > alpha)) {
      //Serial.println("Strike added!\n"); // (uncomment if using native USB)
      strike += 1;

    } else {
      //Serial.println("Strike reset!"); //(uncomment if using native USB)
      strike = 0; // reset the strikes
      accelerating = false;
    }
  
    Serial.println("The strikes right now are: "); // (uncomment if using native USB)
    Serial.println(strike);
  
    if (strike == strikeLimit) {
      strike = 0;
      Serial.println("Person is falling!"); //(uncomment if using native USB)
      falling = true; // Person is falling
    }

    gyroMillis = millis();
  }

  
  /* 
    LED indicator
    Makes the LED blink when its detecting the fall or when it has already detected the fall
  */
  if( (( millis() - lastBlinkMillis ) > ledInterval) & (accelerating || falling)) {
    lastBlinkMillis = millis();

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(ledPin, ledState); //Update LED
  }

  //Change led blink speed depending on the situation
  if (falling) {
      ledInterval = rapidLedInterval; // Blink rapidly
  } 
  else if (!falling & !(strike == 0)) {
    ledInterval = detectingLedInterval - (50*strike); // Start blinking and increase frequency
  } 
  else if (!falling & (strike == 0) & !calibrating) {
    digitalWrite(ledPin, LOW); // If on standby turn the LED off
  }

  /*
    Reset button
    Reacts to the press of the reset button
  */
  if (resetButtonState == HIGH) {
    Serial.println("Resetting");
    falling = false; // Resets the fall signal
    accelerating = false;
    strike = 0; // Resets the strikes
    
    if ((millis() - buttonMillis > buttonInterval)) { // If pressed more than the interval
      //Indicate and recalibrate
      calibrating = true;
      digitalWrite(ledPin, HIGH);
      Serial.println(F("Calculating offsets, do not move MPU6050")); // (uncomment when using native USB)
      mpu.calcOffsets();

      delay(2000);
      digitalWrite(ledPin, LOW);

      calibrating = false;
    }
  } else {
    buttonMillis = millis(); // Stores time just before pressing the button
  }

  /* 
    Help button
    Reacts to the press of the help button
  */
  if (helpButtonState == HIGH) {
    Serial.println("Sending help...");
    falling = true; // Sends the fall signal
  }

  /*
    SIM card module code
  */
}
