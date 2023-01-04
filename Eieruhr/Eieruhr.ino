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
      Define
 ********************************************************************************/

/* Define Bit fuer Taste 1-3 */
#define TA1 BIT(1)
#define TA2 BIT(2)
#define TA3 BIT(3)

/* Define Bit fuer Speaker */
#define LED1 BIT(5)
#define SPEAKER BIT(3)

/* Define Sound-/delayLength Speaker */
#define soundLength 150000
#define delayLength 0.75 * soundLength

/* Define Bit fuer Display */
#define DATA BIT(0)
#define CLK BIT(7)
#define LATCH BIT(4)

/* Define Bit-Maske fuer Ziffern */
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

/* Define Bit fuer Display-Segment */
#define DISP1 0xF1
#define DISP3 0xF4
#define DISP2 0xF2
#define DISP4 0xF8

#define N 2000

/********************************************************************************
      Global variables
 ********************************************************************************/

int isTimer = 0;          /* 1 = Running, 0 = Stop */
volatile int isAlarm = 0; /* 1 = Alarm on, 0 = Alarm off  */

unsigned char digitSeg1, digitSeg2, digitSeg3, digitSeg4; /* stores the digit displayed on the segment */

int minutes = 1;
int seconds = 5;

int Zustand = 0;
int Zustand_alt = 0;
int Flanke = 0;
int count = 0;

/********************************************************************************
      Prototypes
 ********************************************************************************/

/* Display */
void Init_Display();
void GetTime(int timerValue);
void SetSegment(unsigned char digitMask, unsigned char segment);
void SetDisplay();

/*  Alarm */
void Init_Alarm();
void Wait(unsigned long delay);
void LED1_off();
void LED1_on();
void Speaker_on();
void Speaker_off();
void Sound(unsigned long durationOn, unsigned long durationOff);
void Refrain();
void Strophe();
void Melody();

/* Timer */
void Init_T0();
void Init_T1();

/*Tasten-Interrupts*/
// void Init_Tasten();
void Entprellen();
enum State
{
  AN,
  AUS
};

/********************************************************************************
      Init
 ********************************************************************************/

void Init_Display()
{
  SET(DDRB, DATA); /* alle drei Bit Output */
  SET(DDRD, (CLK | LATCH));
  CLEAR(PORTD, (CLK | LATCH)); /* SHIFT und LATCH Low */
}

// Initialisierung des Alarms
void Init_Alarm()
{
  SET(DDRB, LED1); // LED auf Output
  LED1_off();

  SET(DDRD, SPEAKER);  // SPEAKER auf Output
  SET(PORTD, SPEAKER); // SPEAKER AUS
}

// Initialisierung des Timers 0
void Init_T0()
{
  TCCR0A = 0x02;
  TCCR0B = 0x03;
  OCR0A = 250; // Zählweite im CTC-Mode bis 250
  TIMSK0 = 0x03;
  TCNT0 = 0;
  TIFR0 = 0x07;
  Serial.begin(9600);
}

// Initialisierung des Timers 1
void Init_T1()
{
  TCCR2A = 0x02; // CTC-Mode
  TCCR2B = 0x04; // /256
  OCR2A = 250;   // Zählweite im CTC-Mode bis 250
  TIMSK2 = 0x03;
  TCNT2 = 0;
  TIFR2 = 0x07;
}

/* Initialisierung der Tasten-Interrupts */
void Init_Tasten()
{
  CLEAR(DDRC, TA1); // Taste 1 auf Input
  SET(PORTC, TA1);  // Taste 1 auf Pullup (High)

  CLEAR(DDRC, TA2); // Taste 2 auf Input
  SET(PORTC, TA2);  // Taste 2 auf Pullup (High)

  CLEAR(DDRC, TA3); // Taste 1 auf Input
  SET(PORTC, TA3);  // Taste 1 auf Pullup (High)

  SET(PCMSK1, TA1);   // erlaube der TA1 die auslösung des Interrupts
  SET(PCMSK1, TA2);   // erlaube der TA2 die auslösung des Interrupts
  SET(PCMSK1, TA3);   // erlaube der TA3 die auslösung des Interrupts
  SET(PCICR, BIT(1)); // Interrupt-Gruppe 1 (Portc) erlauben
  SET(PCIFR, BIT(1)); // anhängige Interrupts löschen
}

/********************************************************************************
      Timer (ISR)
  /********************************************************************************/

ISR(TIMER0_COMPA_vect)
{
  static int milliseconds = 0;
  if (milliseconds >= 1000)
  { // Fehlerbehandlung
    milliseconds = 0;
  }
  else if (isTimer == 1)
  { // bei isTimer Start läuft Timer
    milliseconds++;
    // Serial.println("Timer 1");
  }

  if (seconds == 0 && isTimer == 1)
  { // Auslösen Alarm bei seconds==0, --seconds bei 1000 Millisekunden
    isAlarm = 1;
    isTimer = 0;
    Serial.println("isAlarm 1");
    Serial.print("isTimer: ");
    Serial.println(isTimer);
  }
  if (milliseconds == 1000 && seconds > 0)
  {
    --seconds;
    milliseconds = 0;
  }
}

ISR(TIMER2_COMPA_vect)
{
  static int seg = 1;

  GetTime(seconds);
  switch (seg)
  {
  case 1:
    SetSegment(digitSeg1, DISP1);
    seg++;           
    break;
  case 2:
    SetSegment(digitSeg2, DISP2);
    seg++;
    break;
  case 3:
    SetSegment(digitSeg3, DISP3);
    seg++;
    break;
  case 4:
    SetSegment(digitSeg4, DISP4);
    seg = 1;
    break;
  }
}

