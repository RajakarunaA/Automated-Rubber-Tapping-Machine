/*
This header file includes ADC related functions of ARTM Tapping Machine
*/

void ADC_Init()
{
	ADCSRA = 0x87;//Enable ADC, with freq/128
	ADMUX = 0xC0;//Vref: Avcc, ADC channel: 0
}

int ADC_Read(char channel)
{
	ADMUX = 0xC0 | (channel & 0x07); //set input channel to read
	ADCSRA |= (1<<ADSC);             //Start ADC conversion
	while (!(ADCSRA & (1<<ADIF)));   //Wait until end of conversion by polling ADC interrupt flag
	ADCSRA |= (1<<ADIF);             //Clear interrupt flag
	_delay_ms(1);                    //Wait a little bit
	return ADCW*2.56;                //Return ADC word
}

void ADC_Close()// Turn off ADC circuite to save power
{
	ADCSRA &= ~(1<<ADEN);
}

