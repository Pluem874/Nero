#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>
#include "addons/TokenHelper.h" // Provide the token generation process info.
#include "addons/RTDBHelper.h" // Provide the RTDB payload printing info and other helper functions.

// Replace with your network credentials
#define WIFI_SSID "Galaxy_A325243"
#define WIFI_PASSWORD "12345678"

// Replace with your Firebase project credentials
#define API_KEY "AIzaSyBt-GMnQLX-fk4kUoBTgCer5jTJ2vXBOXE"
#define DATABASE_URL "https://nero-pipes-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(9600); // Initialize Serial communication with Arduino

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Assign the API key and database URL
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up (anonymous sign-in)
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase sign-up successful");
    signupOK = true;
  } else {
    Serial.printf("Firebase sign-up failed: %s\n", config.signer.signupError.message.c_str());
  }

  // Assign the callback function for token generation
  config.token_status_callback = tokenStatusCallback;

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n'); // Read data from Arduino
    Serial.println("Received: " + data); // Print received data

    // Parse JSON data
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
      Serial.println("Failed to parse JSON");
      return;
    }

    // Extract TDS value
    float tdsValue = doc["TDS"];
    Serial.print("TDS Value: ");
    Serial.println(tdsValue);

    // Send data to Firebase
    if (Firebase.ready() && signupOK) {
      if (Firebase.RTDB.setFloat(&fbdo, "/tdsValue", tdsValue)) {
        Serial.println("Data sent to Firebase");
      } else {
        Serial.println("Failed to send data");
        Serial.println("Reason: " + fbdo.errorReason());
      }
    }
  }
}