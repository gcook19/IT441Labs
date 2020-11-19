/*
 * Garett Cook Most taken from: 
 * Created by Aaron Nelson
 * Lab 6 - User Interface - Garage Door
 * 11-26-19
 * This script is used with the Wemos Relay Shield. It is meant to trigger the opening of a garage door. It listens for a value (of '1') to be posted in an Adafruit.io feed. Then it triggers the relay for one second. It posts in the local MQTT server that this completed.
 * 
 * The MQTT portion of this script was obtained from https://arduinodiy.wordpress.com/2017/11/24/mqtt-for-beginners/
 * 
 * The Adafruit.io portion of this script was obtained from the Arduino example (after adding the Adafruit Library) at File -> Examples -> Adafruit MQTT Library -> mqtt_esp8266_callback
 * 
 * The Relay triggering portion was obtained from https://github.com/petehoffswell/garagedoor/blob/master/garagedoor/garagedoor.ino
 *
    Relay Shield Pins (if soldering doesn't allow shield to be placed on board directly):
        5V: +5VDC
        D1 (triggering pin): D1
        G: GND
 */

#include <ESP8266WiFi.h>            // This contains the libraries that allows the board to connect to wifi
#include "Adafruit_MQTT.h"          // Library to access data from an Adafruit.io online feed
#include "Adafruit_MQTT_Client.h"   // Used to subscribe to an Adafruit.io online feed
#include <PubSubClient.h>           // This is used for communication with the local MQTT Server (Mosquitto)

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "165 E 2100 N"        // Specify the name of your wifi
#define WLAN_PASS       "165e2100n"   // Specify the password for your wifi

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"       // Pulling data from the Adafruit website
#define AIO_SERVERPORT  1883                    // use 8883 for SSL
#define AIO_USERNAME    "gcook19" // Username for Adafruit (goes before the /feed/# MQTT feed)
#define AIO_KEY         "aio_jnYQ60GkYzTrY52eeLINKEXtytUx"  // Obtained by going to io.adafruit.com and clicking AIO Key link in top right. Copy the "Active Key" here

/************************* Mosquitto MQTT Setup ******************************/

const char* mqttServer = "192.168.1.230";       // The location of your local MQTT Server
const int mqttPort = 1883;                      // The port number your MQTT Server is running on (1883 is the default for Mosquito)

WiFiClient buttonRelayClient;                   // Name for our MQTT Wifi connection client
PubSubClient relayClient(buttonRelayClient);    // Creates a partially initialised client instance


/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
// Watch a feed called 'garageTrigger' and subscribe to changes.
Adafruit_MQTT_Subscribe garageTriggerbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/garageTrigger");

/*************************** Sketch Code ************************************/

const int relayPin = D1;            // Defines the pin that controls the relay (Digital pin 1)
int relayState = LOW;               // Initially, the relay should not be on



void setup() {                      // This runs when the board is first turned on
  Serial.begin(9600);             // This allows serial information when connected to a computer (in here, this shows MQTT passed information)

  pinMode(relayPin, OUTPUT);        // The relay pin should be set as an output (it tells it when to be triggered)
  digitalWrite(relayPin, LOW);      // The relay pin should be set to low (it should not be on)
  pinMode(LED_BUILTIN, OUTPUT);     // Prepares the builtin light pin for output (to notify when the board is connected to MQTT servers)
  digitalWrite(LED_BUILTIN, HIGH);  // Initially, turn off the builtin LED light

  /* Connect to WiFi */
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");   // This is shown on the serial if connected to a computer
  Serial.println(WLAN_SSID);        // It displays the wifi it is trying to connect to

  WiFi.mode(WIFI_STA);              // It sets the wifis mode to "Station" (rather than "AP" which is the Access Point mode)
  WiFi.hostname("ButtonRelay");     // Hostname to uniquely identify our device
  WiFi.begin(WLAN_SSID, WLAN_PASS);         // Attempts to connect to wifi using provided credentials

  while (WiFi.status() != WL_CONNECTED) {   // While the wifi is not connected yet:
    delay(500);                     // every half second,
    Serial.print(".");              // it prints a dot on the Serial Monitor to show it is still trying to connect
  }
  Serial.println("");
  Serial.println("WiFi connected"); // When it does connect, show that there was success


  // Setup the Adafruit MQTT subscription for the garageTrigger feed.
  mqtt.subscribe(&garageTriggerbutton);
  Serial.println("Setup Complete"); 
}





