#include <avr/io.h>
#include <avr/interrupt.h>
#include "bitops.h"

#define TA1 BIT(1)
#define TA2 BIT(2)
#define TA3 BIT(3)



#define LED1 BIT(5)
#define LED2 BIT(4)



#define SPEAKER BIT(3)

#define DATA (1 << 0)
#define CLK (1 << 7)
#define LATCH (1 << 4)



void LED1_an(){
  CLEAR(PORTB, LED1);  // LED AN
}

void LED1_aus(){
  SET(PORTB, LED1);  // LED AUS
}
void LED2_an(){
  CLEAR(PORTB, LED2);  // LED AN
}

void LED2_aus(){
  SET(PORTB, LED2);  // LED AUS
}
int i =0;




void Init_Anzeige()
{
  SET(DDRB, DATA);              /* alle drei Bit Output */
  SET(DDRD, (CLK | LATCH));
<<<<<<< Updated upstream
  CLEAR(PORTD, (CLK | LATCH)); /* SHIFT und LATCH Low */


  
=======
  CLEAR(PORTD, (CLK | LATCH));  /* SHIFT und LATCH Low */
>>>>>>> Stashed changes
}

/*Initialisiere PORTB Pin 2 und 5 auf Output */
void Init()
{
  
  
  SET(DDRB, LED1| LED2);     // LED auf Output
  SET(PORTB, LED1| LED2);   // LED AUS

  
  CLEAR(DDRC, TA1 | TA2 | TA3); /* Setze Tasten auf Input */
  SET(DDRD, SPEAKER);           /* SPEAKER auf Output */
  SET(PORTD, SPEAKER);          /* SPEAKER AUS */
}

void InitPCI()
{
  SET(PCMSK1, TA1);   /* erlaube der TA1 die auslösung des Interrupts */
  SET(PCMSK1, TA2);   /* erlaube der TA2 die auslösung des Interrupts */
  SET(PCMSK1, TA3);   /* erlaube der TA3 die auslösung des Interrupts */
  SET(PCICR, BIT(1)); /* Interrupt-Gruppe 1 (Portc) erlauben */
  SET(PCIFR, BIT(1)); /* anhängige Interrupts löschen */
}

void Init_T0() {
  TCCR0A = 0x02;
  TCCR0B = 0x03;
  OCR0A = 250;     //Zählweite im CTC-Mode bis 250
  TIMSK0 = 0x03;
  TCNT0 = 0;    
  TIFR0 = 0x07;
  Serial.begin(115200);
}

ISR(PCINT1_vect)
{
  Init_T0();
  //cli();              /* sperre Interrupts */
  // do something
  
  SET(PCIFR, BIT(1)); /* lösche alle Interrupts */
  sei();              /* Interrupts wieder freigeben */
}

ISR(TIMER0_COMPA_vect){
  //static-Variable wird bei Deklaration mit Wert definiert und bleibt auch bei Verlassen erhalten
  static int milliseconds = 0;

  milliseconds ++;
  if(milliseconds == 100){
    if (i == 0){
    LED1_an();
    i=1;
  } else {
    LED1_aus();
    i=0;
  }
    milliseconds = 0;
  }
}

int main()
{
  Init();
  Init_Anzeige();
  InitPCI();
  sei();
  while (1)
  {
  }
}
