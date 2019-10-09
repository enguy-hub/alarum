/*
 * Ports from Sensor --> ESP8266
 * Sensor's port = ESP8266's port
 *           VIN = 5V 
 *           GND = GND
 *           SDA = 2
 *           SCL = 14
*/

#include "Adafruit_VL53L0X.h"

// References from library
Adafruit_VL53L0X sensor = Adafruit_VL53L0X();
VL53L0X_RangingMeasurementData_t measure;

// Create some distance variables
unsigned long current_distance;
const unsigned int HIGH_DIST = 130;
const unsigned int MED_DIST = 270;
const unsigned int LOW_DIST = 440; // Anything over 440 mm is "out of stock"

// Create an enum for state of the rack
enum state 
{
  High_State,   // High on stock
  Medium_State, // Medium on stock
  Low_State,    // Low on stock
  Out_State    // Out of stock
};

enum state rackState;

void setup() 
{
  Serial.begin(115200);
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // wait until serial port opens for native USB devices
  while (!Serial) 
  {
    delay(1);
  }
    
  Serial.println("Adafruit VL53L0X test");  
  
  if (!sensor.begin()) 
  {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  
  // power 
  Serial.println(F("VL53L0X API Simple Ranging example\n\n")); 
}


void loop() 
{   
  measuring();
  delay(500); 
}

void measuring()
{ 
  // Getting data from the sensor
  sensor.rangingTest(&measure, false);  // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) 
  {  // phase failures have incorrect data
    Serial.print("Current distance to the last item on the rack (mm): "); 
    Serial.println(measure.RangeMilliMeter);
    current_distance = measure.RangeMilliMeter;
    
    //starting the infinite measuring loop
    checkStatus();
    delay(500);
  }
  else 
  {
    Serial.println(" out of range ");
  }

  // Wait at least 60ms before next measurement
  delay(60);
}

void checkStatus()
{
  if(current_distance <= HIGH_DIST) 
  {        
      Serial.println(" Stock status: High ");
      rackState = High_State;
      
      // Wait at least 60ms before next measurement
      delay(60);
  }  
  else if (current_distance <= MED_DIST & current_distance > HIGH_DIST)
  {        
      Serial.println(" Stock status: Med ");
      rackState = Medium_State;
      
      // Wait at least 60ms before next measurement
      delay(60);
  }  
  else if (current_distance <= LOW_DIST & current_distance > MED_DIST)
  {
      Serial.println(" Stock status: Low ");
      rackState = Low_State;
    
      // Wait at least 60ms before next measurement
      delay(60);
  }
  else
  {
      Serial.println(" Stock Status: Out of stock ");
      digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
      delay(200);                      // wait for a second
      digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
      delay(200);                      // wait for a second
      rackState = Out_State;

      // Wait at least 60ms before next measurement
      delay(60);
  }
}
