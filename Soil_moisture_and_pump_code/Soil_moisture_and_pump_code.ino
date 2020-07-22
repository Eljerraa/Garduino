#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#define ONE_WIRE_BUS_1 A0
OneWire ourWire1(ONE_WIRE_BUS_1);
DallasTemperature tempSensor(&ourWire1);

int pumpPin = 2;
byte dryHours = 0;
byte startTime = 7; // Day mode starts at 7am
byte endTime = 19; // Night mode starts at 7pm


void setup() {
  Serial.begin(9600);
  pinMode(pumpPin, OUTPUT);
  tempSensor.begin();
  tempSensor.setResolution(11);
}


void loop() {
  float Temperature = checkTemperature();
  drySoil(10); // The number is the soil threshold
  delay(1000);
}


// Returns the current temperature
float checkTemperature(){
  tempSensor.requestTemperatures();
  float currentTemp = tempSensor.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.println(currentTemp);
  return currentTemp;
}


// Checks soil moisture and returns true (if soil moisture is below the threshold) or false (if soil is ok)
// soilThreshold can change depending on other conditions
bool drySoil(byte soilThreshold){
  int moistureLevel = analogRead(A1);
  
  // Maps the reading from 0-1023 to a number between 0-100
  int friendlyValue = map(moistureLevel, 0, 1023, 0, 100);
  byte moisturePercentage = 100 - friendlyValue;
  
  Serial.print("Moisture Level: ");
  Serial.print(moisturePercentage);
  Serial.println("%");

  if (moisturePercentage <= soilThreshold){
    return true; }
  else {
    return false; 
  }
}


// Between 7am and 7pm, the device will operate in "day mode". Outside of these hours, the device will run in "Night mode".
bool dayMode() {
    tmElements_t tm;

  if (RTC.read(tm)) {
    if ((tm.Hour >= startTime) && (tm.Hour <= endTime)) {
      Serial.print("(Day Mode) Time: ");
      print2digits(tm.Hour);
      Serial.write(':');
      print2digits(tm.Minute);
      Serial.println();
      return true; 
      }
    else {
      Serial.print ("(Night Mode) Time: ");
      print2digits(tm.Hour);
      Serial.write(':');
      print2digits(tm.Minute);
      Serial.println(); 
      return false;
    }
  }
  
// If there's an issue with the clock module, it will default to day mode.
// In day mode, the plant will be watered more.
// In case of clock module failure, over-watering a plant is less harmful than under-watering. 
  else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 has stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running."); 
      Serial.println();}
    else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println(); }
    return true; }
  delay(1000);
}

// For printing the time to the serial monitor. Shows 09:05 instead of 9:5
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0'); }
  Serial.print(number);
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
