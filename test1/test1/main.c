
/* 2022/05/02
 * 볼륨 -ADC_01, CDS-ADC_02 로 연결
 * 푸시버튼 - PF0에 연결, 버튼을 누르면 볼륨의 ADC변환하여 표시
 * 인터럽트버튼 -PE4에 하강에지로 동작, 인터럽트버튼으로 ADC측정 및 ADC인터럽트 활성화, 변환이 끝나면 ISR(ADC_vect)에서 표시함
 * 푸시버튼 동작시(ADMUX=0x01)에는 ADC인터럽트 금지, 인터럽트버튼으로 동작시(ADMUX=0x02) ADC측정후 ADC인터럽트발생하여 표시함
 * 
 */
 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "sgn_port_D_lcd.h"
#include <stdlib.h>

unsigned char pattern[10]={0x00,0xFE,0xFD,0xFB,0xF7,0xEF,0xDF,0xBF,0x7F,0xff};  //for 8 LEDs
unsigned char pattern_Fnd[10]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7C,0x07,0x7F,0x40};  //for 8 FND
// CA type  5611BH model http;//exeobd.com

		static volatile int i=1;
		static volatile unsigned Led_1;
		static volatile unsigned char num;	//숫자를 표시할려면 unsigned char로 선언하고 ASCII코드값을 사용한다
		// 아래에서 초기값을 0인 0x30으로 시작하여 0x3a(":")가 되면 다시 0x30으로 바꿈
		unsigned char msg[16]="1234567890";
		
ISR(INT4_vect)
{	
		PORTA=Led_1;
			lcd_display_clear();
			lcd_display_position(1,1);
			lcd_string("CDS Sensor");
			lcd_display_position(1,12);

		if (Led_1==0x80)		// MSB이 "1"이 되면 LSB부터 점등시키기
					{			Led_1=0x01;		}
		else		{	Led_1=Led_1<<1;		}
		//	ADC인터럽트 허가
		ADCSRA = ADCSRA | 0b00001000;

		ADMUX=0x42;			//VREF=AREF, ADC2 단극성 입력,CDS센서를 ADC2로 연결하였기에 ADMUX를 2로 변경함, 단일모드는 1개만 사용가능
		ADCSRA = ADCSRA | 0b01000000; 		// ADSC=1 변환시작
											//ADC변환되면 변환인터럽트 호출동작
		_delay_ms(2);
		EIFR =(1<<INTF4);					//ISR루틴이 끝나면 확실히 Flag를 '0'으로 만듬

}

ISR(ADC_vect)				//free running mode로 인터럽트 처리하면 다른 동작 캐치 어려움->단일동작으로 실행
							// ADC인터럽트는 1개 밖에 없기 때문에 입력채널 하나만을 사용
{
			// ADC- AD1
			int ADval;
			float Vin, Vref=5.0;
			char buf[10];
 ADCSRA = ADCSRA ^ 0b00001000; 		// ADIE를 1로 EXOR연산으로  인터럽트정지
 ADval = (int)ADCL + ((int)ADCH << 8);
 Vin = (float)ADval*Vref/1023.0;				// val=(Vin*1024)/Vref

lcd_display_position(1, 13); 					// LCD 1행 1열에 표시

//		sprintf(msg, "ADC0 = %.2f[V]", Vin);
lcd_string("     ");								// 천이 넘었을때 뒷자리 지우기
lcd_display_position(1, 13); 					// LCD 1행 1열에 표시

itoa(ADval,buf,10);
lcd_string(buf); 									// LCD에 A/D 변환 결과 표시

// 소숫점으로 표시하기
sprintf(buf,"%.2f",Vin);
lcd_display_position(2, 12); 					// LCD 1행 1열에 표시
lcd_string(buf); 									// LCD에 A/D 변환 결과 표시
lcd_string("V");
_delay_ms(100);

  //ADCSRA = ADCSRA | 0b00001000; 		// ADIE를 1로 인터럽트가능
}

