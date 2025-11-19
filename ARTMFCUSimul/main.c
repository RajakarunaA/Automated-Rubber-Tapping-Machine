/*
================================================================
Automated Rubber Tapping machine by Group Alpha (Group no. 29).
Field Control Unit Simulation Code
================================================================
This file includes Simulation Code for ARTM Field Contrl Unit
*/

#define F_CPU 1000000UL //1MHz to save power
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "eeprom.h"

void settime(int h, int m, int s);
void setA1(int h, int m, int s);
void setA2(int h, int m);

uint8_t state = 0;// state of the FCU

static volatile uint8_t hour = 0;
static volatile uint8_t mins = 0;
static volatile uint8_t secs = 0;
static volatile uint8_t scount = 0;
char buffer[100];

static volatile uint8_t A1hour = 9;
static volatile uint8_t A1mins = 9;
static volatile uint8_t A1secs = 9;
static volatile uint8_t A1flag = 1;

static volatile uint8_t A2hour = 9;
static volatile uint8_t A2mins = 9;
static volatile uint8_t A2flag = 1;


#include "artmuart.h"
#include "artmgsm.h"

char ctm = 0;



int main(void)
{
	
	DDRC = 0xFF;
	PORTC = 0x20;
	DDRD &= ~(1 << PD2);
	PORTD |= 1 << PD2;
	
	
	uart_Initialize();
	gsmInitialize();
	
	MCUCR |= (1<<ISC00 | 1 << ISC01);// Rising edge of INT0 will interrupt
	GIFR &= 1 << INTF0;
	
	TIMSK |= 1 << OCIE1A;// Set Timer/Counter1 Output Compare A Match Interrupt Enable bit. So ISR will run at when counter = 50000
	
	GIFR &= 1 << INTF0;
	GICR |= 1<<INT0; //Enable interrupt for INT0
	sei();// Enable global interrupt bit, (I-flag)
	
	//Set compare value to 50 000
	OCR1AH = 0xC3;
	OCR1AL = 0x50;
	TCCR1B |= 1<<CS10;//Start counting(Without prescaling).
	
	while (1)
	{
		gsmrxmsg();
		
		if(ctm == 1 && state == 1){
			ctm = 0;
			PORTC |= 1 << PC5;
			_delay_ms(100);
			gsmSendSMS("Latex ready for collection...");
		}
		
		if(ctm == 2 && state == 1){
			ctm = 0;
			while(state == 3);
			PORTC |= 1 << PC5;
		}	
	}
}

ISR(TIMER1_COMPA_vect){// This function get executed when Timer/Counter1 Output Compare A Match the value of TCNT1
	TCCR2 &= ~(1 << CS10);// Stop counting
	if(scount > 10){
		secs++;//Increase seconds
		scount = 0;
		if(secs > 59){
			mins++;
			secs = 0;
		}
		if(mins > 59){
			hour++;
			mins = 0;
		}
		if(hour > 23){
			hour = 0;
		}
		if(hour == A1hour && mins == A1mins && secs == A1secs && A1flag){
			A1flag = 0;
			uart_txStr("Alarm 01 occurred\r");
			
			state = 2;
			PORTC &= ~(1 << PC5);
			_delay_ms(200);
			uart_txChar('1');
			ctm = 1;
			
		}
		if(hour == A2hour && mins == A2mins && A2flag){
			A2flag = 0;
			uart_txStr("Alarm 02 occurred\r");
			
			state = 3;
			PORTC &= ~(1 << PC5);
			_delay_ms(200);
			uart_txChar('2');
			ctm = 2;
		}
		
		sprintf(buffer, "\r%02d:%02d:%02d\r", hour, mins, secs);
		uart_txStr(buffer);
	}
	else{
		scount++;
	}
	
	TCCR1B = 0;// Reset counter to 0
	
	//Set compare value to 50 000
	OCR1AH = 0xC3;
	OCR1AL = 0x50;
	
	TCCR1B |= 1<<CS10;//Start counting(Without prescaling).
	
}

void settime(int h, int m, int s){
	hour = h;
	mins = m;
	secs = s;
}

void setA1(int h, int m, int s){
	A1hour = h;
	A1mins = m;
	A1secs = s;
}

void setA2(int h, int m){
	A2hour = h;
	A2mins = m;
}

