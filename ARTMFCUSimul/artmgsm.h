/*
================================================================
Automated Rubber Tapping machine by Group Alpha (Group no. 29).
GSM SIM800L Header File
================================================================
This file includes GSM SIM800L related functions
*/

char uartRxBuffer[255];// UART Rx data store buffer
char uartRxchar = 0;// UART Rx temp char

int uartRxPC = 0;// UART buffer current data store pointer
uint8_t NoOfIns = 0;// No of Instructions pending in the uartRxbuffer
char tempInsBuffer[10];// Temporary retrieved instruction buffer
int InsPointer = 0;// UART buffer current reading pointer


char* masterP = "+94123456789";



void gsmInitialize(){
	_delay_ms(1000);
	//Following settings only need to be run once since we save them to the NVRAM. So they won't change after reboots
	uart_txStr("AT+IPR=4800\r");// Set baud rate to 4800
	uart_txStr("AT&w\r");// // Save setting to NVRAM
	uart_txStr("ATV0\r");// Change the format of GSM responses
	uart_txStr("AT&w\r");// // Save setting to NVRAM
	uart_txStr("AT+CLTS=1\r");// Enable time sync from GSM network
	uart_txStr("AT&w\r");// Save setting to NVRAM
	uart_txStr("AT+CMGF=1\r");// Change message mode to text
	uart_txStr("AT&w\r");// Save setting to NVRAM
	uart_txStr("AT+CNMI=1,2,0,0,0\r");// Change message mode to text
	uart_txStr("AT&w\r");// Save setting to NVRAM
	//End of factory set settings
	
	uart_txStr("AT+CCLK?\r");// Sync time from GSM network
	
}

void gsmSendSMS(char* txt){
	char tbuffer[30];
	sprintf(tbuffer, "AT+CMGS=\"%s\"\r", masterP);// Add number to the command and make a string
	uart_txStr(tbuffer);// Send command string
	_delay_ms(100);
	uart_txStr(txt);// Send SMS body text
	_delay_ms(10);
	uart_txChar(0x1A);// Send "Ctr+Z" (Substitutes character) to indicate end of SMS body
}

ISR(USART_RXC_vect){// When there are received UART data
	uartRxchar = UDR;// Store UDR data in temp char
	if(uartRxchar == ']'){// doing this for simulation purpose since we can't enter '\r' in virtual terminal
		uartRxchar = '\r';
	}
	if(uartRxchar != '\n'){// Just discard '\n' characters
		if(uartRxchar == '\r'){// if '\r' detected increase number of pending instructions
			NoOfIns++;
		}
		uartRxBuffer[uartRxPC] = uartRxchar;
		if (uartRxPC < 254){// Increase RX data store pointer. If it's max make it zero so it wrap around back to start of buffer
			uartRxPC++;
		}
		else{
			uartRxPC = 0;
		}
	}
}

void increaseInsP(){
	if (InsPointer < 254){
		InsPointer++;
	}
	else{
		InsPointer = 0;
	}
}

void gsmSMSdecoder(char* code){
	char tc[3]= {code[0], code[1], '\0'};
	if(!strcmp(tc, "ST")){
		uart_txStr("Setting Time..\r");
		char th[3][3] ={{code[12], code[13], '\0'}, {code[15], code[16], '\0'}, {code[18], code[19], '\0'}};
		settime(atoi(&th[0][0]), atoi(&th[1][0]), atoi(&th[2][0]));
		char gbuff[50];
		sprintf(gbuff, "Time set at %02d:%02d:%02d\r", hour, mins, secs);
		uart_txStr(gbuff);
	}
	
	if(!strcmp(tc, "A1")){
		uart_txStr("Setting Alarm 1..\r");
		char th[3][3] ={{code[3], code[4], '\0'}, {code[6], code[7], '\0'}, {code[9], code[10], '\0'}};
		setA1(atoi(&th[0][0]), atoi(&th[1][0]), atoi(&th[2][0]));
		char gbuff[50];
		sprintf(gbuff, "Alarm 1 set at %02d:%02d:%02d\r", A1hour, A1mins, A1secs);
		uart_txStr(gbuff);
	}
	
	if(!strcmp(tc, "A2")){
		uart_txStr("Setting Alarm 2..\r");
		char th[2][3] ={{code[3], code[4], '\0'}, {code[6], code[7], '\0'}};
		setA2(atoi(&th[0][0]), atoi(&th[1][0]));
		char gbuff[50];
		sprintf(gbuff, "Alarm 2 set at %02d:%02d\r", A2hour, A2mins);
		uart_txStr(gbuff);
	}
	if(!strcmp(code, "WT")){
		char gbuff[50];
		sprintf(gbuff, "Time is %02d:%02d:%02d", hour, mins, secs);
		gsmSendSMS(gbuff);
	}
}

