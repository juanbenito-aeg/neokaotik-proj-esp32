#include <WiFi.h>
#include <PubSubClient.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <ESP32Servo.h>

// Define the Wi-Fi network credentials
const char* ssid = ""; // Name
const char* password = ""; // Password

// Define the MQTT broker parameters
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Create a partially initialized broker client instance
WiFiClient espClient;
PubSubClient client(espClient);

// Create elements to work with the MFRC522 RFID reader
MFRC522DriverPinSimple ss_pin(5);
MFRC522DriverSPI driver{ss_pin}; // Create SPI driver
MFRC522 mfrc522{driver}; // Create MFRC522 instance

// Define the servo and the pin it is connected to
Servo myServo;
const int servoPin = 13;

void setup() {
  // Set software serial baud to 115200
  Serial.begin(115200);

  setUpWiFi();
  setUpBroker();
  setUpMFRC522();
  setUpServo();
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

void moveServo(int startingPos, int endPos) {
  int currentPos = startingPos;
  
  const int stepDelay = 0;

  if (startingPos < endPos) {
    while (currentPos < endPos) {
      myServo.write(++currentPos);
      delay(stepDelay);
    }
  } else {
    while (currentPos > endPos) {
      myServo.write(--currentPos);
      delay(stepDelay);
    }
  }
}

void setUpMFRC522() {
  // Init MFRC522 board
  mfrc522.PCD_Init();
}

void setUpServo() {
  // Tell the servo library which pin it is connected to
  myServo.attach(servoPin);
}

void loop() {
  if (!client.connected()) {
    // Reconnect if the connection to the MQTT broker is lost
    reconnect();
  }
  client.loop(); // Keep the connection alive

  publishRfidCardId();
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

void publishRfidCardId() {
  // Check if there is a new card &, if so, select it
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Save the ID on a String variable & publish it under the topic "tower/cardid"

    String idString = "";
    
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (mfrc522.uid.uidByte[i] < 0x10) {
        idString += "0";
      }

      idString += String(mfrc522.uid.uidByte[i], HEX);
    }

    Serial.print("Card ID: ");
    Serial.println(idString);

    client.publish("tower/cardid", (char*) idString.c_str());

    // Finish reading the current card
    mfrc522.PICC_HaltA();
  }
}
