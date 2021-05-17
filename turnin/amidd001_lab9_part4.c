/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *	Video Demo: https://youtu.be/JcweRVwMBOs
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "avr/interrupt.h"
#endif


volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn()
{
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff()
{
	TCCR1B = 0x00;
}

void TimerISR()
{
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect)
{
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0)
	{
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


unsigned char threeLEDs = 0;
unsigned char blinkingLED = 0;
unsigned char speakerPin = 0;
enum ThreeLEDsSM {Bit0_State, Bit1_State, Bit2_State} Three_State;
enum BlinkingLEDsSM {Bit3On_State, Bit3Off_State} Blinking_State;
enum CombineLEDsSM {Output_State} Combine_State;
enum SpeakerToggleSM {SpeakerOn_State, SpeakerOff_State, Wait_State} Speaker_State;
void TickFct_ThreeLEDs()
{
	switch(Three_State)
	{
		case(Bit0_State):
			Three_State = Bit1_State;
			break;
		case(Bit1_State):
			Three_State = Bit2_State;
			break;
		case(Bit2_State):
			Three_State = Bit0_State;
			break;
		default:
			break;
	}

	switch(Three_State)
	{
		case(Bit0_State):
			threeLEDs = 0x01;
			break;
		case(Bit1_State):
			threeLEDs = 0x02;
			break;
		case(Bit2_State):
			threeLEDs = 0x04;
			break;
		default:
			break;
	}
}

void TickFct_BlinkingLEDs()
{
	switch(Blinking_State)
	{
		case(Bit3On_State):
			Blinking_State = Bit3Off_State;
			break;
		case(Bit3Off_State):
			Blinking_State = Bit3On_State;
			break;
		default:
			break;
	}

	switch(Blinking_State)
	{
		case(Bit3On_State):
			blinkingLED = 0x08;
			break;
		case(Bit3Off_State):
			blinkingLED = 0x00;
			break;
		default:
			break;
	}
}

void TickFct_CombineLEDs()
{
	switch(Combine_State)
	{
		case(Output_State):
			Combine_State = Output_State;
			break;
		default:
			break;
	}	

	switch(Combine_State)
	{
		case(Output_State):
			PORTB = blinkingLED + threeLEDs + speakerPin;
			break;
		default:
			break;
	}
}

void TickFct_Speaker()
{
	unsigned char tmpA = ~PINA & 0x04;
	switch(Speaker_State)
	{
		case(SpeakerOff_State):
			if(tmpA == 0x04)
			{
				Speaker_State = SpeakerOn_State;
			}
			else
			{
				Speaker_State = SpeakerOff_State;
			}
			break;
		case(SpeakerOn_State):
			Speaker_State = Wait_State;
			break;
		case(Wait_State):
			Speaker_State = SpeakerOff_State;
			break;
		default:
			break;
	}

	switch(Speaker_State)
	{
		case(SpeakerOff_State):
			speakerPin = 0x00;
			break;
		case(SpeakerOn_State):
			speakerPin = 0x10;
			break;
		case(Wait_State):
			speakerPin = 0x10;
			break;
		default:
			break;
	}
}


int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0xFF; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
    /* Insert your solution below */
	const unsigned long timerPeriod = 1; //would have used 100 but it specifically says not to use GCD
	TimerSet(timerPeriod);
	TimerOn();
	Three_State = Bit0_State;
	Blinking_State = Bit3On_State;
	Combine_State = Output_State;
	Speaker_State = SpeakerOff_State;
	unsigned long Three_elapsedTime = 300;
	unsigned long Blink_elapsedTime = 1000;
	unsigned long speakerPeriod = 2;
	unsigned long Speaker_elapsedTime = speakerPeriod;
	unsigned char tmpA0 = 0;
	unsigned char tmpA1 = 0;
    while (1)
    {
	tmpA0 = ~PINA & 0x01;
	tmpA1 = ~PINA & 0x02;

	if(Three_elapsedTime >= 300)
	{
		TickFct_ThreeLEDs();
		Three_elapsedTime = 0;

		if(tmpA0 == 0x01)
        	{
                	speakerPeriod++;
        	}
        	if(tmpA1 == 0x02)
        	{
                	if(speakerPeriod > 1)
                	{
                        	speakerPeriod--;
                	}
        	}
	}

	if(Blink_elapsedTime >= 1000)
	{
		TickFct_BlinkingLEDs();
		Blink_elapsedTime = 0;
	}

	if(Speaker_elapsedTime >= speakerPeriod)
	{
		TickFct_Speaker();
		Speaker_elapsedTime = 0;
	}

	TickFct_CombineLEDs();
	while(!TimerFlag){}
	TimerFlag = 0;
	Three_elapsedTime += timerPeriod;
	Blink_elapsedTime += timerPeriod;
	Speaker_elapsedTime += timerPeriod;	
    }
    return 1;
}
