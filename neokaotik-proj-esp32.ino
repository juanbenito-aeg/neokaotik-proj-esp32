#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

// Define the Wi-Fi network credentials
const char* ssid = "IKASLE-LAB"; // Name
const char* password = "Wb_IKAS_LAB"; // Password

// Define the MQTT broker parameters
const char* mqtt_server = "10.50.0.50";
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
const int LED = 25;
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
  setUpLED();
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
  const char* esp32PrivateKey = \
    "-----BEGIN PRIVATE KEY-----\n" \
    "MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQC3n9nFrK2GVDLL\n" \
    "bW5F1UiLjffEQw3qTAsIoeUDCSsFyTJIf6UIYZY9dI8/7psPWPHT1uueuSqVCt7c\n" \
    "lRSO7WIPHbf4ufMoaxmBYuGjKqjHTE0ndoz8/zBYpXRziuH9lUnR7l4j+r3I4w1K\n" \
    "w9cUtYLCTWFgyGDR5R1qB5mKznKBX9eE8PPa9L7Y/Hvr2nPDtxC6AQo1cBEzLR1d\n" \
    "LU5jau4fckAxjuE9McwygQMtwvFuLSDAP9k+wcfYb2pht7MxmgX4RkYdZUbhdarl\n" \
    "Z85hccseY81RXrujx6xKPkFuREm7BRfVSdk/xFTo7pnMN+sy+wcN61mxIz1Tm+1u\n" \
    "W+bZAnl9AgMBAAECggEACRVlRnwWUXDU2m9dZyL35b9uJE1U4ORGH3UyyQWpFS4w\n" \
    "yJlX5qQ9NGkRtQ3uqcGypNIa8c0jKCBTigHt9GwVz72enjVErBkwN9BWbSXtF37U\n" \
    "ykD5FILjN19n9VdHeq3p6vKpwYnj3Y4t2Cb7+tBH2ajOSmVfNMLqw5YAqHcx/uhT\n" \
    "OwVfXRFCNyliMokpXjXnqxuPpoDAW+Oz1FwS3quN+aRmmse+NGq4KyHOtek6Xbeh\n" \
    "3cPVcewKDzNn8S+yZ0xdRZjShdUCTinCksCIok9ix0ffYHaFiDFSYwM/t/2UUfpv\n" \
    "G8Xmza4rwfYoBGYhTGNRebPT7Gq8EucdC+w8ALgiwQKBgQD0zntelCB7Bqq75Pi6\n" \
    "l9Ql+D4WYNg/6mHvE0UixkWLL/jiJSdutdfmxTQc7FPrbBXbD4Hnfv3nmKX35eM7\n" \
    "+habP8QqiRJSq3gxU4wRJwQulSohgk3qFvyzkwUIXdDvAK/jkqo01RNMqe3+xaXd\n" \
    "GmqDgOpFjwiXDLaaST6PqJ4+uQKBgQDABTelMU1LNgDIuCBtkumPPLCSsz0ZaA70\n" \
    "KqUctb/JzzotUEkMLeQxt6+dyguqVfNMNoeuBIETAPld6AngojYlPF64+yvvYWo6\n" \
    "hDBvxvtYGdXPKHXkceC4QUfaNcbflF7o6aF96mCxD1CtJLkWRGtpaYaOgen4HKKu\n" \
    "+g6owxhO5QKBgGo7G8/76WhZiUdF4iVec0ISyiZ92flKZTsQmqSvc0gmBhNkWgsx\n" \
    "v8G1e5q9duEnIwjPXuis89NHK2Y6jPPe32Z5Mk/e8WOMYvjMr3BWBPuN4R1vZQ4a\n" \
    "8YefWcS6qUq9epX/c/q/J8PRzaQL8qDSsLvl+kM7AVhCvQ6QkCRY4GqJAoGAOEjP\n" \
    "OMKsz2Dy9eLSzouDnpLBhx62yB2yVGuznijAIN13RhUEdGd3PgODnJK+vJ0ZBbvu\n" \
    "JVn4o6NjdN0xGAE0MdHySfKFvFMnmiNpNzpyLi7UdzZ66A1hO2EXOKUOuDZ7Y3Yn\n" \
    "+gqbcQr2MOpUvmwrt+hQorD97dYm8XcmmN0m4OkCgYBVDWuR2tvp0qcR/R3W/TwO\n" \
    "kZApa9b41MpDZro1dXecDlEYlJsU9cARCFZ9Kp1JchYWFxHWHDwnNSSCXkrsRO6Y\n" \
    "h5unz2QaGx+gFWyFqmkZEXqlyZkYCl6iH7VpbsZfQw8mtditCQ7TvoWmIE9oJ74w\n" \
    "70uE4ZHAOZbpPywInnb16g==\n" \
    "-----END PRIVATE KEY-----\n"
  ;
  const char* esp32Cert = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIEzjCCAragAwIBAgIUX9V3BHXnAF7wR7yw0QNorrAhCkkwDQYJKoZIhvcNAQEL\n" \
    "BQAwgYkxCzAJBgNVBAYTAkVVMREwDwYDVQQIDAhHSVBVWktPQTERMA8GA1UEBwwI\n" \
    "RE9OT1NUSUExDDAKBgNVBAoMA0FFRzEQMA4GA1UECwwHS0FPVElLQTEQMA4GA1UE\n" \
    "AwwHS0FPVElLQTEiMCAGCSqGSIb3DQEJARYTb3NrYXIuY2Fsdm9AYWVnLmV1czAe\n" \
    "Fw0yNTExMDcxMDM1NDJaFw0yNjExMDcxMDM1NDJaMGMxCzAJBgNVBAYTAkVTMREw\n" \
    "DwYDVQQIDAhHaXB1emtvYTERMA8GA1UEBwwIRG9ub3N0aWExDDAKBgNVBAoMA0FF\n" \
    "RzEQMA4GA1UECwwHS2FvdGlrYTEOMAwGA1UEAwwFRVNQMzIwggEiMA0GCSqGSIb3\n" \
    "DQEBAQUAA4IBDwAwggEKAoIBAQC3n9nFrK2GVDLLbW5F1UiLjffEQw3qTAsIoeUD\n" \
    "CSsFyTJIf6UIYZY9dI8/7psPWPHT1uueuSqVCt7clRSO7WIPHbf4ufMoaxmBYuGj\n" \
    "KqjHTE0ndoz8/zBYpXRziuH9lUnR7l4j+r3I4w1Kw9cUtYLCTWFgyGDR5R1qB5mK\n" \
    "znKBX9eE8PPa9L7Y/Hvr2nPDtxC6AQo1cBEzLR1dLU5jau4fckAxjuE9McwygQMt\n" \
    "wvFuLSDAP9k+wcfYb2pht7MxmgX4RkYdZUbhdarlZ85hccseY81RXrujx6xKPkFu\n" \
    "REm7BRfVSdk/xFTo7pnMN+sy+wcN61mxIz1Tm+1uW+bZAnl9AgMBAAGjUzBRMA8G\n" \
    "A1UdEQQIMAaHBAoyADIwHQYDVR0OBBYEFDgjcEge2fEkR1qn7IWS7Wqm9JndMB8G\n" \
    "A1UdIwQYMBaAFOKJUsmG6a1cDtG5IgDP/XkV0ol5MA0GCSqGSIb3DQEBCwUAA4IC\n" \
    "AQBmJ/1YtZqDsmkrEdL1wGUFGKgRpIUFyBGMQgb0IvXgunFhAZADI1VlmDS4h6hd\n" \
    "BAqmVmdgPD/LFP7sTucVPAPTD4NU7jRVOv0zCCpxPwm3KX/HQ9BVM4BGvZkt78/p\n" \
    "ageErW8jffFziAVtkqnDc+VNzlR4VzQHmnN/pdHX/k/LFBzxAhd3VzJD+kjqkmXC\n" \
    "fCkRmt0z3Mop2NXSJJGGu0jajCNPnRySN0AhUvr5p1aRjc1U9tNvddEo2RG2jkgJ\n" \
    "6I1visjowrx4sHdQFRxZr/6R9PCXvrLYrFNp0gsR7iiieWolulrm2BO5HoXCQsBy\n" \
    "fBgGsxLThN8VKqySjA5AUArZAeHP9x/QMrjJEbCKDIXuF7tEC4GS1BTcjcJxYFEz\n" \
    "h4g4UAjckzZHMBVkRduBfEB28GWRrbbI1hdpvKH0Y6WFA83oT3IlOQvCT94+qZBf\n" \
    "/ij6hwN/mD1UtJSIicR07UJj6t9qnb1w/ZYfDma7vB4Dr+wOmXwWnFgC/wzXYrqS\n" \
    "ry7sLiUCn4RMKSV0Qsqsg8eLZnqFcKGUykYpjfni6wKPzmun8i2jRaILN2Sn8GMu\n" \
    "sOZ3dtttgcIhx7clqWt7LirsoHo/eCL53oyo8k1Ijfy1xsJBahY2CHZhdImKd3Va\n" \
    "x9d5G8k/v9McSucv+OpRnuG8H5fOEam69Xa799N7IvRhiQ==\n" \
    "-----END CERTIFICATE-----\n"
  ;
  const char* caCert = \
    "-----BEGIN CERTIFICATE-----\n" \
    "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\n" \
    "-----END CERTIFICATE-----\n"
  ;

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

