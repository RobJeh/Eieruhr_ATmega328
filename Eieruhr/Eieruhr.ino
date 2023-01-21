#include <avr/io.h>
#include <avr/interrupt.h>
#include "bitops.h"

/********************************************************************************
 *    Define
 ********************************************************************************/

/* Define bit for button 1-3 */
#define TA1 BIT(1)
#define TA2 BIT(2)
#define TA3 BIT(3)

/* Define bit for visual feedback */
#define LED2 BIT(4)
#define LED3 BIT(3)

/* Define bit for speaker */
#define LED1 BIT(5)
#define SPEAKER BIT(3)

/* Define sound-/delaylenght of speaker */
#define SOUNDLENGTH 150000
#define DELAYLENGTH 0.75 * SOUNDLENGTH

/* Define bit for Display */
#define DATA BIT(0)
#define CLK BIT(7)
#define LATCH BIT(4)

/* Define bit mask for digits (HEX) */
#define ZERO 0xC0
#define ONE 0xF9
#define TWO 0xA4
#define THREE 0xB0
#define FOUR 0x99
#define FIVE 0x92
#define SIX 0x82
#define SEVEN 0xF8
#define EIGHT 0x80
#define NINE 0x90
#define BLACK 0xFF
#define POINT 0x7F /* 0b01111111 */

/* Define bit for display segment (HEX) */
#define DISP1 0xF1
#define DISP3 0xF4
#define DISP2 0xF2
#define DISP4 0xF8

/********************************************************************************
 *    Global variables
 ********************************************************************************/

volatile bool isTimer = false;       /* true = Timer running, false = Timer stopped */
volatile bool isAlarm = false;       /* true = Alarm on, false = Alarm off  */
volatile bool isSilent = false;      /* true = Silent mode on, false = Silent mode off  */
volatile bool isLongPressed = false; /* true = Silent mode on, false = Silent mode off  */

unsigned char digitSeg1, digitSeg2, digitSeg3, digitSeg4; /* Stores the digit displayed on the segment */

int seconds = 5; /* Default 5 seconds after reset */

int stateTA1, stateTA2, stateTA3 = 0;             /* Stores the state of the button */
int stateTA1_old, stateTA2_old, stateTA3_old = 0; /* Stores the old state of the button */
int edge1, edge2, edge3 = 0;                      /* Stores the edge detection state of the button */

/********************************************************************************
 *    Prototypes
 ********************************************************************************/

/* Display */
void Init_Display();
void Get_Time(int timerValue);
void Set_Segment(unsigned char digitMask, unsigned char segment);

/*  Feedback LEDs */
void Init_Feedback();
void LED2_OFF();
void LED2_ON();
void LED3_OFF();
void LED3_ON();

/*  Alarm */
void Init_Alarm();
void Wait(unsigned long delay);
void LED1_OFF();
void LED1_ON();
void Speaker_ON();
void Speaker_OFF();
void Sound(unsigned long durationOn, unsigned long durationOff);
void Refrain();
void Strophe();
void Melody();

/* Timer */
void Init_T0();
void Init_T2();

/* Buttons */
void Init_Buttons();
void Get_Buttons();
enum State
{
  PRESSED,
  RELEASED
};

/********************************************************************************
 *    Init
 ********************************************************************************/

/* Initialize the display */
void Init_Display()
{
  SET(DDRB, DATA);             /* All 3 bits output */
  SET(DDRD, (CLK | LATCH));    /* */
  CLEAR(PORTD, (CLK | LATCH)); /* CLK und LATCH Low */
}

void Init_Feedback()
{
  SET(DDRB, LED2 | LED3); /* Set LED2, LED3 to output */
  LED2_OFF();             /* Switch off the LED2 initially */
  LED3_OFF();             /* Switch off the LED3 initially */
}

