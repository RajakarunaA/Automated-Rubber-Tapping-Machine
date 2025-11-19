/*
This header file includes actuator related functions of ARTM Tapping machine
*/

#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

void Cup_rotate()
{
	// Send 1.5ms pulse so servo rotate to 90 degrees
	PORTB |= 1<< PB0;
	_delay_us(2500);
	PORTB &= ~(1<< PB0);
	
	_delay_ms(60000);// Waite till latex flow out to the collecting basket
	
	// Send 1ms pulse so servo rotate to 0 degree
	PORTB |= 1<< PB0;
	_delay_us(1000);
	PORTB &= ~(1<< PB0);
}

void Header_Release()
{
	// Send 1.5ms pulse so servo rotate to 90 degree. So cutting header get released
	PORTB |= 1<< PB3;
	_delay_us(2500);
	PORTB &= ~(1<< PB3);
}

void Heder_Retract()
{
	// Send 1ms pulse so servo rotate to 0 degree retracting the cutting head
	PORTB |= 1<< PB3;
	_delay_us(1000);
	PORTB &= ~(1<< PB3);
}

void Rotate_Stepper_motor()// This same control signals are used to drive both horizontal and vertical stepper motors' drivers
{
	PORTC |= (1<<PC3) | (1<< PC6);
	_delay_ms(2000);
	PORTC &= ~(1<<PC3 | 1<< PC6);

	PORTC |= (1<<PC3) | (1<< PC4);
	_delay_ms(2000);
	PORTC &= ~(1<<PC3 | 1<< PC4);
	
	PORTC |= (1<<PC4) | (1<< PC5);
	_delay_ms(2000);
	PORTC &= ~(1<<PC4 | 1<< PC5);
	
	PORTC |= (1<<PC5) | (1<< PC6);
	_delay_ms(2000);
	PORTC &= ~(1<<PC5 | 1<< PC6);
	
}
void Stop_Stepper_Motor()
{
	PORTC = 0x03;
	_delay_ms(2000);
}


#endif /* MOTOR_CONTROL_H_ */