/*
 * This is my wiew of https://www.instructables.com/How-to-Use-MQTT-With-the-Raspberry-Pi-and-ESP8266/
 * button pin 1 to GND
 * 10k rezistor to pin 2 of button
 * 10k rezistor to 3v3
 * button pin 2 to D7 (GPIO 13)
 * led + to rezistor 100 Ohm
 * rezistor 100 Ohm to D6 (GPIO 12)
 * led - to GND
 */
#include <Bounce2.h> // Used for "debouncing" the pushbutton
#include <ESP8266WiFi.h> // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h> // Allows us to connect to, and publish to the MQTT broker

#define BUTTON_PIN 13
#define LED_PIN 12
// WiFi
// Make sure to update this for your own WiFi network!
const char* ssid = "your-network
const char* wifi_password = "your_password";

// MQTT
// Make sure to update this for your own MQTT Broker!
const char* mqtt_server = "your_server";
const char* mqtt_username = "your_username_for_mqtt";
const char* mqtt_password = "yout_password_for_mqtt";
// The client id identifies the ESP8266 device.
const char* clientID = "Client ID";

const char *outTopic = "your_out_topic";
const char *inTopic = "your_in_topic";
// Initialise the Pushbutton Bouncer object
Bounce bouncer = Bounce();

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;
// 1883 is the default listener port for the Broker
PubSubClient client(mqtt_server, 1883, wifiClient); 

#define DEBUG_MODE true

void callback(char* topic, byte* payload, unsigned int length) {
#ifdef DEBUG_MODE  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
#endif
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW); 
  }

}

void reconnect() {
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
#ifdef DEBUG_MODE    
    Serial.println("Connected to MQTT Broker!");
#endif
    client.subscribe(inTopic);
  }
  else {
#ifdef DEBUG_MODE    
    Serial.println("Connection to MQTT Broker failed...");
    Serial.print("Failed with rc=");
    Serial.println(client.state());
#endif
    digitalWrite(BUILTIN_LED,LOW);
    digitalWrite(LED_PIN,HIGH);    
  }
}

void setup() {
  pinMode(BUILTIN_LED,OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN,INPUT);
  
  bouncer.attach(BUTTON_PIN);
  bouncer.interval(5);
  digitalWrite(BUILTIN_LED, HIGH);
#ifdef DEBUG_MODE
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(ssid);
#endif
  // Connect to the WiFi
  WiFi.begin(ssid, wifi_password);

  // Wait until the connection has been confirmed before continuing
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
#ifdef DEBUG_MODE    
    Serial.print(".");
#endif    
  }
#ifdef DEBUG_MODE
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif
  client.setCallback(callback);
  reconnect();
}

void loop() {
 bouncer.update();
 if (!client.connected()) {
    reconnect();
  }
 client.loop();
  if (bouncer.rose()) {
    // Turn light on when button is pressed down
    // (i.e. if the state of the button rose from 0 to 1 (not pressed to pressed))
    digitalWrite(BUILTIN_LED, LOW);
    // PUBLISH to the MQTT Broker (topic = mqtt_topic, defined at the beginning)
    if (client.publish(outTopic, "Button pressed!")) {
#ifdef DEBUG_MODE      
      Serial.println("Button pushed and message sent!");
#endif      
    }
    // If the message failed to send, we will try again, as the connection may have broken.
    else {
#ifdef DEBUG_MODE      
      Serial.println("Message failed to send. Reconnecting to MQTT Broker and trying again");
#endif      
      reconnect();
      client.publish(outTopic, "Button pressed!");
    }
  }
  else if (bouncer.fell()) {
    // Turn light off when button is released    
    digitalWrite(BUILTIN_LED, HIGH);
  }
}
