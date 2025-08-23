#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// GPIO5 = D1
#define RELAY_PIN 5

// Replace with your WiFi network credentials
#define WLAN_SSID       "SSID"
#define WLAN_PASS       "PASSWORD"

// Replace with your Adafruit IO credentials
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "USERNAME" 
#define AIO_KEY         "API_KEY"      

// Adafruit IO pub/sub topic
#define POWER_FEED      "pc-power-button"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe powerSwitch = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/" POWER_FEED);

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Remote PC Power Controller Booting..."));

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Serial.print(F("Connecting to WiFi: "));
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());

  // Setup the Adafruit IO subscription
  mqtt.subscribe(&powerSwitch);
}

void loop() {
  // Reconnect automatically if the connection is lost
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &powerSwitch) {
      Serial.print(F("Received message on feed '"));
      Serial.print(POWER_FEED);
      Serial.print(F("': "));
      // Cast byte array to char array
      Serial.println((char *)powerSwitch.lastread);

      if (strcmp((char *)powerSwitch.lastread, "1") == 0) {
        pressPowerButton();
      }
    }
  }

  delay(100);
}

void pressPowerButton() {
  Serial.println(F("Sending power button press"));
  digitalWrite(RELAY_PIN, HIGH); 
  delay(200);                    
  digitalWrite(RELAY_PIN, LOW);  
}

void MQTT_connect() {
  if (mqtt.connected()) {
    return;
  }

  Serial.print(F("Connecting to MQTT"));

  int8_t ret;
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println(F("Retrying MQTT connection in 5 seconds"));
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      Serial.println(F("Failed to connect to MQTT after multiple retries. Resetting"));
      ESP.restart();
    }
  }
  Serial.println(F("MQTT Connected!"));
}
