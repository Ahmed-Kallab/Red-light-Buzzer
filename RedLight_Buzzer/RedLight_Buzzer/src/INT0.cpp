#include <INT0.h>


void PCINT0_init(void)
{
    DDRB &= ~(1 << Button_MasterAccess);
	PORTB |= (1 << Button_MasterAccess);
	EICRA |= (1 << ISC01) | (1 << ISC00);
	EIMSK |= (1 << INT0);
	sei();

}