#include <avr/io.h>
#include <avr/interrupt.h>
#include "bitops.h"

#define TA1 BIT(1)
#define TA2 BIT(2)
#define TA3 BIT(3)

#define SPEAKER BIT(3)

#define DATA (1<<0)
#define CLK (1<<7)
#define LATCH (1<<4)

void Init_Anzeige() {
  SET(DDRB, DATA);              /* alle drei Bit Output */
  SET(DDRD, (CLK | LATCH));
  CLEAR(PORTD, (CLK | LATCH)); /* SHIFT und LATCH Low */


  
}

/*Initialisiere PORTB Pin 2 und 5 auf Output */
void Init() {
  CLEAR(DDRC, TA1 | TA2 | TA3);  // Setze Tasten auf Input 
  SET(DDRD, SPEAKER);     // SPEAKER auf Output
  SET(PORTD, SPEAKER);   // SPEAKER AUS
}

void InitPCI(){
  SET(PCMSK1, TA1);  // erlaube der TA1 die auslösung des Interrupts
  SET(PCMSK1, TA2);  // erlaube der TA2 die auslösung des Interrupts
  SET(PCMSK1, TA3);  // erlaube der TA3 die auslösung des Interrupts
  SET(PCICR, BIT(1));  // Interrupt-Gruppe 1 (Portc) erlauben
  SET(PCIFR, BIT(1));  // anhängige Interrupts löschen 
}

ISR(PCINT1_vect) {
  
}

int main() {
  Init();
  Init_Anzeige();
  InitPCI();
  sei();
  while(1) {
    
  }
}
