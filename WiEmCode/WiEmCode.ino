// Including the library
#include <ESP8266WiFi.h>

// Creating variables for the Wifi Name and Password as well as the SMTP email server
const char* ssid = "AlarumWifi"; // Enter the name of your WiFi Network.
const char* password = "Password513";// Enter the Password of your WiFi Network.
char server[] = "mail.smtp2go.com"; // URL of the SMTP Server 

// Reference "espClient" from the WifiClient library
WiFiClient espClient;

void setup()
{
  // Use the serial monitor to view the sensor output with 115200 baud
  Serial.begin(115200);
  delay(10);
  Serial.println(" Alarum is up and running ");

  // Calling the connectWifi & connectSMTPServer functions from below
  connectWifi();
  connectSMTPServer();
  disconnectSTMPServer();
 }

void loop()
{
  
}

void connectWifi()
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

   byte ret = sendEmail(); 
}


byte sendEmail()
{
  // Define the sender email address
  Serial.println(F("Sending From"));
  espClient.println(F("MAIL From: alarumproject@gmail.com"));   // Change to sender email address
  if (!emailResp()) 
    return 0;

  // Define the recipient email address
  Serial.println(F("Sending To"));
  espClient.println(F("RCPT To: nguyeha@mail.uc.edu")); // Change to recipient address
  if (!emailResp()) 
    return 0;

  // Define the data that is going to be send
  Serial.println(F("Sending DATA"));
  espClient.println(F("DATA"));
  if (!emailResp()) 
    return 0;

  //
  Serial.println(F("Sending email"));
  espClient.println(F("To:  nguyeha@mail.uc.edu"));   // Replace with the recipient address
  espClient.println(F("From: alarumproject@gmail.com"));   // Replace with your address

  // Content of the sending email
  espClient.println(F("Subject: Alarum - Current stock status\r\n"));
  espClient.println(F("The current status of the stock is: \r\n"));
  espClient.println(F("Yadadadad"));
  espClient.println(F("Ydadadad"));
  
  // Has to end it "."
  espClient.println(F("."));
  if (!emailResp()) 
    return 0;

}

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