/* Initialize the alarm */
void Init_Alarm()
{
  /* For silent mode LED1 */
  SET(DDRB, LED1); /* Set LED1 to output */
  LED1_OFF();      /* Switch off the LED1 initially */

  /* For alarm mode SPEAKER */
  SET(DDRD, SPEAKER);  /* Set SPEAKER to output */
  SET(PORTD, SPEAKER); /* Switch off the SPEAKER initially */
}

/* Initialize the 8-bit Timer0 */
void Init_T0()
{
  TCCR0A = 0x02; /* CTC-Mode */
  TCCR0B = 0x03; /* VT /64 */
  OCR0A = 250;   /* Count range in CTC mode up to 250 */
  TIMSK0 = 0x02;
}

/* Initialize the 8-bit Timer2 */
void Init_T2()
{
  TCCR2A = 0x02; /* CTC-Mode */
  TCCR2B = 0x04; /* VT /256 */
  OCR2A = 250;   /* Count range in CTC mode up to 250 */
  TIMSK2 = 0x02;
}

/* Initialize the buttons */
void Init_Buttons()
{
  CLEAR(DDRC, TA1); /* button 1 to Input */
  SET(PORTC, TA1);  /* button 1 to Pullup (High) */
  CLEAR(DDRC, TA2); /* button 2 to Input */
  SET(PORTC, TA2);  /* button 2 to Pullup (High) */
  CLEAR(DDRC, TA3); /* button 1 to Input */
  SET(PORTC, TA3);  /* button 1 to Pullup (High) */
}

/********************************************************************************
 *    Timer (ISR)
 ********************************************************************************/

/* ISR for manually triggered "egg timer" (TIMER0) */
ISR(TIMER0_COMPA_vect)
{
  static int milliseconds = 0;
  if (milliseconds > 1000)
  {
    milliseconds = 0; /* Overflow handling */
  }
  else if (isTimer)
  {
    milliseconds++; /* on Timer0 is running (isTimer) -> increase milliseconds */
  }
  if (seconds == 0 && isTimer)
  {
    isAlarm = 1;     /* Trigger alarm at seconds==0 */
    isTimer = false; /* Stop Timer0 at seconds==0 */
  }
  if (milliseconds == 1000 && seconds > 0)
  {
    --seconds;        /* Subtract 1 second at milliseconds==1000 */
    milliseconds = 0; /* Reset milliseconds */
  }
}

/* ISR for display refresh and button handling (TIMER2) */
ISR(TIMER2_COMPA_vect)
{
  static int seg = 1;

  /**** Buttons ****/
  Get_Buttons();
  if (isSilent)
  {
    LED3_ON();
  }
  else
  {
    LED3_OFF();
  }
  /* Button 1 */
  if (edge1 == 1)
  {
    /* on Button 1 rising edge */
    if (stateTA1 == 1 && stateTA1_old == 0)
    {
      LED2_ON();
      if (!isTimer && seconds < 60 * 59)
      {
        seconds += min(60, 60 - (seconds % 60)); /* Calculates to the highest value if this is <60 */
      }
      stateTA1_old = stateTA1;
    }
    else
    {
      LED2_OFF();
      stateTA1_old = 0;
    }
    edge1 = 0;
  }
  /* Button 2 */
  if (edge2 == 1)
  {
    /* on Button 2 rising edge */
    if (stateTA2 == 1 && stateTA2_old == 0)
    {
      LED2_ON();
      if (!isTimer && seconds > 60)
      {
        seconds -= max(60, 60 + (seconds % 60)); /* Calculates to the lowest value if this is <60 */
      }
      stateTA2_old = stateTA2;
    }
    else
    {
      LED2_OFF();
      stateTA2_old = 0;
    }
    edge2 = 0;
  }
  /* Button 3 */
  if (edge3 == 1)
  {
    Serial.println(isSilent);
    /* on Button 3 falling edge */
    if (stateTA3 == 0 && stateTA3_old == 1)
    {
      LED2_OFF();
      Serial.println("falling");
      if (!isLongPressed)
      {
        Serial.println("button3"); /* Debugging */
        if (isTimer)
        {
          isTimer = false; /* when Timer0 is running -> switch off Timer0 */
        }
        else if (!isTimer && !isAlarm)
        {
          isTimer = 1; /* when Timer0 is not running -> switch on Timer0 */
        };
        if (isAlarm == 1 && !isTimer)
        {
          isAlarm = false; /* when alarm is on -> turn off alarm */
        }
        /* Debugging */
        Serial.print("isTimer: ");
        Serial.println(isTimer);
        Serial.print("Alarm An: ");
        Serial.println(isAlarm);
        stateTA3_old = stateTA3;
      }
    }
    else
    {
      if (stateTA3 == 1)
      {
        LED2_ON();
        stateTA3_old = 1;

        Serial.println("rising");
      }
    }
    edge3 = 0;
  }
  /**** Display ****/
  Get_Time(seconds); /* Calculates each segment digit based on the remaining seconds */
  switch (seg)
  {
  case 1:
    Set_Segment(digitSeg1, DISP1); /* Set segment 1 to calculated value */
    seg++;                         /* Set the focus for the next ISR call to the next segment */
    break;
  case 2:
    Set_Segment(CLEAR(digitSeg2, 0x80), DISP2); /* Set segment 2 (and point) to calculated value */
    seg++;                                      /* Set the focus for the next ISR call to the next segment */
    break;
  case 3:
    Set_Segment(digitSeg3, DISP3); /* Set segment 3 to calculated value */
    seg++;                         /* Set the focus for the next ISR call to the next segment */
    break;
  case 4:
    Set_Segment(digitSeg4, DISP4); /* Set segment 4 to calculated value */
    seg = 1;                       /* Set the focus for the next ISR call to the first segment */
    break;
  }
}

