#define BLYNK_TEMPLATE_ID "TMPL6mIc1DGOl"
#define BLYNK_TEMPLATE_NAME "Smart Dustbin"
#define BLYNK_AUTH_TOKEN "6Ln0QWT_zf906ng9Bzw60mGuWhmRJ8Xq"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Blynk setup
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Shaz";
char pass[] = "Alohomora";

BlynkTimer timer;

// Firebase setup
#define API_KEY "AIzaSyCFt0Y8ONhTGKkwh4p4gz9WA7x4RiJjV_I"
#define USER_EMAIL "shaznasalman10@gmail.com"
#define USER_PASSWORD "shaz123"
#define DATABASE_URL "https://smart-dustbin-e6194-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth authDB;
FirebaseConfig config;
String uid;
String databasePath;
String dryBinPath;
String wetBinPath;
FirebaseJson json;

// NTP Client setup
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 19800; // GMT+5:30
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 60000); // Update time every 60 seconds
unsigned long epochTime;
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 1000; // 1 second

// Sensor and servo setup
#define echoPinDry D5
#define trigPinDry D6
#define LED_dry D0
#define echoPinWet D8
#define trigPinWet D4
#define LED_wet D3
#define buzz D1
#define irPin D7

Servo servo;
long durationDry;
int distanceDry;
int binLevelDry = 0;
long durationWet;
int distanceWet;
int binLevelWet = 0;

void initWiFi() {
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

unsigned long getTime() {
  timeClient.update();
  return timeClient.getEpochTime();
}

String getFormattedTime(unsigned long epochTime) {
  char buffer[20];
  struct tm *timeinfo = gmtime((time_t *)&epochTime);
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  return String(buffer);
}

void setup() {
  Serial.begin(9600);
  servo.attach(D2);
  pinMode(irPin, INPUT);
  pinMode(trigPinDry, OUTPUT);
  pinMode(echoPinDry, INPUT);
  pinMode(LED_dry, OUTPUT);
  pinMode(trigPinWet, OUTPUT);
  pinMode(echoPinWet, INPUT);
  pinMode(LED_wet, OUTPUT);
  pinMode(buzz, OUTPUT);

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, SMESensor);

  initWiFi();
  timeClient.begin();

  config.api_key = API_KEY;
  authDB.user.email = USER_EMAIL;
  authDB.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  config.token_status_callback = tokenStatusCallback;
  config.max_token_generation_retry = 5;
  Firebase.begin(&config, &authDB);

  Serial.println("Getting User UID");
  while ((authDB.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  uid = authDB.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  databasePath = "/UsersData/" + uid + "/readings";
  dryBinPath = databasePath + "/DryBin";
  wetBinPath = databasePath + "/WetBin";
}

void ultrasonicDry() {
  digitalWrite(trigPinDry, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinDry, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinDry, LOW);
  durationDry = pulseIn(echoPinDry, HIGH);
  distanceDry = durationDry * 0.034 / 2;
  binLevelDry = map(distanceDry, 21, 1, 0, 100);
  Blynk.virtualWrite(V0, distanceDry);
  Blynk.virtualWrite(V3, binLevelDry);
  Serial.print("Distance Dry Bin: ");
  Serial.print(distanceDry);
  Serial.println(" cm");
}

void ultrasonicWet() {
  digitalWrite(trigPinWet, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinWet, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinWet, LOW);
  durationWet = pulseIn(echoPinWet, HIGH);
  distanceWet = durationWet * 0.034 / 2;
  binLevelWet = map(distanceWet, 21, 1, 0, 100);
  Blynk.virtualWrite(V1, distanceWet);
  Blynk.virtualWrite(V4, binLevelWet);
  Serial.print("Distance Wet Bin: ");
  Serial.print(distanceWet);
  Serial.println(" cm");
}

void SMESensor() {
  int ir = digitalRead(irPin);
  if (ir == LOW) {
    servo.write(h90);
    Serial.println("Bin Open");
    for (int i = 0; i < 50; i++) {
    //  Serial.println("Bin Open");
      Blynk.virtualWrite(V2, 90);
      ultrasonicDry();
      ultrasonicWet();
      delay(100);
    }
    servo.write(0);
    Blynk.virtualWrite(V2, 0);
  } else {
    Serial.println("Bin close");
    ultrasonicDry();
    ultrasonicWet();
    delay(200);
  }
  checkBinLevelDry();
  checkBinLevelWet();
  lightcontrol();
  sendToFirebase();
}

void checkBinLevelDry() {
  if (distanceDry <= 5) {
    Serial.println("Dry Bin is full or near full!");
    Blynk.logEvent("dry_bin_full", "The dry dustbin is full or near full!");
    digitalWrite(LED_dry, HIGH);
    digitalWrite(buzz, HIGH);
  }
}

void checkBinLevelWet() {
  if (distanceWet <= 5) {
    Serial.println("Wet Bin is full or near full!");
    Blynk.logEvent("wet_bin_full", "The wet dustbin is full or near full!");
    digitalWrite(LED_wet, HIGH);
    digitalWrite(buzz, HIGH);
  }
}

void lightcontrol() {
  int buz = digitalRead(buzz);
  if (buz == LOW) {
    digitalWrite(LED_dry, LOW);
    digitalWrite(LED_wet, LOW);
  } else {
    digitalWrite(LED_dry, HIGH);
    digitalWrite(LED_wet, HIGH);
  }
}

void sendToFirebase() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    epochTime = getTime();
    String formattedTime = getFormattedTime(epochTime);

    json.set("/timestamp", formattedTime);
    json.set("/level", binLevelDry);
    Firebase.RTDB.setJSON(&fbdo, dryBinPath.c_str(), &json);

    json.set("/timestamp", formattedTime);
    json.set("/level", binLevelWet);
    Firebase.RTDB.setJSON(&fbdo, wetBinPath.c_str(), &json);

    Serial.printf("Dry Bin Level: %d, Wet Bin Level: %d\n", binLevelDry, binLevelWet);
  }
}

void loop() {
  Blynk.run();
  timer.run();
}

