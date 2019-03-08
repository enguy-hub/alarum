/*
 * Ports from Sensor --> ESP8266
 * Sensor's port = ESP8266's port
 *           VIN = 5V 
 *           GND = GND
 *           SDA = 2
 *           SCL = 14
*/

// Including the library
#include <ESP8266WiFi.h>
#include "Adafruit_VL53L0X.h"

// References from library
Adafruit_VL53L0X sensor = Adafruit_VL53L0X();
VL53L0X_RangingMeasurementData_t measure;

// Reference "espClient" from the WifiClient library
WiFiClient espClient;

// Variables for the Wifi Name and Password as well as the SMTP email server
const char* ssid = "lans"; // Enter the name of your WiFi Network.
const char* password = "password";// Enter the Password of your WiFi Network.
char server[] = "mail.smtp2go.com"; // URL of the SMTP Server 

// Variables for distance sensor
unsigned long current_distance;
const unsigned int HIGH_DIST = 170;
const unsigned int MED_DIST = 300;
const unsigned int LOW_DIST = 460; // Anything over 460 mm is "out of stock"

// Create an enum for state of the rack
enum state 
{
  High_State,   // High on stock    #0
  Medium_State, // Medium on stock  #1
  Low_State,    // Low on stock     #2
  Out_State     // Out of stock     #3
};

enum state rackState;

