#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "tpk_lcd8.h"
#include "define.h"
#include <util/twi.h>

void BeepOn(BYTE count);
void BeepOff(void);

void InitPort(void);
void InitTimer0(void);
void InitTimer2(void);

void lcd_display_number(int number);

void Delay(unsigned long delay);
void Delay_us(unsigned int time_us);
void Delay_ms(unsigned int time_ms);

int abcde;

void  CGRAM_Set()
{
	unsigned char doo[]={0x0e,0x08,0x0e,0x00,0x1f,0x04,0x04,0x00};
	unsigned char der[]={0x01,0x1d,0x11,0x17,0x11,0x1d,0x01,0x00};
	unsigned char gee[]={0x00,0x1d,0x09,0x15,0x15,0x01,0x01,0x00};
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
		lcd_data(doo[i]);
		_delay_ms(1);
	}
	lcd_command(0x48);		// 0x48 ~ 0x4F
	_delay_ms(1);

	for (i=0;i<8;i++)
	{
		lcd_data(der[i]);
		_delay_ms(1);
	}
	lcd_command(0x50);		//0x50 ~ 0x57
	_delay_ms(1);

	for (i=0;i<8;i++)
	{
		lcd_data(gee[i]);
		_delay_ms(1);
	}
	lcd_command(0x58);		//0x58 ~ 0x5F
	_delay_ms(1);

}

void TWI_master_transmit(unsigned char addr, int data)
{
	TWCR=0xa4;
	while(!(TWCR & 0x80) || ( (TWSR & 0xf8) != 0x08));
	TWDR = addr<<1;
	TWCR = 0x84;
	while(!(TWCR & 0x80) || ((TWSR & 0xf8) != 0x18));
	TWDR = data;
	TWCR = 0x84;
	while(!(TWCR & 0x80) || ((TWSR & 0xf8) != 0x28));
	TWCR =0x94;
}

unsigned char TWI_master_receive(unsigned char addr)
{
	unsigned char data;

	TWCR= 0xa4;
	while(!(TWCR & 0x80) || ((TWSR & 0xf8) != 0x08));

	TWDR = (addr<<1) | 1;
	TWCR = 0x84;
	while(!(TWCR & 0x80) || ((TWSR & 0xf8) != 0x40));

	TWCR = 0x84;
	while(!(TWCR & 0x80) || ((TWSR & 0xf8) != 0x58));

	data=TWDR;
	TWCR = 0x94;

	return data;
}

unsigned char ascii_number[10]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};
unsigned char pattern[10]={0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0xff};

BYTE TimerFlag;
WORD Timer1Cnt;
BYTE Timer1Flag;

BYTE scan_key, old_key44,Key44_Data, scan_chatt_count,key44_event;

BYTE    Beep_OnOff;
BYTE    Beep_Count;
WORD    Beep_Timer;
BYTE	KeyData;

unsigned int time_ms, time_us;
unsigned long delay;

int random_Num=0;
int score=0;
int gamecount=0;
int number;
int high_score, s1_score, s2_score;
int s1_data;
int s11_data;
int s2_data;
int s22_data;
int a,b;



int main(void)
{
	SPH = 0x10;
	SPL = 0xff;
	
	a=eeprom_read_byte(2);
	
	if(a!=1)
	{
		eeprom_write_byte(1,0);
		eeprom_write_byte(2,1);
		eeprom_write_byte(3,0);
		eeprom_write_byte(4,0);
	}
	
	InitPort();
	InitTimer0();
	InitTimer2();
	Delay_ms(100);
	
	abcde = 256-44;
	
	sei();
	
	Beep_OnOff = ON;
	BeepOn(3);
	KeyData=0;
	lcd_init();
	
	CGRAM_Set();

	for(int i=0; i<1; i++)
	{
		lcd_init();
		lcd_display_position(1,1);
		lcd_data(0x00);
		lcd_data(0x01);
		lcd_data(0x02);
		lcd_string(" Game");
		Delay_ms(600);
	}
	
	lcd_display_clear();
	lcd_display_position(1,1);
	lcd_string("1.Play");
	lcd_display_position(1,9);
	lcd_string("2.Score");
	lcd_display_position(2,1);
	lcd_string("3.TWI");
	Delay_ms(50);
	
	while(1)
	{
		Scan_Key();
		
		if(Key44_Data==1)
		{
			Play();
			break;
		}
		
		if(Key44_Data==2)
		{
			Score();
			break;
		}
		
		if(Key44_Data==3)
		{
			TWI();
			break;
		}
	}
}

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 112;

	TimerFlag = 1;
	
	Timer1Cnt++;
	if(Timer1Cnt >= 10)
	{
		Timer1Cnt = 0;
		Timer1Flag = 1;
	}
}

