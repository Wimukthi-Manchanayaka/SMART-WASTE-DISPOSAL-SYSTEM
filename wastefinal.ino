#include <Servo.h>

Servo servo1;
const int trigPin = 5;//ultrasonic
const int echoPin = 3;
long duration;
int distance = 0;
int potPin = A0; // Input pin for the soil moisture sensor
int soil = 0;
int fsoil = 0;
int servoPos = 90; // Initial servo position

void setup() 
{
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
  servo1.attach(8);
  servo1.write(servoPos); // Set servo to neutral position at startup
}

void loop() 
{
  distance = 0; // Reset distance value at the start of each loop

  // Measure distance using the ultrasonic sensor
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(7);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    delayMicroseconds(10);
    duration = pulseIn(echoPin, HIGH);
    distance += duration * 0.034 / 2;
    delay(10);
  }
  distance = distance / 2;
  Serial.print("Distance: ");
  Serial.println(distance);

  if (distance < 20 && distance > 1)
  {
    delay(1000);
    soil = 0; // Reset soil value at the start of each reading
    fsoil = 0; // Reset fsoil value at the start of each reading

    // Read and average soil moisture values
    for (int i = 0; i < 3; i++)
    {
      soil = analogRead(potPin);
      soil = constrain(soil, 485, 1023);
      fsoil += map(soil, 485, 1023, 100, 0);
      delay(75);
    }
    fsoil = fsoil / 3;
    Serial.print("Moisture: ");
    Serial.print(fsoil);
    Serial.println("%");

    if (fsoil > 50)
    {
      delay(1000);
      Serial.println("WET");
      rotateServo(180); // Move servo to 180 degrees slowly
      delay(3000);
    } 
    else
    {
      delay(1000);
      Serial.println("DRY");
      rotateServo(0); // Move servo to 0 degrees slowly
      delay(3000);
    }

    rotateServo(90); // Reset servo to 90 degrees slowly
  }
  
  // Reset distance and fsoil values
  distance = 0;
  fsoil = 0;
  delay(1000);
}

// Function to rotate servo slowly to the desired position
void rotateServo(int targetPos)
{
  int increment = 1; // Degree increments
  int delayTime = 15; // Delay between each increment (controls speed)

  if (targetPos > servoPos)
  {
    for (int newPos = servoPos; newPos <= targetPos; newPos += increment)
    {
      servo1.write(newPos); // Move servo to new position
      delay(delayTime); // Wait for the servo to reach the position
      servoPos = newPos; // Update current servo position
    }
  }
  else if (targetPos < servoPos)
  {
    for (int newPos = servoPos; newPos >= targetPos; newPos -= increment)
    {
      servo1.write(newPos); // Move servo to new position
      delay(delayTime); // Wait for the servo to reach the position
      servoPos = newPos; // Update current servo position
    }
  }
}
