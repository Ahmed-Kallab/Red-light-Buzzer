#include <RGLED.h>


void Red()
{
    PORTD |= (1 << Red_LED);
    PORTD &= ~(1 << Green_LED);
}

void Green()
{
    PORTD &= ~(1 << Red_LED);
    PORTD |= (1 << Green_LED);
}

void OFF()
{
    PORTD &= ~(1 << Red_LED);
    PORTD &= ~(1 << Green_LED);
}