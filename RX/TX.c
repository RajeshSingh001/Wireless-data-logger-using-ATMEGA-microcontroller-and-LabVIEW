#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include"lcd.h"

/**************************************
FAN
B0-B3		D3-D0

HTR
D2,D3,D4,A7		D3,D2,D1,D0

LIGHT
C4,C5,C6,C7		D3,D2,D1,D0

***************************************/
#define LIGHT_B0	PB0
#define LIGHT_B1	PB1
#define LIGHT_B2	PB2
#define LIGHT_B3	PB3

#define LIGHT_PORT_DDR	DDRB
#define LIGHT_PORT		PORTB

#define HTR_B0	PA7
#define HTR_B1	PD4
#define HTR_B2	PD3
#define HTR_B3	PD2

#define HTR_PORT_DDR_A	DDRA	
#define HTR_PORT_DDR_D	DDRD

#define HTR_PORT_A	PORTA
#define HTR_PORT_D	PORTD

#define FAN_B0	PC5
#define FAN_B1	PC6
#define FAN_B2	PC7
#define FAN_B3	PC4

#define FAN_PORT_DDR	DDRC
#define FAN_PORT		PORTC


volatile uint8_t DATA_RX[6];
volatile uint8_t RX_POS=0;
volatile uint8_t PACKET_Received=0;

volatile uint8_t LDR;
volatile uint8_t Temprature_Sensor=0;
volatile uint8_t Humidity_Sensor=0;

volatile uint8_t LIGHT_Value=0;
volatile uint8_t FAN_Value=0;
volatile uint8_t HTR_Value=0;

volatile uint8_t LDR_Value=0;
volatile uint8_t HUMIDITY_Value=0;
volatile uint8_t TEMPRATURE_Value=0;

volatile uint8_t DIMMER_LEVEL[16]={0x0F,0x0E,0XD,0X0C,0X0B,0X0A,0X09,0X08,0X07,0X06,0X05,0X04,0X03,0X02,0X01,0X00};
//volatile uint8_t DIMMER_PER[16]={100,85,80,75,70,65,60,50,40,30,25,20,15,10,5,0};
volatile uint8_t DIMMER_PER[15]={93,83,77,72,67,62,55,45,35,27,22,17,12,7,0};

volatile uint8_t LDR_LEVEL=0;
volatile uint8_t Temprature_LEVEL=0;
volatile uint8_t HUMIDITY_LEVEL=0;

volatile uint8_t PREV_LDR_LEVEL=0,PREV_LDR=0,PREV_LDR_VALUE=0;
volatile uint8_t PREV_Temprature_LEVEL=0;
volatile uint8_t DIFF_LDR=0;
volatile uint8_t DIFF_TEMPRATURE=0;
volatile uint8_t PREV_TEMPRATURE_Value=0;
volatile uint8_t PREV_Temprature_Sensor=0;
volatile uint8_t PREV_HUMIDITY_LEVEL=0;
volatile uint8_t PREV_HUMIDITY_Value=0;
volatile uint8_t PREV_Humidity_Sensor=0;
volatile uint8_t Initial_LDR_Reference=0;