/********************************************************************************
 *    Feedback LEDs
 ********************************************************************************/

/* Switch on LED2  */
void LED2_OFF()
{
  SET(PORTB, LED2);
}

/* Switch off LED2 */
void LED2_ON()
{
  CLEAR(PORTB, LED2);
}

/* Switch on LED3  */
void LED3_OFF()
{
  SET(PORTB, LED3);
}

/* Switch off LED3 */
void LED3_ON()
{
  CLEAR(PORTB, LED3);
}

/********************************************************************************
 *    Alarm
 ********************************************************************************/

/* Switch on LED1  */
void LED1_OFF()
{
  SET(PORTB, LED1);
}

/* Switch off LED1 */
void LED1_ON()
{
  CLEAR(PORTB, LED1);
}

/* Switch on Speaker */
void Speaker_ON()
{
  CLEAR(PORTD, SPEAKER);
}

/* Switch off Speaker */
void Speaker_OFF()
{
  SET(PORTD, SPEAKER);
}

/* Wait function */
void Wait(unsigned long delay)
{
  for (volatile unsigned long i = 0; i < delay; i++) /* count to delay -> wait */
    ;
}

/* Creating a sound */
void Sound(unsigned long durationOn, unsigned long durationOff)
{
  if (isSilent)
  {
    LED1_ON();
    Wait(durationOn);
    LED1_OFF();
    Wait(durationOff);
  }
  else
  {
    Speaker_ON();
    Wait(durationOn);
    Speaker_OFF();
    Wait(durationOff);
  }
}

