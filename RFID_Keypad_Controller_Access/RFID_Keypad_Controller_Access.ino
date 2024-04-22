//INFORMATION ON PINS
/* RFID
  RFID RC522          Arduino MEGA2560
    SS/SDA              53
    SCK                 52
    MOSI                51
    MISO                50
    GND                 GND
    RST                 5
    3.3V                3.3V

  /*
   KEYPAD
   A - 38
   B - 39
   C - 32
   D - 37
   E - 35
   F - 40
   G - 34
   DP - 36

   D1 - 47
   D2 - 46
   D3 - 45
   D4 - 44
*/


#include <Key.h>
#include <Keypad.h>
#include <SevSeg.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <string.h>

using namespace std;

//Constant Values
#define RST_PIN         52
#define SS_PIN          53

#define BUZZ_PIN        11
#define SERVO_PIN       10

#define A   38
#define B   39
#define C   32
#define D   37
#define E   35
#define F   40
#define G   34
#define DP  36

#define D1    47
#define D2    46
#define D3    45
#define D4    44


const byte ROWS = 4;
const byte COLS = 4;

char keys [ROWS] [COLS] =
{ {'1', '2', '3', 'A' },
  {'4', '5', '6', 'B' },
  {'7', '8', '9', 'C' },
  {'*', '0', '#', 'D' },
};

byte rowPins[ROWS] = {29, 27, 25, 23};
byte colPins[COLS] = {28, 26, 24, 22};

//  OBJECTS
Servo myservo;                          //SERVO Object
MFRC522 mfrc522(SS_PIN, RST_PIN);       //RFID Object
Keypad myKeypad = Keypad                //KEYPAD Object
                  (makeKeymap(keys) ,
                   rowPins, colPins,
                   ROWS, COLS);
SevSeg sevseg;


char setPassword[5] = "1AD*";
char keyNum[5];

String addKey;

int openSeconds = 5000;



char key_pressed = 0; // Variable to store incoming keys

String tagUID = "33 65 F0 15";


boolean RFIDMode = true; // boolean to change modes

void setup() {
  Serial.begin(9600);



  byte NumberOfDisplays = 4;

  byte NumberOfSharedCommon[] = {D4, D3, D2, D1}; // digitpins - D1, D2, D3, D4

  byte segmentPins[] = {A, B, C, D, E, F, G, DP} ;

  bool resistorOnSegments = 0;

  bool disableDecPoint = true;

  bool updateWithDelays = false;

  bool leadingZeros = false;


  byte hardwareConfig = COMMON_CATHODE;

  sevseg.begin(hardwareConfig, NumberOfDisplays, NumberOfSharedCommon, segmentPins,
               resistorOnSegments, updateWithDelays, leadingZeros);

  sevseg.setBrightness(90);

  //Buzzer
  pinMode(BUZZ_PIN, OUTPUT);

  myservo.write(0);
  myservo.attach(SERVO_PIN); // Attached to digital pin 10 PWM pin with ~

}

void loop() {

  //displayNumber();
  //sevseg.refreshDisplay();
  if (RFIDMode == true) {

    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent())
    {
      return;
    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
    {
      return;
    }
    
    //Show UID on serial monitor
    Serial.print("UID tag :");
    String content = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    Serial.print("Message : ");
    content.toUpperCase();
    
    if (content.substring(1) == "33 65 F0 15") //change here the UID of the card/cards that you want to give access
    {
      validBuzz();
      //move to unlocked position
      for (int pos = 0; pos <= 180; pos += 5)
      {
        myservo.write(pos);
      }
      delay(openSeconds);
      Serial.println("Authorized access");
    }

    else   {
      invalidBuzz();
      RFIDMode = false; // Make RFID mode false
      Serial.println(" Access denied");
      delay(3000);
      myservo.write(0);
      delay(500);
    }
  }
  // If RFID mode is false, it will look for keys from keypad
  if (RFIDMode == false) {
    key_pressed = myKeypad.getKey(); // Storing keys
    if (key_pressed)
    {
      char customKey = myKeypad.getKey();
      if (customKey && ( strlen(keyNum) <= 4)) {
        keyPress();
        addKey = String(customKey);
        strcat( keyNum, addKey.c_str() );
        sevseg.setChars(keyNum);
        sevseg.refreshDisplay();
      }
      else {
        checkPassword();
      }
    }
  }
}
//-------------------------------------------------------------------

void displayNumber() {
  char customKey = myKeypad.getKey();
  if ( strlen(keyNum) <= 4) {
    if (customKey) {
      keyPress();
      addKey = String(customKey);
      strcat( keyNum, addKey.c_str() );
      sevseg.setChars(keyNum);
      sevseg.refreshDisplay();
    }
  }
  else {
    checkPassword();
  }

}
//-------------------------------------------------------------------
void keyPress() {
  tone(BUZZ_PIN, 500);
  delay(50);
  noTone(BUZZ_PIN);
}

//-------------------------------------------------------------------
void checkPassword() {
  keyNum[4] = NULL;
  keyNum[5] = NULL;

  if ( strcmp(keyNum, setPassword) == 0 ) {
    RFIDMode = true; // Make RFID mode true

    sevseg.setChars("----");
    sevseg.refreshDisplay();
    //you're in!
    validBuzz();
    Serial.println("You got it right");


    //move to unlocked position
    for (int pos = 0; pos <= 180; pos += 5)
    {
      myservo.write(pos);
      delay(5);
    }
    delay(openSeconds);

    restartDisplay();
  }
  else {
    RFIDMode = true; // Make RFID mode true
    sevseg.setChars("####");
    sevseg.refreshDisplay();
    //wrong password!
    invalidBuzz();

    restartDisplay();
  }
}

//-------------------------------------------------------------------
void restartDisplay() {
  //return to lock position
  myservo.write(0);
  delay(500);

  memset(keyNum, 0, sizeof(keyNum));

  sevseg.refreshDisplay();
}

void validBuzz() {
  tone(BUZZ_PIN, 2000);
  delay(1000);
  noTone(BUZZ_PIN);
}

void invalidBuzz() {
  tone(BUZZ_PIN, 1500);
  delay(1000);
  noTone(BUZZ_PIN);
}