ISR(TIMER2_OVF_vect)
{
	TCNT2 = (abcde); 		//(64/14.7456M) * 44 = 191 usec
	
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

void Scan_Key(void)
{
	Key44_Data=999;
	if(TimerFlag)
	{
		Key44_Scan();
		TimerFlag = 0;
	}
	
	if(Timer1Flag)
	{
		Timer1Flag = 0;
	}
	if(key44_event == 1)
	{
		Key44_Job_Exe();
		key44_event = 0;
	}
}

void Play(void)
{
	random_Num=0;
	score=0;
	gamecount=0;
	
	while(1)
	{
		score=0;
		gamecount=0;
		
		lcd_display_clear();
		abcde=256-44;
		BeepOn(1);
		lcd_display_position(1,8);
		lcd_string("5"); Delay_ms(500);
		BeepOn(1);
		lcd_display_position(1,8);
		lcd_string("4"); Delay_ms(500);
		BeepOn(1);
		lcd_display_position(1,8);
		lcd_string("3"); Delay_ms(500);
		BeepOn(1);
		lcd_display_position(1,8);
		lcd_string("2"); Delay_ms(500);
		BeepOn(1);
		lcd_display_position(1,8);
		lcd_string("1"); Delay_ms(500);
		lcd_display_clear();
		abcde=262;
		BeepOn(1);
		_delay_ms(100);
		abcde=330;
		BeepOn(1);
		_delay_ms(100);
		abcde=392;
		BeepOn(1);
		_delay_ms(100);
		abcde=523;
		BeepOn(1);
		_delay_ms(100);
		lcd_display_position(1,3);
		lcd_string("game start!!"); Delay_ms(750);
		lcd_display_clear();
		
		lcd_display_position(1,1);
		lcd_string("Count:");
		lcd_display_number(0);
		lcd_display_position(2,1);
		lcd_string("Score:");
		lcd_display_number(0);
		Delay_ms(900);
		
		
		while(1)
		{
			gamecount++;
			lcd_display_position(1,7);
			lcd_string("   ");
			lcd_display_position(1,7);
			lcd_display_number(gamecount);
			lcd_string("/10");
			
			if(gamecount<11)
			{
				random_Num=rand()%8+1;
				PORTA=pattern[random_Num];
				
				for(int i=0; i<100; i++)
				{
					if(TimerFlag)
					{
						Key44_Scan();
						TimerFlag = 0;
					}
					
					if(Timer1Flag)
					{
						Timer1Flag = 0;
					}
					if(key44_event == 1)
					{
						Key44_Job_Exe();
						key44_event = 0;
					}
					

					if(Key44_Data==random_Num)
					{
						score=score+10;
						abcde=256-44;
						BeepOn(2);
						PORTA=0x99;
						lcd_display_position(2,12);
						lcd_string("      ");
						lcd_display_position(2,12);
						lcd_string("Nice!");
						lcd_display_position(1,1);
						PORTF=0b00100000;
						
						break;
					}
					
					Delay_ms(8);
					
				}
				if(Key44_Data!=random_Num)
				{
					abcde=17;
					BeepOn(2);
					lcd_display_position(2,12);
					lcd_string("      ");
					lcd_display_position(2,12);
					lcd_string("Miss!");
					PORTF=0b00000100;
					
				}
				
				PORTA=0x81;
				Key44_Data=999;
				
				lcd_display_position(2,7);
				lcd_string("   ");
				lcd_display_position(2,7);
				lcd_display_number(score);

				Delay_ms(500);
			}
			if(gamecount>10)
			{
				lcd_display_clear();
				lcd_display_position(1,5);
				lcd_string("GAMEOVER");
				for(int i=0; i<2; i++){
					for(int j=1; j<9; j++){
						PORTA=pattern[j];
						Delay_ms(100);
					}
				}
				PORTA=0xff;
				PORTF=0xff;
				for(int i=1; i<6; i++){
					lcd_display_position(2,3);
					lcd_string("             ");
					Delay_ms(500);
					lcd_display_position(2,4);
					lcd_string("SCORE : ");
					lcd_display_number(score);
					Delay_ms(500);
				}
				break;
			}
		}
		
		
		high_score=eeprom_read_byte(1);
		lcd_display_position(1,2);
		lcd_string("HIGH SCORE : ");
		lcd_display_number(high_score);
		Delay_ms(2000);
		if(score>high_score)
		{
			high_score=score;
			
			for(int i=1; i<6; i++){
				lcd_display_position(1,1);
				lcd_string("                ");
				PORTA=0xff;
				PORTF=0b00000100;
				Delay_ms(500);
				lcd_display_position(1,1);
				lcd_string("HIGH SCORE : ");
				lcd_display_number(high_score);
				PORTA=0x00;
				PORTF=0b00100000;
				Delay_ms(400);
			}
			eeprom_write_byte(1,high_score);
			
			Delay_ms(2500);
			
			
		}
		break;
	}
}

void Score(void)
{
	high_score=eeprom_read_byte(1);
	s1_score=eeprom_read_byte(3);
	s2_score=eeprom_read_byte(4);
	lcd_display_clear();
	lcd_display_position(1,1);
	lcd_string("HIGH:");
	lcd_display_number(high_score);
	lcd_display_position(2,1);
	lcd_string("s1:");
	lcd_display_number(s1_score);
	lcd_display_position(2,8);
	lcd_string("s2:");
	lcd_display_number(s2_score);
	Delay_ms(2000);
	
	lcd_display_clear();
	Key44_Data=999;
}

void TWI(void)
{
	TWBR=10;
	TWSR=0x00;
	
	lcd_display_clear();
	lcd_display_position(1,1);
	lcd_string("receiving");
	
	lcd_display_position(1,10);
	lcd_data('.');
	Delay_ms(90);
	lcd_data('.');
	Delay_ms(90);
	
	s11_data=TWI_master_receive(0x01);
	s1_data=s11_data;
	_delay_ms(100);
	s22_data=TWI_master_receive(0x02);
	s2_data=s22_data;
	
	lcd_data('.');
	Delay_ms(90);
	lcd_data('!');
	Delay_ms(90);
	
	lcd_display_position(2,1);
	lcd_display_number(s1_data);
	lcd_display_position(2,8);
	lcd_display_number(s2_data);
	Delay_ms(1000);
	
	lcd_display_clear();
	lcd_display_position(1,1);
	lcd_string("transmitting");
	
	lcd_display_position(1,13);
	lcd_data('.');
	Delay_ms(90);
	lcd_data('.');
	Delay_ms(90);
	
	high_score=eeprom_read_byte(1);
	TWI_master_transmit(0x01,high_score);
	TWI_master_transmit(0x02,high_score);
	TWI_master_transmit(0x01,s2_data);
	TWI_master_transmit(0x02,s1_data);
	
	lcd_data('.');
	Delay_ms(90);
	lcd_data('!');
	Delay_ms(90);
	lcd_display_position(2,1);
	lcd_string("complete!");
	Delay_ms(500);
	
	eeprom_write_byte(3,s1_data);
	eeprom_write_byte(4,s2_data);
	
	s1_data=eeprom_read_byte(3);
	s2_data=eeprom_read_byte(4);
}

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
	DDRF=0xff;
	PORTF=0xff;
	
	DDRE=0xf0;
	PORTE=0x0f;
	
	DDRA=0xff;
	PORTA=0xff;
	
	PORTC=0xff;
	DDRC= 0xf0;
	
	PORTB=0x00;		// for LCD DB0-DB7
	DDRB= 0xFF;
	
	_delay_ms(1000);
	PORTA=0X00;
	
	PORTG = 0xFF;
	DDRG = 0xFF;
	
}