void main()
{
	LIGHT_PORT_DDR=0xFF;
	LIGHT_PORT=0xFF;

	FAN_PORT_DDR|=(1<<FAN_B3);
	FAN_PORT_DDR|=(1<<FAN_B2);
	FAN_PORT_DDR|=(1<<FAN_B1);
	FAN_PORT_DDR|=(1<<FAN_B0);

	FAN_PORT|=(1<<FAN_B3);
	FAN_PORT|=(1<<FAN_B2);
	FAN_PORT|=(1<<FAN_B1);
	FAN_PORT|=(1<<FAN_B0);

	HTR_PORT_DDR_A|=(1<<HTR_B0);
	HTR_PORT_DDR_D|=(1<<HTR_B1);
	HTR_PORT_DDR_D|=(1<<HTR_B2);
	HTR_PORT_DDR_D|=(1<<HTR_B3);	

	HTR_PORT_A|=(1<<HTR_B0);
	HTR_PORT_D|=(1<<HTR_B1);
	HTR_PORT_D|=(1<<HTR_B2);
	HTR_PORT_D|=(1<<HTR_B3);	

	InitLCD(0);
	sei();
	LCDClear();
	LCDWriteStringXY(0,1,"SYSTEM OK");
	
	USARTInit(95);
	while(1)
	{
		if(PACKET_Received==1)
		{
			if((DATA_RX[0]==0xEA)&&(DATA_RX[4]==0x0D))
			{	
				if(DATA_RX[1]==0x70)
				{
					Initial_LDR_Reference=DATA_RX[2];
				}
				
				if((DATA_RX[1]&0x80)==0x80)		//Auto
				{
					if(DATA_RX[1]==0x81)
					{
						TEMPRATURE_Value=DATA_RX[2];
						Temprature_Sensor=DATA_RX[3];

						if(Temprature_Sensor<(TEMPRATURE_Value))
						{
							if(PREV_TEMPRATURE_Value==0)
							{
								Temprature_LEVEL=(TEMPRATURE_Value-Temprature_Sensor);
							
								int l=0;
								for(l=0;l<15;l++)
								{
									if(DIMMER_PER[l]<=Temprature_LEVEL)
										break;
								}
									Temprature_LEVEL=l-1;
							}
							else
							{
								if(Temprature_Sensor>PREV_TEMPRATURE_Value)
								{
									DIFF_TEMPRATURE=Temprature_Sensor-PREV_TEMPRATURE_Value;
								}
								else
								{
									DIFF_TEMPRATURE=PREV_TEMPRATURE_Value-Temprature_Sensor;
								}	
								if(DIFF_LDR<15)
								{
									Temprature_Sensor=PREV_Temprature_Sensor;
								}
							
								Temprature_LEVEL=(TEMPRATURE_Value-Temprature_Sensor);
							
								int l=0;
								for(l=0;l<14;l++)
								{
									if(DIMMER_PER[l]<=Temprature_LEVEL)
										break;
								}
								Temprature_LEVEL=l-1;
							}
							PREV_Temprature_LEVEL=Temprature_LEVEL;
							PREV_Temprature_Sensor=Temprature_Sensor;
							PREV_TEMPRATURE_Value=TEMPRATURE_Value;
							SET_TEMP_LEVEL();
						}
						else 
						{
							HTR_PORT_A|=(1<<HTR_B0);
							HTR_PORT_D|=(1<<HTR_B1);
							HTR_PORT_D|=(1<<HTR_B2);
							HTR_PORT_D|=(1<<HTR_B3);	
						}
					}//Temprature Receive

					if(DATA_RX[1]==0x82)
					{	
						LDR_Value=DATA_RX[2];
						LDR=DATA_RX[3];		
/******************************************************************************/
						if(LDR<LDR_Value)
						{
							if(PREV_LDR_VALUE==0)
							{
								LDR_LEVEL=(LDR_Value-LDR);
							
								int l=0;
								for(l=0;l<15;l++)
								{
									if(DIMMER_PER[l]<=LDR_LEVEL)
										break;
								}
									LDR_LEVEL=l-1;
							}
							else
							{
								if(LDR>PREV_LDR_VALUE)
								{
									DIFF_LDR=LDR-PREV_LDR_VALUE;
								}
								else
								{
									DIFF_LDR=PREV_LDR_VALUE-LDR;
								}	
								if(DIFF_LDR<15)
								{
									LDR=PREV_LDR;
								}
								
								LDR_LEVEL=(LDR_Value-LDR);
								int l=0;
								for(l=0;l<15;l++)
								{
									if(DIMMER_PER[l]<=LDR_LEVEL)
										break;
								}
									LDR_LEVEL=l-1;
							}

							PREV_LDR_LEVEL=LDR_LEVEL;
							PREV_LDR=LDR;
							PREV_LDR_VALUE=LDR_Value;
							SET_Light_LEVEL();
						}
						else if(LDR_Value<=Initial_LDR_Reference)
						{
							LIGHT_PORT=0xFF;
						}

						/*else
						{
							
							
							
							
							
							
							
							
							
							LDR_LEVEL=(LDR-LDR_Value);
							if(LDR_LEVEL>LDR_Value)
							{
								LDR_LEVEL=14;
								goto XX;
							}
							else	
								LDR_LEVEL=LDR_Value-LDR_LEVEL;
	
							
							int l=0;
							for(l=0;l<15;l++)
							{
								if(DIMMER_PER[l]<=LDR_LEVEL)
									break;
							}
								//if(l!=0)
									LDR_LEVEL=14-l;
							XX:
								l=0;
							//	SET_Light_LEVEL();
						}*/	
/******************************************************************************/
					}//LIGHT Receive
					if(DATA_RX[1]==0x83)
					{
						HUMIDITY_Value=DATA_RX[2];
						Humidity_Sensor=DATA_RX[3];

						if(Humidity_Sensor>HUMIDITY_Value)
						{
							if(PREV_HUMIDITY_Value==0)
							{
								HUMIDITY_LEVEL=(Humidity_Sensor-HUMIDITY_Value);
							
								int l=0;
								for(l=0;l<15;l++)
								{
									if(DIMMER_PER[l]<=HUMIDITY_LEVEL)
										break;
								}
									HUMIDITY_LEVEL=l-1;;
							}
							else
							{							
								HUMIDITY_LEVEL=(Humidity_Sensor-HUMIDITY_Value);
								int l=0;
								for(l=0;l<14;l++)
								{
									if(DIMMER_PER[l]<=HUMIDITY_LEVEL)
										break;
								}
								HUMIDITY_LEVEL=l-1;
							}
							PREV_HUMIDITY_LEVEL=HUMIDITY_LEVEL;
							PREV_Humidity_Sensor=Humidity_Sensor;
							PREV_HUMIDITY_Value=HUMIDITY_Value;
							SET_HUMIDITY_LEVEL();
						}
						else
						{
							FAN_PORT|=(1<<FAN_B3);
							FAN_PORT|=(1<<FAN_B2);
							FAN_PORT|=(1<<FAN_B1);
							FAN_PORT|=(1<<FAN_B0);
						}
					}//Humidity Receive

					LCDClear();
					LCDWriteStringXY(5,0,"AUTO MODE");
					LCDWriteStringXY(0,1,"H: ");
					LCDWriteInt(HUMIDITY_Value,3);
					LCDWriteString("    ");
					LCDWriteInt(Humidity_Sensor,3);
			
					LCDWriteStringXY(0,2,"T: ");
					LCDWriteInt(TEMPRATURE_Value,3);
					LCDWriteString("    ");
					LCDWriteInt(Temprature_Sensor,3);

					LCDWriteStringXY(0,3,"L: ");
					LCDWriteInt(LDR_Value,3);			
					LCDWriteString("    ");
					LCDWriteInt(LDR,3);
				}
				else						//	Semi Auto
				{
					if(DATA_RX[1]==0x03)
					{
						FAN_Value=DATA_RX[2];
					}
					if(DATA_RX[1]==0x02)
					{
						LIGHT_Value=DATA_RX[2];
					}
					if(DATA_RX[1]==0x01)
					{
						HTR_Value=DATA_RX[2];
					}
					LCDClear();
					LCDWriteStringXY(3,0,"SEMI-AUTO MODE");
					LCDWriteStringXY(0,1,"FAN LEVEL:");
					LCDWriteStringXY(0,2,"HTR LEVEL:");
					LCDWriteStringXY(0,3,"LGT LEVEL:");
			
					LCDWriteIntXY(11,1,FAN_Value,3);
					LCDWriteIntXY(11,2,HTR_Value,3);
					LCDWriteIntXY(11,3,LIGHT_Value,3);

					SET_FAN();
					SET_HTR();
					SET_LIGHT();
				}
				RX_POS=0;
				PACKET_Received=0;
			}
			else
			{
				RX_POS=0;
				PACKET_Received=0;
			}
		}

	}
}

