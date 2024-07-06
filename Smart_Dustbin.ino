// #define BLYNK_TEMPLATE_ID "TMPL6mIc1DGOl"
// #define BLYNK_TEMPLATE_NAME "Smart Dustbin"
// #define BLYNK_AUTH_TOKEN "6Ln0QWT_zf906ng9Bzw60mGuWhmRJ8Xq"

// #define BLYNK_PRINT Serial
// #include <ESP8266WiFi.h>
// #include <BlynkSimpleEsp8266.h>

// char auth[] = BLYNK_AUTH_TOKEN;
// char ssid[] = "Shaz";
// char pass[] = "Alohomora";

// BlynkTimer timer;

// #define echoPin D5
// #define trigPin D6

// #include<Servo.h>
// Servo servo;
// long duration;
// int distance; 
// int binLevel=0;

// void SMESensor()
// {
//   int ir=digitalRead(D7);
//   if(ir==HIGH)
//   {    
//     servo.write(90);
//     for(int i=0; i<50; i++)
//     {
//       Blynk.virtualWrite(V2, 90);
//       ultrasonic(); 
//       delay(100);
//     }
//     servo.write(0);
//     Blynk.virtualWrite(V2, 0);
//   }
//   if(ir==LOW)
//   {
    
//     ultrasonic();
//     delay(200);
//   }
// }
// void ultrasonic()
// {
//     digitalWrite(trigPin, LOW);
//     delayMicroseconds(2);
//     digitalWrite(trigPin, HIGH);
//     delayMicroseconds(10);
//     digitalWrite(trigPin, LOW);
//     duration = pulseIn(echoPin, HIGH);
//     distance = duration * 0.034 / 2; //formula to calculate the distance for ultrasonic sensor
//     binLevel=map(distance, 21, 0, 0,100); // ADJUST BIN HEIGHT HERE
//     Blynk.virtualWrite(V0, distance);
//     Blynk.virtualWrite(V1, binLevel);
// }
// void setup()
// {
//   Serial.begin(9600);
//   servo.attach(D2);
//   pinMode(D7, INPUT);
//   pinMode(trigPin, OUTPUT); 
//   pinMode(echoPin, INPUT); 
//   Blynk.begin(auth, ssid, pass);
//   delay(2000);
//   timer.setInterval(1000L, SMESensor);
// }

// void loop() 
// {
//   Blynk.run();
//   timer.run();
// }

#define BLYNK_TEMPLATE_ID "TMPL6mIc1DGOl"
#define BLYNK_TEMPLATE_NAME "Smart Dustbin"
#define BLYNK_AUTH_TOKEN "6Ln0QWT_zf906ng9Bzw60mGuWhmRJ8Xq"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include "FirebaseESP8266.h"

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Shaz";
char pass[] = "Alohomora";

BlynkTimer timer;

#define FIREBASE_HOST "https://smart-dustbin-e6194-default-rtdb.firebaseio.com/" //firebase ID
#define FIREBASE_AUTH "NCKluQCvdxAmF79ONprKBsbEyrIstV1R7mxUAWkt" //secret key

//dry bin
#define echoPinDry D5
#define trigPinDry D6
#define LED_dry D0

//wet bin
#define echoPinWet D8
#define trigPinWet D4
#define LED_wet D3 

#define buzz D1  //buzzer
#define irPin D7  //IR


Servo servo;
long durationDry;
int distanceDry; 
int binLevelDry = 0;

long durationWet;
int distanceWet; 
int binLevelWet = 0;


void SMESensor() {
  int ir = digitalRead(irPin);
  if (ir == HIGH) {   
    servo.write(90);
    for (int i = 0; i < 50; i++) {
      Serial.println("Bin Open");
      Blynk.virtualWrite(V2, 90); //V2 servo in Blynk
      ultrasonicDry();
      ultrasonicWet(); 
      delay(100);
    }
    servo.write(0);
    Blynk.virtualWrite(V2, 0);
  } else {
    ultrasonicDry();
    ultrasonicWet();
    delay(200);
  }
  lightcontrol();
  checkBinLevelDry();
  checkBinLevelWet();
}


