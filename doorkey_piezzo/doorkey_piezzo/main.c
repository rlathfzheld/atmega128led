/*
 * door_key.c
 *
 * Created: 2022-09-30 오후 12:28:33
 * Author :  교원송영진
 */ 


///////////////////////////////////////////////////////////////////////////////
// Project Name  	: LED-A,FND-B,LCD control & +5V-C, LCD-D port,Key scan-E port
// Function      	: 4x4 KEY_SCAN
// File Name    	: main.c					
// Version / Date : 1.0 / 2017.10.13	20:42 by Song					
///////////////////////////////////////////////////////////////////////////////
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "define.h"
#include "sgn_port_D_lcd.h"
#include <stdio.h>
#define __DELAY_BACKWARD_COMPATIBLE__	//7.0에서 _delay_ms(변수)로 사용가능 기능 선언.변수=100;언더바2개 앞뒤


unsigned char pattern_Fnd[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7C,0x07,0x7F,0x6F};  //for 8 FND
							// CC type  5611BH model http;//exeobd.com
//  8 LEDS =porta
/*============================================================================*/
/*							VARIABLE DEFINE																								*/
/*============================================================================*/

BYTE TimerFlag;
WORD Timer1Cnt;
BYTE Timer1Flag;
BYTE Lcd_clearflag;

BYTE	scan_key,old_key44,Key44_Data,scan_chatt_count,key44_event;
BYTE    scan_key_position,eeprom_add;								//LCD display position

BYTE 	set_key_flag,open_key_flag;// ,key_input[5];
unsigned char	key_input[5];
int scan_key_value;	//스캔_키 값을 10진수 양수로 표현하기 위한 변수

//// 피에조주파수변경용 변수 static
static int timer2_tcnt=0;
BYTE    Beep_OnOff;
BYTE    Beep_Count;
WORD    Beep_Timer;
//unsigned char scan_key_value;	//스캔_키 값을 10진수 양수로 표현하기 위한 변수
/*============================================================================*/
/*							FUNCTION DEFINE																								*/
/*============================================================================*/
void New_key_set(void);
void Open_key_press(void);

void InitPort(void);
void InitTimer0(void);
void LED_On(void);

void Delay(unsigned long delay);
void Delay_us(WORD time_us);
void Delay_ms(unsigned int time_ms);

void InitLCD(void);
void WriteLcdCmd(BYTE cmd);
void WriteLcdData(BYTE loc, BYTE data);

void Init_Display(void);
void sputsLcdLine1(unsigned char *str);
void sputsLcdLine2(unsigned char *str);
//////////피에조 용 ////////////
void BeepOn(BYTE count);
void BeepOff(void);
void InitTimer2(void);
//////////////////////////////
void 	Key44_Scan(void);	
void	Key44_Job_Exe(void);

void 	Key44_0_0(void);void 	Key44_0_1(void);void 	Key44_0_2(void);
void 	Key44_0_3(void);void 	Key44_0_4(void);void 	Key44_0_5(void);
void 	Key44_0_6(void);void 	Key44_0_7(void);void 	Key44_0_8(void);
void 	Key44_0_9(void); 	

void 	Key44_A(void);
void 	Key44_B(void);
void 	Key44_C(void);
void 	Key44_D(void);
void 	Key44_E(void);
void 	Key44_F(void);
				
void    EEPROM_test(void);		//eeprom test
void 	lcd_Display(void);

unsigned char EEPROM_read(unsigned char addr);
void EEPROM_write(unsigned char addr, unsigned char data);
////////////////////////////////////////////////////////////////////////////////

const BYTE hexdigit[] PROGMEM = "0123456789ABCDEF";//flash memory사용방법, <avr/pgmspace.h>와 같이 PROGMEM사용 
unsigned char msg[16]; //="1234567890";
unsigned char num;	//숫자를 표시할려면 unsigned char로 선언하고 ASCII코드값을 사용한다
char buf[10];  		 //itoa함수 사용용
/////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////

