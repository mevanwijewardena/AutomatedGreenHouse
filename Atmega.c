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
int adc_value;
float lux_val;
uint8_t c=0,I_RH=0,D_RH=0,I_Temp=0,D_Temp=0,CheckSum=0;
float moisture;
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
int  switchcr(){
	int keypressed = PIND;
	 _delay_ms(5);
	DDRD ^=0b11111111;//making rows as inputs and columns as output
	_delay_ms(1);
	PORTD ^= 0b11111111;//powering columns
	_delay_ms(1);
	keypressed |=PIND;
	return keypressed;
}
int main(void)
{
    MCUCSR = (1<<JTD);
    MCUCSR = (1<<JTD);
    DDRC = 0xFF;
	DDRA|=0x02;;
    uint8_t Water=0;
    uint8_t Humidity=0;
    uint8_t Temperature=0;
	uint8_t keypressed=0;
	uint8_t Fertilizer=0;
	int i;
    Lcd4_Init();
    Lcd4_Set_Cursor(1,1);
    Lcd4_Write_String("C:Set levels");
	/*for(i=0;i<5;i++)
	{
		_delay_ms(50);
		Lcd4_Shift_Left();
	}
	for(i=0;i<5;i++)
	{
		_delay_ms(50);
		Lcd4_Shift_Right();
	}*/
    Lcd4_Set_Cursor(2,1);
    Lcd4_Write_String("D:Display values");
	int processor_state=0;
	int prev_state=0;
    while(1){
		while(1){
			_delay_ms(1500);
			Request();		/* send start pulse */
			Response();		/* receive response */
			//sprintf(message3, "Humidity: %d.%d %",I_RH,D_RH );
			//Lcd4_Write_String(message3);
			
			I_RH=Receive_data();	/* store first eight bit in I_RH */
			D_RH=Receive_data();	/* store next eight bit in D_RH */
			I_Temp=Receive_data();	/* store next eight bit in I_Temp */
			D_Temp=Receive_data();	/* store next eight bit in D_Temp */
			CheckSum=Receive_data();/* store next eight bit in CheckSum */
			
			if ((I_RH + D_RH + I_Temp + D_Temp) == CheckSum)
			{
				if(I_RH >84){
					
					PORTA|=0x02;
				}
				else if(I_RH<80){
					PORTA &=~(0x02);
				}
				break;
				

			}
			
		}
		if(processor_state==0){
			if(prev_state!=processor_state){
				Lcd4_Clear();
				Lcd4_Set_Cursor(1,1);
				Lcd4_Write_String("C:Set levels");
				Lcd4_Set_Cursor(2,1);
				Lcd4_Write_String("D:Display values");
			}
			while(1){
				int m1=rowdet();
				 if(m1){
					 keypressed=switchcr();
					 if(keypressed==0xDE){
						 prev_state=processor_state;
						 processor_state=1;
					 }
					 else if(keypressed==0xEE){
						 prev_state=processor_state;
						 processor_state=2;
					 }
					 else{
						 prev_state=processor_state;
						 processor_state=0;
					 }
					 _delay_ms(220);
					 break;
				 }
			   _delay_ms(700);
			   m1=rowdet();
			   if(m1){
				   keypressed=switchcr();
				   if(keypressed==0xDE){
					   prev_state=processor_state;
					   processor_state=1;
				   }
				   else if(keypressed==0xEE){
					   prev_state=processor_state;
					   processor_state=2;
				   }
				   else{
					   prev_state=processor_state;
					   processor_state=0;
				   }
				   _delay_ms(220);
				   break;
			   }
			   _delay_ms(700);
			   Request();		/* send start pulse */
			   Response();		/* receive response */
			   //sprintf(message3, "Humidity: %d.%d %",I_RH,D_RH );
			   //Lcd4_Write_String(message3);
			   
			   I_RH=Receive_data();	/* store first eight bit in I_RH */
			   D_RH=Receive_data();	/* store next eight bit in D_RH */
			   I_Temp=Receive_data();	/* store next eight bit in I_Temp */
			   D_Temp=Receive_data();	/* store next eight bit in D_Temp */
			   CheckSum=Receive_data();/* store next eight bit in CheckSum */
			   
			   if ((I_RH + D_RH + I_Temp + D_Temp) == CheckSum)
			   {
				   
				   if(I_RH >84){
					   PORTA|=0x02;
				   }
				   else if(I_RH<80){
					   PORTA &=~(0x02);
				   }
				   

			   }
			   
			 
			 
			  
			}
				
		}
		else if(processor_state==1){
		  if(processor_state!=prev_state){
			Lcd4_Clear();
			Lcd4_Set_Cursor(1,1);
			Lcd4_Write_String("A:Water");
			Lcd4_Set_Cursor(2,1);
			Lcd4_Write_String("C:Fertilizer");
			_delay_ms(2000);
			Lcd4_Clear();
			Lcd4_Set_Cursor(1,1);
			Lcd4_Write_String("D:Humidity");
			Lcd4_Set_Cursor(2,1);
			Lcd4_Write_String("B:Back");
			_delay_ms(2000);
		  }
		  int m1=rowdet();
		  while(1){
		    if(m1){
				keypressed=switchcr();
				if(keypressed==0x7E){
					prev_state=processor_state;
					processor_state=3;
				}
				else if(keypressed==0xDE){
					prev_state=processor_state;
					processor_state=4;
				}
				else if(keypressed==0xEE){
					prev_state=processor_state;
					processor_state=5;
				}
				else if(keypressed==0xBE){
					prev_state=processor_state;
					processor_state=0;
				}
				else{
					prev_state=processor_state;
					processor_state=1;
				}
				_delay_ms(220);
				break;
			}
			_delay_ms(220);
			m1=rowdet();
			 
		  }
			
		}
		else if(processor_state==2){
			if(processor_state!=prev_state){
				Lcd4_Clear();
				Lcd4_Set_Cursor(1,1);
				Lcd4_Write_String("A:Water");
				Lcd4_Set_Cursor(2,1);
				Lcd4_Write_String("C:Fertilizer");
				_delay_ms(2000);
				Lcd4_Clear();
				Lcd4_Set_Cursor(1,1);
				Lcd4_Write_String("D:Soil Humidity");
				Lcd4_Set_Cursor(2,1);
				Lcd4_Write_String("B:Back");
				_delay_ms(2000);
				Lcd4_Clear();
				Lcd4_Set_Cursor(1,1);
				Lcd4_Write_String("*:Hum/Temp");
				Lcd4_Set_Cursor(2,1);
				Lcd4_Write_String("#:Light");
			}
			int m1=rowdet();
			while(1){
				if(m1){
					keypressed=switchcr();
					if(keypressed==0x7E){
						prev_state=processor_state;
						processor_state=6;
					}
					else if(keypressed==0xDE){
						prev_state=processor_state;
						processor_state=7;
					}
					else if(keypressed==0xEE){
						prev_state=processor_state;
						processor_state=8;
					}
					else if(keypressed==0xBE){
						prev_state=processor_state;
						processor_state=0;
					}
					else if(keypressed==0xE7){
						prev_state=processor_state;
						processor_state=9;
					}
					else if(keypressed==0xED){
						prev_state=processor_state;
						processor_state=10;
					}
					else{
						prev_state=processor_state;
						processor_state=2;
					}
					_delay_ms(220);
					break;
				}
				_delay_ms(220);
				m1=rowdet();
				
			}
			
			   
				
		}
		else if(processor_state==3){
			 Lcd4_Clear();
			 Lcd4_Set_Cursor(1,1);
			 Lcd4_Write_String("Enter the value");
			 Lcd4_Set_Cursor(2,1);
			 int pos=0;
			 int m1=rowdet();
			 int temp=0;
			 while(pos<5){
				 if(m1){
					 keypressed=switchcr();
					 Lcd4_Set_Cursor(2,1+pos);
					 pos++;
					 if(pos!=5 && keypressed==0x77){
						 Lcd4_Write_Char('1');
						 temp=(10*temp)+1;
					 }
					 else if(pos!=5 && keypressed==0x7B){
						 Lcd4_Write_Char('2');
						 temp=(10*temp)+2;
					 }
					 else if(pos!=5 && keypressed==0x7D){
						 Lcd4_Write_Char('3');
						 temp=(10*temp)+3;
					 }
					 else if(pos!=5 && keypressed==0xB7){
						 Lcd4_Write_Char('4');
						 temp=(10*temp)+4;
					 }
					 else if(pos!=5 && keypressed==0xBB){
						 Lcd4_Write_Char('5');
						 temp=(10*temp)+5;
					 }
					 else if(pos!=5 && keypressed==0xBD){
						 Lcd4_Write_Char('6');
						 temp=(10*temp)+6;
					 }
					 else if(pos!=5 && keypressed==0xD7){
						 Lcd4_Write_Char('7');
						 temp=(10*temp)+7;
					 }
					 else if(pos!=5 && keypressed==0xDB){
						 Lcd4_Write_Char('8');
						 temp=(10*temp)+8;
					 }
					 else if(pos!=5 && keypressed==0xDD){
						 Lcd4_Write_Char('9');
						 temp=(10*temp)+9;
					 }
					 else if(pos!=5 && keypressed==0xEB){
						 Lcd4_Write_Char('0');
					 }
					 else if(keypressed==0xBE){
						 prev_state=processor_state;
						 processor_state=1;
						 break;
					 }
					 else if(keypressed==0x7E){
						 prev_state=processor_state;
						 processor_state=0;
						 Water=temp;
						 break;
					 }
					 else{
						 prev_state=processor_state;
						 processor_state=3;
						 break;
					 } 
					 
						 
				 }
				  _delay_ms(220);
				 m1=rowdet();
			 }
		}
		else if(processor_state==4){
				Lcd4_Clear();
				Lcd4_Set_Cursor(1,1);
				Lcd4_Write_String("Enter the value");
				Lcd4_Set_Cursor(2,1);
				int pos=0;
				int m1=rowdet();
				int temp=0;
				while(pos<5){
					if(m1){
						keypressed=switchcr();
						Lcd4_Set_Cursor(2,1+pos);
						pos++;
						if(pos!=5 && keypressed==0x77){
							Lcd4_Write_Char('1');
							temp=(10*temp)+1;
						}
						else if(pos!=5 && keypressed==0x7B){
							Lcd4_Write_Char('2');
							temp=(10*temp)+2;
						}
						else if(pos!=5 && keypressed==0x7D){
							Lcd4_Write_Char('3');
							temp=(10*temp)+3;
						}
						else if(pos!=5 && keypressed==0xB7){
							Lcd4_Write_Char('4');
							temp=(10*temp)+4;
						}
						else if(pos!=5 && keypressed==0xBB){
							Lcd4_Write_Char('5');
							temp=(10*temp)+5;
						}
						else if(pos!=5 && keypressed==0xBD){
							Lcd4_Write_Char('6');
							temp=(10*temp)+6;
						}
						else if(pos!=5 && keypressed==0xD7){
							Lcd4_Write_Char('7');
							temp=(10*temp)+7;
						}
						else if(pos!=5 && keypressed==0xDB){
							Lcd4_Write_Char('8');
							temp=(10*temp)+8;
						}
						else if(pos!=5 && keypressed==0xDD){
							Lcd4_Write_Char('9');
							temp=(10*temp)+9;
						}
						else if(pos!=5 && keypressed==0xEB){
							Lcd4_Write_Char('0');
						}
						else if(keypressed==0xBE){
							prev_state=processor_state;
							processor_state=1;
							break;
						}
						else if(keypressed==0x7E){
							prev_state=processor_state;
							processor_state=0;
							Fertilizer=temp;
							break;
						}
						else{
							prev_state=processor_state;
							processor_state=3;
							break;
						}
						
						
					}
					 _delay_ms(220);
					m1=rowdet();
				}
			
		}
		else if(processor_state==5){
				Lcd4_Clear();
				Lcd4_Set_Cursor(1,1);
				Lcd4_Write_String("Enter the value");
				Lcd4_Set_Cursor(2,1);
				int pos=0;
				int m1=rowdet();
				int temp=0;
				while(pos<5){
					if(m1){
						keypressed=switchcr();
						Lcd4_Set_Cursor(2,1+pos);
						pos++;
						if(pos!=5 && keypressed==0x77){
							Lcd4_Write_Char('1');
							temp=(10*temp)+1;
						}
						else if(pos!=5 && keypressed==0x7B){
							Lcd4_Write_Char('2');
							temp=(10*temp)+2;
						}
						else if(pos!=5 && keypressed==0x7D){
							Lcd4_Write_Char('3');
							temp=(10*temp)+3;
						}
						else if(pos!=5 && keypressed==0xB7){
							Lcd4_Write_Char('4');
							temp=(10*temp)+4;
						}
						else if(pos!=5 && keypressed==0xBB){
							Lcd4_Write_Char('5');
							temp=(10*temp)+5;
						}
						else if(pos!=5 && keypressed==0xBD){
							Lcd4_Write_Char('6');
							temp=(10*temp)+6;
						}
						else if(pos!=5 && keypressed==0xD7){
							Lcd4_Write_Char('7');
							temp=(10*temp)+7;
						}
						else if(pos!=5 && keypressed==0xDB){
							Lcd4_Write_Char('8');
							temp=(10*temp)+8;
						}
						else if(pos!=5 && keypressed==0xDD){
							Lcd4_Write_Char('9');
							temp=(10*temp)+9;
						}
						else if(pos!=5 && keypressed==0xEB){
							Lcd4_Write_Char('0');
						}
						else if(keypressed==0xBE){
							prev_state=processor_state;
							processor_state=1;
							break;
						}
						else if(keypressed==0x7E){
							prev_state=processor_state;
							processor_state=0;
							Humidity=temp;
							break;
						}
						else{
							prev_state=processor_state;
							processor_state=3;
							break;
						}
						
						
					}
					_delay_ms(220);
					m1=rowdet();
					
				}
				
		}
		else if(processor_state==6){
			Lcd4_Clear();
			char *message1[16];
			Lcd4_Set_Cursor(1,1);
			sprintf(message1, "Water: %d", Water);
			Lcd4_Write_String(message1);
			int m1=rowdet();
			while(1){
				if(m1){
					keypressed=switchcr();
					if(keypressed==0xBE){
						prev_state=processor_state;
						processor_state=2;
						
					}
					else{
						prev_state=processor_state;
						processor_state=6;
					}
					_delay_ms(220);
					break;
					
					
				}
				_delay_ms(220);
				m1=rowdet();
			}
			
				
		}
		else if(processor_state==7){
			Lcd4_Clear();
			char *message1[16];
			Lcd4_Set_Cursor(1,1);
			sprintf(message1, "Fertilizer: %d", Fertilizer);
			Lcd4_Write_String(message1);
			int m1=rowdet();
			while(1){
				if(m1){
					keypressed=switchcr();
					if(keypressed==0xBE){
						prev_state=processor_state;
						processor_state=2;
						
					}
					else{
						prev_state=processor_state;
						processor_state=7;
					}
					_delay_ms(220);
					break;
					
					
				}
				_delay_ms(220);
				m1=rowdet();
			}
		}
		else if(processor_state==8){
			Lcd4_Clear();
			Lcd4_Set_Cursor(1,1);
			_delay_ms(100);
			ADC_Init();
			while(1){
				adc_value = ADC_Read(0);	/* Copy the ADC value */
				moisture = 100-(adc_value*100.00)/1023.00;
				char array[10];
				Lcd4_Set_Cursor(1,1);
				Lcd4_Write_String("Moisture: ");
				dtostrf(moisture,3,2,array);
				strcat(array,"%   ");	/* Concatenate unit of % */
				Lcd4_Set_Cursor(2,1);	/* set column and row */
				Lcd4_Write_String(array);	/* Print moisture on second row */
				memset(array,0,10);
				int m1=rowdet();
				if(m1){
					keypressed=switchcr();
					if(keypressed==0xBE){
						prev_state=processor_state;
						processor_state=2;
						
					}
					else{
						prev_state=processor_state;
						processor_state=8;
					}
					_delay_ms(220);
					break;
					
				}
				_delay_ms(1000);
				m1=rowdet();
				if(m1){
					keypressed=switchcr();
					if(keypressed==0xBE){
						prev_state=processor_state;
						processor_state=2;
						
					}
					else{
						prev_state=processor_state;
						processor_state=8;
					}
					_delay_ms(220);
					break;
					
				}
				_delay_ms(220);
				
				
				
			}
			
			
		}
		else if(processor_state==9){
			Lcd4_Clear();
			Lcd4_Set_Cursor(1,1);
			char *message3[16];
			sprintf(message3, "Humidity:%d.%d%",I_RH,D_RH );
			Lcd4_Write_String(message3);
			Lcd4_Set_Cursor(2,1);
			char *message4[16];
			sprintf(message4, "Temp:%d.%dC",I_Temp,D_Temp );
			Lcd4_Write_String(message4);
			int m1=rowdet();
			while(1){
				if(m1){
					keypressed=switchcr();
					if(keypressed==0xBE){
						prev_state=processor_state;
						processor_state=2;
						
					}
					else{
						prev_state=processor_state;
						processor_state=9;
					}
					_delay_ms(220);
					break;
					
					
				}
				_delay_ms(1000);
				Request();		/* send start pulse */
				Response();		/* receive response */
				//sprintf(message3, "Humidity: %d.%d %",I_RH,D_RH );
				//Lcd4_Write_String(message3);
				
				I_RH=Receive_data();	/* store first eight bit in I_RH */
				D_RH=Receive_data();	/* store next eight bit in D_RH */
				I_Temp=Receive_data();	/* store next eight bit in I_Temp */
				D_Temp=Receive_data();	/* store next eight bit in D_Temp */
				CheckSum=Receive_data();/* store next eight bit in CheckSum */
				
				if ((I_RH + D_RH + I_Temp + D_Temp) == CheckSum)
				{	
					Lcd4_Set_Cursor(1,1);
					 sprintf(message3, "Humidity:%d.%d%",I_RH,D_RH );
					 Lcd4_Write_String(message3);
					 Lcd4_Set_Cursor(2,1);
					 sprintf(message4, "Temp:%d.%dC",I_Temp,D_Temp );
					 Lcd4_Write_String(message4);
					 if(I_RH >84){
						 
						 PORTA|=0x02;
					 }
					 else if(I_RH<80){
						 PORTA &=~(0x02);
					 }

				}
				
				
				_delay_ms(220);
				m1=rowdet();
			}
		}
		else if(processor_state==10){
				Lcd4_Clear();
				Lcd4_Set_Cursor(1,1);
				_delay_ms(100);
				ADC_Init();
				while(1){
					adc_value = ADC_Read(1);	/* Copy the ADC value */
					moisture = (adc_value*5.00)/1023.00;
					float lu=moisture/10000.0;
					float mic=lu*1000000;
					float lux=mic*2.0;
					char array[10];
					Lcd4_Set_Cursor(1,1);
					Lcd4_Write_String("Light: ");
					dtostrf(lux,3,2,array);
					strcat(array,"LUX  ");	
					char*message[16];
					
					Lcd4_Set_Cursor(2,1);	/* set column and row */
					Lcd4_Write_String(array);	/* Print moisture on second row */
					memset(array,0,10);
					int m1=rowdet();
					if(m1){
						keypressed=switchcr();
						if(keypressed==0xBE){
							prev_state=processor_state;
							processor_state=2;
							
						}
						else{
							prev_state=processor_state;
							processor_state=10;
						}
						_delay_ms(220);
						break;
						
					}
					_delay_ms(1000);
					m1=rowdet();
					if(m1){
						keypressed=switchcr();
						if(keypressed==0xBE){
							prev_state=processor_state;
							processor_state=2;
							
						}
						else{
							prev_state=processor_state;
							processor_state=8;
						}
						_delay_ms(220);
						break;
						
					}
					_delay_ms(220);
					
					
					
				}
				
			}
			
		
	
		
      
	  
  }





   
    /*Lcd4_Clear();
    Lcd4_Set_Cursor(2,1);
    Lcd4_Write_Char('e');
    Lcd4_Write_Char('S');
    _delay_ms(2000);*/
    return 0;
}
