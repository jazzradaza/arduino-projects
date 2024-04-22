//---------------------------------------------------------------------------------------------------------

#define RDA 0x80
#define TBE 0x20  

// UART Pointers
volatile unsigned char *myUCSR0A  = (unsigned char*)0x00C0;
volatile unsigned char *myUCSR0B  = (unsigned char*)0x00C1;
volatile unsigned char *myUCSR0C  = (unsigned char*)0x00C2;
volatile unsigned int  *myUBRR0   = (unsigned int* )0x00C4;
volatile unsigned char *myUDR0    = (unsigned char*)0x00C6;
// GPIO Pointers
volatile unsigned char *portB     = (unsigned char*) 0x25;
volatile unsigned char *portDDRB  = (unsigned char*) 0x24;
// Timer Pointers
volatile unsigned char *myTCCR1A  = (unsigned char*) 0x80;
volatile unsigned char *myTCCR1B  = (unsigned char*) 0x81;
volatile unsigned char *myTCCR1C  = (unsigned char*) 0x82;
volatile unsigned char *myTIMSK1  = (unsigned char*) 0x6F;
volatile unsigned char *myTIFR1   = (unsigned char*) 0x36;
volatile unsigned int  *myTCNT1   = (unsigned int* ) 0x84;

byte in_char;
//This array holds the tick values
unsigned int ticks[13]= {18181, 17167, 16194, 15296, 14440, 13628, 12820, 12139, 11461, 10810, 10204, 9627};
//This array holds the charachters to be entered, index echos the index of the ticks
unsigned char input[12]= {'a', 'A', 'b', 'c', 'C', 'd', 'D', 'e', 'f', 'F', 'g', 'G'};

unsigned int currentTicks = 65535;
unsigned char timer_running = 0;

//-------------------------------------------------------------------------------------------------------//

void setup() 
{ 
  set_PB_as_output(7);              // set PB6 to output
  write_pb(7, 0);                   // set PB6 LOW
  
  setup_timer_regs();               // setup the Timer for Normal Mode, with the TOV interrupt enabled
  U0Init(9600);                     // Start the UART
}

//-------------------------------------------------------------------------------------------------------//

void loop() 
{
     while (kbhit()==0){};                              // if we recieve a character from serial
     in_char = getChar();                               // read the character           
     putChar(in_char);                                  // echo it back

     if (in_char == 'q' || in_char == 'Q')              // if it's the quit character
     {
        currentTicks = 65535;                           // set the current ticks to the max value
        if(timer_running == 1)                          // if the timer is running
        {
          *myTCCR1B &= 0xF8;                            // stop the timer
          timer_running = 0;                            // set the flag to not running
          write_pb(7, 0);                              // set PB6 LOW
        }
     }
    
    // otherwise we need to look for the char in the array
    else
    {
        for(int i = 0; i < 13; i++)
        {
          if(in_char == input[i])                       // Checks to see what character the input is
          {
            currentTicks = ticks[i];                    // Set the appropiate ticks
          }
        }
        // if the timer is not already running, start it
        if(timer_running != 1)
        {
          *myTCCR1B |=  0x01;                           // start the timer
          timer_running = 1;                            // set the running flag
        }
    }
}
// Set up functions ------------------

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


//-------------------------------------------------------------------------------------------------------//

// Timer setup function
void setup_timer_regs()
{
  // setup the timer control registers
  *myTCCR1A= 0x00;
  *myTCCR1B= 0X00;
  *myTCCR1C= 0x00;
  *myTIFR1 |= 0x01;                                     // reset the TOV flag
  *myTIMSK1 |= 0x01;                                    // enable the TOV interrupt
}
// TIMER OVERFLOW ISR
ISR(TIMER1_OVF_vect)
{
  *myTCCR1B &= 0xF8;                                    // Stop the Timer
  *myTCNT1 =  (unsigned int) (65535 -  (unsigned long) (currentTicks)); // Load the Count
  *myTCCR1B |=  0x01;                                   // Start the Timer
  // if it's not the STOP amount
  if(currentTicks != 65535)
  {
    //*portB ^= 0x40;                                     // XOR to toggle PB6
    *portB ^= 0x80;                                     // XOR to toggle PB6
  }
}

//-------------------------------------------------------------------------------------------------------//
/*
 * UART FUNCTIONS
 */
 
void U0Init(int U0baud)
{
 unsigned long FCPU = 16000000;
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 // Same as (FCPU / (16 * U0baud)) - 1;
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;
 *myUBRR0  = tbaud;
}
unsigned char kbhit()
{
  return *myUCSR0A & RDA;
}
unsigned char getChar()
{
  return *myUDR0;
}
void putChar(unsigned char U0pdata)
{
  while((*myUCSR0A & TBE)==0);
  *myUDR0 = U0pdata;
}