void setup()
{
  // initialize digital pin LED_BUILTIN as an output for signaling.
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Use the serial monitor to view the sensor output with 115200 baud
  Serial.begin(115200);
  delay(10);

  // Wait until serial port opens for native USB devices
  while (!Serial) 
  {
    delay(1);
  }

  // Inspecting connection with the sensor
  Serial.println("Adafruit VL53L0X test");  
  if (!sensor.begin()) 
  {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  
  // Power on
  Serial.println(F("Alarum activated - ESP8266 and VL53L0X are connected\n\n"));

  // Calling the "wifiConnect" function from below
  connectingWifi();
  measuring();
 }

void loop()
{
  // Enabled this (and disabled the one in "setup" for the infinite loop of distance measuring
  //measuring();
  //delay(5000);  
}


/*
 * This section below has the function for connecting the microcontroller to the Wifi
 * 
*/


// This function is used to connect the microcontroller to the Wifi
void connectingWifi()
{
  // Entering Wifi name and password from the above reference
  Serial.println("");
  Serial.print("Connecting To: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // If it couldn't connect to the wifi send out a sugesstion prompt
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("*");
  }

  // Display that the microcontroller is connected to the Wifi and show its IP Address
  Serial.println("");
  Serial.println("WiFi Connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


/*
 * The below section has all the functions for the sensor
 * 
*/


// This function is used to measure the current distance on the rack
void measuring()
{ 
  // Getting data from the sensor
  sensor.rangingTest(&measure, false);  // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) 
  {  // phase failures have incorrect data
    Serial.print("Distance from the closet item (mm): "); 
    Serial.println(measure.RangeMilliMeter);
    current_distance = measure.RangeMilliMeter;

    // Start the infinite loop of measuring
    checkStatus();
    delay(5000);
  }
  else 
  {
    Serial.println(" out of range ");
  }

  // Wait at least 60ms before next measurement
  delay(60);
}


// This function is used to check on the current status of bottle organizer 
void checkStatus()
{
  if(current_distance <= HIGH_DIST) 
  {    
      rackState = High_State;    
      Serial.println(" Current Stock Status: High on stock ");     
      
      // Wait at least 60ms before next measurement
      delay(60);
  }  
  else if (current_distance <= MED_DIST & current_distance > HIGH_DIST)
  {    
      rackState = Medium_State;    
      Serial.println(" Current Stock Status: Medium on stock ");      
      
      // Wait at least 60ms before next measurement
      delay(60);
  }  
  else if (current_distance <= LOW_DIST & current_distance > MED_DIST)
  {
      rackState = Low_State;
      Serial.println(" Current Stock Status: Low on stock ");
    
      // Wait at least 60ms before next measurement
      delay(60);
  }
  else
  {
      rackState = Out_State;
      Serial.println(" Current Stock Status: Out of stock ");
      digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
      delay(200);                      // wait for a second
      digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
      delay(200);                      // wait for a second
      
      // Wait at least 60ms before next measurement
      delay(60);
  }

  byte ret = connectSMTPServer();
}


/*
 * This next section has all the functions for connecting to the SMTP Server and sending email
 * 
*/


// This function is used to connect the microcontroller to the SMTP Server
byte connectSMTPServer()
{
  // Connecting to the email SMTP server (SMTP2GO for Alarum)
  if (espClient.connect(server, 2525) == 1) 
  {
    // If the connection is successful, a 250 Response Code will show up.
    Serial.println(F("Connected to the email server"));
  } 
    else 
    {
      Serial.println(F("Connection failed"));
        return 0;
    }
  if (!emailResp()) 
    return 0;

  // HOLA test for the server
  Serial.println(F("Sending HOLA"));
  espClient.println("EHLO www.example.com");
  if (!emailResp()) 
  return 0;

  // Authorizing with SMTP Server, will get a Response 334 for success.
  Serial.println(F("Sending authorization login"));
  espClient.println("AUTH LOGIN");
  if (!emailResp()) 
    return 0;

  // Send the encoded SMTP Username and Password. When success, will get a 235 Response.
  // Sending STMP2GO username with base64 encoded code
  Serial.println(F("Sending User"));
  espClient.println("YWxhcnVtcHJvamVjdEBnbWFpbC5jb20="); //Insert your base64, ASCII encoded Username
  if (!emailResp()) 
    return 0;
    
  // Sending STMP2GO password with base64 encoded code
  Serial.println(F("Sending Password")); 
  espClient.println("Q2luY2lubmF0aTUxMw=="); //Insert your base64, ASCII encoded Password
  if (!emailResp()) 
    return 0;

   byte ret1 = sendEmail();
   byte ret2 = disconnectSTMPServer();   
}


// This function is used to send the alert email using the above SMTP Server
byte sendEmail()
{
  // Define the sender email address
  Serial.println(F("Sending From"));
  espClient.println(F("MAIL From: alarumproject@gmail.com"));   // Change to sender email address
  if (!emailResp()) 
    return 0;

  // Define the recipient email address
  Serial.println(F("Sending To"));
  espClient.println(F("RCPT To: nguyehd@mail.uc.edu")); // Change to recipient address
  if (!emailResp()) 
    return 0;

  // Define the data that is going to be send
  Serial.println(F("Sending DATA"));
  espClient.println(F("DATA"));
  if (!emailResp()) 
    return 0;

  //
  Serial.println(F("Sending email"));
  espClient.println(F("To:  nguyehd@mail.uc.edu"));   // Replace with the recipient address
  espClient.println(F("From: alarumproject@gmail.com"));   // Replace with your address

  // Content of the sending email
  espClient.println(F("Subject: Alarum - Current stock status\r\n"));
  espClient.println(F("This email is a notification of the current status of the bottle organizer \r\n"));

  // Checking the condition and send email according to that
  if(rackState <= High_State) // rackState <= High_State current_distance
  {   
      //espClient.println(High_State);    
      espClient.println(F(" Current Status of The Bottle Organizer: HIGH on stock "));
      
      // Wait at least 60ms before next measurement
      delay(60);
  }  
  else if (rackState <= Medium_State & rackState > High_State) // rackState <= Medium_State & rackState > High_State
  {    
      //espClient.println(Medium_State);
      espClient.println(F(" Current Status of The Bottle Organizer: MEDIUM on stock "));     
      
      // Wait at least 60ms before next measurement
      delay(60);
  }  
  else if (rackState <= Low_State & rackState > Medium_State) // rackState <= Low_State & rackState > Medium_State
  {
      //espClient.println(Low_State);
      espClient.println(F(" Current Status of The Bottle Organizer: Low on stock "));
    
      // Wait at least 60ms before next measurement
      delay(60);
  }
  else 
  {    
      //espClient.println(Out_State);
      espClient.println(F(" Current Status of The Bottle Organizer: Out of stock "));
      
      // Wait at least 60ms before next measurement
      delay(60);
  }
  
  // Has to end it "."
  espClient.println(F("."));
  if (!emailResp()) 
    return 0;
}


// This function is used to disconnect the microcontroller from the SMTP Server
byte disconnectSTMPServer()
{
    Serial.println(F("Sending QUIT"));
  espClient.println(F("QUIT"));
  if (!emailResp()) 
    return 0;
    
  espClient.stop();
  Serial.println(F("Disconnected from the email server"));
    return 1;
}


// This function is used to reponse back to the SMTP Server *DO NOT DELETE*
byte emailResp()
{
  byte responseCode;
  byte readByte;
  int loopCount = 0;
  
    while (!espClient.available()) 
    {
      delay(1);
      loopCount++;
      // Wait for 20 seconds and if nothing is received, stop.
      if (loopCount > 20000) 
      {
        espClient.stop();
        Serial.println(F("\r\nTimeout"));
          return 0;
      }
    }
  
    responseCode = espClient.peek();
    while (espClient.available())
    {
      readByte = espClient.read();
      Serial.write(readByte);
    }
  
    if (responseCode >= '4')
    {
      // efail();
      return 0;
    }
    return 1;
}
