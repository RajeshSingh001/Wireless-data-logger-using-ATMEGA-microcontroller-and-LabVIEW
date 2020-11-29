#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include"lcd.h"
/*********************************
B0-Autonomous
B6-Semiautonomous

FAN
B1-UP
B2-Down

Heater
B4-UP
B5-Down

Light
D2-UP
D3-Down

OK-B3

LDR -A0

***********************************/

#define SW_DDR	DDRB
#define SW_PORT	PORTB
#define SW_PIN	PINB

#define SW_FAN_UP	PB1
#define SW_FAN_DWN	PB2

#define SW_HTR_UP	PB4
#define SW_HTR_DWN	PB5

#define SW_OK	PB3

#define SW_AUTO	PB0
#define SW_SEMI_AUTO	PB6

/*#define SW_FAN_UP_VALUE		2
#define SW_FAN_DWN_VALUE	4

#define SW_HTR_UP_VALUE		16
#define SW_HTR_DWN_VALUE	32

#define SW_AUTO_VALUE		1
#define SW_SEMIAUTO_VALUE	64

#define SW_OK_VALUE			8
*/
volatile SW_PIN_VALUE[7]={2,4,16,32,1,64,8};
/***********************************************************/
#define SW_DDR_1	DDRD
#define SW_PORT_1	PORTD
#define SW_PIN_1	PIND

#define SW_LIGHT_UP			PD2
#define SW_LIGHT_DWN		PD3

#define SW_LIGHT_UP_VALUE	4
#define SW_LIGHT_DWN_VALUE	8


volatile uint8_t SW_POS=0;
volatile uint8_t SW_PRESS=0;
volatile uint8_t SW[9];
volatile unsigned int LDR=0;
volatile uint8_t SENSOR_RX[14];
volatile uint8_t RX_POS=0;

volatile uint8_t LIGHT_Value=0;
volatile uint8_t FAN_Value=0;
volatile uint8_t HTR_Value=0;

volatile uint8_t LDR_Value=0;
volatile uint8_t Initial_LDR_Value=0;
volatile uint8_t HUMIDITY_Value=0;
volatile uint8_t TEMPRATURE_Value=0;
volatile uint8_t Initial_HUMIDITY_Value=0;
volatile uint8_t Initial_TEMPRATURE_Value=0;

volatile uint8_t SET_HM=0;
volatile uint8_t SET_TEMP=0;

volatile uint8_t AUTO=0;
volatile uint8_t SEMI_AUTO=0;
volatile uint8_t OK=0;
volatile uint8_t FAN_UP=0;
volatile uint8_t FAN_DWN=0;
volatile uint8_t HTR_UP=0;
volatile uint8_t HTR_DWN=0;
volatile uint8_t LIGHT_UP=0;
volatile uint8_t LIGHT_DWN=0;

volatile uint8_t HUMIDITY_UP=0;
volatile uint8_t HUMIDITY_DWN=0;
volatile uint8_t LDR_UP=0;
volatile uint8_t LDR_DWN=0;
volatile uint8_t TEMPRATURE_UP=0;
volatile uint8_t TEMPRATURE_DWN=0;

