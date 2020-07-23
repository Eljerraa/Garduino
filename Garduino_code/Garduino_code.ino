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


// Returns the current temperature
float checkTemperature(){
  tempSensor.requestTemperatures();
  float currentTemp = tempSensor.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.println(currentTemp);
  return currentTemp;
}


// Returns the current moisture level as a percentage
byte moistureLevel(byte soilThreshold){
  int  moistureReading = analogRead(A1);
  
  // Maps the reading from 0-1023 to a number between 0-100
  int mapValue = map(moistureReading, 0, 1023, 0, 100);
  byte moisturePercentage = 100 - mapValue;
  
  Serial.print("Moisture Level: ");
  Serial.print(moisturePercentage);
  Serial.println("%");
  return moisturePercentage;
}
  

// Shows the current time on the Serial monitor
void printTime() {
  Serial.print("Time: ");
  print2digits(tm.Hour);
  Serial.write(':');
  print2digits(tm.Minute);
  Serial.println();
}

// For printing the time to the serial monitor. e.g. Shows 09:05 instead of 9:5
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0'); }
  Serial.print(number);
}

// Activates the pump for 4 seconds
void waterPlant(){
  Serial.println("Watering plant...");
  digitalWrite(pumpPin, HIGH);
  delay(4000);
  digitalWrite(pumpPin, LOW);
  Serial.println("Stopped watering.");
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
  printTime();
  
// Checks every 2 hours, on the hour.
 if ((currentMinute == 0) && (currentHour % 2 == 1)) {
  
  // Checks if it's spring/summer time (April to October)
  if ((currentMonth >= 4) && (currentMonth <=10)) {
    
    // Summer Mode
    // Checks if it's dawn or dusk (6am-8am OR 8pm-10pm)
    if (((currentHour >= 6) && (currentHour <=8)) || ((currentHour >= 20) && (currentHour <=22))) {      
      soilThreshold = 50;
      Serial.println("Current mode: Summer Dawn / Dusk"); }
      
    // Between 10am and 6pm is summer day time.
    // The plant must be watered more when it's hot.
    else if ((currentHour >= 10) && (currentHour <=18)){
      float temperature = checkTemperature();
      if (temperature >= 35){
        soilThreshold = 45;
        Serial.println("Current mode: Summer Hot"); }
      else {
        soilThreshold = 35; 
        Serial.println("Current mode: Summer Normal"); }
    }      
    // Outside of these hours is summer night time.
    else {
      soilThreshold = 35;
      Serial.println("Current mode: Summer Night"); }      
  }  
  else {
    // Winter mode
    // Checks if it's dawn or dusk (8am-10am OR 4pm-6pm (Winter Mode))
    if (((currentHour >= 8) && (currentHour <=10)) || ((currentHour >= 16) && (currentHour<=18))) {
      soilThreshold = 40;
      Serial.println("Current mode: Winter Dawn / Dusk"); }
      
    // Between 12noon and 8pm is winter day time.
    // The plant must NOT be watered when it's too cold.
    else if ((currentHour >= 12) && (currentHour <=20)){
      float temperature = checkTemperature();
      if (temperature <= 10){
        Serial.println("Current mode: Winter Cold");
        soilThreshold = 0; }
      else {
        soilThreshold = 30;
        Serial.println("Current mode: Winter Normal");}
    }
    // Outside of these hours is winter night time. The plant should not be watered at all.
    else {
      soilThreshold = 0;
      Serial.println("Current mode: Winter Night");}
 }
 
/* Checks the soil moisture against the current threshold.
 * If it needs watering, the pump will be activated for 4 seconds, wait for 2 minutes, then check again.
 * The plant should realistically be well-watered by the 5th pump.
 * If it still thinks it needs water after the 5th pump, this could indicate an issue with
 * either the moisture sensor, or the pump itself. In this case, the red warning LED will light up.
*/
byte currentMoisture = moistureLevel(soilThreshold);
if (currentMoisture <= soilThreshold){
  Serial.println("Detected that the plant needs water...");
byte waterCount = 0;
  while ((currentMoisture <= 65) && (waterCount < 5)) {
    waterPlant();
    waterCount ++;
    Serial.print("Water count: ");
    Serial.print(waterCount);
    Serial.println();
    delay (120000);
  }
  if (waterCount >=5) {
  digitalWrite(redLED, HIGH);
  Serial.println("Watering unsuccessful, please check water pump and/or moisture sensor.");
  }
 }
else {
  Serial.println("The plant doesn't currently need watering");
}
}
else {
  Serial.print("Next check scheduled at ");
  if (currentHour % 2 == 0) {
    Serial.print(currentHour+2); }
  else {
    Serial.print(currentHour+1); }
  Serial.print(":00");
  Serial.println();
  delay(30000);
}
}

// If there's an issue with the clock module, the red warning LED comes on.
else {
  if (RTC.chipPresent()) {
    Serial.println("The DS1307 has stopped.  Please run the SetTime");
    Serial.println("example to initialize the time and begin running."); 
    Serial.println(); }
  else {
    Serial.println("DS1307 read error!  Please check the circuitry.");
    Serial.println(); }
  digitalWrite(redLED, HIGH); }
  delay(1000);
 }
