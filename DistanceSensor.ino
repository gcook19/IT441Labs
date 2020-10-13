#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

//define pins 
int echoPin = D6;
int triggerPin  = D7;

long duration;
long distance;  

const char*  baseURL = "http://192.168.1.244" ;
const char* redURL = "http://192.168.1.244/red";
const char* yellowURL = "http://192.168.1.244/yellow"; 
const char* greenURL = "http://192.168.1.244/green";
const char* blinkURL = "http://192.168.1.244/blink";

//Wifi settings
char ssid[] = "165 E 2100 N";
char pass[] = "165e2100n";



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

  WiFi.hostname("Name"); 
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
  
  
  Serial.print("Ultra Sound Ready...\n"); 
  
  pinMode(echoPin, INPUT);
  pinMode(triggerPin, OUTPUT); 

}

void loop() {
  
  //taken from the in class example 
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


  if(distance > 20  ){
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