void loop() { 
  Serial.println("Loop Started");
  // This constantly cycles through, ensuring both MQTT servers are connected and watching if information comes across Adafruit.io
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  if(!mqtt.connected())
  {
    MQTT_connect();
  }
  
  if (!relayClient.connected()){    // If Adafruit.io is no longer connected
    mosquittoConnect();             // Attempt to reconnect
  }

  Serial.println("Connections Ready....Loop Called"); 
  // this is our 'wait for incoming subscription packets' busy subloop
  Serial.println("Attempting to create *subscribed.......");
  //Adafruit_MQTT_Subscribe *subscription = mqtt.readSubscription(5000);
  Serial.println("*subscribe created"); 
  // Adafruit will listen to the feed we subscribed to
  while (true) {
    Serial.println("in while loop");
    Adafruit_MQTT_Subscribe *subscription = mqtt.readSubscription(5000);
    if (subscription == &garageTriggerbutton) {
      // Something was picked up in the feed (a value was posted to Adafruit)
      Serial.print(F("Got: "));
      String information = "";      // A string to insert the characters read
      char* ch = (char *)garageTriggerbutton.lastread;
      information.concat(ch);       // Add the read character to the information string
      Serial.println(information);

      if (information == "1") {     // A triggering value of "1" was obtained
        Serial.println("Starting the trigger");   // Show it was received in the Serial Monitor
        relayClient.publish("garage/buttonRelay", "Triggered");   // Show it is being triggered in the Local Mosquitto Server
        digitalWrite(relayPin, HIGH);             // Activate the relay
        delay(1000);                              // Wait one second (adjust according to your garage door button)
        digitalWrite(relayPin, LOW);              // Deactivate the relay (to act as a pulse)
        relayClient.publish("garage/buttonRelay", "Waiting");     // Show job completed in Local Mosquitto server
        information = "0";          // Done otherwise it would keep the last value (1) and continue to "read" triggered commands
      }
    }
  }
}





// Function to connect and reconnect as necessary to the Adafruit.io MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;
  Serial.println("MQTT_CONNECT function called"); 
  // Stop if already connected.
  if (mqtt.connected()) {
    Serial.println("Already Connected"); 
    return;
  }

  Serial.print("Connecting to Adafruit... ");

  uint8_t retries = 3;                  // It will attempt to reconnect 3 times before quitting entirely
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying Adafruit connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;    // Count against one of the retries because there was no success
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("Adafruit Connected!");  // Connection was successful!
  
}





// Function to connect and reconnect as necessary to the local Mosquitto MQTT server.
void mosquittoConnect() {              // When first connecting to MQTT or when it becomes disconnected, run this script
  relayClient.setServer(mqttServer, mqttPort);     // Setup the server connection with information the user provided
  while(!relayClient.connected()) {    // If the MQTT server is not connected or becomes disconnected, try again
    Serial.print("Connecting to Mosquitto... ");
    if (relayClient.connect("ButtonRelay")) {      // Try to connect with this name
      Serial.println("Mosquitto Connected!");
    } else {
      Serial.print("failed... Re");   // Repeatedly says "failed... Reconnecting" as this loop cycles through
      // Serial.print(client.state());              // Show in the Serial Monitor what the current state of the server connection is
      delay(2000);                    // Wait two seconds before trying to connect to the server again
    }
  } 
}
