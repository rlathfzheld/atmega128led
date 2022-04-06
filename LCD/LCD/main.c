/*
 * Kimchi.c
 *
 * Created: 2019-03-26 오후 4:51:56
 * Author : User
 */ 

/*
 * lcd_2.c
 *
 * Created: 2019-03-26 오후 4:36:57
 * Author : User
 */ 

//	LED FND LCD  Button		//
//	LED-A port, FND-B port, LCD -Control C port(0,1,2) Data D port
// 	+5V - C port PC4,5,6,7 번으로 제공,  Pull up Button- F port 0번-내부 풀업 사용
//  Common Cathode Type FND연결시 ISP cable download연결이 않됨,CC의 그라운드를 뺴놓고
//  다운로드 한뒤 MCU를 동작시킨 뒤 CC FND의 그라운드를 연결하고 동작시켜야 함. 전류 부족문제?
//	사용자 만든 문자 "김치있어요?"를 CGRAM 0x40 ~ 0x67까지 저장하고,CGROM에 있는 ?(0x3F)를  표시함
//  LCD 초기화가 잘 안되면 전류가 부족함. PC4-7번의 +5V를 두선이상 뽑아서 전류를 키워서 사용하면 됨
//  2017-4-19   ,동영상 F:\Atmega128\Lecture_2017_1_Code\ex_prac6-6\LED_FND_LCD\LED_FND_LCD_INT.mp4
//  추가적으로 인터럽트 버튼을 int04(portE4)로 연결 ,포트 F의 풀업기능 사용않됨.저항을 연결하여
//  인터럽트핀으로 신호를 입력시켜야 함
//
//
//
#ifndef F_CPU 
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <util/delay.h>
#include "sgn_port_D_lcd.h"
#include <avr/interrupt.h>
//#include "lcd_D.h"


unsigned char pattern[10]={0x00,0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F,0xff};  //for 8 LEDs
unsigned char pattern_Fnd[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7C,0x07,0x7F,0x40};  //for 8 FND
// CA type  5611BH model http;//exeobd.com

void  CGRAM_Set()			//CGRAMSET()을 main()의 위로 올리면 waring문구가 없어짐.
// 선언하는 함수를 메인의 앞으로 보냄

{
	unsigned char kim[]={0x1d,0x05,0x05,0x00,0x1f,0x11,0x11,0x1f};
	unsigned char chi[]={0x01,0x09,0x1d,0x09,0x15,0x15,0x15,0x15};
	unsigned char it[]={0x09,0x15,0x15,0x09,0x00,0x0a,0x15,0x15};
	unsigned char eo[]={0x09,0x15,0x17,0x15,0x09,0x01,0x00,0x00};
	unsigned char yo[]={0x0e,0x11,0x11,0x0e,0x00,0x0a,0x1f,0x00};
	unsigned char sol[]={ 0x04,0x0e,0x15,0x1f,0x1f,0x02,0x0c,0x1f};
	int i;
	_delay_ms(1);
	lcd_command(0x40);		//0x40 ~ 0x47 까지 CGRAM Address를 만든다, 0x40의 4는 CGRAM어드레스설정모드에서
				// DB7,DB0 =0 1로 정해지고 나머지 AGC[5:0]값만 만들면 됨.그림6-4의 CGRAM Address는
				// DB5~DB0 = 000 000 ~ 111 111이므로 0x40~0x47,0x48~0x4F, ~ , 0x78~0x7F까지 8자 저장가능
				// LCD에 표시 할때는 lcd_data(주소값을 0x00 ~ 0x07까지) 로 화면에 나타남
				// 그림6-3의 CGRAM(1)로 표시된 부분에 해당됨.
	_delay_ms(1);

	for (i=0;i<8;i++)
	{
		lcd_data(kim[i]);
		_delay_ms(1);
	}
	lcd_command(0x48);		// 0x48 ~ 0x4F
	_delay_ms(1);

	for (i=0;i<8;i++)
	{
		lcd_data(chi[i]);
		_delay_ms(1);
	}
	lcd_command(0x50);		//0x50 ~ 0x57
	_delay_ms(1);

	for (i=0;i<8;i++)
	{
		lcd_data(it[i]);
		_delay_ms(1);
	}
	lcd_command(0x58);		//0x58 ~ 0x5F
	_delay_ms(1);

	for (i=0;i<8;i++)
	{
		lcd_data(eo[i]);
		_delay_ms(1);
	}
	lcd_command(0x60);		//0x60 ~ 0x67
	_delay_ms(1);

	for (i=0;i<8;i++)
	{
		lcd_data(yo[i]);
		_delay_ms(1);
	}
	lcd_command(0x68);		//0x50 ~ 0x57
	_delay_ms(1);
	for (i=0;i<8;i++)
	{
		lcd_data(sol[i]);
		_delay_ms(1);
	}
	
}

