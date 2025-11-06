#include <WiFi.h>
#include <WiFiClientSecure.h>
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
const char* mqtt_server = "mqtts:10.50.0.50";
const int mqtt_port = 8883;

// Create a partially initialized broker client instance
WiFiClientSecure espClient;
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
  setUpSSL();
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

void setUpSSL() {
  // Define private key & certificates
  const char* esp32PrivateKey = "-----BEGIN PRIVATE KEY-----\nMIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQC3n9nFrK2GVDLL\nbW5F1UiLjffEQw3qTAsIoeUDCSsFyTJIf6UIYZY9dI8/7psPWPHT1uueuSqVCt7c\nlRSO7WIPHbf4ufMoaxmBYuGjKqjHTE0ndoz8/zBYpXRziuH9lUnR7l4j+r3I4w1K\nw9cUtYLCTWFgyGDR5R1qB5mKznKBX9eE8PPa9L7Y/Hvr2nPDtxC6AQo1cBEzLR1d\nLU5jau4fckAxjuE9McwygQMtwvFuLSDAP9k+wcfYb2pht7MxmgX4RkYdZUbhdarl\nZ85hccseY81RXrujx6xKPkFuREm7BRfVSdk/xFTo7pnMN+sy+wcN61mxIz1Tm+1u\nW+bZAnl9AgMBAAECggEACRVlRnwWUXDU2m9dZyL35b9uJE1U4ORGH3UyyQWpFS4w\nyJlX5qQ9NGkRtQ3uqcGypNIa8c0jKCBTigHt9GwVz72enjVErBkwN9BWbSXtF37U\nykD5FILjN19n9VdHeq3p6vKpwYnj3Y4t2Cb7+tBH2ajOSmVfNMLqw5YAqHcx/uhT\nOwVfXRFCNyliMokpXjXnqxuPpoDAW+Oz1FwS3quN+aRmmse+NGq4KyHOtek6Xbeh\n3cPVcewKDzNn8S+yZ0xdRZjShdUCTinCksCIok9ix0ffYHaFiDFSYwM/t/2UUfpv\nG8Xmza4rwfYoBGYhTGNRebPT7Gq8EucdC+w8ALgiwQKBgQD0zntelCB7Bqq75Pi6\nl9Ql+D4WYNg/6mHvE0UixkWLL/jiJSdutdfmxTQc7FPrbBXbD4Hnfv3nmKX35eM7\n+habP8QqiRJSq3gxU4wRJwQulSohgk3qFvyzkwUIXdDvAK/jkqo01RNMqe3+xaXd\nGmqDgOpFjwiXDLaaST6PqJ4+uQKBgQDABTelMU1LNgDIuCBtkumPPLCSsz0ZaA70\nKqUctb/JzzotUEkMLeQxt6+dyguqVfNMNoeuBIETAPld6AngojYlPF64+yvvYWo6\nhDBvxvtYGdXPKHXkceC4QUfaNcbflF7o6aF96mCxD1CtJLkWRGtpaYaOgen4HKKu\n+g6owxhO5QKBgGo7G8/76WhZiUdF4iVec0ISyiZ92flKZTsQmqSvc0gmBhNkWgsx\nv8G1e5q9duEnIwjPXuis89NHK2Y6jPPe32Z5Mk/e8WOMYvjMr3BWBPuN4R1vZQ4a\n8YefWcS6qUq9epX/c/q/J8PRzaQL8qDSsLvl+kM7AVhCvQ6QkCRY4GqJAoGAOEjP\nOMKsz2Dy9eLSzouDnpLBhx62yB2yVGuznijAIN13RhUEdGd3PgODnJK+vJ0ZBbvu\nJVn4o6NjdN0xGAE0MdHySfKFvFMnmiNpNzpyLi7UdzZ66A1hO2EXOKUOuDZ7Y3Yn\n+gqbcQr2MOpUvmwrt+hQorD97dYm8XcmmN0m4OkCgYBVDWuR2tvp0qcR/R3W/TwO\nkZApa9b41MpDZro1dXecDlEYlJsU9cARCFZ9Kp1JchYWFxHWHDwnNSSCXkrsRO6Y\nh5unz2QaGx+gFWyFqmkZEXqlyZkYCl6iH7VpbsZfQw8mtditCQ7TvoWmIE9oJ74w\n70uE4ZHAOZbpPywInnb16g==\n-----END PRIVATE KEY-----\n";
  const char* esp32Cert = "-----BEGIN CERTIFICATE-----\nMIIEITCCAgkCFF/VdwR15wBe8Ee8sNEDaK6wIQo9MA0GCSqGSIb3DQEBCwUAMIGJ\nMQswCQYDVQQGEwJFVTERMA8GA1UECAwIR0lQVVpLT0ExETAPBgNVBAcMCERPTk9T\nVElBMQwwCgYDVQQKDANBRUcxEDAOBgNVBAsMB0tBT1RJS0ExEDAOBgNVBAMMB0tB\nT1RJS0ExIjAgBgkqhkiG9w0BCQEWE29za2FyLmNhbHZvQGFlZy5ldXMwHhcNMjUx\nMTA1MTIwNDE3WhcNMjYxMTA1MTIwNDE3WjAQMQ4wDAYDVQQDDAVFU1AzMjCCASIw\nDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALef2cWsrYZUMsttbkXVSIuN98RD\nDepMCwih5QMJKwXJMkh/pQhhlj10jz/umw9Y8dPW6565KpUK3tyVFI7tYg8dt/i5\n8yhrGYFi4aMqqMdMTSd2jPz/MFildHOK4f2VSdHuXiP6vcjjDUrD1xS1gsJNYWDI\nYNHlHWoHmYrOcoFf14Tw89r0vtj8e+vac8O3ELoBCjVwETMtHV0tTmNq7h9yQDGO\n4T0xzDKBAy3C8W4tIMA/2T7Bx9hvamG3szGaBfhGRh1lRuF1quVnzmFxyx5jzVFe\nu6PHrEo+QW5ESbsFF9VJ2T/EVOjumcw36zL7Bw3rWbEjPVOb7W5b5tkCeX0CAwEA\nATANBgkqhkiG9w0BAQsFAAOCAgEAMD7XYzrJUfX7pMcuxwmPRHmfIT47x8vhLqXc\nsJHEn+Zh+jprEfCaA9+U5GqPwR5YIOpvQA8j+yMtU5EH6mos15/GpyTiJvNEx3IC\nFmPPFn3ywjR9/+GPkg5iz/VapmWQD6HBY3zWLHPZNdlfEGe9oNTTudVfHJJPe78n\n2yjw0xJWkBou3cLfnCCeiBuoVD1XoAQk2qbrgW/Wzfnwy1lz8sP/giE5i8Qf6osx\nbs5xjgHnId4QkOfYA4VNTEyc60o+Qg2fXldLA40CKyUBhNfQXBCDENr2RBg4zQcG\ndSKePm6HEC454Vl+OW9eD34DXQ3YzyCtI0CK066BWXTuVHEDAEgyl50urO7qhl8I\nLvsRC6RMgeXDtXiJjiMW7gkRiC3+ZF6jrn3YGRThsQF7mIaFeYQK9C48OkUzTx43\nIyvOxkFsMO81dBFqxwWxAUvs9yLhIw7Hf7gCElqVi2nWmdJVSMz/V6pMI/eKHE8Z\nudOift7INSfyjh5aGSd7L9mXC8aaiauLnsHQ2Y64tJ1RrTW2bXoKN2YOLtkSctvp\n08pRTYMJGbqL5UUvj74bH27z054i9iXIxw7/6I4eznwxFixG80H5ibkOZPMv703p\nEAIcd81ii6J2FY2v9RDt47R9qvkPshCJyewZH0cSAQWRDkEsIlR+zZSbLmSBl2qd\n5Je/ZKM=\n-----END CERTIFICATE-----\n";
  const char* caCert = "-----BEGIN CERTIFICATE-----\n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\n-----END CERTIFICATE-----\n";

  // Attach private key & certificates to the client
  espClient.setPrivateKey(esp32PrivateKey);
  espClient.setCertificate(esp32Cert);
  espClient.setCACert(caCert);
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
