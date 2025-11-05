#include <WiFi.h>
#include <PubSubClient.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

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

String lastReadCardId = "";

// Define the servo and the pin it is connected to
Servo myServo;
const int servoPin = 13;

// Define the pin Buzzer
const int buzzer = 22;

// Colors LEDs
const int LED_RED = 14;
const int LED_GREEN = 26;
const int LED_BLUE = 27;

void setup() {
  // Set software serial baud to 115200
  Serial.begin(115200);

  setUpWiFi();
  setUpBroker();
  setUpMFRC522();
  setUpServo();
  setUpBuzzer();
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
  
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (topicString == "tower/access") {
    listenTowerAccess(message);
  } else if (topicString == "tower/door") {
    listenTowerDoor(message);
  }
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

void setUpBuzzer() {
  pinMode(buzzer, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    // Reconnect if the connection to the MQTT broker is lost
    reconnect();
  }
  client.loop(); // Keep the connection alive

  publishRfidCardId();
}

void lightLEDs(int redValue, int greenValue, int blueValue) {
  analogWrite(LED_RED, redValue);
  analogWrite(LED_GREEN, greenValue);
  analogWrite(LED_BLUE, blueValue);
}

void offLEDs() {
  pinMode(LED_RED, LOW);
  pinMode(LED_GREEN, LOW);
  pinMode(LED_BLUE, LOW);
}

void listenToCardAccessAndWhistle(bool isAuthorized) {  
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);

  if (!isAuthorized) {
    delay(200);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
  }
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
  client.subscribe("tower/access");
  client.subscribe("tower/door");
}

void listenTowerAccess(String message) {
  StaticJsonDocument<200> doc;
  doc["isDoorOpen"] = true;
  doc["cardId"] = lastReadCardId.c_str();

  if (message == "authorized") {
    lightLEDs(0, 255, 0);
    listenToCardAccessAndWhistle(true);
    offLEDs();
    moveServo(0, 95); 

    char jsonOutput[256]; 
    serializeJson(doc, jsonOutput); 
    client.publish("tower/door", jsonOutput);
  } else {
    lightLEDs(0, 255, 255);
    listenToCardAccessAndWhistle(false);
    offLEDs();
  }
}

void listenTowerDoor(String message) {
  JsonDocument messageAsJson;
  deserializeJson(messageAsJson, message);

  const bool isDoorOpen = messageAsJson["isDoorOpen"];
  
  if (!isDoorOpen) {
    moveServo(95, 0);
  }
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

    lastReadCardId = (char*) idString.c_str();

    client.publish("tower/cardid", lastReadCardId.c_str());

    // Finish reading the current card
    mfrc522.PICC_HaltA();
  }
}