int main()
{
	int i=0;
	unsigned char num;	//숫자를 표시할려면 unsigned char로 선언하고 ASCII코드값을 사용한다
	// 아래에서 초기값을 0인 0x30으로 시작하여 0x3a(":")가 되면 다시 0x30으로 바꿈

	DDRA=0xFF;   	// for LEDs
	DDRB=0xFF;		// for FND
	DDRD=0xFF;		// for LCD DB0-DB7
	DDRF=0x00;		// for Pull-up button,PF0
	PORTF=0xFF; 	// Pull-up Start

	DDRC=0xFF;    	// LCD control for PC0 ~ PC2,Output +5V of LCD VCC, VL at PC4 ~PC7
	PORTC=0xF0;    	// +5V output
	_delay_ms(1000);	// for the +5V stable
	//PORTA=pattern[i];
	PORTA=0x00;
	PORTB=0x00;		// all segment on
	lcd_init();
	_delay_ms(10);

	lcd_display_position(1,1);
	lcd_data('0');
	PORTA=0xFF;
	_delay_ms(1000);

	lcd_display_position(1,1);

	lcd_string("Hellow AVR");
	_delay_ms(1000);
	
	CGRAM_Set();
	lcd_display_position(1,12);
	
	lcd_data(0x00);
	lcd_data(0x05);
	_delay_ms(1000);
	
	num=0x30;
	lcd_display_position(2,0);
	lcd_data(num);
	_delay_ms(1000);
	PORTA=0x00;
	_delay_ms(1000);


	EIMSK = 0b11110000;			// INT4~INT7 인터럽트 인에이블
	EICRB = 0b10101010;			// INT4~INT7 하강 에지
	SREG = 0x80;	 			// 전역 인터럽트 인에이블


	while(1)
	{

		while(!(~PINF & 0x01)); //스위치 눌림 기다림 PINF=1111 1111 ,~PINF=0000000
		// 0000 0000 & 0000 0001 =0x00, !(0x00)= 1 (True)
		// ->안 눌리면 참으로 계속 기다림
		// 버튼 눌리면 PINC=1111 1110,~PINC=0x01
		// 0x01 & 0x01 =0x01이고 !(0x01) = 0x00 (False)로
		// 빠져나감, 다음 문장 수행함
		_delay_ms(20);			//debouncing time


		if(++i==9) i=0;
		PORTA=pattern[i];
		PORTB=pattern_Fnd[i];

		lcd_display_position(2,i+1);
		lcd_data(++num);
		_delay_ms(100);

		if(num==0x38){
			//김치 출력
			CGRAM_Set();				// void를 빼면 변수 지정하지 않은 waring 발생
			lcd_display_position(2,11);
			lcd_data(0x00);					//CGRAM의 내용을 표시할 때는 lcd_data(주소)의 주소값이 0x00부터 0x07까지
			lcd_data(0x01);					// 써 주면 됨 ,8개의 사용자 문자를  표시 가능함.
			lcd_data(0x02);
			lcd_data(0x03);
			lcd_data(0x04);
			lcd_data(0x3F);					// "?"로 CGRAM의 주소를 써주면 '물음표'가 표시됨
			//
		}
		if(num==0x39){
			num=0x30;
			lcd_display_clear();
			lcd_display_position(1,1);
			lcd_string("Hellow AVR");
			
			CGRAM_Set();	
			lcd_display_position(1,12);
			lcd_data(0x00);	
			lcd_data(0x05);	
			
			
			lcd_display_position(2,0);
			lcd_data(num);



		}


		_delay_ms(200);

		while(~PINF & 0x01);	//위와 반대로 동작 ! 연산이 없음
		_delay_ms(20);			//debouncing time
	}
}

ISR(INT4_vect)
{
	PORTB = 0x36; 		// PORTB = " 1 1"
	_delay_ms(100);
}

ISR(INT5_vect)
{
	PORTB = 0x09; 		// PORTB = "="
	_delay_ms(100);
}

ISR(INT6_vect)
{
	PORTB = 0b11111011; 		// PORTB = 0xfb
}

ISR(INT7_vect)
{
	PORTB = 0b11110111; 		// PORTB = 0xf7
}

