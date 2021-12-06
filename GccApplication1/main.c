/*
 * GccApplication1.c
 *
 * Created: 12/5/2021 1:56:42 PM
 * Author : Levko Nikitin
 */ 

#include <avr/io.h>


int main(void)
{
  DDRD &= ~((1<<2) | (1<<3)); // setting the pins for input 
  DDRB |= (1<<4)| (1<<5); 
  
  while(1)
  {
	  if((PIND & (1<<2))!= 0)
	  {
		PORTB &= ~(1<<5);
	  }
	 if((PIND & (1<<3))!= 0)
	  {
		PORTB |= (1<<5); 
	  }
	  
  }
  
  return 0; 
}

