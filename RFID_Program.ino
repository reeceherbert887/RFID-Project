#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

//RFID
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);
bool systemStarted = false;

//Ultrisonic Set Up
const int trigPin = 8; //Orange
const int echoPin = 7; //Trig 

//LEDs Set Up
const int led1 = A2;  // Urgent
const int led2 = A3;  // High Risk
const int led3 = A4;  // Low Risk
const int led4 = A5; // No Risk

//Buzzer 
const int buzzerPin = 4;

//Servos And Servo Control
Servo leftServo;
Servo rightServo;
const int leftPin = 2;
const int rightPin = 3;
const int leftStop = 94;
const int rightStop = 93;
int forwardLeft = leftStop + 8;
int forwardRight = rightStop - 10;

void setup() {
  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Waiting For Tag");

  //Ultrisonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //LEDs
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  //Buzzer 
  pinMode(buzzerPin, OUTPUT);

  //Servos/Serial Monitor
  Serial.print("Left: ");
  Serial.print(forwardLeft);
  Serial.print(" | Right: ");
  Serial.println(forwardRight);
  leftServo.attach(leftPin);  // Left Wheel
  rightServo.attach(rightPin); // Right Wheel

  stopMoving();  // Keep Wheels Still On Power-Up
}

void loop() {
  if(!systemStarted) {
    if(checkForValidCard()){
      Serial.println("Accsess Granted");
      tone(buzzerPin, 1000, 500);
      delay(1000);
      systemStarted = true;
    } else {
      return;
    }
  }

  float distance = getDistance();
  Serial.print("Distance: ");
  Serial.println(distance);

  if (distance <= 15) {
    stopMoving();
    beep(600, 4);
    delay(200);
    uTurn();
  } else if (distance <= 20) {
    slowForward();
    beep(500, 3);
  } else {
    goForward();
    noTone(buzzerPin);
  }

  // LED Feedback
  updateLEDs(distance);
  delay(100);
}
//RFID Control, Data Handling (For The Correct Cards) 
bool checkForValidCard() {
  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial()) return false;

  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) content += "0";
    content += String(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) content += " ";
  }

  content.toUpperCase();
  Serial.print("Scanned UID: ");
  Serial.println(content);

  return (content == "12 07 F2 EF" || content == "56 C9 DE A1");
}

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if(duration == 0) {
    return 999;
  }

  float distance = duration * 0.034 / 2;
  // Clamp Anything Under 2 cm (Noise) As Invalid
  if (distance < 2) return 999;

  return distance;
}

void goForward() {
  leftServo.write(forwardLeft);   // FS90R - Tweak As Needed
  rightServo.write(forwardRight); // Opposite Direction
}

void slowForward() {
  leftServo.write(forwardLeft - 5);   // Slightly Slower
  rightServo.write(forwardRight + 5);
}

void stopMoving() {
  leftServo.write(leftStop);
  rightServo.write(rightStop);
}

void reverse(int speed) {
  leftServo.write(leftStop - speed);   // Move In Reverse Direction
  rightServo.write(rightStop + speed); // Move In Reverse Direction
}

void uTurn() {
  unsigned long startTime = millis();

  // Reverse Until Clear Or 20 Seconds Max
  while (getDistance() <= 15 && millis() - startTime < 20000) {
    reverse(10);
    updateLEDs(getDistance());
    delay(100);
  }

  stopMoving();
  delay(500);

  leftServo.write(forwardLeft);
  rightServo.write(leftStop);

  delay(1800);

  stopMoving();
  delay(300);

  goForward();
}

void beep(int frequency, int count) {
  for (int i = 0; i < count; i++) {
    tone(buzzerPin, frequency);
    delay(100);
    noTone(buzzerPin);
    delay(100);
  }
}

void updateLEDs(float distance) {
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);

  if (distance <= 15) {
    digitalWrite(led4, HIGH);
  } else if (distance <= 30) {
    digitalWrite(led3, HIGH);
  } else if (distance <= 45) {
    digitalWrite(led2, HIGH);
  } else if (distance <= 55) {
    digitalWrite(led1, HIGH);
  }
}
