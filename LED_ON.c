/*
#include <avr/io.h>

int main(void)
{
	DDRA = 0xFF; // A��Ʈ�� ��� ��º���� �����Ѵ�

	while(1)
	{
		PORTA = 0x03; // A��Ʈ 0��, 1������ 5V�� ������ 2~7�� �ɿ��� 0V�� ������
	}

	return 0;
}
*/

	
/*
Atmega128) LED ���������� �ѱ�   AVR LED���� / �Ӻ����  
2013. 10. 30. 2:05
����http://blog.naver.com/kgg1959/30178558094
*/

#include <avr/io.h>
#include <avr/delay.h>

void main()
{
      PORTB=0x00;
      DDRB=0xff;
      PORTB=0xff;

 
      while(1)
     {
  
          _delay_ms(10000);

          if(PORTB==0x00)
             PORTB=0xff;
         else
             PORTB=PORTB<<1;

      } 
} 