/* Melody as a "Refrain" */
void Refrain()
{
  if (isAlarm == 1)
  {
    Sound(SOUNDLENGTH, DELAYLENGTH);
    Sound(SOUNDLENGTH, DELAYLENGTH);
    Sound(SOUNDLENGTH, DELAYLENGTH);
    Sound(SOUNDLENGTH, DELAYLENGTH / 4);
    Sound(SOUNDLENGTH / 2, DELAYLENGTH / 2);
    Sound(SOUNDLENGTH, DELAYLENGTH);
    Sound(SOUNDLENGTH, DELAYLENGTH / 4);
    Sound(SOUNDLENGTH / 2, DELAYLENGTH / 2);
    Sound(SOUNDLENGTH * 2, DELAYLENGTH * 2);
  }
}
/* Melody as a "Strophe" */
void Strophe()
{
  if (isAlarm == 1)
  {
    Sound(SOUNDLENGTH, DELAYLENGTH);
    Sound(SOUNDLENGTH, DELAYLENGTH / 2);
    Sound(SOUNDLENGTH / 2, DELAYLENGTH / 2);
    Sound(SOUNDLENGTH, DELAYLENGTH);
    Sound(SOUNDLENGTH / 2, DELAYLENGTH / 2);
    Sound(SOUNDLENGTH / 2, DELAYLENGTH / 2);
    Sound(SOUNDLENGTH / 4, DELAYLENGTH / 4);
    Sound(SOUNDLENGTH / 4, DELAYLENGTH / 4);
    Sound(SOUNDLENGTH / 2, DELAYLENGTH);

    Sound(SOUNDLENGTH / 2, DELAYLENGTH / 2);
    Sound(SOUNDLENGTH, DELAYLENGTH);
    Sound(SOUNDLENGTH / 2, DELAYLENGTH / 2);
    Sound(SOUNDLENGTH / 2, DELAYLENGTH / 2);
    Sound(SOUNDLENGTH / 4, DELAYLENGTH / 4);
    Sound(SOUNDLENGTH / 4, DELAYLENGTH / 4);
    Sound(SOUNDLENGTH / 2, DELAYLENGTH);

    Sound(SOUNDLENGTH / 2, DELAYLENGTH / 2);
    Sound(SOUNDLENGTH, DELAYLENGTH);
    Sound(SOUNDLENGTH, DELAYLENGTH / 2);
    Sound(SOUNDLENGTH / 4, DELAYLENGTH / 4);
    Sound(SOUNDLENGTH, DELAYLENGTH);
    Sound(SOUNDLENGTH, DELAYLENGTH / 2);
    Sound(SOUNDLENGTH / 4, DELAYLENGTH / 4);
    Sound(SOUNDLENGTH * 2, DELAYLENGTH);
  }
}

/* Melody of chorus and verse */
void Melody()
{
  if (isAlarm == 1)
  {
    for (volatile unsigned long i = 0; i < 1; i++)
    {
      Refrain();
      Refrain();
      Strophe();
      Strophe();
      Wait(DELAYLENGTH * 2);
    }
    isAlarm = false; /* set Alarm-Flag back to 0 */
    seconds = 60;
  }
}

/********************************************************************************
 *    Display
 ********************************************************************************/

/* Compute the minutes and secound digits from the timer value */
void Get_Time(int timerValue)
{
  int minValue = timerValue / 60;
  int secValue = timerValue % 60;
  int minDigit1 = minValue / 10;
  int minDigit2 = minValue % 10;
  int secDigit1 = secValue / 10;
  int secDigit2 = secValue % 10;

  switch (minDigit1)
  {
  case 0:
    digitSeg1 = ZERO;
    break;
  case 1:
    digitSeg1 = ONE;
    break;
  case 2:
    digitSeg1 = TWO;
    break;
  case 3:
    digitSeg1 = THREE;
    break;
  case 4:
    digitSeg1 = FOUR;
    break;
  case 5:
    digitSeg1 = FIVE;
    break;
  case 6:
    digitSeg1 = SIX;
    break;
  case 7:
    digitSeg1 = SEVEN;
    break;
  case 8:
    digitSeg1 = EIGHT;
    break;
  case 9:
    digitSeg1 = NINE;
    break;
  }
  switch (minDigit2)
  {
  case 0:
    digitSeg2 = ZERO;
    break;
  case 1:
    digitSeg2 = ONE;
    break;
  case 2:
    digitSeg2 = TWO;
    break;
  case 3:
    digitSeg2 = THREE;
    break;
  case 4:
    digitSeg2 = FOUR;
    break;
  case 5:
    digitSeg2 = FIVE;
    break;
  case 6:
    digitSeg2 = SIX;
    break;
  case 7:
    digitSeg2 = SEVEN;
    break;
  case 8:
    digitSeg2 = EIGHT;
    break;
  case 9:
    digitSeg2 = NINE;
    break;
  }
  switch (secDigit1)
  {
  case 0:
    digitSeg3 = ZERO;
    break;
  case 1:
    digitSeg3 = ONE;
    break;
  case 2:
    digitSeg3 = TWO;
    break;
  case 3:
    digitSeg3 = THREE;
    break;
  case 4:
    digitSeg3 = FOUR;
    break;
  case 5:
    digitSeg3 = FIVE;
    break;
  case 6:
    digitSeg3 = SIX;
    break;
  case 7:
    digitSeg3 = SEVEN;
    break;
  case 8:
    digitSeg3 = EIGHT;
    break;
  case 9:
    digitSeg3 = NINE;
    break;
  }
  switch (secDigit2)
  {
  case 0:
    digitSeg4 = ZERO;
    break;
  case 1:
    digitSeg4 = ONE;
    break;
  case 2:
    digitSeg4 = TWO;
    break;
  case 3:
    digitSeg4 = THREE;
    break;
  case 4:
    digitSeg4 = FOUR;
    break;
  case 5:
    digitSeg4 = FIVE;
    break;
  case 6:
    digitSeg4 = SIX;
    break;
  case 7:
    digitSeg4 = SEVEN;
    break;
  case 8:
    digitSeg4 = EIGHT;
    break;
  case 9:
    digitSeg4 = NINE;
    break;
  }
}

