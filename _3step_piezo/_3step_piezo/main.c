/*
 * piezo.c
 *
 * Created: 2022-09-22 오후 5:05:40
 * Author : 교원송영진
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "define.h"

///////////////////////// variables define  ////////////////////////////////

BYTE TimerFlag;
WORD Timer1Cnt;
BYTE Timer1Flag;

BYTE	scan_key,old_key44,Key44_Data,scan_chatt_count,key44_event;
BYTE    Beep_OnOff;
BYTE    Beep_Count;
WORD    Beep_Timer;
BYTE    KeyPress;
BYTE    KeyOld;
BYTE    KeyData;

//// 주파수변경용 변수 static
static int timer2_tcnt=0;

/////////////////////////   prototype declaration   /////
void KeyScan();
void BeepOn(BYTE count);
void BeepOff(void);
void InitPort(void);
void InitTimer0(void);
void InitTimer2(void);
void Delay(unsigned long delay);
void Delay_ms(unsigned int time_ms); //Time Delay for ms
void Delay_us(WORD time_us); //measured by oscilloscope


int main(void)
{
	SPH = 0x10;   SPL = 0xff; // Stack Poniter Setting.
	InitPort();
	InitTimer0(); 	//Init Timer0, Init Register
	InitTimer2(); 	//Init Timer2, Init Register (for beep)
	Delay_ms(100);	//Wait for more than 30msec
	sei();		//SREG = 0x80;	//SREG.I = 1
	Beep_OnOff = ON;
	KeyOld = 0xF0;
	BeepOn(4);		//테스트용
	KeyData=0;
	
	while(1)
	{
		if(TimerFlag) 		//job EXE every 10 msec
			{
				KeyScan();
				TimerFlag = 0;		//Clear 10msec EVT flag			}
			}
	
		if(key44_event == 1)
				{

//  주파수변경을 위한 ovf시간 바꿈
//  static변수로 timer2_cnt 선언하여 3종류 음발생
					BeepOn(2);	 //음 2회 발생
					timer2_tcnt++;
					if(timer2_tcnt>=4)timer2_tcnt=0;
///////////////////////////////////////////////////////					
						PORTA=0xFF;	 //LED확인용
						Delay_ms(200);
						PORTA=0x00;
						key44_event = 0;   	//Clear KEY EVT flag
				}
	}
}
////////////////////////////////////////////////////////////////////////////////
//	KeyScan() ROUTINE
////////////////////////////////////////////////////////////////////////////////
void KeyScan()	// Port E4 button_pressed
{
		if(!(PINE & 0x10))		//버튼 누르면
			{	scan_chatt_count++;			}
		if(scan_chatt_count>=5) {	// 같은 키 5번 입력되면
			key44_event=1;
			scan_chatt_count=0;
		}
		
	// button PE4_pullup
		Delay_ms(2);
		PORTE=0xFF;
}


////////////////////////////////////////////////////////////////////////////////
//	TIMER0 INTERRUPT SERVICE ROUTINE
////////////////////////////////////////////////////////////////////////////////
	
ISR(TIMER0_OVF_vect)
{
	TCNT0 = 112;	//TCNT =184 ->(1024/14.7456MHz)*(256-112)=10msec
	
	TimerFlag = 1;	//Set 10msec EVT flag ! (EXE @ main routine)

	Timer1Cnt++;
	if(Timer1Cnt >= 100)	//10 x 1000 = 10 sec
	{
		Timer1Cnt = 0;
		Timer1Flag = 1;	//Set 1sec EVT flag ! (EXE @ main routine)
	}
}

////  키를 누를때마다 BeepOn(BYTE count) 를 호출하면 됨
//    포트G0을 일반포트이지만 강제로 1-0-1-0를 반복시켜서 주파수를 만들어냄
//    버저는 PG0에 연결하여 사용
//    타이머는 오버플로우인터럽트발생마다 음생산

ISR(TIMER2_OVF_vect)
{
//  주파수변경을 위한 ovf시간 바꿈
//  static변수로 timer2_cnt 선언하여 3종류 음발생
	if(timer2_tcnt==0)
		TCNT2 = (256-44); 		//(64/14.7456M) * 44 = 191 usec
	else if(timer2_tcnt==1)
		TCNT2 = (256-100); 		//(64/14.7456M) * 44 = 191 usec
	else if(timer2_tcnt==2)
		TCNT2 = (256-156); 		//(64/14.7456M) * 44 = 191 usec
	else
	TCNT2 = (256-200); 		//(64/14.7456M) * 44 = 191 usec
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
void InitPort(void)
{
	DDRA=0xFF;	// LED확인용
	PORTA=0XFF;
	
// button PE4_pullup 
	DDRE = 0x00;	  // input
	PORTE= 0xFF;
	
	DDRG = 0xFF; 	//PG0:Beep Out
	PORTG = 0xFF; 	//PG0:Init low
	Delay_ms(2000);
	PORTG = 0xFE; 	//PG0:Init low

	PORTA=0X00;
}
////////////////////////////////////////////////////////////////////////////////
// INIT Timer0 Interrupt
////////////////////////////////////////////////////////////////////////////////
void InitTimer0(void)
{
	TCCR0 = 0x07; 	// Trigger => Fclock/1024
	TCNT0 = 0x00;	// clear Timer Counter Register
	TIMSK = 0x01; 	// Timer0 Interrupt Enable Mask Register
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
void Delay(unsigned long delay)
{
	while(delay--)	asm("nop");
}

////////////////////////////////////////////////////////////////////////////////

void Delay_us(WORD time_us) //measured by oscilloscope
{
	WORD i;
	
	for(i=0; i<time_us; i++)	//4Cycle +
	{
		asm("PUSH R0"); 	//2Cycle +
		asm("NOP"); 	//1Cycle +
		asm("NOP"); 	//1Cycle +
		asm("NOP"); 	//1Cycle +
		asm("NOP"); 	//1Cycle +
		asm("NOP"); 	//1Cycle +
		asm("POP R0");	//2Cycle = 13Cycle = 1us for 14.7456MHz
	}
}
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




