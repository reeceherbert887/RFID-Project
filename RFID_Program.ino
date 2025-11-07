#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <Servo.h>

#define SS_PIN 10 //SS Pin 
#define RST_PIN 9 //RST Pin
#define BUZZER 4 //Buzzer

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 Instance.
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
Servo servo_5;

int delayTime = 200;

void setup()
{
Serial.begin(9600); // Initiate A Serial Communication
SPI.begin(); // Initiate SPI Bus
mfrc522.PCD_Init(); // Initiate MFRC522

lcd.begin(16, 2);
servo_5.attach(5, 500, 2500);
lcd.setCursor(0,0);
lcd.print("Door Locked");
Serial.println("System Ready");
pinMode(7, OUTPUT); //Red
pinMode(6, OUTPUT); // Green
pinMode(4, OUTPUT); //Buzzer
digitalWrite(7, HIGH); //Red
digitalWrite(6, LOW); //Green
 servo_5.write(0); //Servo Starts At 0deg (Locked)
}
void loop()
{
// Look For New Cards
if ( ! mfrc522.PICC_IsNewCardPresent())
{
return;
}
// Select One Of The Cards
if ( ! mfrc522.PICC_ReadCardSerial())
{
return;
}

// Show UID On Serial Monitor
    Serial.print("UID tag: ");
    String content = "";

for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      content += "0"; // Add Leading Zero If Needed, But No Space
    }
    content += String(mfrc522.uid.uidByte[i], HEX);

    if (i < mfrc522.uid.size - 1) { //Add Space If Not The Last Byte
      content += " ";
    }
    Serial.print("Byte ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.println();
  }
    content.toUpperCase(); // Convert To Uppercase For Proper Matching
    Serial.println(content); // Debugging: Print Scanned UID
    Serial.print("Content: "); //Debugging
    Serial.println(content); //Debugging

    // Known Valid UIDs
    String validCard1 = "12 07 F2 EF";
    String validCard2 = "56 C9 DE A1";

    // Debugging The Comparison
 if (content == validCard1) {
        Serial.println("Valid card 1 detected");
    } else if (content == validCard2) {
        Serial.println("Valid card 2 detected");
    } else {
        Serial.println("Unknown card");
    }

    Serial.print("Message : ");

    if (content == validCard1 || content == validCard2) // Check For Valid UIDs
    {
        lcd.setCursor(0, 0);
        lcd.print("Access Granted");
        Serial.println("Access Granted");
        digitalWrite(7, LOW); //Red
        digitalWrite(6, HIGH); //Green
        
       
        tone(4, 440);
        delay(delayTime);
        tone(4, 494);
        delay(delayTime);
        tone(4, 523);
        delay(delayTime);
        noTone(4);

        // Move Servo To Unlock (180 Degrees)
        Serial.println("Door Unlocked");
      for (int pos = 0; pos <= 180; pos += 1) {
        servo_5.write(pos);
        delay(15);
      }

      delay(5000);  // Give Time For The User To Read
 
  // Move Servo Back To Lock (0 Degrees)
  Serial.println("Door Locked");
      for (int pos = 180; pos >= 0; pos -= 1) {
        servo_5.write(pos);
        delay(15);
      }
        
    }

    else
    {
      Serial.println("Buzzer: Incorrect Card");
        lcd.setCursor(0,0);
      lcd.print("Incorrect Card");
      Serial.println("Incorrect Card");
      digitalWrite(7, HIGH); //Red 
      digitalWrite(6, LOW); //Green
      tone(4, 1000, 500); //Buzzer 

      delay(2000);
    }
    
    
    lcd.clear();  // Now Alear After A Delay
    lcd.setCursor(0,0);
    lcd.print("Door Locked");
    Serial.println("Door Locked");
    digitalWrite(7, HIGH); //Red
    digitalWrite(6, LOW); //Green
}