int main()
{
			// ADC- AD1
			int ADval;
			float Vin, Vref=5.0;
			char buf[10];
	DDRA=0xFF;   	// for LEDs
	DDRB=0xFF;		// for FND
	DDRD=0xFF;		// for LCD DB0-DB7
	DDRF=0x00;		// for Pull-up button,PF0  //ADC0 -PF1사용
	PORTF=0x01; 	// Pull-up Start  PF0만 버튼 사용

	DDRC=0xFF;    	// LCD control for PC0 ~ PC2,Output +5V of LCD VCC, VL at PC4 ~PC7
	PORTC=0xF0;    	// +5V output
	_delay_ms(1000);	// for the +5V stable
	//PORTA=pattern[i];
	PORTA=0x00;
	PORTB=0x00;		// all segment on

	num=0x30;
	lcd_init();
	lcd_display_position(1,1);
	lcd_string("Hellow AVR");

	lcd_display_position(2,1);
	lcd_string(msg);			// char로 msg배열을 만들면 ASCII code로 표현되고 header file에서 lcd_data(str[i++]); 로 표시 될때
	                                   //  예) lcd_data("1" => 0x31)이고 결국 lcd_data(0x31)이 됨. RS=1(data reg.), R/W =0(write)하게 되고
								    //      CGROM의 문자패턴에서 숫자 '1'이 DDRAM의 (2,1)위치 주소에 기록되면서 자동으로 LCD에 표시함.
	PORTA=0xFF;
	_delay_ms(100);

	Led_1=0x01;
	DDRE	=0x00;			// in port E
	PORTE =0xF0;			// for pull-up
	EIMSK = 0b11110000;	// INT4~INT7 인터럽트 인에이블, 풀업저항 회로사용
	EICRB = 0b10101010;	// INT4~INT7 하강 에지
	
	ADMUX=0x41;			// VREF=AREF, 볼륨을 센서로 사용하기 위한 ADC1 단극성 입력으로 초기화
						// 인터럽트버튼을 사용하면 CDS를 센서로 종작시키기 위하여 ADC2 단극성 입력으로 설정변경 
	ADCSRA=0x87;		// 단일변환,128분주 0b1000 1111; ADEN=1,ADSC=0변환정지, ADFR=0 단일변환, ADIF=0클리어시키지 않음, ADIE=0인터럽트가능,ADPS=111(128)
	SREG = 0x80;		// 전역 인터럽트 인에이블
//	ADC인터럽트 정지
ADCSRA = ADCSRA & 0b111110111;


	while(1)
	{

//	버튼으로 볼륨값을 읽기 위한 
//  ADC_인터럽트 기능을 사용할 경우 너무 빨리 표시 됨
	ADMUX=0x41;			//VREF=AREF, ADC1 단극성 입력 
	ADCSRA = ADCSRA | 0b00010000; 		// ADSC=0 변환중지,ADIF=0초기화

	while(!(~PINF & 0x01)); //스위치 눌림 기다림 PINF=1111 1111 ,~PINF=0000000
		_delay_ms(20);			//debouncing time
					lcd_display_clear();
			lcd_display_position(1,1);
			lcd_string("Volume ADC");
		ADCSRA = ADCSRA | 0b01000000; 		// ADSC=1 변환시작

//  단일런링모드	, 인터럽트 처리	
		while((ADCSRA & 0b00010000) == 0);		// ADIF=1 ?, 변환완료 체크
		ADval = (int)ADCL + ((int)ADCH << 8);
		Vin = (float)ADval*Vref/1023.0;				// val=(Vin*1024)/Vref
		lcd_display_position(1, 13); 					// LCD 1행 1열에 표시
		//		sprintf(msg, "ADC0 = %.2f[V]", Vin);

		itoa(ADval,buf,10);
		lcd_string(buf); 									// LCD에 A/D 변환 결과 표시

	// 소숫점으로 표시하기 
		sprintf(buf,"%.2f",Vin);
		lcd_display_position(2, 12); 					// LCD 1행 1열에 표시
		lcd_string(buf); 									// LCD에 A/D 변환 결과 표시
		lcd_string("V");
		_delay_ms(100);

//
		while(~PINF & 0x01);	//위와 반대로 동작 ! 연산이 없음
		_delay_ms(20);			//debouncing time

	}
}


/* 
printf() scanf(),sprintf() 사용
1)솔루션탐색기 -프로젝트명 오른쪽버튼-Add Library -Name 에서 libprintf_fit와 libscanf_min 선택  -OK
   프로젝트 익스플로어에 해당 라이브러리가 추가됨
2) 프로젝트 - ADC_01 속성(Alt+F7) -Toolchain - AVR/GNU Linker -General - use vprintf library(-Wl,-u,vfprintf) 선택 -파일저장(Cntl+S)
*/
