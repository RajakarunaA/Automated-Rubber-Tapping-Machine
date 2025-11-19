/*
================================================================
Automated Rubber Tapping machine by Group Alpha (Group no. 29).
LoadCell and HX711 Header File
================================================================
This file includes LoadCell and HX711 related codes
*/
unsigned long ReadCell(){
	unsigned long Count;// Since we are storing a 24bit value
	unsigned char i;
	
	DDRC = DDRC | 1 << DDC1;// Make data pin output
	PORTC = PORTC | 1 << PC1;// Make data pin high
	PORTC = PORTC & ~(1 << PC0);// Make clock pin low
	Count=0;
	
	DDRC = DDRC & ~(1 << DDC1);// Make data pin input
	while(PINC & 1 << PC1);// Waite until data pin goes low. Which means data can be retrieved
	// Then send 24 clock pulse and get 24 bit data bit by bit
	for (i=0;i<24;i++){
		PORTC = PORTC | 1 << PC0;// Make clock pin High
		Count=Count<<1;// Left shift current Count value by 1, so we can store about to receive bit at the right most bit (LSB) of Count variable
		PORTC = PORTC & ~(1 << PC0);// Make clock pin Low
		if(!(PINC & 1 << PC1)) Count++;// If data pin is low, we make right most bit LSB 0. Since data is received in 2's complement format we store the negation of received data
	}
	//Send final clock pulse (25th), so data pin will be back to high
	PORTC = PORTC | 1 << PC0;
	_delay_us(1);
	PORTC = PORTC & ~(1 << PC0);
	Count = Count + 1;// add 1 to the negation of received value. So now we have data in binary format
	return(Count/20969);// return read data
}


