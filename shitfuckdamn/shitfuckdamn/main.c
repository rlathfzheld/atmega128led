/*
	timer1-Mode8,PWM,Phase and Frequency Correct
	TCNT1=Timer1 value
	ICR1 =Timer1의 최대값 설정(50Hz신호발생)
	OCR1A = Sets when the PWM신호가 바뀌는 시점을 설정 -High level구간 설정용
    OC1A = PWM신호 출력핀, PB5


*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "sgn_port_D_lcd.h"

#define F_CPU 16000000UL

unsigned char tim0_cnt,pwm_seq;
unsigned int pwm_data[5]={600,1050,1500,1950,2400};	//timer1의 TCNT1과 OCR1A를 비교하여 같으면 "0"으로 출력전환하기 위한 듀티비 설정
											// Timer1을 이용하기 때문에 주기는 8분주된 0.5uS를 사용하여 600이면 600*0.5us =300uS인테 다운카운팅에  High
											// 로 되고 업카운팅에 "LOW"되어서 600uS PWM파형 만든다.  http://blog.naver.com/ga1267/220093754561
											//  duty raio 600uS   ---   -90도
											//            1050uS  ---   -45도
											//            1500uS  ---     0도
											//            1950uS  ---   +45도
											//            2400uS  ---   +90도


ISR(INT4_vect)		// 외부인터럽트로 방향설정하기-누를때마다 방향전환
{	
			SREG=0x00;
			pwm_seq=(tim0_cnt++)%5;	// tim0_cnt의 값을 증가하여 배열 8개중 값을 다음번값으로 이동시키기 위함.듀티비 조정용
			
			if (pwm_seq == 0){
				lcd_display_clear();
				lcd_display_position(1,1);
				lcd_string("-90");
			}
			else if (pwm_seq == 1){
				lcd_display_clear();
				lcd_display_position(1,1);
				lcd_string("-45");
			}
			else if (pwm_seq == 2){
				lcd_display_clear();
				lcd_display_position(1,1);
				lcd_string("0");
			}
			else if (pwm_seq == 3){
				lcd_display_clear();
				lcd_display_position(1,1);
				lcd_string("45");
			}
			else if (pwm_seq == 4){
				lcd_display_clear();
				lcd_display_position(1,1);
				lcd_string("90");
			}
			
			OCR1A=pwm_data[pwm_seq];		// 업카운팅 매치시0으로 다운카운팅시 1로 출력만듬. 
			if(	tim0_cnt>=80)	tim0_cnt=0;	// 카운터 초기화
			_delay_ms(20);					// 버튼 채터링	
			SREG=0x80;	
			
}


//타이머0의 오버플로우인터럽트는 사용하지 않음	//	
//ISR(TIMER0_OVF_vect)		// 타이머0의 노말모드 오버플로우로 자동으로 방향설정하기
//{
	//tim0_cnt++;				// 오버플로우발생 후 
	//if(tim0_cnt==100){		// tim0_cnt가 250번 증가할때까지 pwm_seq값은 그대로 유지함. 즉 같은 pwm파형을 계속 만들어 내기 위한 지속시간
			//pwm_seq=(pwm_seq+1)%8;	//250번 동안 유지한 후 pwm_seq의 값을 증가하여 배열 값을 다음번값으로 이동시키기 위함.듀티비 조정용
			//OCR1A=pwm_data[pwm_seq];				//업카운팅 매치시0으로 다운카운팅시 1로 출력만듬. 
//
			//tim0_cnt=0;				//초기화
	//}
//}


int main(void)
{
	DDRC=0xff;				// Power 
	PORTC=0xff;				// +5V
/*
							Timer/Counter1
*/	
	DDRB=0b00100000;		// PB5 output direction for Timer/Counter1 signal 
	TCCR1A=0b10000000;  	// com1a1 com1a0 = 1, 0 : clear OCnA on compare match  when up-counting.Set OC1A on compare match when downcounting(Phase and frequency
							//							correct PWM mode)
							// com1b1 com1b0 = 0, 0 : Normal port,OCnB disconnected
							// com1c1 com1c0 = 0, 0 : Normal port,OCnC disconnected
							// WGM11 WGM10 =   0, 0  : Waveform Generation MOde지정, TCCR1B의 WGM13, WGM12비트와 연동하여 
							//						  동작모드 지정,즉 WGM13,WGM12,WGM11,WGM10의 4비트로 설정함으로 총16가지 설정가능 
							//						  WGM11,WGM10 =0 0 이므로 Fast PWM,10bit혹은 Fast PWM방식인데 아래의 TCCR1B 레지스터를 참조해야 함	

	TCCR1B=0b00010010;		// ICNC1,ICES1 =0 0;bit5는 사용 않고, WGM13,WGM12 =1 0; Mode 8,ICR1 =>top, TCNT1과 OCR1A는 비교매치시 0 또는 1로 토글
							// CS12, CS11,CS0 -clock source select; 010=clk(i/o) / 8 분주비 16MHz/8=2MHz, T=0.5uS

	ICR1=20000;				//분주비8로 클럭 2MHz를 사용하여 20mS을 만들기위해서는 0.5uS * 40,000 =20mS.즉 클럭40,000개면 20mS주기를 만들수 있다.하지만
							//모드8에서는 카운터가 업-다운중에 비교매치가 발생함으로 40,000/2=20,000으로 해야함.
							//업카운팅 매치시0으로 다운카운팅시 1로 출력만듬. 

	OCR1A=600;				// initial value, 0 degree
/*
		Timer0 사용 않함
*/
//
	//TCCR0=0b00000111;		// Normal Mode, Prescaler 1024, Upcounting only
	//TCNT0=0x00;				// initial value

//	외부인터럽트 입력버튼용 PE4번 이용하고 내부풀업저항 사용함
	DDRE=0x00;
	PORTE=0xf0;

	EIMSK=0b11110000;		//외부인터럽트 PE4-7사용
	EICRB=0b10101010;		// 하강엣지에서 동작

//	TIMSK=0x01;				// Timer0 Overflow on compare match,타이머로 자동으로 방향전환시에는 동작시켜줌
	lcd_display_clear();
	tim0_cnt=0;				

	pwm_seq=0;				// PWM Waveform 종류 지정용 인덱스
	SREG=0x80;				// asm "sei()"; 전역인잍터럽트 1 

	while(1);
}

