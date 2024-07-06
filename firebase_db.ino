#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Shaz"
#define WIFI_PASSWORD "Alohomora"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCFt0Y8ONhTGKkwh4p4gz9WA7x4RiJjV_I"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "shaznasalman10@gmail.com"
#define USER_PASSWORD "shaz123"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://smart-dustbin-e6194-default-rtdb.firebaseio.com/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String messagePath = "/message";
String timePath = "/timestamp";

// Parent Node (to be updated in every loop)
String parentPath;

FirebaseJson json;

// Define NTP Client to get time
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 19800; // GMT+5:30
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 60000); // Update time every 60 seconds

// Variable to save current epoch time
unsigned long epochTime;

// Timer variables (send new readings every second)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 1000; // 1 second

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Function to get current epoch time
unsigned long getTime() {
  timeClient.update();
  return timeClient.getEpochTime();
}

// Function to convert epoch time to human-readable date and time
String getFormattedTime(unsigned long epochTime) {
  char buffer[20];
  struct tm *timeinfo = gmtime((time_t *)&epochTime);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  return String(buffer);
}

void setup() {
  Serial.begin(115200);

  initWiFi();
  timeClient.begin();

  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void loop() {
  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Get current epoch time
    epochTime = getTime();
    Serial.print("Epoch time: ");
    Serial.println(epochTime);

    // Convert epoch time to human-readable format
    String formattedTime = getFormattedTime(epochTime);
    Serial.print("Formatted time: ");
    Serial.println(formattedTime);
    String message = "Someone has arrived";

    parentPath = databasePath + "/" + String(epochTime);

    json.set(messagePath.c_str(), message);
    json.set(timePath.c_str(), formattedTime);
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }
}
