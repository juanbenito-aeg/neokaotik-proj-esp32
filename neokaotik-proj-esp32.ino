#include <WiFi.h>
#include <PubSubClient.h>

// Define the Wi-Fi network credentials
const char* ssid = "MOVISTAR_D72C"; // Name
const char* password = "BT5kXJ43awzk2CPwp2vV"; // Password

// Define the MQTT broker parameters
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Set software serial baud to 115200
  Serial.begin(115200);

  setUpWiFi();
  setUpBroker();
}

void setUpWiFi() {
  Serial.print("Connecting to ");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected to the Wi-Fi network");
}

void setUpBroker() {
  // Set the server details
  client.setServer(mqtt_server, mqtt_port);

  // Set the function that is called when new messages arrive
  client.setCallback(processMessages);
}

void processMessages(char* topic, byte* payload, unsigned int length) {
  String topicString = String(topic);
  
  // Process messages
  // if (topicString == "") {}
}

void loop() {
  if (!client.connected()) {
    // Reconnect if the connection to the MQTT broker is lost
    reconnect();
  }
  client.loop(); // Keep the connection alive
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Use a unique client ID so that subscriptions always work
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());

    if (client.connect(client_id.c_str())) {
      Serial.println(" Connected!");
    } else {
      Serial.print(" Failed with state ");
      Serial.print(client.state());
      Serial.println(". Try again in 5 seconds");
      delay(5000);
    }
  }

  // Subscribe to topics
  // client.subscribe("");
}
