//Garett Cook IT&C 441 Lab 4 ultrasound edit 


#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Arduino.h> 

//define pins 
int echoPin = D6;
int triggerPin  = D7;

long duration;
long distance;  

const char*  baseURL = "http://192.168.1.244" ;
const char* redURL = "http://192.168.1.244/red";
const char* yellowURL = "http://192.168.1.244/yellow"; 
const char* greenURL = "http://192.168.1.244/green";
const char* blinkURL = "http://192.168.1.244/off";


//Wifi settings
char ssid[] = "165 E 2100 N";
char pass[] = "165e2100n";

//MQTT settings
//These settings and the mqtt code were taken from the in class example and modified to meet the needs of this senario. vc cvbcm          m
const char* server = "192.168.1.106";
char* topic = "GARAGE";
String macAddr = WiFi.macAddress();
String host = "arduino-" + macAddr.substring(15); 

//Start MQTT wifi settings. 
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);  

//make the functions 

void askForRed(){

  //taken from https://techtutorialsx.com/2016/07/21/esp8266-post-requests/
  HTTPClient http;

  http.begin(redURL); 
  http.addHeader("Content-Type", "text/plain");

  int httpCode = http.POST("Message from ESP8266");   //Send the request
  String payload = http.getString();

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload
 
  http.end();  //Close connection
  Serial.print("Red http request ended\n"); 
}

void askForYellow(){

  //taken from https://techtutorialsx.com/2016/07/21/esp8266-post-requests/
  HTTPClient http;

  http.begin(yellowURL); 
  http.addHeader("Content-Type", "text/plain");

  int httpCode = http.POST("Message from ESP8266");   //Send the request
  String payload = http.getString();

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload
 
  http.end();  //Close connection
 
  Serial.print("Yellow request ended\n"); 
}

void askForGreen(){

  //taken from https://techtutorialsx.com/2016/07/21/esp8266-post-requests/
  HTTPClient http;

  http.begin(greenURL); 
  http.addHeader("Content-Type", "text/plain");

  int httpCode = http.POST("Message from ESP8266");   //Send the request
  String payload = http.getString();

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload
 
  http.end();  //Close connection
  Serial.print("Green request ended \n"); 
}


void askForBlink(){
  //taken from https://techtutorialsx.com/2016/07/21/esp8266-post-requests/
  HTTPClient http;

  http.begin(blinkURL); 
  http.addHeader("Content-Type", "text/plain");

  int httpCode = http.POST("Message from ESP8266");   //Send the request
  String payload = http.getString();

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload
 
  http.end();  //Close connection
  Serial.print("Blink curl\n");
}


void setup() {

  Serial.begin(9600); 
  //Connect to Wifi
  Serial.print("Connecting to Wifi.");

  WiFi.hostname(host); 
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
  //server.begin();
  //Serial.println("Server Started...");
  
  Serial.println("Connecting to MQTT Broker"); // Serial debug
  mqttClient.setServer(server, 1883);   // Set up MQTT Connection Info
  mqttClient.setCallback(callback);     // Function to call when new messages are received
  mqttClient.connect(host.c_str());     // Connect to the broker with unique hostname
  mqttClient.subscribe(topic);       // Subscribe to the LED topic on the broker
  Serial.println(mqttClient.state());   // Show debug info about MQTT state  


  //UltraSound Setup
  Serial.print("Getting Ultra Sound Ready...\n"); 
  pinMode(echoPin, INPUT);
  pinMode(triggerPin, OUTPUT); 
  Serial.print("Ultra Sound Ready.\n");
}

void loop() {
  mqttClient.loop();
}

void callback(char* topicChar, byte* payload, unsigned int length) {
  String topicNOW = (String)topicChar;     // Convert the char* to a String
  String message = "";                  // Convert the byte* payload to String
  // There won't be a null character to terminate, so we manually copy each character
  for (int i = 0; i < length; i++) {    // For each character of the payload
    message += (char)payload[i];        // append to the message string
  }

  Serial.print("Message arrived [");    // Serial Debug
  Serial.print(topic);                  //    Print the topic name [in brackets]
  Serial.print("] ");                   //
  Serial.println(message);              //    And the message

  if (topicNOW == (String)topic) { 
    if(message == "OPEN" || message == "1"){
      //taken from the in class example of ultrasounds  
      digitalWrite(triggerPin, LOW);    // Turn off the trigger and let things quiet down
      delay(20);                  // Let it sit for 20 milliseconds
      digitalWrite(triggerPin, HIGH);   // Turn on the trigger to start measurement
      delayMicroseconds(10);      // Send a very short pulse (10us)
      digitalWrite(triggerPin, LOW);    // Turn off TRIG pin - this will start the ECHO pin high
      duration = pulseIn(echoPin, HIGH);   // Reads ECHO, returns the travel time in microseconds
      
      //Print out the distance
      Serial.print("Distance: "); // debug out
      distance = duration*0.013504/2; 
      Serial.print(distance); 
      Serial.print("<-saved num, real num-> "); 
      Serial.print(duration*0.013504/2); // Calculate the distance in inches (13,504 in/s)
      Serial.print("in.");
      //Serial.print(duration*0.0343/2);   // Calculate the distance in cm (34,300 cm/s)
      Serial.print("\n");
      
      
      if(distance > 20 ){
          //trigger green state
          //indicates need to be closer 
          askForGreen(); 
       }
       else if (distance < 19 && distance > 10 )
       {
        //trigger yellow state
        //getting close to target zone
        askForYellow(); 
       }
       else if(distance < 9 && distance > 5 ) 
       {
        //trigger red state 
        // they are in the target zone
        askForRed(); 
       }
       else if(distance < 5 )
       {
        //trigger blink state
        //they are past the target zone and need to back up
        askForBlink(); 
       } 
       
      delay(2000); 
    }
    else{
      askForBlink();  
    }
  }
}