volatile int A1=0,SA1=0;
volatile uint8_t update=0;
volatile unsigned int Humidity_Sensor=0,Temprature_Sensor=0;
volatile uint16_t reth,retl,Result=0,ADC_READINGS=0;
volatile uint8_t Initial_Reference=0;
volatile uint8_t Initial=0;
void main()
{
	SW_DDR=0x00;							//SW PORTB 
	SW_PORT=0xFF;

	SW_DDR_1&=~(1<<SW_LIGHT_UP);			//SW PORTD
	SW_DDR_1&=~(1<<SW_LIGHT_DWN);			//SW PORTD

	SW_PORT_1|=(1<<SW_LIGHT_UP);
	SW_PORT_1|=(1<<SW_LIGHT_DWN);

	InitADC();
//	adc_init();

	USARTInit(95);
	InitLCD(0);
	sei();

	LCDClear();
	LCDWriteStringXY(0,1,"Initializing... ");
	for(int i=0;i<100;i++)
	{
		LCDWriteIntXY(17,1,i,3);
		_delay_ms(5);
	}	
	LCDClear();
	LCDWriteStringXY(3,0,"HYBRID REMOTE");
	LCDWriteStringXY(7,2,"SYSTEM");
	_delay_ms(400);

	LDR=0;
	for(int l=0;l<3;l++)
		LDR+=ReadADC(0);
		
	LDR=(LDR/12);

	LDR=((255-LDR)*100)/255;
	Initial_LDR_Value=LDR;
	LDR_Value=LDR;

/*	Initial_TEMPRATURE_Value=Temprature_Sensor;
	Initial_HUMIDITY_Value=Humidity_Sensor;
	TEMPRATURE_Value=Temprature_Sensor;
	HUMIDITY_Value=Humidity_Sensor;
*/
	while(1)
	{		
		Humidity_Sensor=(((SENSOR_RX[3]-48)*100)+((SENSOR_RX[4]-48)*10)+(SENSOR_RX[5]-48));		//3,4,5
		Temprature_Sensor=(((SENSOR_RX[9]-48)*100)+((SENSOR_RX[10]-48)*10)+(SENSOR_RX[11]-48));//3,4,5
	
		LDR=0;
		for(int l=0;l<3;l++)
			LDR+=ReadADC(0);
		
		LDR=(LDR/12);

		LDR=((255-LDR)*100)/255;

		SW_CHECK();
		SW_DATA();

		if((AUTO==1)||(A1==1))		//Autonomous
		{
			A1=1;
			SA1=0;
			SW_CHECK();
			SW_DATA();

			if(HUMIDITY_UP==1)
			{
				update=1;
				Initial=1;
			/*	if(Humidity_Sensor>HUMIDITY_Value)
					HUMIDITY_Value++;

				if(HUMIDITY_Value>100)
					HUMIDITY_Value=100;*/
				HUMIDITY_Value=Humidity_Sensor;
			}
			if(HUMIDITY_DWN==1)
			{
				Initial=1;
				update=1;
				if(HUMIDITY_Value>0)
				HUMIDITY_Value--;
			}

			if(TEMPRATURE_UP==1)
			{
				Initial=1;
				update=2;
				TEMPRATURE_Value++;
				if(TEMPRATURE_Value>50)
					TEMPRATURE_Value=50;
			}
			if(TEMPRATURE_DWN==1)
			{
				Initial=1;
				update=2;
				/*if(TEMPRATURE_Value>0)
					TEMPRATURE_Value--;*/
					//TEMPRATURE_Value=0;
				TEMPRATURE_Value=Temprature_Sensor;
			}

			if(LDR_UP==1)
			{
				Initial=1;
				update=3;
				LDR_Value++;
				if(LDR_Value>100)
					LIGHT_Value=100;
			}
			if(LDR_DWN==1)
			{
				Initial=1;
				update=3;
				if(LDR_Value>0)
					LDR_Value--;
				if(LDR_Value<Initial_LDR_Value)
					LDR_Value=Initial_LDR_Value;
			}

			if(OK==1)
			{
			//	if((TEMPRATURE_UP==1)||(TEMPRATURE_DWN==1))
				if(Initial_Reference==0)
				{
					USARTWriteChar(0xEA);
					USARTWriteChar(0x70);
					USARTWriteChar(Initial_LDR_Value);
					USARTWriteChar(0x00);
					USARTWriteChar(0x0D);
					Initial_Reference=1;
					_delay_ms(100);
				}
				if(update==2)
				{
					USARTWriteChar(0xEA);
					USARTWriteChar(0X81);
					USARTWriteChar(TEMPRATURE_Value);
					USARTWriteChar(Temprature_Sensor);
					USARTWriteChar(0x0D);
				}
				//else if((LDR_UP==1)||(LDR_DWN==1))
				else if(update==3)
				{
					USARTWriteChar(0xEA);
					USARTWriteChar(0X82);	
					USARTWriteChar(LDR_Value);
					USARTWriteChar(LDR);
					USARTWriteChar(0x0D);
				}
				//else if((HUMIDITY_UP==1)||(HUMIDITY_DWN==1))
				else if(update==1)
				{
					USARTWriteChar(0xEA);
					USARTWriteChar(0X83);
					USARTWriteChar(HUMIDITY_Value);
					USARTWriteChar(Humidity_Sensor);
					
					USARTWriteChar(0x0D);
				}
			//	update=0;
			Initial=1;
			}
			
				if(Initial==0)
				{
					LCDClear();
					LCDWriteStringXY(5,0,"AUTO MODE");
					LCDWriteStringXY(0,1,"H: ");
					LCDWriteInt(Humidity_Sensor,3);
					LCDWriteString("    ");
					LCDWriteInt(Humidity_Sensor,3);
					LCDWriteString("  %");
			
					LCDWriteStringXY(0,2,"T: ");
					LCDWriteInt(Temprature_Sensor,3);
					LCDWriteString("    ");
					LCDWriteInt(Temprature_Sensor,3);
					LCDWriteString("  C");

					LCDWriteStringXY(0,3,"L: ");
					LCDWriteInt(LDR_Value,3);			
					LCDWriteString("    ");
					LCDWriteInt(LDR,3);
					LCDWriteString("  %");
					TEMPRATURE_Value=Temprature_Sensor;
					HUMIDITY_Value=Humidity_Sensor;
				}
				else
				{
					LCDClear();
					LCDWriteStringXY(5,0,"AUTO MODE");
					LCDWriteStringXY(0,1,"H: ");
					LCDWriteInt(HUMIDITY_Value,3);
					LCDWriteString("    ");
					LCDWriteInt(Humidity_Sensor,3);
					LCDWriteString("  %");
				
					LCDWriteStringXY(0,2,"T: ");
					LCDWriteInt(TEMPRATURE_Value,3);
					LCDWriteString("    ");
					LCDWriteInt(Temprature_Sensor,3);
					LCDWriteString("  C");

					LCDWriteStringXY(0,3,"L: ");
					LCDWriteInt(LDR_Value,3);			
					LCDWriteString("    ");
					LCDWriteInt(LDR,3);
					LCDWriteString("  %");
				}
				

		}//autonomous
		if((SEMI_AUTO==1)||(SA1==1))		//Semi Autonomous
		{
			A1=0;
			SA1=1;
			SW_CHECK();
			SW_DATA();

			if(FAN_UP==1)
			{
				update=4;
				FAN_Value++;
				if(FAN_Value>15)
					FAN_Value=15;
			}
			if(FAN_DWN==1)
			{
				update=4;
				if(FAN_Value>0)
					FAN_Value--;
			}

			if(LIGHT_UP==1)
			{
				update=5;
				LIGHT_Value++;
				if(LIGHT_Value>15)
					LIGHT_Value=15;
			}
			if(LIGHT_DWN==1)
			{
				update=5;
				if(LIGHT_Value>0)
					LIGHT_Value--;
			}

			if(HTR_UP==1)
			{
				update=6;
				HTR_Value++;
				if(HTR_Value>15)
					HTR_Value=15;
			}
			if(HTR_DWN==1)
			{
				update=6;
				if(HTR_Value>0)
					HTR_Value--;
			}
			if(OK==1)
			{
				//if((HTR_UP==1)||(HTR_DWN==1))
				if(update==6)
				{
					USARTWriteChar(0xEA);
					USARTWriteChar(0X01);
					USARTWriteChar(HTR_Value);
					USARTWriteChar(0X00);
					USARTWriteChar(0x0D);
				}
				//else if((LIGHT_UP==1)||(LIGHT_DWN==1))
				else if(update==5)
				{
					USARTWriteChar(0xEA);
					USARTWriteChar(0X02);
					USARTWriteChar(LIGHT_Value);
					USARTWriteChar(0X00);
					USARTWriteChar(0x0D);
				}
				//else if((FAN_UP==1)||(FAN_DWN==1))
				else if(update==4)
				{
					USARTWriteChar(0xEA);
					USARTWriteChar(0X03);
					USARTWriteChar(FAN_Value);
					USARTWriteChar(0X00);
					USARTWriteChar(0x0D);
				}
			//	update=0;
			}//ok
			LCDClear();
			LCDWriteStringXY(3,0,"SEMI-AUTO MODE");
			LCDWriteStringXY(0,1,"FAN LEVEL:");
			LCDWriteStringXY(0,2,"HTR LEVEL:");
			LCDWriteStringXY(0,3,"LGT LEVEL:");
			
			LCDWriteIntXY(11,1,FAN_Value,3);
			LCDWriteIntXY(11,2,HTR_Value,3);
			LCDWriteIntXY(11,3,LIGHT_Value,3);
			
		}//Semi AUTO
		_delay_ms(300);
		//Humidity_Sensor=(((SENSOR_RX[3]-48)*100)+((SENSOR_RX[4]-48)*10)+(SENSOR_RX[5]-48));		//3,4,5
		//Temprature_Sensor=(((SENSOR_RX[9]-48)*100)+((SENSOR_RX[10]-48)*10)+(SENSOR_RX[11]-48));//3,4,5
	}//While
}

