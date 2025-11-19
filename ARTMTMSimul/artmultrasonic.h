/*
================================================================
Automated Rubber Tapping machine by Group Alpha (Group no. 29).
Ultrasonic Module HCSR04 Header File
================================================================
This file includes Ultrasonic Module HCSR04 related codes
*/
static volatile unsigned int tCount = 0xFFFF;// Variable to hold the counter value from TCNT1.
static volatile char i = 0; //Variable to indicate the state of ultrasonic sensor. i = 1 mean sensor is measuring the distance.
char distance = 0;// A variable to store the measured distance in cm. Only need to store up to 15, so 8bit char is enough

int Is_Bucket_Present()
{
	TIMSK |= 1 << OCIE1A;// Set Timer/Counter1 Output Compare A Match Interrupt Enable bit. So ISR will run at when counter = 870 (15cm)
	//_delay_ms(10000);// Waite 10s, till bucket is placed
	
	//Send a 15us pulse to the trigger pin
	PORTD = PORTD | 1<<PD4;
	_delay_us(15);
	PORTD = PORTD & ~(1<<PD4);
	_delay_us(15);
	//Then echo pin will go hight. That interrupt will cause function ISR(INT0_vect) to get executed for the first time.
	//After detecting reflected sound wave, echo pin will go low. That interrupt will cause function ISR(INT0_vect) to get executed for the second time.
	//If echo pin didn't go low before the counter count to 870(=15cm at 1MHz), Compare A match interrupt will cause ISR(TIMER1_COMPA_vect) function to get executed.
	
	while(tCount == 0xFFFF);// Waite till tCount get read by ISR
	distance = tCount/58;//Sine CPU run at 1MHz this will store measured distance in cm.
	TIMSK &= ~(1 << OCIE1A);// Unset Timer/Counter1 Output Compare A Match Interrupt Enable bit, so no surprise interrupts
	
	tCount = 0xFFFF;// Change tCount to default value, to be check in next run for the while loop
	return (distance < 15) ? 1 : 0; // If something detected with in 15cm return 1, else return 0
}

ISR(INT0_vect)// This function will get executed whenever the micro controller is interrupted by echo pin at INT0.
{
	if(i == 0)//This part get executed when interrupted by a rising edge(for the first time, since i = 0)
	{
		OCR1AH = 0x03;// Counter value for 15cm at 1MHz. Since we only need to detect something with in range of 15cm
		OCR1AL = 0x66;//So if nothing was detected until counter count up to 0x0366(870 in decimal) we stop counting
		TCCR1B |= 1<<CS10;//Start counting(Without prescaling).
		i = 1;// So in next interrupt false part will get executed
	}
	else//This part get executed when interrupted by the falling edge(for the second time, since we set i = 1 previously)
	{
		TCCR1B = 0;// Stop counting
		tCount = TCNT1;// Take the value of counter to our variable
		TCNT1 = 0;// Reset counter to 0
		i = 0; // So in next interrupt true part will get executed
	}
}

ISR(TIMER1_COMPA_vect){// This function get executed when Timer/Counter1 Output Compare A Match the value of TCNT1
	TCCR1B = 0;// Stop counting
	tCount = 870;//SettCount to the max value that we care. Literally 870, since that's the match
	TCNT1 = 0;// Reset counter to 0
	i = 0;// So in next interrupt true part of ISR(INT0_vect) will get executed
}