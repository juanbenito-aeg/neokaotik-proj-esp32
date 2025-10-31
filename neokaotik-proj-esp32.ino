#include <WiFi.h>

// Define the Wi-Fi network credentials
const char *ssid = ""; // Name
const char *password = ""; // Password

void setup() {
  // Set software serial baud to 115200
  Serial.begin(115200);

  setUpWiFi();
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

void loop() {}