ISR(USART_RXC_vect)
{
	if(PACKET_Received==0)
	{
		DATA_RX[RX_POS]=UDR;
		if((DATA_RX[RX_POS]==0x0D)||(RX_POS==6))
		{
			PACKET_Received=1;
		}
		else
			RX_POS++;
	}
}

void SET_FAN()
{
	//FAN_Value	
	int l=0;
	for(l=0;l<16;l++)
	{
		if(FAN_Value==DIMMER_LEVEL[l])
			break;
	}
	
	if((l&0x01)==0x01)
	{
		FAN_PORT|=(1<<FAN_B0);
	}
	else
	{
		FAN_PORT&=~(1<<FAN_B0);
	}
	if((l&0x02)==0x02)
	{
		FAN_PORT|=(1<<FAN_B1);
	}
	else
	{
		FAN_PORT&=~(1<<FAN_B1);
	}
	if((l&0x04)==0x04)
	{
		FAN_PORT|=(1<<FAN_B2);
	}
	else
	{
		FAN_PORT&=~(1<<FAN_B2);
	}
	if((l&0x08)==0x08)
	{
		FAN_PORT|=(1<<FAN_B3);
	}
	else
	{
		FAN_PORT&=~(1<<FAN_B3);
	}
}

void SET_HTR()
{

//LIGHT_Value	
	int l=0;
	for(l=0;l<16;l++)
	{
		if(HTR_Value==DIMMER_LEVEL[l])
			break;
	}
	
	if((l&0x01)==0x01)
	{
		HTR_PORT_A|=(1<<HTR_B0);
	}
	else
	{
		HTR_PORT_A&=~(1<<HTR_B0);
	}
	if((l&0x02)==0x02)
	{
		HTR_PORT_D|=(1<<HTR_B1);
	}
	else
	{
		HTR_PORT_D&=~(1<<HTR_B1);
	}
	if((l&0x04)==0x04)
	{
		HTR_PORT_D|=(1<<HTR_B2);
	}
	else
	{
		HTR_PORT_D&=~(1<<HTR_B2);
	}
	if((l&0x08)==0x08)
	{
		HTR_PORT_D|=(1<<HTR_B3);
	}
	else
	{
		HTR_PORT_D&=~(1<<HTR_B3);
	}
}