void gsmrxmsg(){
	while(NoOfIns){
		uint8_t i = 0;
		while(uartRxBuffer[InsPointer] != ' ' && uartRxBuffer[InsPointer] != '\r'){// Retrieve first part of the message from GSM
			tempInsBuffer[i] = uartRxBuffer[InsPointer];
			increaseInsP();
			i++;
		}
		tempInsBuffer[i] = '\0';
		if(uartRxBuffer[InsPointer] == '\r'){
			NoOfIns--;
		}
		increaseInsP();
		
		//Then execute relevant actions according to the code
		if(!strcmp(tempInsBuffer, "SMS")){// When GSM boot completed
			//GSM up
			state = 1;
			for (int y = 0; y < 6; y++){
				increaseInsP();
			}
			NoOfIns--;
			continue;
		}
		
		if(!strcmp(tempInsBuffer, "+CMT:")){// When SMS received
			while(uartRxBuffer[InsPointer] != '\r'){// Skip other informations
				increaseInsP();
			}
			_delay_ms(1000);
			increaseInsP();
			NoOfIns--;
			char smscodeBuffer[20];
			i = 0;
			while(uartRxBuffer[InsPointer] != '\r'){
				smscodeBuffer[i] = uartRxBuffer[InsPointer];
				increaseInsP();
				i++;
			}
			NoOfIns--;
			increaseInsP();
			smscodeBuffer[i] = '\0';
			
			gsmSMSdecoder(smscodeBuffer);
			continue;
		}
		
		if(!strcmp(tempInsBuffer, "+CCLK:")){// When we requested a time sync
			
			char returntimeBuffer[25];
			i = 0;
			while(uartRxBuffer[InsPointer] != '\r'){
				returntimeBuffer[i] = uartRxBuffer[InsPointer];
				increaseInsP();
				i++;
			}
			NoOfIns--;
			increaseInsP();
			returntimeBuffer[i] = '\0';
			uart_txStr("Syncing Time..\r");
			char th[3][3] ={{returntimeBuffer[10], returntimeBuffer[11], '\0'}, {returntimeBuffer[13], returntimeBuffer[14], '\0'}, {returntimeBuffer[16], returntimeBuffer[17], '\0'}};
			settime(atoi(&th[0][0]), atoi(&th[1][0]), atoi(&th[2][0]));
			continue;
		}
		
		if(!strcmp(tempInsBuffer, "BTT")){// Got temp data
			char tempBuffer[25];
			i = 0;
			while(uartRxBuffer[InsPointer] != '\r'){
				tempBuffer[i] = uartRxBuffer[InsPointer];
				increaseInsP();
				i++;
			}
			tempBuffer[i] = '\0';
			eeprom_write_float((float*)20, atof(tempBuffer)); 
			NoOfIns--;
			continue;
		}
		
		if(!strcmp(tempInsBuffer, "BTS")){// Got soil moisture data
			state = 1;
			char tempBuffer[25];
			i = 0;
			while(uartRxBuffer[InsPointer] != '\r'){
				tempBuffer[i] = uartRxBuffer[InsPointer];
				increaseInsP();
				i++;
			}
			tempBuffer[i] = '\0';
			eeprom_write_float((float*)40, atof(tempBuffer));
			NoOfIns--;
			continue;
		}
		
		if(!strcmp(tempInsBuffer, "COM")){// Got soil moisture data
			state = 1;
			NoOfIns--;
			continue;
		}
	}
}