/* Put a digit on a segment */
void Set_Segment(unsigned char digitMask, unsigned char segment)
{
  for (int i = 0; i < 8; i++)
  {
    if (digitMask & BIT(7))
    {
      SET(PORTB, DATA);
    }
    else
    {
      CLEAR(PORTB, DATA);
    }
    digitMask <<= 1;
    SET(PORTD, CLK);
    CLEAR(PORTD, CLK);
  }
  for (int i = 0; i < 8; i++)
  {
    if (segment & BIT(7))
    {
      SET(PORTB, DATA);
    }
    else
    {
      CLEAR(PORTB, DATA);
    }
    segment <<= 1;
    SET(PORTD, CLK);
    CLEAR(PORTD, CLK);
  }
  SET(PORTD, LATCH);
  CLEAR(PORTD, LATCH);
}

/*********************************************************************************
 *    Buttons
 *********************************************************************************/

/* Read a button value and return the state as an State enum */
enum State ReadButton(unsigned char button)
{
  if (PINC & button)
  {
    return PRESSED;
  }
  else
  {
    return RELEASED;
  }
}

/* Get all button states */
void Get_Buttons()
{
  static int longPress = 0;
  enum State T1_new;
  enum State T2_new;
  enum State T3_new;
  static enum State T1_old = RELEASED;
  static enum State T2_old = RELEASED;
  static enum State T3_old = RELEASED;

  T1_new = ReadButton(TA1);
  T2_new = ReadButton(TA2);
  T3_new = ReadButton(TA3);

  if (T1_new != T1_old)
  {
    stateTA1 = T1_new;
    edge1 = 1;
  }
  if (T2_new != T2_old)
  {
    stateTA2 = T2_new;
    edge2 = 1;
  }
  if (T3_new != T3_old)
  {
    stateTA3 = T3_new;
    edge3 = 1;
  }
  else
  {
    if (stateTA3 == 1)
    {
      longPress++;
      if (longPress >= 1000)
      {
        isSilent = !isSilent;
        isLongPressed = true;
      }
      else
      {
        isLongPressed = false;
      }
    }
    else
    {
      longPress = 0;
    }
  }

  T1_old = T1_new;
  T2_old = T2_new;
  T3_old = T3_new;
}

/********************************************************************************
 *    Main
 ********************************************************************************/

int main()
{
  Serial.begin(9600); /* Debugging */
  Init_Display();
  Init_Feedback();
  Init_Alarm();
  Init_T0();
  Init_T2();
  sei();
  while (1)
  {
    if (isAlarm == 1)
    {
      Melody();
    }
  }
}
