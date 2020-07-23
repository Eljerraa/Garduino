#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#define ONE_WIRE_BUS_1 A0
OneWire ourWire1(ONE_WIRE_BUS_1);
DallasTemperature tempSensor(&ourWire1);

byte pumpPin = 2;
byte redLED = 3;
byte soilThreshold;
tmElements_t tm;    // For reading the clock

void setup() {
  Serial.begin(9600);
  pinMode(pumpPin, OUTPUT);
  pinMode(redLED, OUTPUT);
  tempSensor.begin();
  tempSensor.setResolution(11);
}


void loop() {
  
// First, checks that the clock module is working.
// Displays an error message and lights the red LED if it can't get a clock reading.
// In case the clock starts working again, the red LED will switch off.
if (RTC.read(tm)) {
  
  byte currentHour = tm.Hour;
  byte currentMinute = tm.Minute;
  byte currentMonth = tm.Month;
  digitalWrite(pumpPin, LOW);
  
// Checks every 2 hours, on the hour
 if ((currentMinute == 0) && (currentHour % 2 == 0)) {
  
  // Checks if it's spring/summer time (April to October)
  if ((currentMonth >= 4) && (currentMonth <=10)) {
    
    // Summer Mode
    // Checks if it's dawn or dusk (6am-8am OR 8pm-10pm)
    if (((currentHour >= 6) && (currentHour <=8)) || ((currentHour >= 20) && (currentHour <=22))) {
      soilThreshold = 50; }
      
    // Between 10am and 6pm is summer day time.
    // The plant must be watered more when it's hot.
    else if ((currentHour >= 10) && (currentHour <=18)){
      float temperature = checkTemperature();
      if (temperature >= 35){
        soilThreshold = 45; }
      else {
        soilThreshold = 35; }
    }      
    // Outside of these hours is summer night time.
    else {
      soilThreshold = 35; }      
  }  
  else {
    // Winter mode
    // Checks if it's dawn or dusk (8am-10am OR 4pm-6pm (Winter Mode))
    if (((currentHour >= 8) && (currentHour <=10)) || ((currentHour >= 16) && (currentHour<=18))) {
      soilThreshold = 40; }
      
    // Between 12noon and 8pm is winter day time.
    // The plant must NOT be watered when it's too cold.
    else if ((currentHour >= 12) && (currentHour <=20)){
      float temperature = checkTemperature();
      if (temperature <= 10){
        soilThreshold = 0; }
      else {
        soilThreshold = 30; }
    }
    // Outside of these hours is winter night time. The plant should not be watered at all.
    else {
      soilThreshold = 0; }
 }
 
// Checks the soil moisture against the current Threshold
drySoil(soilThreshold);
 }
}

// Shows an error and lights the red LED if there's an issue with the clock module
else {
  if (RTC.chipPresent()) {
    Serial.println("The DS1307 has stopped.  Please run the SetTime");
    Serial.println("example to initialize the time and begin running."); 
    Serial.println();}
  else {
    Serial.println("DS1307 read error!  Please check the circuitry.");
    Serial.println(); }
  digitalWrite(redLED, HIGH); }
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


// Checks soil moisture and calls the water pump function if below the threshold
void drySoil(byte soilThreshold){
  int moistureLevel = analogRead(A1);
  
  // Maps the reading from 0-1023 to a number between 0-100
  int mapValue = map(moistureLevel, 0, 1023, 0, 100);
  byte moisturePercentage = 100 - mapValue;
  
  Serial.print("Moisture Level: ");
  Serial.print(moisturePercentage);
  Serial.println("%");

  if (moisturePercentage <= soilThreshold){
    
    // Watering the plant
    Serial.println("Watering plant...");
    digitalWrite(pumpPin, HIGH);
    delay(4000);
    digitalWrite(pumpPin, LOW);
    Serial.println("Stopped watering.");
    delay(4000);
  }
  else {
    return;
  }
}


// Between 7am and 7pm, the device will operate in "day mode". Outside of these hours, the device will run in "Night mode".
void printTime() {
  Serial.print("Time: ");
  print2digits(tm.Hour);
  Serial.write(':');
  print2digits(tm.Minute);
  Serial.println();
}
  


// For printing the time to the serial monitor. Shows 09:05 instead of 9:5
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0'); }
  Serial.print(number);
}