void InitTimer0(void)
{
	TCNT0=0x00;
	TCCR0=0x07;
	TIMSK=0x01;
}

void InitTimer2(void)
{
	TCCR2 = 0x03; 	// Trigger => Fclock/64
	TCNT2 = (256-44);	// Timer Counter Register
	TIMSK |= BIT6; 	// Timer2 Interrupt Enable Mask Register
}

void Delay(unsigned long delay)
{
	while(delay--) asm("NOP");
}

void Delay_us(unsigned int time_us)
{
	WORD i;
	for(i=0; i<time_us;i++)
	{
		asm("PUSH R0");    //2Cycle +
		asm("NOP");     //1Cycle +
		asm("NOP");     //1Cycle +
		asm("NOP");     //1Cycle +
		asm("NOP");     //1Cycle +
		asm("NOP");     //1Cycle +
		asm("NOP");     //1Cycle +
		asm("NOP");     //1Cycle +
		asm("NOP");     //1Cycle +
		asm("NOP");     //1Cycle +
		asm("POP R0");     //2Cycle = 13Cycle = 1us for 14.7456MHz
	}
}

void Delay_ms(unsigned int time_ms)
{
	WORD i;
	for(i=0; i<time_ms; i++)
	{
		Delay_us(250);
		Delay_us(250);
		Delay_us(250);
		Delay_us(250);
	}
}