void SW_DATA()
{
	if(SW[0]==1)
	{
		FAN_UP=1;
		HUMIDITY_UP=1;
	}
	else
	{
		FAN_UP=0;
		HUMIDITY_UP=0;
		if(SW[1]==1)
		{
			FAN_DWN=1;
			HUMIDITY_DWN=1;
		}
		else
		{
			FAN_DWN=0;
			HUMIDITY_DWN=0;
		}
	}

	if(SW[2]==1)
	{
		HTR_DWN=1;
		TEMPRATURE_DWN=1;
	}
	else
	{
		HTR_DWN=0;
		TEMPRATURE_DWN=0;
		if(SW[3]==1)
		{
			HTR_UP=1;
			TEMPRATURE_UP=1;
		}
		else
		{
			TEMPRATURE_UP=0;
			HTR_UP=0;
		}
	}
	if(SW[4]==1)
		SEMI_AUTO=1;
	else
		SEMI_AUTO=0;
	
	if(SW[5]==1)
		AUTO=1;
	else
		AUTO=0;

	if(SW[6]==1)
		OK=1;
	else
		OK=0;
		
	if(SW[7]==1)
	{
		LIGHT_UP=1;
		LDR_UP=1;
	}
	else
	{
		LIGHT_UP=0;
		LDR_UP=0;
		if(SW[8]==1)
		{
			LIGHT_DWN=1;
			LDR_DWN=1;
		}
		else
		{
			LIGHT_DWN=0;	
			LDR_DWN=0;
		}
	}
}
void SW_CHECK()
{
/*******************************************/
	SW_PRESS=0;
	SW_POS=0;
//	for(int l=0;l<9;l++)
//		SW[l]=0;

	for(int k=0;k<7;k++)
	{
		SW_PRESS=0;
		if(!(SW_PIN&SW_PIN_VALUE[k]))
		{
			for(int l=0;l<10;l++)
			{
				if(!(SW_PIN&SW_PIN_VALUE[k]))
				{
					SW_PRESS=1;
				}
				else
				{
					SW_PRESS=0;
					break;
				}
			}
		}
		if(SW_PRESS==1)
		{
			SW[SW_POS]=1;
		}
		else
		{
			SW[SW_POS]=0;
		}
		SW_POS++;
	}			
	
	SW_PRESS=0;
	if(!(SW_PIN_1&SW_LIGHT_UP_VALUE))
	{
		for(int l=0;l<10;l++)
		{
			if(!(SW_PIN_1&SW_LIGHT_UP_VALUE))
			{
				SW_PRESS=1;
			}
			else
			{
				SW_PRESS=0;
				break;
			}
		}
	}
	if(SW_PRESS==1)
	{
		SW[SW_POS]=1;
	}
	else
	{
		SW[SW_POS]=0;
	}
	SW_POS++;
	
	
	SW_PRESS=0;
	if(!(SW_PIN_1&SW_LIGHT_DWN_VALUE))
	{
		for(int l=0;l<10;l++)
		{
			if(!(SW_PIN_1&SW_LIGHT_DWN_VALUE))
			{
				SW_PRESS=1;
			}
			else
			{
				SW_PRESS=0;
				break;
			}
		}
	}
	if(SW_PRESS==1)
	{
		SW[SW_POS]=1;
	}
	else
	{
		SW[SW_POS]=0;
	}
	SW_POS++;
}

ISR(USART_RXC_vect)
{
	SENSOR_RX[RX_POS]=UDR;
	RX_POS++;

	if(SENSOR_RX[RX_POS-1]==0x0D)
	{
		RX_POS=0;
	}
}
ISR(USART_TXC_vect)
{
}

/************** Free Running Mode **********************/

/*void adc_init()
{
	ADMUX = 0b00000000;	// if data is Right align & input Channel 0 Fix
    ADCSRA=	0b11001110;
	SFIOR =	0b00000000;	//Free Running Mode
}

ISR(ADC_vect)
{
	retl = ADCL;
   	reth = ADCH;
	Result+=(reth<<8)|retl;
	
	ADC_READINGS++;

	if(ADC_READINGS==10)
	{
		LDR=Result/10;
		Result=0;
		ADC_READINGS=0;
	}
	ADCSRA|=(1<<ADSC);
}
*/