void SET_LIGHT()
{

//LIGHT_Value	
	int l=0;
	for(l=0;l<16;l++)
	{
		if(LIGHT_Value==DIMMER_LEVEL[l])
			break;
	}
	
	if((l&0x01)==0x01)
	{
		LIGHT_PORT|=(1<<LIGHT_B0);
	}
	else
	{
		LIGHT_PORT&=~(1<<LIGHT_B0);
	}
	if((l&0x02)==0x02)
	{
		LIGHT_PORT|=(1<<LIGHT_B1);
	}
	else
	{
		LIGHT_PORT&=~(1<<LIGHT_B1);
	}
	if((l&0x04)==0x04)
	{
		LIGHT_PORT|=(1<<LIGHT_B2);
	}
	else
	{
		LIGHT_PORT&=~(1<<LIGHT_B2);
	}
	if((l&0x08)==0x08)
	{
		LIGHT_PORT|=(1<<LIGHT_B3);
	}
	else
	{
		LIGHT_PORT&=~(1<<LIGHT_B3);
	}
}
void SET_Light_LEVEL()
{
	int l=0;
	l=LDR_LEVEL;
	LCDWriteIntXY(16,1,LDR_LEVEL,3);
	LCDWriteStringXY(14,0,"   ");
	LCDWriteIntXY(16,0,LDR,3);
	
	if((l&0x01)==0x01)
	{
		LIGHT_PORT|=(1<<LIGHT_B0);
	}
	else
	{
		LIGHT_PORT&=~(1<<LIGHT_B0);
	}
	if((l&0x02)==0x02)
	{
		LIGHT_PORT|=(1<<LIGHT_B1);
	}
	else
	{
		LIGHT_PORT&=~(1<<LIGHT_B1);
	}
	if((l&0x04)==0x04)
	{
		LIGHT_PORT|=(1<<LIGHT_B2);
	}
	else
	{
		LIGHT_PORT&=~(1<<LIGHT_B2);
	}
	if((l&0x08)==0x08)
	{
		LIGHT_PORT|=(1<<LIGHT_B3);
	}
	else
	{
		LIGHT_PORT&=~(1<<LIGHT_B3);
	}
}

void SET_TEMP_LEVEL()
{
	int l=0;
	l=Temprature_LEVEL;
	LCDWriteIntXY(16,1,Temprature_LEVEL,3);
	
	if((l&0x01)==0x01)
	{
		HTR_PORT_A|=(1<<HTR_B0);
	}
	else
	{
		HTR_PORT_A&=~(1<<HTR_B0);
	}
	if((l&0x02)==0x02)
	{
		HTR_PORT_D|=(1<<HTR_B1);
	}
	else
	{
		HTR_PORT_D&=~(1<<HTR_B1);
	}
	if((l&0x04)==0x04)
	{
		HTR_PORT_D|=(1<<HTR_B2);
	}
	else
	{
		HTR_PORT_D&=~(1<<HTR_B2);
	}
	if((l&0x08)==0x08)
	{
		HTR_PORT_D|=(1<<HTR_B3);
	}
	else
	{
		HTR_PORT_D&=~(1<<HTR_B3);
	}
}

void SET_HUMIDITY_LEVEL()
{
	int l=0;
	l=HUMIDITY_LEVEL;
	LCDWriteIntXY(16,1,HUMIDITY_LEVEL,3);
	
	if((l&0x01)==0x01)
	{
		FAN_PORT|=(1<<FAN_B0);
	}
	else
	{
		FAN_PORT&=~(1<<FAN_B0);
	}
	if((l&0x02)==0x02)
	{
		FAN_PORT|=(1<<FAN_B1);
	}
	else
	{
		FAN_PORT&=~(1<<FAN_B1);
	}
	if((l&0x04)==0x04)
	{
		FAN_PORT|=(1<<FAN_B2);
	}
	else
	{
		FAN_PORT&=~(1<<FAN_B2);
	}
	if((l&0x08)==0x08)
	{
		FAN_PORT|=(1<<FAN_B3);
	}
	else
	{
		FAN_PORT&=~(1<<FAN_B3);
	}
}
