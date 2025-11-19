/*
================================================================
Automated Rubber Tapping machine by Group Alpha (Group no. 29).
UART Header File
================================================================
This file includes UART related functions
*/

void uart_Initialize(){// Initialize UART according to our project requirements
	UBRRH=00;
	UBRRL=12;// Set baud rate 4800bps (At 1MHz)
	UCSRB|=(1<<RXCIE) | (1<<RXEN) | (1<<TXEN);// Enable RX interrupt, RX and TX
	UCSRC=(1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0) ;// Set UART data frame to 8 bit
}

void uart_txChar(char data)// Send a char over UART
{
	while (!( UCSRA & (1<<UDRE)));// wait for the register to free from data
	UDR = data; // load data in the register
}

void uart_txStr(char* b){// Send a null terminated string over UART
	while(*b != '\0'){
		uart_txChar(*b++);
	}
}

void uart_txByte(int t){// Convert a number to a string and send via UART
	char a[9];
	itoa(t, a, 10);
	uart_txStr(a);
	
}