/********************************************************************************
      Alarm
  /********************************************************************************/

// LED 1 anschalten
void LED1_off()
{
  SET(PORTB, LED1);
}
// LED 1 ausschalten
void LED1_on()
{
  CLEAR(PORTB, LED1);
}
// Lautsprecher anschalten
void Speaker_on()
{
  // CLEAR(PORTD, SPEAKER);    //zu Testzwecken ausgeschalten
  LED1_on();
}
// Lautsprecher ausschalten
void Speaker_off()
{
  // SET(PORTD, SPEAKER);      //zu Testzwecken ausgeschalten
  LED1_off();
}

// Funktion zum Erzeugen einer Verzögerung
void Wait(unsigned long delay)
{
  for (volatile unsigned long i = 0; i < delay; i++) // zähle bis delay -> Pause
    ;
}

// Erzeugen eines Sounds
void Sound(unsigned long durationOn, unsigned long durationOff)
{
  Speaker_on();
  Wait(durationOn);
  Speaker_off();
  Wait(durationOff);
}

// Melody als Refrain
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
// Melody als Strophe
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

// Melody aus Refrain und Strophe
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
    isAlarm = 0; // Alarm-Flag wieder auf 0 setzen
    seconds = 60;
  }
}

/********************************************************************************
      Display
 ********************************************************************************/

/* Compute the minutes and secound digits from the timer value */
void GetTime(int timerValue)
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

/* DARSTELLUNG DER ZAHLEN FÜR 4 DIGIT */
void SetSegment(unsigned char digitMask, unsigned char segment)
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

void SetDisplay()
{
  GetTime(seconds);

  SetSegment(digitSeg4, DISP4);
  Wait(1000);
  SetSegment(digitSeg3, DISP3);
  Wait(1000);
  SetSegment(digitSeg2, DISP2);
  Wait(1000);
  SetSegment(digitSeg1, DISP1);
  Wait(1000);
}

//********************************************************************************
//        Tasten
//********************************************************************************

enum State liesTaste1()
{
  if (PINC & TA1)
  {
    Serial.println("Taste1");
    if (isTimer == 0 && seconds < 60 * 59)
    {
      seconds += min(60, 60 - (seconds % 60)); // Rechnet auf höchsten Wert, wenn dieser <60 entfernt liegt
      Serial.print("isTimer:  ");
      Serial.println(isTimer);
      Serial.print("minutes: ");
      Serial.println(seconds / 60);
    }
    return AN;
  }
  else
  {
    return AUS;
  }
}

void Entprellen()
{
  enum State T_neu;
  static enum State T_alt = AUS;
  static int Counter;

  T_neu = liesTaste1();
  if (T_neu != T_alt)
  {
    Counter = 0;
  }
  else
  {
    Counter++;
    if (Counter == N)
    {
      Zustand = T_neu;
      Flanke = 1;
    }
    if (Counter > N + 1)
    {
      Counter = N + 1;
    }
    T_alt = T_neu;
  }
}

ISR(PCINT1_vect)
{
  if (~PINC & TA1)
  {
    Serial.println("Taste1");
    if (isTimer == 0 && seconds < 60 * 59)
    {
      seconds += min(60, 60 - (seconds % 60)); // Rechnet auf höchsten Wert, wenn dieser <60 entfernt liegt
      Serial.print("isTimer:  ");
      Serial.println(isTimer);
      Serial.print("minutes: ");
      Serial.println(seconds / 60);
    }
  }
  if (~PINC & TA2)
  {
    Serial.println("Taste2");
    if (isTimer == 0 && seconds > 60)
    {
      seconds -= max(60, 60 + (seconds % 60)); // Rechnet auf niedrigsten Wert, wenn dieser <60 entfernt liegt
      Serial.print("isTimer:  ");
      Serial.println(isTimer);
      Serial.print("minutes: ");
      Serial.println(seconds / 60);
    }
  }
  if (~PINC & TA3)
  {
    Serial.println("Taste3");

    if (isTimer == 1)
    {
      isTimer = 0;
    } // Wechsel des isTimeres
    else if (isTimer == 0 && !isAlarm)
    {
      isTimer = 1;
    };

    if (isAlarm == 1 && isTimer == 0)
    {
      isAlarm = 0;
    }

    Serial.print("isTimer");
    Serial.println(isTimer);
    Serial.print("Alarm An");
    Serial.println(isAlarm);
  }
}

/********************************************************************************
      Main
 ********************************************************************************/

int main()
{
  Serial.begin(9600); // zu Testzwecken
  Init_Display();
  Init_Alarm();
  Init_T0();
  Init_T1();
  Init_Tasten();
  sei();

  while (1)
  {
    // SetDisplay();
    /*
      Entprellen();
      if (Flanke == 1) {    // Ta 1 steigende Flanke?
      if (Zustand == 1 && Zustand_alt == 0) {
          LED1_on();
          Zustand_alt = Zustand;
        } else {
          LED1_off();
          Zustand_alt = 0;
          count++;
        }
        Flanke = 0;
      }

    */
    // Serial.println(isAlarm);
    //  Triggerung in Timer-ISR, Ausführung in Main, damit Tasten-Interupt Alarm Abschalten kann

    if (isAlarm == 1)
    {
      Melody();
    }
  }
}
