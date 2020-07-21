#include "Wire.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#define ONE_WIRE_BUS_1 A0

int pumpPin = 2;
byte dryHours = 0;

OneWire ourWire1(ONE_WIRE_BUS_1);
DallasTemperature tempSensor(&ourWire1);

void setup() {
  Serial.begin(9600);
  pinMode(pumpPin, OUTPUT);
  tempSensor.begin();
  tempSensor.setResolution(11);
}

void loop() {
}


float checkTemperature(){
  tempSensor.requestTemperatures();
  float currentTemp = tempSensor.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.println(currentTemp, 1);
  return currentTemp;
}




// Checks soil moisture and returns true (if soil moisture is below the threshold) or false (if soil is ok)
// soilThreshold can change depending on other conditions
bool drySoil(byte soilThreshold){
  int moistureLevel = analogRead(A0);
  
  // Maps the reading from 0-1023 to a number between 0-100
  int friendlyValue = map(moistureLevel, 0, 1023, 0, 100);
  byte moisturePercentage = 100 - friendlyValue;
  
  Serial.print("Moisture Level: ");
  Serial.print(moisturePercentage);
  Serial.println("%");

  if (moisturePercentage <= soilThreshold){
    return true; }
  else {
    return false; }
}


void waterPlant(byte dryHours){
  Serial.println("Watering plant...");
  digitalWrite(pumpPin, HIGH);
  delay(4000);
  digitalWrite(pumpPin, LOW);
  Serial.println("Stopped watering.");
  delay(4000);
  dryHours = 0;
}