int main(void)
{			   	
//unsigned char data;
//char msg[16];
//int i;
Lcd_clearflag=0;
   	SPH = 0x10;
   	SPL = 0xff; 		// Stack Poniter Setting.
		
	InitPort();
	InitTimer0();   	//Init Timer0, Init Register
	InitTimer2(); 	//Init Timer2, Init Register (for beep)
		
	Delay_ms(100);		//Wait for more than 30msec		
	lcd_init();
	scan_key_position=12;
	eeprom_add=0;

	sei(); 				//Interrupt Enabled (I bit of SREG is set)	

	lcd_display_position(1,1);
	lcd_string("Hellow AVR");
	_delay_ms(1000);
	num=0x30;
	lcd_display_clear();
	lcd_display_position(1,1);
	Beep_OnOff = ON;
	BeepOn(4);		//테스트용
	lcd_Display();
								
	while(1)
	{

		if(TimerFlag)  				//job EXE every 10 msec
		{
        	Key44_Scan();				        							
			TimerFlag = 0;			//Clear 10msec EVT flag	이벤트 플레그		
		}	
						
		if(Timer1Flag)  			//job EXE every 1 sec
		{
					
			Timer1Flag = 0;			//Clear 1sec EVT flag			
		}	
				
		if(key44_event == 1)		//key44_scan()함수에서 안정된 키값이 발생됨
		{			
			Key44_Job_Exe();		//안정된 키값으로 원하는 동작시킴, LCD표시/FND표시/LED구동등 
	//  주파수변경을 위한 ovf시간 바꿈
	//  static변수로 timer2_cnt 선언하여 3종류 음발생
			BeepOn(2);	 //음 2회 발생
			timer2_tcnt++;
			if(timer2_tcnt>=4)timer2_tcnt=0;
	///////////////////////////////////////////////////////
			key_input[eeprom_add]=Key44_Data;									
			key44_event = 0;	//Clear KEY EVT flag			

			lcd_display_position(2, 1);

		if( Key44_Data <= 10)	eeprom_add++;

	   if ( (set_key_flag == 1) && (eeprom_add >=5) ) New_key_set();
	   if ( (open_key_flag == 1) && (eeprom_add >=5) ) Open_key_press();

		_delay_ms(200);

		}

	}					
}



void New_key_set(void)
{
//////////////////////////////////////////////////////
// 5개 번호가 입력되지 않으면 입력되는 키값 보여주기 
//////////////////////////////////////////////////////
	unsigned char data;
	lcd_display_position(1,1);
	lcd_string("press new_Key No ");
	lcd_display_position(2, 1);

		for (int i=0;i<=4;i++)
			{
			 EEPROM_write(i,key_input[i]);
			 key_input[i]=99;
			}
		lcd_display_clear();
		lcd_display_position(1, 1);
		lcd_string("Saved No.Reading");
		lcd_display_position(2, 1);
		lcd_string("                     ");
		_delay_ms(100);

	for(int i=0 ; i <= 0x04 ; i++) 
		 {
		data = EEPROM_read(i);	// EEPROM 리드
		sprintf(msg, "Addr:0x%2x = %d", i, data);
		lcd_display_position(2, 1);
		lcd_string(msg);
		_delay_ms(200);           	// 15ms 이상 대기 
			   
		 }
	
	 	eeprom_add=0;
		set_key_flag=0;
		scan_key_position=12;
     lcd_Display();

}

////////////////////////////////////////////////////
//		Open_key_press
////////////////////////////////////////////////////
void Open_key_press(void)

{
//////////////////////////////////////////////////////
// 5개 번호가 입력되지 않으면 입력되는 키값 보여주기 
//////////////////////////////////////////////////////
	unsigned char read_data, key_correct_flag;

		key_correct_flag=1;
		for (int i=0;i<=4;i++)
			{ 
				read_data=EEPROM_read(i);
				if ( key_input[i] == read_data ) 
					{key_correct_flag=key_correct_flag*1;}
				else
				    {key_correct_flag=key_correct_flag*0;}
			}
	  


// 결과 보여주기
	if (key_correct_flag==1)
	     {
			lcd_display_clear();
			lcd_display_position(1, 1);
			lcd_string("Goooood jooob");
			lcd_display_position(2, 1);
			lcd_string(" LEDs Onnnn...");
			_delay_ms(100);
			Led_On();
					_delay_ms(1000);
					PORTA=0X00;
	     }
	    else
        {
			lcd_display_clear();
			lcd_display_position(1, 1);
			lcd_string("Baaaaad jooob");
			lcd_display_position(2, 1);
			lcd_string(" LEDs Offff...");
			_delay_ms(100);
        }
	scan_key_position=12;
	eeprom_add=0;
	open_key_flag=0;
     lcd_Display();

}


//*============================================================================*/
//*							FUNCTIONS																											
//*============================================================================*/