void lcd_display_number(int number)
{
	int number1=0;
	int number10=0;
	int number100=0;

	if(number<10){lcd_data(ascii_number[number]); Delay_ms(10);}

	if(number>=10 && number<100)
	{
		number10=number/10;
		number1=number%10;
		lcd_data(ascii_number[number10]);
		Delay_ms(10);
		lcd_data(ascii_number[number1]);
		Delay_ms(10);
	}

	if(number>=100)
	{
		number100=number/100;
		number10=number/10-(number100)*10;
		number1=number%10;
		lcd_data(ascii_number[number100]);
		Delay_ms(10);
		lcd_data(ascii_number[number10]);
		Delay_ms(10);
		lcd_data(ascii_number[number1]);
		Delay_ms(10);
	}
}

void Key44_Scan(void)
{
	BYTE	scan_count;
	
	if(scan_chatt_count>=CHAT_COUNT)
	{
		PORTE = 0x0F;
		Delay(10);
		if((PINE&0x0F) == 0x0F)	scan_chatt_count = 0;
	}
	else
	{
		scan_count=0;
		
		PORTE=0xEF;
		Delay(10);
		
		if((PINE&0x01)==0){scan_key=1;scan_count++;}
		if((PINE&0x02)==0){scan_key=2;scan_count++;}
		if((PINE&0x04)==0){scan_key=3;scan_count++;}
		if((PINE&0x08)==0){scan_key=4;scan_count++;}
		
		
		PORTE=0xDF;
		Delay(10);
		
		if((PINE&0x01)==0){scan_key=5;scan_count++;}
		if((PINE&0x02)==0){scan_key=6;scan_count++;}
		if((PINE&0x04)==0){scan_key=7;scan_count++;}
		if((PINE&0x08)==0){scan_key=8;scan_count++;}
		
		
		PORTE=0xBF;
		Delay(10);
		
		if((PINE&0x01)==0){scan_key=9;scan_count++;}
		if((PINE&0x02)==0){scan_key=10;scan_count++;}
		if((PINE&0x04)==0){scan_key=11;scan_count++;}
		if((PINE&0x08)==0){scan_key=12;scan_count++;}
		
		
		PORTE=0x7F;
		Delay(10);
		
		if((PINE&0x01)==0){scan_key=9;scan_count++;}
		if((PINE&0x02)==0){scan_key=14;scan_count++;}
		if((PINE&0x04)==0){scan_key=15;scan_count++;}
		if((PINE&0x08)==0){scan_key=16;scan_count++;}
		
		PORTE = 0xFF;
		
		if(scan_count != 1)	return;
		
		if(scan_key != old_key44)
		{
			old_key44 = scan_key;
			scan_chatt_count = 0;
		}
		else
		{
			scan_chatt_count++;
			if(scan_chatt_count>=CHAT_COUNT)
			{
				Key44_Data = old_key44;
				old_key44 = 0;
				key44_event=1;
			}
		}
	}
}

