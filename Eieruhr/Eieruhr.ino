#include <avr/io.h>
#include <avr/interrupt.h>
#include "bitops.h"

/* Ausgabe der Zahl '2' auf Stelle 2 der 7-Segmentanzeige */

/* Anschluss:
      Schiebedaten  Port B Bit 0
      Schiebetakt   Port D Bit 7
      Ãœbernahmetakt Port D Bit 4

   Ansteuerung:
      - Beide Taktleitungen im RuheisTimer auf Low
      - Ausgabe von 2 8-Bit Worten als Schiebedaten, MSB First
      - nach Ausgabe jedes Bits ein Low-High-Low Impuls am Schiebetakt
      - nach Ausgabe der beiden Bytes ein Low-High-Low Impuls am Ãœbernahmetakt

      Byte 1    Byte 2
      Segmente  Stelle    Segmentbits 0 => Segment an
      -----------------   Stellenbit  1 => Stelle an
      hgfedcba xxxx3210

      Stelle 0 ist ganz rechts, 3 links

    Bsp.
      10100100 11110100   Segmente a, b, d, e und g der Stelle 2 leuchten

         a
        ---
     f | g | b
        ---
     e |   | c
        ---
         d
*/

/********************************************************************************
 *    Define
 ********************************************************************************/

/* Define bit for button 1-3 */
#define TA1 BIT(1)
#define TA2 BIT(2)
#define TA3 BIT(3)

/* Define bit for speaker */
#define LED1 BIT(5)
#define SPEAKER BIT(3)

/* Define sound-/delayLength of speaker */
#define soundLength 150000
#define delayLength 0.75 * soundLength

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
#define POINT 0x7F // 0b01111111

/* Define bit for display segment (HEX) */
#define DISP1 0xF1
#define DISP3 0xF4
#define DISP2 0xF2
#define DISP4 0xF8

/********************************************************************************
 *    Global variables
 ********************************************************************************/

volatile bool isTimer = false; /* true = Timer running, false = Timer stopped */
volatile bool isAlarm = false; /* true = Alarm on, false = Alarm off  */

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
void Init_T1();

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
  SET(DDRB, DATA);             /* All 3 signals output */
  SET(DDRD, (CLK | LATCH));    /* */
  CLEAR(PORTD, (CLK | LATCH)); /* CLK und LATCH Low */
}

/* Initialize the alarm */
void Init_Alarm()
{
  /* For silent mode LED1 */
  SET(DDRB, LED1); /* Set LED1 to output */w
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
  Serial.begin(9600);
}

/* Initialize the 8-bit Timer2 */
void Init_T1()
{
  TCCR2A = 0x02; /* CTC-Mode */
  TCCR2B = 0x04; /* VT /256 */
  OCR2A = 250;   /* Count range in CTC mode up to 250 */
  TIMSK2 = 0x02;
}

