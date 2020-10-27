/*This code was copied from the following. I only had to make minor changes in order to get it to work
 * Created by Aaron Nelson
 * Lab 4 Event Bus
 * 10-26-19
 * This script is used with the reed switch. It will post to the MQTT server when the magnet becomes connected or disconnected.
 *  
 * The MQTT portion of this script was obtained from https://arduinodiy.wordpress.com/2017/11/24/mqtt-for-beginners/
 *
    Reed Switch Pins:
        One Wire: Pin12 (D6)
        The Other Wire: GND
*/

#include <ESP8266WiFi.h>            // This contains the libraries that allows the board to connect to wifi
#include <PubSubClient.h>           // This is used for communication with the MQTT Server

const char* ssid = "YOUR WIFI HERE";       // Specify the name of your wifi
const char* password = "YOUR PASSWORD HERE";      // Specify the password for your wifi

const char* mqttServer = "192.168.1.106"; // The location of your MQTT Server
const int mqttPort = 1883;                // The port number your MQTT Server is running on (1883 is the default for Mosquitto)

WiFiClient doorSwitchClient;        // Name for our MQTT connection client
PubSubClient client(doorSwitchClient);    // Creates a partially initialized client instance

int reed_switch = 12;               // Pin D6 is connected to one of the switch wires. If using a different pin, specify that here
int reed_status;                    // When this is 0, it means it is connected to the magnet. A 1 means it is disconnected and seperated
int last_reed_status;               // Keeps the last value of the reed magnet (to only publish to the sensor when a CHANGE is made)


void setup() {                      // This runs when the board is first turned on
  Serial.begin(115200);             // This allows serial information when connected to a computer (this will tell when the magnet is connected)
  pinMode(LED_BUILTIN, OUTPUT);     // Prepares the builtin light pin for output (which turns on when the reed magnet is connected)
  digitalWrite(LED_BUILTIN, HIGH);  // Initially, turn off the builtin LED light
  pinMode(reed_switch, INPUT_PULLUP);     // Initialize the reed switch pin for input. Pullup is necessary for when the magnet becomes reconnected
  reed_status = digitalRead(reed_switch); // Read the current value of the magnet switch
  last_reed_status = reed_status;   // Store the initial reading of the magnet switch to detect state changes

  /* Connect to WiFi */
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");   // This is shown on the serial if connected to a computer
  Serial.println(ssid);             // It displays the wifi it is trying to connect to

  WiFi.mode(WIFI_STA);              // It sets the wifis mode to "Station" (rather than "AP" which is the Access Point mode)
  WiFi.hostname("DoorSwitch");      // Hostname to uniquely identify our device
  WiFi.begin(ssid, password);       // Attempts to connect to wifi using provided credentials

  while (WiFi.status() != WL_CONNECTED) {     // While the wifi is not connected yet:
    delay(500);                     // every half second,
    Serial.print(".");              // it prints a dot on the Serial Monitor to show it is still trying to connect
  }
  Serial.println("");
  Serial.println("WiFi connected"); // When it does connect, show that there was success

  connectMQTT();                    // Runs the function to set up our MQTT connection

  // Blink LED quickly to show that it is connected to wifi and the MQTT server
  digitalWrite(LED_BUILTIN, LOW);   // light turns on
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);  // light turns off
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);   // light turns on
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);  // light turns off
  
  client.publish("GARAGE", "Activated");  // Post in the device channel that this device is on
}





void loop() {                       // This constantly cycles through, detecting a change in the reed switch and posting to the MQTT server if there is one
  if (!client.connected()){         // Occasionally the client drops connection randomly and stops working
    Serial.println("MQTT was Unexpectedly Disconnected");
    connectMQTT();                  // If the connection dropped, try to reconnect to our server
  }
  
  reed_status = digitalRead(reed_switch);     // On each loop through, check if the magnet is connected or not

  // Respond to state changes
  if (reed_status == 0 && last_reed_status == 1) {    // If the magnet just barely became connected
      digitalWrite(LED_BUILTIN, LOW);         // The LED light turns on
      Serial.println("connected");            // Displays it is connected in the serial monitor
      client.publish("GARAGE", "1");       // Posts a value to the MQTT server
      last_reed_status = 0;         // Remembers the last state was connected (to only work off state changes)
  }
  if (reed_status == 1 && last_reed_status == 0) {    // If the magnet just barely became disconnected
       digitalWrite(LED_BUILTIN, HIGH);       // The LED light turns off
       Serial.println("disconnected");        // Displays it is disconnected in the serial monitor
       client.publish("garage/doorSwitch", "0");      // Posts a value to the MQTT server
       last_reed_status = 1;         // Remembers the last state was connected (to only work off state changes)
  }
  delay(10);    // Delay is used so that little random (dis)connections aren't detected (like if the magnet is barely on the verge of connection)
}





void connectMQTT() {              // When first connecting to MQTT or when it becomes disconnected, run this script
  client.setServer(mqttServer, mqttPort);     // Setup the server connection with information the user provided
  while(!client.connected()) {    // If the MQTT server is not connected or becomes disconnected, try again
    Serial.println("connecting to MQTT...");
    if (client.connect("DoorSwitch")) {        // Try to connect with this name
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("failed... Re");           // Repeatedly says "failed... Reconnecting" as this loop cycles through
      // Serial.print(client.state());           // Show in the Serial Monitor what the current state of the server connection is
      delay(2000);                // Wait two seconds before trying to connect to the server again
    }
  } 
}