void Key44_Job_Exe(void)
{
	switch(Key44_Data)
	{
		case  1  : Key44_0_0(); break;
		case  2  : Key44_0_1(); break;
		case  3  : Key44_0_2(); break;
		case  4  : Key44_0_3(); break;
		case  5  : Key44_0_4(); break;
		case  6  : Key44_0_5(); break;
		case  7  : Key44_0_6(); break;
		case  8  : Key44_0_7(); break;
		case  9  : Key44_0_8(); break;
		case  10 : Key44_0_9(); break;
		case  11 : Key44_A();  break;
		case  12 : Key44_B();  break;
		case  13 : Key44_C();  break;
		case  14 : Key44_D();  break;
		case  15 : Key44_E();  break;
		case  16 : Key44_F();  break;
		
		
		default  : break;
	}
}

void Key44_0_0(void){}//lcd_display_position(1,14);lcd_string(" 1 ");}
void Key44_0_1(void){}//lcd_display_position(1,14);lcd_string(" 2 ");}
void Key44_0_2(void){}//lcd_display_position(1,14);lcd_string(" 3 ");}
void Key44_0_3(void){}//lcd_display_position(1,14);lcd_string(" 4 ");}
void Key44_0_4(void){}//lcd_display_position(1,14);lcd_string(" 5 ");}
void Key44_0_5(void){}//lcd_display_position(1,14);lcd_string(" 6 ");}
void Key44_0_6(void){}//lcd_display_position(1,14);lcd_string(" 7 ");}
void Key44_0_7(void){}//lcd_display_position(1,14);lcd_string(" 8 ");}
void Key44_0_8(void){}//lcd_display_position(1,14);lcd_string(" 9 ");}
void Key44_0_9(void){}//lcd_display_position(1,14);lcd_string(" 0 ");}

void Key44_A(void){/*lcd_display_position(1,14);lcd_string(" A ");*/}
void Key44_B(void){/*lcd_display_position(1,14);lcd_string(" B ");*/}
void Key44_C(void){/*lcd_display_position(1,14);lcd_string(" C ");*/}
void Key44_D(void){/*lcd_display_position(1,14);lcd_string(" D ");*/}
void Key44_E(void){/*lcd_display_position(1,14);lcd_string(" E ");*/}
void Key44_F(void){/*lcd_display_position(1,14);lcd_string(" F ");*/}

void Key44_a(void){/*lcd_display_position(1,14); lcd_string(" a ");*/}
void Key44_b(void){/*lcd_display_position(1,14); lcd_string(" b ");*/}
void Key44_c(void){/*lcd_display_position(1,14); lcd_string(" c ");*/}
void Key44_d(void){/*lcd_display_position(1,14); lcd_string(" d ");*/}
void Key44_e(void){/*lcd_display_position(1,14); lcd_string(" e  );*/}
void Key44_f(void){/*lcd_display_position(1,14); lcd_string(" f ");*/}
void Key44_g(void){/*lcd_display_position(1,14); lcd_string(" g ");*/}
void Key44_h(void){/*lcd_display_position(1,14); lcd_string(" h ");*/}
void Key44_i(void){/*lcd_display_position(1,14); lcd_string(" i ");*/}
void Key44_j(void){/*lcd_display_position(1,14); lcd_string(" j ");*/}
void Key44_k(void){/*lcd_display_position(1,14); lcd_string(" k ");*/}
void Key44_l(void){/*lcd_display_position(1,14); lcd_string(" l ");*/}
void Key44_m(void){/*lcd_display_position(1,14); lcd_string(" m ");*/}
void Key44_n(void){/*lcd_display_position(1,14); lcd_string(" n ");*/}
void Key44_o(void){/*lcd_display_position(1,14); lcd_string(" o ");*/}
void Key44_p(void){/*lcd_display_position(1,14); lcd_string(" p ");*/}