/* Initialisierung der buttons-Interrupts */
void Init_Buttons()
{
  CLEAR(DDRC, TA1); /* button 1 auf Input */
  SET(PORTC, TA1);  /* button 1 auf Pullup (High) */

  CLEAR(DDRC, TA2); /* button 2 auf Input */
  SET(PORTC, TA2);  /* button 2 auf Pullup (High) */

  CLEAR(DDRC, TA3); /* button 1 auf Input */
  SET(PORTC, TA3);  /* button 1 auf Pullup (High) */
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
    /* Debugging */
    Serial.println("isAlarm 1");
    Serial.print("isTimer: ");
    Serial.println(isTimer);
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
  /* Debugging */
  /* Wait(100000); */
  /**** Buttons ****/
  Get_Buttons();
  if (edge1 == 1)
  {
    /* on Button 1 rising edge */
    if (stateTA1 == 1 && stateTA1_old == 0)
    {
      LED1_ON();
      Serial.println("button1"); /* Debugging */
      if (!isTimer && seconds < 60 * 59)
      {
        seconds += min(60, 60 - (seconds % 60)); /* Calculates to the highest value if this is <60 */
        /* Debugging */
        Serial.print("isTimer:  ");
        Serial.println(isTimer);
        Serial.print("minutes: ");
        Serial.println(seconds / 60);
      }
      stateTA1_old = stateTA1;
    }
    else
    {
      LED1_OFF();
      stateTA1_old = 0;
    }
    edge1 = 0;
  }
  if (edge2 == 1)
  {
    /* on Button 2 rising edge */
    if (stateTA2 == 1 && stateTA2_old == 0)
    {
      LED1_ON();
      Serial.println("button2"); /* Debugging */
      if (!isTimer && seconds > 60)
      {
        seconds -= max(60, 60 + (seconds % 60)); /* Calculates to the lowest value if this is <60 */
        /* Debugging */
        Serial.print("isTimer:  ");
        Serial.println(isTimer);
        Serial.print("minutes: ");
        Serial.println(seconds / 60);
      }
      stateTA2_old = stateTA2;
    }
    else
    {
      LED1_OFF();
      stateTA2_old = 0;
    }
    edge2 = 0;
  }
  if (edge3 == 1)
  {
    /* on Button 3 rising edge */
    if (stateTA3 == 1 && stateTA3_old == 0)
    {
      LED1_ON();
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
      Serial.print("isTimer");
      Serial.println(isTimer);
      Serial.print("Alarm An");
      Serial.println(isAlarm);
      stateTA3_old = stateTA3;
    }
    else
    {
      LED1_OFF();
      stateTA3_old = 0;
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
  /* CLEAR(PORTD, SPEAKER);    /*zu Testzwecken ausgeschalten*/
  LED1_ON();
}
/* Switch off Speaker */
void Speaker_OFF()
{
  /* SET(PORTD, SPEAKER);      /*zu Testzwecken ausgeschalten*/
  LED1_OFF();
}

/* Funktion zum Erzeugen einer Verzögerung*/
void Wait(unsigned long delay)
{
  for (volatile unsigned long i = 0; i < delay; i++) /* zähle bis delay -> Pause*/
    ;
}

/* Erzeugen eines Sounds*/
void Sound(unsigned long durationOn, unsigned long durationOff)
{
  Speaker_ON();
  Wait(durationOn);
  Speaker_OFF();
  Wait(durationOff);
}

/* Melody als Refrain*/
void Refrain()
{
  if (isAlarm == 1)
  {
    Sound(soundLength, delayLength);
    Sound(soundLength, delayLength);
    Sound(soundLength, delayLength);
    Sound(soundLength, delayLength / 4);
    Sound(soundLength / 2, delayLength / 2);
    Sound(soundLength, delayLength);
    Sound(soundLength, delayLength / 4);
    Sound(soundLength / 2, delayLength / 2);
    Sound(soundLength * 2, delayLength * 2);
  }
}
/* Melody als Strophe*/
void Strophe()
{
  if (isAlarm == 1)
  {
    Sound(soundLength, delayLength);
    Sound(soundLength, delayLength / 2);
    Sound(soundLength / 2, delayLength / 2);
    Sound(soundLength, delayLength);
    Sound(soundLength / 2, delayLength / 2);
    Sound(soundLength / 2, delayLength / 2);
    Sound(soundLength / 4, delayLength / 4);
    Sound(soundLength / 4, delayLength / 4);
    Sound(soundLength / 2, delayLength);

    Sound(soundLength / 2, delayLength / 2);
    Sound(soundLength, delayLength);
    Sound(soundLength / 2, delayLength / 2);
    Sound(soundLength / 2, delayLength / 2);
    Sound(soundLength / 4, delayLength / 4);
    Sound(soundLength / 4, delayLength / 4);
    Sound(soundLength / 2, delayLength);

    Sound(soundLength / 2, delayLength / 2);
    Sound(soundLength, delayLength);
    Sound(soundLength, delayLength / 2);
    Sound(soundLength / 4, delayLength / 4);
    Sound(soundLength, delayLength);
    Sound(soundLength, delayLength / 2);
    Sound(soundLength / 4, delayLength / 4);
    Sound(soundLength * 2, delayLength);
  }
}

/* Melody aus Refrain und Strophe*/
void Melody()
{
  if (isAlarm == 1)
  {
    Serial.println("Melody");
    for (volatile unsigned long i = 0; i < 1; i++)
    {
      Refrain();
      Serial.println("Refrain 1 zuende");
      Refrain();
      Serial.println("Rerain 2 zuende");
      Strophe();
      Serial.println("Strophe 1 zuende");
      Strophe();
      Serial.println("Strophe 2 zuende");
      Wait(delayLength * 2);
    }
    isAlarm = false; /* Alarm-Flag wieder auf 0 setzen*/
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
  Init_Alarm();
  Init_T0();
  Init_T1();
  sei();
  while (1)
  {
    if (isAlarm == 1)
    {
      Melody();
    }
  }
}
