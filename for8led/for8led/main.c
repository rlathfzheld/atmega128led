/*
 * for문8LED.c
 *
 * Created: 2022-03-16 오전 11:00:25
 * Author : konyang
 */ 
#define F_CPU 16000000UL 
#include <avr/io.h> 
#include <util/delay.h>



int main(void)
{
	unsigned char buff;
	int i; DDRA = 0xFF;
	PORTA = 0x00;

    /* Replace with your application code */
    while (1) 
    {
		buff = 0x00; 
		if(buff == 0x00)
		{
			for(i = 0; i < 9; i++)
			{
				PORTA = buff;
				_delay_ms(500);
				buff = (buff << 1) + 1;
			}
		}
		else if(buff == 0xFF)
		{
			for(i = 0; i < 9; i++)
			{
				PORTA = buff;
				_delay_ms(500);
				buff = (buff << 1) + 1;
			}
		}
	 return 0;
    }
}

