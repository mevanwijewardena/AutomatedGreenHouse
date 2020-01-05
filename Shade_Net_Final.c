#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz clock speed
#endif
#define D4 eS_PORTC2
#define D5 eS_PORTC3
#define D6 eS_PORTC4
#define D7 eS_PORTC5
#define RS eS_PORTC6
#define EN eS_PORTC7

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include "lcd.h" //Can be download from the bottom of this article
#define DHT11_PIN 6
uint8_t Water=0;
float Humidity=75.0;
float Soil_hum=50.0;
int processor_state=0;
int prev_state=-1;
int adc_value;
int lux_val;
uint8_t c=0,I_RH=0,D_RH=0,I_Temp=0,D_Temp=0,CheckSum=0;
float moisture=0;
float light=0;
void ADC_Init()
{
	DDRA|=0x0;		/*  Make ADC port as input  */
	ADCSRA = 0x87;		/*  Enable ADC, fr/128  */
}
int ADC_Read(int a)
{
	if(a==0){
		ADMUX = 0x40;		/* Vref: Avcc, ADC channel: 0  */
		ADCSRA |= (1<<ADSC);
		while ((ADCSRA &(1<<ADIF))==0);	/* monitor end of conversion interrupt flag */
		ADCSRA |=(1<<ADIF);	/* set the ADIF bit of ADCSRA register */
		return(ADCW);		/* return the ADCW */
	}
	else{
		ADMUX = 0x42;		/* Vref: Avcc, ADC channel: 0  */
		ADCSRA |= (1<<ADSC);
		while ((ADCSRA &(1<<ADIF))==0);	/* monitor end of conversion interrupt flag */
		ADCSRA |=(1<<ADIF);	/* set the ADIF bit of ADCSRA register */
		return(ADCW);		/* return the ADCW */
	}
	
}
void water(){
	ADC_Init();
	adc_value = ADC_Read(0);	/* Copy the ADC value */
	moisture = 100-(adc_value*100.00)/1023.00;
	float p=((moisture-Soil_hum)/Soil_hum)*(100.0);
	ADC_Init();
	adc_value = ADC_Read(1);
	light=(adc_value*100.00)/1023.00;
	if(p>5){
		DDRA|=(1<<3);
		PORTA&=~(1<<3);
		DDRA|=(1<<4);
		PORTA&=~(1<<4);
	}
	else if(p<-5){
		DDRA|=(1<<3);
		PORTA|=(1<<3);
		DDRA|=(1<<4);
		PORTA|=(1<<4);
	}
}
void Request()
{
	DDRA |= (1<<DHT11_PIN);
	PORTA |= (1<<DHT11_PIN); /* add this line */
	_delay_ms(100); /* add this line */
	PORTA &= ~(1<<DHT11_PIN);	/* set to low pin */
	_delay_ms(20);			/* wait for 20ms */
	PORTA |= (1<<DHT11_PIN);	/* set to high pin */
}

