/*
#include <avr/io.h>

int main(void)
{
	DDRA = 0xFF; // A포트를 모두 출력보드로 설정한다

	while(1)
	{
		PORTA = 0x03; // A포트 0번, 1번에서 5V를 보내고 2~7번 핀에서 0V를 보낸다
	}

	return 0;
}
*/

	
/*
Atmega128) LED 순차적으로 켜기   AVR LED예제 / 임베디드  
2013. 10. 30. 2:05
복사http://blog.naver.com/kgg1959/30178558094
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
