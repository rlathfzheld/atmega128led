/*
 *switch8LED.c
 *
 * Created: 2022-03-16 오전 11:00:25
 * Author : konyang
 */ 

//버튼으로 LED 동작 좌우 시프트 시키기

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int button_flag1;

int main(void)
{
	DDRA=0xFF;   // for LEDs
	DDRF=0x00;	 // for Pull-up button,PF0
	PORTF=0xFF;  // Pull-up Start
//	char Temp=0x00;
	PORTA=0x00;
	while(1)		// LED test
	{
		PORTA=(PORTA<<1)|0x01;	// PORTA 사용

//		Temp=(Temp<<1)|0x01;	// PORTA 사용
//		PORTA=Temp;
		_delay_ms(500);

//		if(Temp == 0xFF)	// 왼쪽으로 스캔 추가하기
		if(PORTA == 0xFF)	// 왼쪽으로 스캔 추가하기
				{	
			PORTA=0x00;
//			Temp=0x00;
			break;
		}
	}
	

	button_flag1=0;
	while(!(~PINF & 0x01));		//버튼 누르면 빠져나감
	/*
		스위치 눌림 기다림 PINF=1111 1111 ,~PINF=0000000
		아래와 같이 동작하나 코드의 가독성을 높이기 위하여 ~연산자를 사용하고 부정연산자! 를 사용하여
		버튼이 누르면 False가 되어 다음 문장을 수행시키기 위함이다

		1) 버튼이 안 눌리면
		!(~PINF & 0x01); PINF = 1111 1111,  ~(PINF)=0000000
						 !(0000 0000 & 0000 0001) = !(0x00) = 0xff (True)
						---> 안 눌리면 참으로 계속 기다림
		2)버튼 눌리면 PF0가 "Low"로 됨
		 ~(PINF)=~(1111 1110)=0x01
		 !(0x01 & 0x01) = !(0x01) = 0x00 (False)로	빠져나감, 
		 
		 while문을 빠져나가게 되면 바운스현상을 잡을 수 없기 때문에 
		 일정시간 지연을 두고 버튼이 떼는 순간 동작하기 위하여 다음 문장을 추가
		 하면 보다 확실한 동작이 됨
	*/

	_delay_ms(20);			// debouncing time
	while(~PINF & 0x01);	// 스위치를 떼는 순간 동작됨 

	while(1)
	{
	  
		// 동작 토글용 검사
		if(button_flag1 == 0)			//Led_On (8 -->1) 시작
		{

			for(int i=0;i<8;i++)
			{
				PORTA=(PORTA>>1)|0x80;
				//Temp=(Temp>>1)|0x80;
				//PORTA=Temp;
				_delay_ms(300);
			}
			
		}
		else
	    {
			for(int i=0;i<8;i++)
			{
				PORTA=(PORTA<<1)|0x01;
				//Temp=(Temp<<1)|0x01;
				//PORTA=Temp;
				_delay_ms(200);
			}
			
	    }
			
		while(!(~PINF & 0x01));		//버튼 누르면 빠져나감		
			 _delay_ms(20);
			 PORTA=0x00;
//			 Temp=0x00;
			button_flag1=~button_flag1;	
	
		while(~PINF & 0x01);		//스위치를 떼는 순간 동작됨
	}
	
}