//Check Dry Bin Level
void ultrasonicDry() {
  digitalWrite(trigPinDry, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinDry, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinDry, LOW);
  durationDry = pulseIn(echoPinDry, HIGH);
  distanceDry = durationDry * 0.034 / 2; // Formula to calculate the distance for Dry ultrasonic sensor
  binLevelDry = map(distanceDry, 21, 1, 0, 100); 
  Blynk.virtualWrite(V0, distanceDry);//V0 distance in Blynk
  Blynk.virtualWrite(V3, binLevelDry);//V3 dry bin level in Blynk

  Serial.print("Distance Dry Bin: ");
  Serial.print(distanceDry);
  Serial.println(" cm");

   // Uploading the dry bin level to Firebase
  // Firebase.setInt("Dry_Bin_Level", binLevel);
  // if (Firebase.failed()) {
  //   Serial.print("setting /Dry_Bin_Level failed:");
  //   Serial.println(Firebase.error());
  
}


//Check Wet Bin Level
void ultrasonicWet() {
  digitalWrite(trigPinWet, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinWet, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinWet, LOW);
  durationWet = pulseIn(echoPinWet, HIGH);
  distanceWet = durationWet * 0.034 / 2; // Formula to calculate the distance for Wet ultrasonic sensor
  binLevelWet = map(distanceWet, 21, 1, 0, 100); 
  Blynk.virtualWrite(V0, distanceWet);//V0 distance in Blynk
  Blynk.virtualWrite(V1, binLevelWet);//V1 wet bin level in Blynk

  Serial.print("Distance Wet Bin: ");
  Serial.print(distanceWet);
  Serial.println(" cm");

  // // Uploading the wet bin level to Firebase
  // Firebase.setInt("Wet_Bin_Level", binLevelWet);
  // if (Firebase.failed()) {
  //   Serial.print("setting /Wet_Bin_Level failed:");
  //   Serial.println(Firebase.error());
  // }

}

//dry bin level
void checkBinLevelDry() {
  if (distanceDry <= 5) { // Check if the Dry distance is less than or equal to 20 cm
    Serial.println("Dry Bin is full or near full!");
    Blynk.logEvent("dry_bin_full", "The dry dustbin is full or near full!"); // Send Blynk notification Dry
    digitalWrite(LED_dry, HIGH);
    digitalWrite(buzz, HIGH);
  }
}

//wet bin level
void checkBinLevelWet() {
  if (distanceWet <= 5) { // Check if the Dry distance is less than or equal to 20 cm
    Serial.println("Wet Bin is full or near full!");
    Blynk.logEvent("wet_bin_full", "The wet dustbin is full or near full!"); // Send Blynk notification Dry
    digitalWrite(LED_wet, HIGH);
    digitalWrite(buzz, HIGH);
  }
}

void setup() {
  Serial.begin(9600);

  servo.attach(D2);//servo attach
  pinMode(irPin, INPUT);//IR

  pinMode(trigPinDry, OUTPUT); //dry
  pinMode(echoPinDry, INPUT);
  pinMode(LED_dry, OUTPUT);

  pinMode(trigPinWet, OUTPUT); //wet 
  pinMode(echoPinWet, INPUT);
  pinMode(LED_wet, OUTPUT);

  pinMode(buzz, OUTPUT);//buzzer
  
  Blynk.begin(auth, ssid, pass);
  delay(2000);
  timer.setInterval(1000L, SMESensor);
}

void lightcontrol(){
 int buz= digitalRead(buzz);
 if (buz==LOW){
  digitalWrite(LED_dry,LOW);
 }else {
  digitalWrite(LED_dry,HIGH);
 }

}

void loop() {

  Blynk.run();
  timer.run();


}
