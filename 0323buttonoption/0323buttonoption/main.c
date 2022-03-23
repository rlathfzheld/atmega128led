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
	int t;
	DDRA=0xFF;   // for LEDs
	DDRF=0x00;	 // for Pull-up button,PF0
	PORTF=0xFF;  // Pull-up Start
//	char Temp=0x00;
	PORTA=0x00;
	while(1)		// LED test
	{
		for(int i=0;i<8;i++)
		{
			PORTA=(PORTA<<1)|0x01;
			//Temp=(Temp<<1)|0x01;
			//PORTA=Temp;
			_delay_ms(100);
		}
		PORTA = 0x00;
		for(int i=0;i<8;i++)
		{
			PORTA=(PORTA>>1)|0x80;
			//Temp=(Temp>>1)|0x80;
			//PORTA=Temp;
			_delay_ms(100);
		}
//		PORTA=(PORTA<<1)|0x01;	// PORTA 사용

//		Temp=(Temp<<1)|0x01;	// PORTA 사용
//		PORTA=Temp;
//		_delay_ms(500);

//		if(Temp == 0xFF)	// 왼쪽으로 스캔 추가하기
		if(PORTA == 0xFF)	// 왼쪽으로 스캔 추가하기
				{	
			PORTA=0x00;
//			Temp=0x00;
			break;
		}
	}


	 while(1){
		 if(!(PINF&0x01)){
			 PORTA=0B00001001;         // LED 1개 ON
			 _delay_ms(50);             // 키를 누를 때의 채터링 방지용 딜레이
			 t=0;
			 while(t<1500){             // 1.5초 체크
				 _delay_ms(1); t++;
				 if(PINF&0x01)break;    // 중간에 스위치에서 손을 떼면 빠져나감
			 }
			 if(t>=1500){ 
				 PORTA=0xFF; 
				 _delay_ms(500);
				 } // LED 8개 ON // 길게 누른 경우
			 else       { 
				 PORTA=0x0F; 
				  _delay_ms(500);
				  } // LED 4개 ON // 짧게 누른 경우
			 while(!(PINE&0x01));       // 아직도 키를 누르고 있다면 뗄 때까지 대기
			 _delay_ms(50);             // 키를 뗄 때의 채터링 방지용 딜레이
		 }  
	 }
 }