void InitPort(void)
{	

  	DDRA= 0xFF; 	//LED data
  	PORTA= 0xFF;
	           
    		
  	PORTB= 0xFF;         
  	DDRB= 0xFF;     //FND
    	
  	PORTC= 0xFF;    // LCD control for PC0 ~ PC2,Output +5V of LCD VCC, VL at PC4 ~PC7
  	DDRC= 0xF0; 	  
    	    		    	
	PORTF=0x00;		// for LCD DB0-DB7
	DDRF= 0xFF;			
    
    
  	DDRE = 0xF0;	//input PE0~PE3 for key scan,using internal Pull-up  
				    // 스위치의 한쪽은 "0" -접지가 연결되어야 한다.접지를 연결하기 위하여 상위 4비트는 
					//모두 "1"로 출력한 상태에서 1번 스위치가 눌리면 PE0=1이 된다. 따라서 버튼이 눌린
					// 곳의 풀업저항의 접지를 만들어줘야하고 1110->1101->1011->0111을 출력시켜서 한번에 
					// 하나의 접시상태를 만들어준다
 	PORTE= 0x0F;	//4x4 KEY_SCAN  , output PE4~PE7 for 1110 & 1101 & 1011 & 0111
					//,Out High on PE0~PE3 for PULL-UP
					

 	_delay_ms(1000);
	 PORTA=0X00;
	 PORTB=0X00;
  	DDRD = 0xF8;	//16x2 LCD control : PF5=E, PF4=RW, PF3=RS
///피에조 출력용
	DDRG = 0xFF; 	//PG0:Beep Out
	PORTG = 0xFE; 	//PG0:Init low	  

}

////////////////////////////////////////////////////////////////////////////////
// INIT Timer0 Interrupt
////////////////////////////////////////////////////////////////////////////////

void InitTimer0(void)
{
  	TCCR0 = 0x07;   	// Trigger => Fclock/1024
  	TCNT0 = 0x00;		// clear Timer Counter Register
	TIMSK = 0x01;   	// Timer0 Interrupt Enable Mask Register   	
}
////////////////////////////////////////////////////////////////////////////////
// INIT Timer2 Interrupt
////////////////////////////////////////////////////////////////////////////////

void InitTimer2(void)
{
	TCCR2 = 0x03; 	// Trigger => Fclock/64
	TCNT2 = (256-44);	// Timer Counter Register
	TIMSK |= BIT6; 	// Timer2 Interrupt Enable Mask Register
}
////////////////////////////////////////////////////////////////////////////////

////  키를 누를때마다 BeepOn(BYTE count) 를 호출하면 됨
//    포트G0을 일반포트이지만 강제로 1-0-1-0를 반복시켜서 주파수를 만들어냄
//    버저는 PG0에 연결하여 사용
//    타이머는 오버플로우인터럽트발생마다 음생산

////////////////////////////////////////////////////////////////////////////////
//		TIMER0 INTERRUPT SERVICE ROUTINE
////////////////////////////////////////////////////////////////////////////////

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 112;			//TCNT =184 -->(1024/14.7456MHz)*(256-112)=10msec

	TimerFlag = 1;			//Set 10msec EVT flag ! (EXE @ main routine)
	
	Timer1Cnt++;
	if(Timer1Cnt >= 100)	//10 x 100 = 1 sec
	{
		Timer1Cnt = 0;
		Timer1Flag = 1;		//Set 1sec EVT flag ! (EXE @ main routine)
	}
}

ISR(TIMER2_OVF_vect)
{
	//  주파수변경을 위한 ovf시간 바꿈
	//  static변수로 timer2_cnt 선언하여 3종류 음발생
	if(timer2_tcnt==0)
	TCNT2 = (256-120); 		//(64/14.7456M) * 44 = 191 usec
	else if(timer2_tcnt==1)
	TCNT2 = (256-150); 		//(64/14.7456M) * 44 = 191 usec
	else if(timer2_tcnt==2)
	TCNT2 = (256-180); 		//(64/14.7456M) * 44 = 191 usec
	else
	TCNT2 = (256-220); 		//(64/14.7456M) * 44 = 191 usec
	///////////////////////////////////////////////////////
	
	if((Beep_OnOff==OFF)|(Beep_Count==0))	PORTG &= ~BUZ_SOUND;
	else
	{
		Beep_Timer--;
		if(Beep_Timer>=200)
		{
			PORTG ^= BUZ_SOUND;	//Beep Gen
		}
		else if(Beep_Timer>0)
		{
			PORTG &= ~BUZ_SOUND;	//Beep off		else if(Beep_Timer==0)
		}
		else if(Beep_Timer==0)
		{
			Beep_Count--;
			if(Beep_Count==0)	BeepOff();
			else
			{
				PORTG ^= BUZ_SOUND;	//Beep Gen
				Beep_Timer = 400;
			}
		}
	}
}
/*----------------------------------------------------*/
// Beep ON/OFF
/*----------------------------------------------------*/
void BeepOn(BYTE count)
{
	if(Beep_OnOff==ON)
	{
		Beep_Timer = 400;
		Beep_Count = count;
	}
}

void BeepOff(void)
{
	Beep_Timer = 400;
	Beep_Count = 0;
}
void Delay(unsigned long delay)
{
	while(delay--)	asm("nop");
}

////////////////////////////////////////////////////////////////////////////////

