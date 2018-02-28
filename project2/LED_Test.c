#include <avr/io.h>
#include "LED_Test.h"
/**
 * \file LED_Test.c
 * \brief Small set of test functions for controlling LEDs on a AT90USBKey
 * 
 * \mainpage Simple set of functions to control the state of the onboard
 *  LEDs on the AT90USBKey. 
 *
 * \author Alexander M. Hoole
 * \date October 2006
 */

void init_LED_D2(void)
{
	DDRD |= LED_D2;		//Set LED to output (pins 4 and 5)
	PORTD = 0x00;		//Initialize port to LOW (turn off LEDs)
}

void init_LED_D5(void)
{
	DDRD |= LED_D5;		//Set LED to output (pins 6 and 7)
	PORTD = 0x00;		//Initialize port to LOW (turn off LEDs)
}

void enable_LED(unsigned int mask)
{
	PORTD = mask;		//Initialize port to high
}

void disable_LEDs(void)
{
		PORTD = 0x00;	//Initialize port to high
}
