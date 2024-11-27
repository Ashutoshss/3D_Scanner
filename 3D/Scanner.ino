#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <SPI.h>
#include <SD.h>

//Variable
float D_fixed = 150;
volatile bool limitSwitchTriggered = false;  // Flag to indicate the interrupt
int i = 0;
float angle = 0.0;
float x = 0.0;
float y = 0.0;
float z = 0.0;

//Pins
const int buzzerPin = 3;
const int chipSelect = 4;
const int limitSwitchPin = 2;    // Pin where the limit switch is connected

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);
  
  if (!lox.begin()) {
    Serial.println("Failed to initialize VL53L0X. Check wiring!");
    while (1);
  }
  Serial.println("VL53L0X initialized successfully.");
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }
  Serial.println("card initialized.");
  pinMode(buzzerPin, OUTPUT);  
  pinMode(limitSwitchPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(limitSwitchPin), limitSwitchISR, FALLING);

  DataLogger(x,y,z);    // initialize the data with cordinate 0.0 ,0.0, 0.0  
}

void loop() {
  if (!limitSwitchTriggered){
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);
    
    if (measure.RangeStatus != 4) {
      float D_measured = measure.RangeMilliMeter;
      float Radius = D_fixed - D_measured;
      x = Radius * cos(radians(angle));
      y = Radius * sin(radians(angle));
      // rotate the disc
      DataLogger(x,y,z);
      while(!limitSwitchTriggered){
        // move the actuator
        z = z + 8;  // in  mm 
      }
    } else {
      Serial.println("Out of range.");
    }
    delay(10);
  }
  else{
    Serial.println("limitSwitchTriggered");
    digitalWrite(buzzerPin, HIGH);  // Turn on the buzzer
    delay(500);                     // Keep the buzzer on for 500 ms
    digitalWrite(buzzerPin, LOW);
    delay(500);
    }
}

void limitSwitchISR() {
  limitSwitchTriggered = true;  
}
void DataLogger(float x, float y, float z){
  File myFile = SD.open("datalog.txt", FILE_WRITE);

  if (myFile) {
    myFile.print(x);
    myFile.print(",");
    myFile.print(y);
    myFile.print(",");
    myFile.println(z);
    myFile.close();
  }
  else {
    Serial.println("error opening datalog.txt");
  }
}
