/*
================================================================
Automated Rubber Tapping machine by Group Alpha (Group no. 29).
Tapping Machine Simulation Code
================================================================
This file includes Simulation Code for ARTM Tapping Machine
*/

#define F_CPU 1000000UL //1MHz to save power
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "artmuart.h"
#include "artmultrasonic.h"
#include "ADC.h"
#include "artmloadcell.h"
#include "motor_control.h"

char Rxchar ='\0';


int main(void)
{
	DDRA &= ~(1 << DDA0 | 1 << DDA1);// Make Temperature and soil moisture sensor pins inputs
	PORTA &= ~(1 << PA0 | 1 << PA1);// Disable pull up resisters for them
	
	DDRB = 0xFF;// Initialize PORTB for steppers
	PORTB = 0x00;//Make sure they are default 0
	
	DDRC = 0xFF;// Servos and dc motor
	PORTC = 0x00;
	
	
	DDRD = 0x49;// Initialize PORTD
	PORTD = 0x06;
	
	uart_Initialize();// Initialize USART

	MCUCR |= 1<<ISC00;// Any logic change on INT0 will interrupt
	MCUCR |= 1<<ISC10;// Any logic change on INT1 will interrupt
	sei();// Enable global interrupt bit, (I-flag)
	
	while (1)
	{
		//	Sleeping......
	}
}


void cut()
{
	PORTB |= (1<< PB1);// Start cutting head (DC motor)
	Header_Release();// Release cutting head
	while (PINC & 1 << PINC7)//Since this button is configured as open drain, when it's got pressed by moving cutting head PINC7 will become 0, So loop stop
	{
		Rotate_Stepper_motor();// Keep running both horizontals and vertical stepper motors. So overall moment will be diagonal
		
	}
	Stop_Stepper_Motor();
	Heder_Retract();// Retract cutting head
	PORTB &= ~(1<< PB1);// Stop cutting head (DC motor)
}

void get_Measure(int arr[2])// Measure temperature and soil moisture level when instructed
{
	ADC_Init();
	int temp =ADC_Read(00);
	arr[0]= temp /10;
	temp=ADC_Read(01);
	arr[1]=temp;
	ADC_Close();// To save power we close ADC circuit
}

void Collect_ready()
{
	int temp1=ReadCell();
	_delay_ms(2000);
	while(temp1-ReadCell()<-5)// Keep checking the weight of latex cup until weight change is minimum
	{
		temp1=ReadCell();
		_delay_ms(2000);
	}
	uart_txStr("COM\r"); //Then send 'C' to control unit indicating latex are ready to be collected
}

ISR(INT1_vect)// When PIR sensor detect something it'll interrupt INT1 pin. Then this function get executed
{
	PORTD |= 1 << PD7;// Turn on collecting bucket placement guide laser
	GIFR &= ~(1<<INT0);// Clear INT0 flag in case if it was set
	GICR |= 1 << INT0;
	sei();
	_delay_ms(2000);
	PORTD &= ~(1 << PD7);// Turn off collecting bucket placement guide laser
	if(Is_Bucket_Present())// This function will return 1 if bucket is present with in 15 cm
	{
		Cup_rotate();// If bucket is detected, pour latex into it
	}
	GICR &= ~(1 << INT0);
	cli();
}

ISR(USART_RXC_vect)// Since we enabled USART receive interrupt bit. When ever tapping machine receive data by USART, this function will execute
{
	Rxchar=UDR;
	if(Rxchar == '1'){
		Rxchar='\0';
		cut();//Execute cutting function
		
		GIFR &= ~(1<<INT1);// Clear INT1 flag in case if it was set
		GICR |= 1 << INT1;
		sei();
		Collect_ready();//We'll stay looping in this function till latex stop seeping. Then send "C' to control unit indicating latex are ready to be collected
		GICR &= ~(1 << INT1);
		cli();
	}
	if(Rxchar == '2'){
		Rxchar='\0';
		int Data[2];
		get_Measure(Data);// Take temperature and soil moisture measurements
		
		char arr1[20];
		sprintf(arr1, "BTT %d\r", Data[0]);
		uart_txStr(arr1);
		
		sprintf(arr1, "BTS %d\r", Data[1]);
		uart_txStr(arr1);

		
	}
}