void Response()				/* receive response from DHT11 */
{
	//Lcd4_Set_Cursor(1,1);
	DDRA &= ~(1<<DHT11_PIN);
	//PORTB &= ~(1<<DHT11_PIN);
	while((PINA & (1<<DHT11_PIN)));
	//Lcd4_Write_String("Done");
	while((PINA & (1<<DHT11_PIN))==0);
	while((PINA & (1<<DHT11_PIN)));
}
uint8_t Receive_data()			/* receive data */
{
	for (int q=0; q<8; q++)
	{
		while((PINA & (1<<DHT11_PIN)) == 0);  /* check received bit 0 or 1 */
		_delay_us(30);
		if(PINA & (1<<DHT11_PIN))/* if high pulse is greater than 30ms */
		c = (c<<1)|(0x01);	/* then its logic HIGH */
		else			/* otherwise its logic LOW */
		c = (c<<1);
		while(PINA & (1<<DHT11_PIN));
	}
	return c;
}
void humidify(){
	Request();		/* send start pulse */
	Response();		/* receive response */
	//sprintf(message3, "Humidity: %d.%d %",I_RH,D_RH );
	//Lcd4_Write_String(message3);
	
	I_RH=Receive_data();	/* store first eight bit in I_RH */
	D_RH=Receive_data();	/* store next eight bit in D_RH */
	I_Temp=Receive_data();	/* store next eight bit in I_Temp */
	D_Temp=Receive_data();	/* store next eight bit in D_Temp */
	CheckSum=Receive_data();/* store next eight bit in CheckSum */
	/*Lcd4_Set_Cursor(2,1);
	Lcd4_Write_String("System booting");*/
	
	if ((I_RH + D_RH + I_Temp + D_Temp) == CheckSum)
	{
		
		float p=((((float) I_RH)-(Humidity))/(Humidity))*100;
		if(p<-5){
			DDRA|=0x02;;
			PORTA|=0x02;
		}
		else if(p>5){
			DDRA|=0x02;;
			PORTA &=~(0x02);
		}
	}
}
int rowdet(){
	DDRD=0xF0;//taking column pins as input and row pins as output
	_delay_ms(1);
	PORTD=0x0F;// powering the row ins
	_delay_ms(1);
	if (PIND!=0b00001111){
		return 1;
	}
	return 0;
}
char  switchcr(){
	int keypressed = PIND;
	_delay_ms(5);
	DDRD ^=0b11111111;//making rows as inputs and columns as output
	_delay_ms(1);
	PORTD ^= 0b11111111;//powering columns
	_delay_ms(1);
	keypressed |=PIND;
	if(keypressed==0x77){
		return '1';
	}
	else if(keypressed==0x7B){
		return '2';
	}
	else if(keypressed==0x7D){
		return '3';
	}
	else if(keypressed==0x7E){
		return 'A';
	}
	else if(keypressed==0xB7){
		return '4';
	}
	else if(keypressed==0xBB){
		return '5';
	}
	else if(keypressed==0xBD){
		return '6';
	}
	else if(keypressed==0xBE){
		return 'B';
	}
	else if(keypressed==0xD7){
		return '7';
	}
	else if(keypressed==0xDB){
		return '8';
	}
	else if(keypressed==0xDD){
		return '9';
	}
	else if(keypressed==0xDE){
		return 'C';
	}
	else if(keypressed==0xE7){
		return '*';
	}
	else if(keypressed==0xEB){
		return '0';
	}
	else if(keypressed==0xED){
		return '#';
	}
	else if(keypressed==0xEE){
		return 'D';
	}
}
void getnum(int ty){
	int pos=0;
	int temp=0;
	int br=0;
	Lcd4_Set_Cursor(1,1);
	while(1){
		for(int ti=0;ti<10;ti++){
			_delay_ms(220);
			int m1=rowdet();
			if(m1){
				if(pos==0){
					Lcd4_Clear();
				}
				char keyp=switchcr();
				if(keyp=='1' && pos<2){
					temp=(10*temp)+1;
					Lcd4_Set_Cursor(1,pos+1);
					Lcd4_Write_Char(keyp);
					pos++;
				}
				else if(keyp=='2' && pos<2){
					temp=(10*temp)+2;
					Lcd4_Set_Cursor(1,pos+1);
					Lcd4_Write_Char(keyp);
					pos++;
				}
				else if(keyp=='3' && pos<2){
					temp=(10*temp)+3;
					Lcd4_Set_Cursor(1,pos+1);
					Lcd4_Write_Char(keyp);
					pos++;
				}
				else if(keyp=='4' && pos<2){
					temp=(10*temp)+4;
					Lcd4_Set_Cursor(1,pos+1);
					Lcd4_Write_Char(keyp);
					pos++;
				}
				else if(keyp=='5' && pos<2){
					temp=(10*temp)+5;
					Lcd4_Set_Cursor(1,pos+1);
					Lcd4_Write_Char(keyp);
					pos++;
				}
				else if(keyp=='6' && pos<2){
					temp=(10*temp)+6;
					Lcd4_Set_Cursor(1,pos+1);
					Lcd4_Write_Char(keyp);
					pos++;
				}
				else if(keyp=='7' && pos<2){
					temp=(10*temp)+7;
					Lcd4_Set_Cursor(1,pos+1);
					Lcd4_Write_Char(keyp);
					pos++;
				}
				else if(keyp=='8' && pos<2){
					temp=(10*temp)+8;
					Lcd4_Set_Cursor(1,pos+1);
					Lcd4_Write_Char(keyp);
					pos++;
				}
				else if(keyp=='9' && pos<2){
					temp=(10*temp)+9;
					Lcd4_Set_Cursor(1,pos+1);
					Lcd4_Write_Char(keyp);
					pos++;
				}
				else if(keyp=='0' && pos<2){
					temp=(10*temp);
					Lcd4_Set_Cursor(1,pos+1);
					Lcd4_Write_Char(keyp);
					pos++;
				}
				else if(keyp=='B'){
					prev_state=processor_state;
					processor_state=1;
					return;
				}
				if(ty==0 &&  keyp=='A'){
					Soil_hum=(float) temp;
					prev_state=processor_state;
					processor_state=1;
					return ;
				}
				else if(ty==1 && keyp=='A'){
					Humidity=(float) temp;
					prev_state=processor_state;
					processor_state=1;
					return ;
				}
				
			}
		}
		water();
		humidify();
	}
}
int main(void)
{
	MCUCSR = (1<<JTD);
	MCUCSR = (1<<JTD);
	DDRC = 0xFF;
	DDRA|=0x02;
	Lcd4_Init();
	Lcd4_Clear();
	Lcd4_Set_Cursor(1,1);
	Lcd4_Write_String("System booting");
	_delay_ms(2500);
	water();
	humidify();
	while(1){
		if(processor_state==0){
			if(prev_state!=processor_state){
				Lcd4_Clear();
				Lcd4_Set_Cursor(1,1);
				Lcd4_Write_String("C:Set levels");
				Lcd4_Set_Cursor(2,1);
				Lcd4_Write_String("D:Display values");
			}
			int br=0;
			while(1){
				for(int ti=0;ti<10;ti++){
					_delay_ms(220);
					int m1=rowdet();
					if(m1){
						char keyp=switchcr();
						if(keyp=='C'){
							prev_state=processor_state;
							processor_state=1;
							br=1;
							break;
						}
						else if(keyp=='D'){
							prev_state=processor_state;
							processor_state=2;
							br=1;
							break;
						}
					}
					
				}
				if(br==1){
					break;
				}
				water();
				humidify();
				
			}
		}
		else if(processor_state==1){
			if(processor_state!=prev_state){
				Lcd4_Clear();
				Lcd4_Set_Cursor(1,1);
				Lcd4_Write_String("A:Water");
				Lcd4_Set_Cursor(2,1);
				Lcd4_Write_String("D:Humi,B:Back");
			}
			int br=0;
			while(1){
				for(int ti=0;ti<10;ti++){
					_delay_ms(220);
					int m1=rowdet();
					if(m1){
						char keyp=switchcr();
						if(keyp=='A'){
							prev_state=processor_state;
							processor_state=3;
							br=1;
							break;
						}
						else if(keyp=='D'){
							prev_state=processor_state;
							processor_state=4;
							br=1;
							break;
						}
						else if(keyp=='B'){
							prev_state=processor_state;
							processor_state=0;
							br=1;
							break;
						}
						
					}
				}
				if(br==1){
					break;
				}
				water();
				humidify();
				
			}
		}
		else if(processor_state==2){
			if(processor_state!=prev_state){
				Lcd4_Clear();
				Lcd4_Set_Cursor(1,1);
				Lcd4_Write_String("A:Moi,*:H/T");
				Lcd4_Set_Cursor(2,1);
				Lcd4_Write_String("#:Li,B:Back");
			}
			int br=0;
			while(1){
				for(int ti=0;ti<10;ti++){
					_delay_ms(220);
					int m1=rowdet();
					if(m1){
						char keyp=switchcr();
						if(keyp=='A'){
							prev_state=processor_state;
							processor_state=5;
							br=1;
							break;
						}
						else if(keyp=='*'){
							prev_state=processor_state;
							processor_state=6;
							br=1;
							break;
						}
						else if(keyp=='#'){
							prev_state=processor_state;
							processor_state=7;
							br=1;
							break;
						}
						else if(keyp=='B'){
							prev_state=processor_state;
							processor_state=0;
							br=1;
							break;
						}
					}
				}
				if(br==1){
					break;
				}
				water();
				humidify();
			}
			
		}
		else if(processor_state==3){
			Lcd4_Clear();
			Lcd4_Set_Cursor(1,1);
			Lcd4_Write_String("Enter the value");
			Lcd4_Set_Cursor(2,1);
			char array[16];
			Lcd4_Set_Cursor(2,1);
			dtostrf(Soil_hum,3,2,array);
			strcat(array,"%   ");	/* Concatenate unit of % */
			Lcd4_Set_Cursor(2,1);	/* set column and row */
			Lcd4_Write_String(array);
			getnum(0);
		}
		else if(processor_state==4){
			Lcd4_Clear();
			Lcd4_Set_Cursor(1,1);
			Lcd4_Write_String("Enter the value");
			char array[16];
			Lcd4_Set_Cursor(2,1);
			dtostrf(Humidity,3,2,array);
			strcat(array,"%   ");	/* Concatenate unit of % */
			Lcd4_Set_Cursor(2,1);	/* set column and row */
			Lcd4_Write_String(array);
			getnum(1);
		}
		else if(processor_state==5){
			int br=0;
			Lcd4_Clear();
			while(1){
				Lcd4_Set_Cursor(1,1);
				Lcd4_Write_String("Moisture: ");
				char array[16];
				dtostrf(moisture,3,2,array);
				strcat(array,"%   ");	/* Concatenate unit of % */
				Lcd4_Set_Cursor(2,1);	/* set column and row */
				Lcd4_Write_String(array);	/* Print moisture on second row */
				memset(array,0,10);
				for(int ti=0;ti<10;ti++){
					_delay_ms(220);
					int m1=rowdet();
					if(m1){
						char keyp=switchcr();
						if(keyp=='B'){
							prev_state=processor_state;
							processor_state=2;
							br=1;
							break;
						}
						
					}
				}
				if(br==1){
					break;
				}
				water();
				humidify();
			}
			
		}
		else if(processor_state==6){
			int br=0;
			Lcd4_Clear();
			while(1){
					Lcd4_Set_Cursor(1,1);
					char *message3[16];
					sprintf(message3, "Humidity:%d.%d%",I_RH,D_RH );
					Lcd4_Write_String(message3);
					Lcd4_Set_Cursor(2,1);
					char *message4[16];
					sprintf(message4, "Temp:%d.%dC",I_Temp,D_Temp );
					Lcd4_Write_String(message4);
				for(int ti=0;ti<10;ti++){
					_delay_ms(220);
					int m1=rowdet();
					if(m1){
						char keyp=switchcr();
						if(keyp=='B'){
							prev_state=processor_state;
							processor_state=2;
							br=1;
							break;
						}
						
					}
				}
				if(br==1){
					break;
				}
				water();
				humidify();
			}
			
		}
		else if(processor_state==7){
			int br=0;
			Lcd4_Clear();
			while(1){
				Lcd4_Set_Cursor(1,1);
				Lcd4_Write_String("Light: ");
				char array[16];
				dtostrf(light,3,2,array);
				strcat(array,"%   ");	/* Concatenate unit of % */
				Lcd4_Set_Cursor(2,1);	/* set column and row */
				Lcd4_Write_String(array);	/* Print moisture on second row */
				memset(array,0,10);
				for(int ti=0;ti<10;ti++){
					_delay_ms(220);
					int m1=rowdet();
					if(m1){
						char keyp=switchcr();
						if(keyp=='B'){
							prev_state=processor_state;
							processor_state=2;
							br=1;
							break;
						}
						
					}
				}
				if(br==1){
					break;
				}
				water();
				humidify();
			}
			
			
		}
		
	}
	return 0;
}