void setUpLED() {
  pinMode(LED, OUTPUT);
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
  defaultLEDs();


  if (!client.connected()) {
    // Reconnect if the connection to the MQTT broker is lost
    reconnect();
  }
  client.loop(); // Keep the connection alive

  publishRfidCardId();
}

void defaultLEDs() {
  lightLEDs(0, 255, 165);
}

void ligthLED(int value) {
  digitalWrite(LED, value);
}

void lightLEDs(int redValue, int greenValue, int blueValue) {
  analogWrite(LED_RED, redValue);
  analogWrite(LED_GREEN, greenValue);
  analogWrite(LED_BLUE, blueValue);
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
    defaultLEDs();
    ligthLED(HIGH);
    moveServo(0, 95); 

    char jsonOutput[256]; 
    serializeJson(doc, jsonOutput); 
    client.publish("tower/door", jsonOutput);
  } else {
    lightLEDs(0, 255, 255);
    listenToCardAccessAndWhistle(false);
  }
}

void listenTowerDoor(String message) {
  JsonDocument messageAsJson;
  deserializeJson(messageAsJson, message);

  const bool isDoorOpen = messageAsJson["isDoorOpen"];
  
  if (!isDoorOpen) {
    moveServo(95, 0);
    ligthLED(LOW);
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
