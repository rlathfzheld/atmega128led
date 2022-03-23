#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
unsigned char pattern[10]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x00,0x00};  //하나만 켜지기 for 8 LEDs
unsigned char pattern_Fnd[10]={0b0100000,0b1111001,0b1000100,0b1010000,0b0011001,0b0010010,0b0000010,0b0111000,0b0000000};  //for 8 FND
// CA type  5611BH model http;//exeobd.com
//https://github.com/abcbank/2019_HW_winter/blob/master/doc/boot4dim_AVR.md  //AVR HW스터디

int main()
{
	int i=1;				// FND에 "1"~"8" 표시하기
	unsigned Led_1;
	
	DDRA=0xFF;  		 // for LEDs;		// for FND
	DDRC=0xFF;  	  	// LCD control for PC0 ~ PC2,Output +5V of LCD VCC, VL at PC4 ~PC7
	DDRF=0x00;		// for Pull-up button PF0,PG0는 동작안됨   	
	PORTF=0xFF; 		// Pull-up Start

	_delay_ms(100);	// for the +5V stable
	
	//PORTA=pattern[i];
	PORTA=0x00;
	PORTC=0x00;		// all segment on
	PORTA=0xFF;					//initial High for All Leds
	
	_delay_ms(1000);
	Led_1=0x01;
	
	while(1)
	{
		//		while(PINF & 0x01);		//스위치 눌림 기다림 - 안눌리면 True로 만들어 기다리게 함
		// 아래 구문도 동작에 문제 없음
		while(!(~PINF & 0x01)); //스위치 눌림 기다림 - 안눌리면 True로 만들어 기다리게 함
		//안눌림)  PINF=1111 1111 ,~PINF=0000000
		//			0000 0000 & 0000 0001 =0x00, !(0x00)= 1 (True)
		//			->안 눌리면 참으로 계속 기다림
		// 눌리면) PINC=1111 1110,~PINC=0x01
		//				0x01 & 0x01 =0x01이고 !(0x01) = 0x00 (False)로
		//				빠져나감, 다음 문장 수행함
		_delay_ms(20);			//debouncing time

		if(i==9) i=1;
		PORTA=pattern[i];      //LED
		PORTA=Led_1;
		
		if (Led_1==0x80)		// MSB이 "1"이 되면 LSB부터 점등시키기
		{
			Led_1=0x01;
		}
		else
		{	Led_1=Led_1<<1;}
		
		PORTC=pattern_Fnd[i++];  //FND
		_delay_ms(200);

		while(~PINF& 0x01);	//위와 반대로 "1"이되면 빠져나가도록 동작하기 위함 ! 버튼을 계속 누르고 있으면 "0"이 입력되어
		// 전체적인 while loop가 연속으로 동작되어 LED,FND가 연속동작됨. 버튼을 누를때 한번만 동작하기 위함
		// 버튼을 떼면 "1"이되어 게산시 'False'로 빠져나간다
		_delay_ms(20);			//debouncing time

	}
}

	
