unsigned char *myTCCR1A = (unsigned char *)0x80;  //found on page 158 of DataSheet
unsigned char *myTCCR1B = (unsigned char *)0x81;  //found on page 160 of DataSheet
unsigned char *myTCCR1C = (unsigned char *)0x82;  //found on page 161 of DataSheet
unsigned char *myTIMSK1 = (unsigned char *)0x6F;  //found on page 166 of DataSheet
unsigned char *myTCNT1 = (unsigned char *)0x85;   //found on page 162 of DataSheet
unsigned char *myTIFR1 = (unsigned char *)0x36;   //found on page 167 of DataSheet
unsigned char *portDDRB = (unsigned char *)0x24;  //found on page 100 of DataSheet
unsigned char *portB = (unsigned char *)0x25;     //found on page 100 of DataSheet
unsigned char* pin_b = (unsigned char*) 0x23;

//Sets the frequency values
#define freqA         440
#define freqB         494
#define freqCMiddle   523       // C is the pivot
#define freqD         587
#define freqE         659
#define freqF         698
#define freqG         784


#define digitalPinNumber     13 //to be used with pre-existing arduino functions
#define pinNumber     7         // to be used with native registers
#define myTime        500      // 1000 millisecond delay   

void my_delay(unsigned int ticks);

int duration(500); // sets the duration of note being played

void setup() {

  Serial.begin(9600);   //Sets the baud rate of Arduino to 9600 bits per second

  set_PB_as_output(7);
  write_pb(7, 0);

  *myTCCR1A = 0x00;
  *myTCCR1B = 0x00;
  *myTCCR1C = 0x00;

  bitClear(TCCR1B, WGM13);         //Set CTC mode
  bitSet(TCCR1B, WGM12);



  //Reset Timer1
  *myTCNT1 = 0;

  Serial.println("Press the keys below to generate sounds from your buzzer using frequencies!");
  Serial.println("Keys: a-b-c-d-e-f-g");

}

void loop() {
  unsigned char inputChar;


  if (Serial.available() > 0) {                 // if a character is received
    inputChar = Serial.read();
    inputChar = tolower(inputChar);


    if ((int)inputChar != 10) {                 // if not line feed, arduino sends line feed in loop

      Serial.write("You have pressed the key: ");
      Serial.write(inputChar);
      Serial.write('\n');
      Serial.write((int)sizeof(inputChar));

      switch (inputChar) {
        case 'a':
          tone(digitalPinNumber, freqA, myTime);
          break;
        case 'b':
          tone(digitalPinNumber, freqB, myTime);
          break;
        case 'c':
          tone(digitalPinNumber, freqCMiddle, myTime);
          break;
        case 'd':
          tone(digitalPinNumber, freqD, myTime);
          break;
        case 'e':
          tone(digitalPinNumber, freqE, myTime);
          break;
        case 'f':
          tone(digitalPinNumber, freqF, myTime);
          break;
        case 'g':
          tone(digitalPinNumber, freqG, myTime);
          break;
        default:
          Serial.println("You did not select a valid input, please try again.");
          break;
      }
    }
  }
}

void set_PB_as_output(unsigned char pin_num) {
  *portDDRB |= 0x01 << pin_num;
}

void write_pb(unsigned char pin_num, unsigned char state) {

  if (state == 0) { //pin is low
    *portB &= ~(0x01 << pin_num);
    //this sets whatever pin is in the pin_num to 0(low) and keeps everything else the same.
  } else { //pin is high
    *portB |= (0x01 << pin_num); //sets it high
  }
}

void myTone(byte pin, uint16_t frequency, uint16_t duration) { // input parameters: Arduino pin number, frequency in Hz, duration in milliseconds

  unsigned long startTime = millis();
  //pinMode(13, OUTPUT);
  unsigned long halfPeriod = 1000000L / frequency / 2;

  while (millis() - startTime < duration)
  {
    write_pb(7, 1);
    delayMicroseconds(halfPeriod);
    write_pb(7, 0);
    delayMicroseconds(halfPeriod);

  }
  //pinMode(13, INPUT);
}

void trial(byte pin, uint16_t frequency, uint16_t duration)
{ // input parameters: Arduino pin number, frequency in Hz, duration in milliseconds

  unsigned long startTime = millis();

  unsigned long halfPeriod = 1000000L / frequency / 2;

  //pinMode(pin,OUTPUT);

  while (millis() - startTime < duration)
  {
    digitalWrite(pin, HIGH);
    my_delay(halfPeriod);
    digitalWrite(pin, LOW);
    my_delay(halfPeriod);
  }
  //pinMode(pin,INPUT);
}

void my_delay(unsigned int freq) {
  double period = 1.0 / double(freq);                   // tau of the freqency
  double half_period = period / 2.0f;                   // 50% duty cycle
  double clk_period = 0.0000000625;                     // tau of the clock
  unsigned int ticks = half_period / clk_period;        // calculate the  ticks

  //Set Timer1 mode 1

  *myTCCR1B &= ~(0b111 << CS10);                     // stop the timer

  *myTCNT1 = (unsigned int) (65536 - ticks);            // set the counts

  *myTCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);      // start the timer with prescaler of 1024


  while ((*myTIFR1 & 0x01) == 0);                       // wait for overflow

  *myTCCR1B &= ~(0b111 << CS10);                     // stop the timer

  *myTIFR1 |= (1 << OCF1A);;                        // reset TOV
}