void Delay_us(WORD time_us) //measured by oscilloscope
{
	WORD i;
	
	for(i=0; i<time_us; i++)//4Cycle +    
	{	    
		asm("PUSH R0");   	//2Cycle +
	  	asm("NOP");   		//1Cycle +
	  	asm("NOP");   		//1Cycle +	    
	  	asm("NOP");   		//1Cycle +	    
	  	asm("NOP");   		//1Cycle +	    
	  	asm("NOP");   		//1Cycle +	    	    
	  	asm("POP R0");	   	//2Cycle = 13Cycle = 1us for 14.7456MHz
	}
}

////////////////////////////////////////////////////////////////////////////////

void Delay_ms(unsigned int time_ms) //Time Delay for ms
{
	WORD i;
	
  	for( i=0; i<time_ms; i++)
  	{
  		Delay_us(250);
    	Delay_us(250);
    	Delay_us(250);
    	Delay_us(250);
  	}    
}
	
/*----------------------------------------------------------*/
// 16x2 LCD Module Control Routines !!
/*----------------------------------------------------------*/

void InitLCD(void)
{ 		
	WriteLcdCmd(0x38);		//CMD 38H
	Delay(100);		
	WriteLcdCmd(0x08);		//CMD 08H
	Delay(100);		
	WriteLcdCmd(0x01);		//CMD 01H
	Delay(2000);			
	WriteLcdCmd(0x0C);		//CMD 0CH
	Delay(100);			
	WriteLcdCmd(0x06);		//CMD 06H
}

void WriteLcdCmd(BYTE cmd)
{ 	
	PORTF &= ~LCD_RS;	//RS,RW = 0,0 (WRITE CMD)
	PORTF &= ~LCD_RW;	
	Delay(10);	
	
	PORTF |= LCD_EN;	//E = 1	
	Delay(10);	
	
	LCD_DATA = cmd;		//CMD 
	Delay(10);
	
	PORTF &= ~LCD_EN;	//E = 0
	Delay(10);
	
	PORTF &= 0xC7;		//E,RS,RW = 0,0,0
	Delay(10);	
}

void WriteLcdData(BYTE loc, BYTE data)
{ 		
	WriteLcdCmd(loc);	//set location

	PORTF |= LCD_RS;	//RS,RW = 1,0 (WRITE DATA)
	PORTF &= ~LCD_RW;	
	Delay(10);	
	
	PORTF |= LCD_EN;	//E = 1	
	Delay(10);	
	
	LCD_DATA = data;	//CMD 
	Delay(10);
	
	PORTF &= ~LCD_EN;	//E = 0
	Delay(10);
	
	PORTF &= 0xC7;		//E,RS,RW = 0,0,0
	Delay(10);	
}

/*----------------------------------------------------------*/
void Init_Display(void)
{
	sputsLcdLine1("TEST KIT:MTK-AT8");
	sputsLcdLine2(" KEY data = ..  ");
}

/*----------------------------------------------------------*/
void sputsLcdLine1(unsigned char *str)
{
	int i;

	for(i=0; *(str+i)!=0; i++)	WriteLcdData(0x80+i,*(str+i));
}

void sputsLcdLine2(unsigned char *str)
{
	int i;

	for(i=0; *(str+i)!=0; i++)	WriteLcdData(0xC0+i,*(str+i));
}

////////////////////////////////////////////////////////////////////////////////
//		4x4 KEY SCAN ROUTINE
////////////////////////////////////////////////////////////////////////////////

void Key44_Scan(void)
{	
	BYTE	scan_count;	
	
	if(scan_chatt_count>=CHAT_COUNT)
	{
		PORTE = 0x0F;									//PE0~3:HIGH-풀업초기화, PE4~7:LOW->출력비트 초기화
		Delay(10);	
		if((PINE&0x0F) == 0x0F)	scan_chatt_count = 0; 	//포트가 0x0F로 안정화된 것 확인 및 채터링카운트값 초기화
	}
	else
	{		
		scan_count = 0;
		
		PORTE = 0xEF;	// 첫번째 스위치 k1,k2,k3,k4를 체크하기 위한 루틴
						// scan_out 1110(E), 0xEF의 1111(F)는 PE0~3까지 pull-up을 사용하기 위한 값
						// 1110(E)는 버튼 k1에 내부풀업을 시켰으므로 PE4(책p124.은 PC4)는 "0"을 출력하여
						// 접지를 만든다.  버튼1번이 눌리면 PINE=11101110이 입력됨
						// PINE의 상위니블 1110은 출력값이고 하위니블 1110의 "0"은 풀업시킨 버튼이 눌려서 읽어들인 값.
		Delay(10);	
// 예1) 키1번이 눌린경우
		if((PINE&BIT0) == 0)	{scan_key=1;scan_count++;}	// PINE & 0x01 =11101110&00000001=0x00, 0이됨.즉 버튼1번이 눌림
		if((PINE&BIT1) == 0)	{scan_key=4;scan_count++;}	// PINE & 0x02 =11101110&00000010=0x02,즉 버튼2번 안눌림		
		if((PINE&BIT2) == 0)	{scan_key=7;scan_count++;}	// PINE & 0x04 =11101110&00000100=0x04,즉 버튼3번 안눌림		
		if((PINE&BIT3) == 0)	{scan_key=11;scan_count++;}	// PINE & 0x08 =11101110&00001000=0x08,즉 버튼4번 안눌림		
// scan_key=1,scan_count=1

//예2) 키2번이 눌린경우
/*		if((PINE&BIT0) == 0)	{scan_key=1;scan_count++;}	// PINE & 0x01 =11101101&00000001=0x01,즉 버튼1번 안눌림
		if((PINE&BIT1) == 0)	{scan_key=4;scan_count++;}	// PINE & 0x02 =11101101&00000010=0x00,0 이됨.즉 버튼2번이 눌림		
		if((PINE&BIT2) == 0)	{scan_key=7;scan_count++;}	// PINE & 0x04 =11101101&00000100=0x04,즉 버튼3번 안눌림		
		if((PINE&BIT3) == 0)	{scan_key=11;scan_count++;}	// PINE & 0x08 =11101101&00001000=0x08,즉 버튼4번 안눌림		
		 scan_key=4,scan_count=1
 예3)키3번이 눌린경우
		if((PINE&BIT0) == 0)	{scan_key=1;scan_count++;}	// PINE & 0x01 =11101101&00000001=0x01,즉 버튼1번 안눌림
		if((PINE&BIT1) == 0)	{scan_key=4;scan_count++;}	// PINE & 0x02 =11101101&00000010=0x02,즉 버튼2번 안눌림		
		if((PINE&BIT2) == 0)	{scan_key=7;scan_count++;}	// PINE & 0x04 =11101101&00000100=0x00,0이됨.즉 버튼3번이 눌림		
		if((PINE&BIT3) == 0)	{scan_key=11;scan_count++;}	// PINE & 0x08 =11101101&00001000=0x08,즉 버튼4번 안눌림		
		 scan_key=7,scan_count=1
 예4)키4번이 눌린경우
		if((PINE&BIT0) == 0)	{scan_key=1;scan_count++;}	// PINE & 0x01 =11101101&00000001=0x01,즉 버튼1번 안눌림
		if((PINE&BIT1) == 0)	{scan_key=4;scan_count++;}	// PINE & 0x02 =11101101&00000010=0x02,즉 버튼2번 안눌림		
		if((PINE&BIT2) == 0)	{scan_key=7;scan_count++;}	// PINE & 0x04 =11101101&00000100=0x04,즉 버튼3번 안눌림		
		if((PINE&BIT3) == 0)	{scan_key=11;scan_count++;}	// PINE & 0x08 =11101101&00001000=0x00, 0이됨.즉 버튼4번이 눌림		
		 scan_key=11,scan_count=1
		 
		 scan_key=1	:첫번째버튼 k1이 눌리면 버튼값을 1로 배정함
		 scan_key=4	:두번째버튼 k2이 눌리면 버튼값을 4로 배정함	
		 scan_key=7	:세번째버튼 k3이 눌리면 버튼값을 7로 배정함	
		 scan_key=11        :네번째버튼 k4이 눌리면 버튼값을 11로 배정함
////////////////--------------------------------//////////////
//////////////////////   버튼 값 키배정 //////////////////////
//				--------------------------------			//
//						1	2	3	13						//
//						4	5	6	14
//						7	8	9	15						//
//						11	10	12	16						//
//////////////////////////////////////////////////////////////
*/
		PORTE = 0xDF;	//scan_out 1101 ,두번째 k5,k6,k7,k8을 체크하기 위한 루틴
		Delay(10);				
		if((PINE&BIT0) == 0)	{scan_key=2;scan_count++;} // PINE & 0x01 =11011110&00000001=0x00,즉 버튼5번이 눌림
		if((PINE&BIT1) == 0)	{scan_key=5;scan_count++;} // PINE & 0x02 =11011101&00000010=0x00,즉 버튼6번이 눌림			
		if((PINE&BIT2) == 0)	{scan_key=8;scan_count++;} // PINE & 0x04 =11011011&00000001=0x00,즉 버튼7번이 눌림			
		if((PINE&BIT3) == 0)	{scan_key=10;scan_count++;}// PINE & 0x08 =11010111&00000001=0x00,즉 버튼8번이 눌림			

		PORTE = 0xBF;	//scan_out 1011, 세번째 k9,k10,k11,k12를 체크하기 위한 루틴
		Delay(10);				
		if((PINE&BIT0) == 0)	{scan_key=3;scan_count++;}
		if((PINE&BIT1) == 0)	{scan_key=6;scan_count++;}			
		if((PINE&BIT2) == 0)	{scan_key=9;scan_count++;}			
		if((PINE&BIT3) == 0)	{scan_key=12;scan_count++;}			

		PORTE = 0x7F;	//scan_out 0111 ,네번째 k13,k14,k15,k16을 체크하기 위한 루틴
		Delay(10);				
		if((PINE&BIT0) == 0)	{scan_key=13;scan_count++;}
		if((PINE&BIT1) == 0)	{scan_key=14;scan_count++;}			
		if((PINE&BIT2) == 0)	{scan_key=15;scan_count++;}			
		if((PINE&BIT3) == 0)	{scan_key=16;scan_count++;}			
		
		PORTE = 0xFF;	//상위니블을 1111로 만듬.다시 위쪽의 하나만 0이고 나머지는 1로 스캔 하기전 모든 비트를 1로 초기화시킴
								
		if(scan_count != 1)	return;	//if double key or no key, return,
									// scan_count값이 0이면  한번도 안눌린것임. 또는 2가 된것은 두개의 키가 눌려서 ++로 동시눌림을 
									//하여 두개눌린것을 버리기 위한용, 하나만 눌리면 scan_count=1이 되고 전의 값과 새로운 값이 같은가를
									//기준으로 채터링함.채터링이 계속되면 "scan_key != old_key44"상태가 반복될 것이고,
									//"scan_key = old_key44"이되면 채터링이 끝나고 키가 안정적인 상태임을 나타냄	
									// "scan_key = old_key44"인 상태가 5회이상되면 눌린키의 안정된 값으로 판단하여 
									//	key44_event = 1 로 메인루틴에게 눌린키의 값이 있다는 플랙을 보내줌	
			
		if(scan_key != old_key44)	// 채터링이 계속되면 키값이 바뀜.
		{
			old_key44 = scan_key;
			scan_chatt_count = 0;	//채터링이 계속되면 같은 키값이 5번 연속될때까지 값을 버림-scan_chatt_count를 초기화함. 
									//같은 값이 5회이상되면 눌린키값으로  인정하기 위함.		
		}
		else						//채터링이 끝나고 키값이 안정되면, 전의 값과 새로운 값이 같으면	scan_chatt_count값을 1로 만들고
									// 이값이 5회이상 지속되면 눌린키의 번호를 	Key44_Data 로 넘김
		{
			scan_chatt_count++;
			if(scan_chatt_count>=CHAT_COUNT)	//5회이상 되면 
			{
				Key44_Data = old_key44;			//scan_key값을 key44_data에 저장하고 메인으로 복귀할려고 함, 전역변수로 지정되어
												// 있으므로 메인에서 key44_data, scan_key
				old_key44 = 0;					// 값으로 눌린 키번호/값을 확인할 수 있음.
														
				key44_event = 1;				//Set KEY EVT flag ! (EXE @ main routine),키44이벤트가 발생한것을 기록함.							
			}			
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//		KEY_DATA_EXE
////////////////////////////////////////////////////////////////////////////////

void Key44_Job_Exe(void)
{

//	WriteLcdData(0xCC,pgm_read_byte(&hexdigit[Key44_Data/10]));		
//	WriteLcdData(0xCD,pgm_read_byte(&hexdigit[Key44_Data%10]));	
	
	
	switch(Key44_Data)
	{ 
    	case  1		:	Key44_0_0();	break;		        			
    	case  2		:	Key44_0_1();	break;		        			
    	case  3		:	Key44_0_2();	break;		    
    	case  4		:	Key44_0_3();	break;		        			
    	case  5		:	Key44_0_4();	break;		        			
    	case  6		:	Key44_0_5();	break;		        			
    	case  7		:	Key44_0_6();	break;		        			
    	case  8		:	Key44_0_7();	break;		        			
    	case  9		:	Key44_0_8();	break;		        			
    	case  10	:	Key44_0_9();	break;
    					        			
    	case  11	:	Key44_A();		break;
    	case  12	:	Key44_B();		break;    			
    	case  13	:	Key44_C();		break;    			
    	case  14	:	Key44_D();		break;    			
    	case  15	:	Key44_E();		break;    			
    	case  16	:	Key44_F();		break;    			      			    					
      	default		:	break;
	}
}
//byte scan_key_position,eeprom_add;
void Key44_0_0(void)
{
//	PORTB=pattern_Fnd[0];  //십진수 "1" 표시로 변경
	PORTB=pattern_Fnd[1];  //십진수 "1" 표시로 변경

	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
	lcd_display_position(2,scan_key_position++);
	lcd_data(0x31);//CGROM의 주소를 인수로 써주면 lcd에 숫자를 표시함 '1'=0x31
//	key_data=0x31;
//	EEPROM_write(eeprom_add++,0x31);

}
void Key44_0_1(void)
{
	PORTB=pattern_Fnd[2];

	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
	lcd_display_position(2,scan_key_position++);
//	EEPROM_write(eeprom_add++,0x32);
//	lcd_display_position(2,2);// lcd의 line1번에 먼저 표시하고 2라인에 표시해야 커서가 아래에 위치함
	lcd_data(0x32);	//CGROM의 주소를 인수로 써주면 lcd에 숫자를 표시함 '1'=0x31



}
void Key44_0_2(void)
{
	PORTB=pattern_Fnd[3];

	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
	lcd_display_position(2,scan_key_position++);
//	lcd_display_position(2,3);
	lcd_data(0x33);
//	EEPROM_write(eeprom_add++,0x33);
}
void Key44_0_3(void)
{
	PORTB=pattern_Fnd[4];


	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
//	itoa(scan_key,buf,10);
	lcd_string("  ");
	_delay_ms(5);
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
	lcd_display_position(2,scan_key_position++);
//	lcd_display_position(2,4);
	lcd_data(0x34);
//	EEPROM_write(eeprom_add++,0x34);
}
void Key44_0_4(void)
{
	PORTB=pattern_Fnd[5];

	
	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
//	_delay_ms(5);
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
	lcd_display_position(2,scan_key_position++);
//	lcd_display_position(2,5);
	lcd_data(0x35);
//	EEPROM_write(eeprom_add++,0x35);
}
void Key44_0_5(void)
{
	PORTB=pattern_Fnd[6];

	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
//	_delay_ms(5);
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
	lcd_display_position(2,scan_key_position++);
//	lcd_display_position(2,6);

	lcd_data(0x36);
//	EEPROM_write(eeprom_add++,0x36);
}
void Key44_0_6(void)
{
	PORTB=pattern_Fnd[7];

	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
//	_delay_ms(5);
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
	lcd_display_position(2,scan_key_position++);
//	lcd_display_position(2,7);

	lcd_data(0x37);
//	EEPROM_write(eeprom_add++,0x37);
}
void Key44_0_7(void)
{
	PORTB=pattern_Fnd[8];

	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
//	_delay_ms(5);
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
	lcd_display_position(2,scan_key_position++);
//	lcd_display_position(2,8);

	lcd_data(0x38);
//	EEPROM_write(eeprom_add++,0x38);
}
void Key44_0_8(void)
{
	PORTB=pattern_Fnd[9];
	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
//	_delay_ms(5);
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
	lcd_display_position(2,scan_key_position++);
//	lcd_display_position(2,9);

	lcd_data(0x39);
//	EEPROM_write(eeprom_add++,0x39);
}

void Key44_0_9(void)
{
	PORTB=pattern_Fnd[0];	// FND에 "0"표시
	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
//	_delay_ms(5);
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_data(0x30);				//scan_key값이 10진수로 변환되어 buf에 저장된 값이 10 이므로 0으로 만들어줌 
//	lcd_string(buf);			//윗 라인에서 0으로 표시하여 없앰
	lcd_display_position(2,scan_key_position++);
//	lcd_display_position(2,10);

	lcd_data(0x30);
//	EEPROM_write(eeprom_add++,0x30);
}

void Key44_A(void)
{
	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
	set_key_flag=1;
	lcd_display_clear();
	lcd_display_position(1,1);
	lcd_string("press new_Key");
	lcd_display_position(2, 1);
	lcd_string("             ");

}

void Key44_B(void)
{	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
//	_delay_ms(5);
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
//	lcd_display_position(2,12);

//	lcd_data(0x42);
	open_key_flag = 1;

	lcd_display_clear();
	lcd_display_position(1,1);
	lcd_string("press KeyVal.");
	lcd_display_position(2, 1);
	lcd_string("             ");

}

void Key44_C(void)
{	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
//	_delay_ms(5);
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
//	lcd_display_position(2,13);

//		lcd_data(0x43);
}

void Key44_D(void)
{	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
//	_delay_ms(5);
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
//	lcd_display_position(2,14);

//	lcd_data(0x44);
}

void Key44_E(void)
{	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
//	_delay_ms(5);
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
//	lcd_display_position(2,15);

//	lcd_data(0x45);
}

void Key44_F(void)
{	lcd_display_position(1,15);	//스캔된 값을 lcd위라인에 표시,표시하기 위해서는 sprintf()함수를 사용해야함
	lcd_string("  ");			//십진수표시된 값을 지우기위한 문자열로 공백뒤에 다시 포지션을 지정해줘야 함
//	_delay_ms(5);
	lcd_display_position(1,15);
	itoa(scan_key,buf,10);
	lcd_string(buf);
//	lcd_display_position(2,16);

//	lcd_data(0x46);
}

////////////////////////////////////////////////////////////////////////////////
void EEPROM_test(void)
{
unsigned char data;
char msg[16];
int i;

	lcd_display_OnOff(1, 0, 0);	// 커셔, 블랭크 Off
	lcd_display_clear();
	lcd_string("EEPROM Write");
	lcd_display_position(2, 1);
	for(i=0x30 ; i < 0x39 ; i++) 	{
		EEPROM_write(i, i);		// EEPROM 라이트 
		lcd_display_position(2, 1);
		sprintf(msg, "Addr:0x%2x = %c", i, i);
		lcd_string(msg);
		_delay_ms(50);
	}

	_delay_ms(1000);

	lcd_display_clear();
	lcd_string("EEPROM Read");
	lcd_display_position(2, 1);
	for(i=0x30 ; i < 0x39 ; i++) 
	{
		data = EEPROM_read(i);	// EEPROM 리드
		sprintf(msg, "Addr:0x%2x = %c", i, data);
		lcd_display_position(2, 1);
		lcd_string(msg);
	}
}

void EEPROM_write(unsigned char addr, unsigned char data)
{
	while(EECR & 0x02); 		// EEWE=0일 때까지 대기
	SREG = 0x00; 				// 전역 인터럽트 디스에이블
	EEAR = addr; 				// EEPROM에 라이트할 주소
	EEDR = data; 				// EEPROM에 데이터 라이트
	EECR |= 0b00000100; 		// EEMWE=1
	EECR |= 0b00000010; 		// EEWE=1
	SREG = 0x80; 				// 전역 인터럽트 인에이블
}	

unsigned char EEPROM_read(unsigned char addr)
{
	while (EECR & 0x02); 		// EEWE=0일 때까지 대기
	EEAR = addr; 				// EEPROM에 리드할 주소
	EECR |= 0b00000001; 		// EERE=1
	
	return EEDR; 				// EEPROM에서 데이터 리드
}

void lcd_Display(void)
{
	lcd_display_clear();
	lcd_display_position(1,1);
	lcd_string("DoorKey System");
	lcd_display_position(2, 1);
	lcd_string("PressKey 11or12");

	_delay_ms(50);

}
/////////////////////////////

void Led_On(void)
{
	PORTA=0x01;
	_delay_ms(500);
	for (int i=0;i<=8;i++)
		{
		  PORTA= PORTA<<1;
		_delay_ms(500);
		}
}

/*
#define DBC0 (*(volatile unsigned short *)0xFFFB6) 로 사용

0x8000은 상수이므로 주소타입이 아님. 0x8000을 주소타입으로 변환하여 메모리에 할당하되 무부호8비트 데이터를 저장하는 장소를
 가르키는 용도로 사용하겠다.즉 0x8000번지의 내용은 무부호 1바이트인 데이터이고 위치는 0x8000번이다.
  "(*("는 캐스팅연산자 의 두번째 괄호는 우선순위를 나타내고 *는 포인터로 LCD_DATA를 치환하라이며 -포인터명- 앞의 괄호는 전체처리부분을
   괄호로 묶어야한다는 매크로사용시주의할점때문에 묶어주었다. 0x8000자체가 리터럴상수이므로 상수로 메모리를 접근할수 없다.
    0x8000=100; 은 0x8000번에 100을 넣고싶은데 상수에 상수를 넣는 꼴이 된다.고로0x8000을 주소값이라고 명시해줘야한다.
	 ->*(*)0x8000이라고 하면됨.앞으로 *는 포인터이고 (*)는 0x8000을 주소타입으로 변환한다는캐스팅연산이며
	  연산자우선순위를 위한 괄호를 사용하였고. 앞의 *를 통하여 주소를 나타낸 포인터임을 표시하였다.
	   하지만 0x8000번지에 들어갈 데이터타입을 지정하지 않았다. *(unsigned int*)0x8000=100;으로 하면
	    데이터크기도 지정하였으나 메모리를 사용하기 위하여 앞에 *(volatile unsigned int*)0x8000=100;으로 하면 완벽하다.
		 이것을 매크로명령으로 만들기 우해선 #define Mem_add (*(volatile unsigned int*)0x8000) 로 선언해주면된다. 
		 사용할때는 Mem_add=100;으로 하면되고 이는 메모리의 0x8000번지에 양의정수 100을 저장하게 된다.


*/
/* 
printf() scanf(),sprintf() 사용
1)솔루션탐색기 -프로젝트명 오른쪽버튼-Add Library -Name 에서 libprintf_fit와 libscanf_min 선택  -OK
   프로젝트 익스플로어에 해당 라이브러리가 추가됨
2) 프로젝트 - ADC_01 속성(Alt+F7) -Toolchain - AVR/GNU Linker -General - use vprintf library(-Wl,-u,vfprintf) 선택 -파일저장(Cntl+S)
*/