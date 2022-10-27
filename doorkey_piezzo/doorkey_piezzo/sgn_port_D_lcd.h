//************************************************************************
// 16*2 Character LCD 8bit Interface for WinAVR
// RS = PC0, R/W = PC1, EN = PC2, DB7~DB0 : PD7~PD0 ,PC4-PC7: +5V Power 
//************************************************************************
#define F_CPU 16000000UL
void lcd_busy(void)
{   

	DDRD = 0x00;
	do {    
		PORTC &= 0xf6; 	// RS = 0    
		PORTC |= 0xf2; 	// RW = 1   
		PORTC |= 0xf4; 	// E = 1      
	
    } while(PIND & 0x80);	//'0'�� not_busy�� escape loop
     
	DDRD = 0xff;	// confirm the output direction

	_delay_ms(20);
}

void lcd_command(unsigned char cmd)	// cmd=0x40 : DB7=0,DB6=1,DB5~DB0(CGRAM address)
{
	lcd_busy();			// if not busy,
	PORTD = cmd;     		// �ּҰ��� D_port�� ��������
	PORTC &= 0xf6;    		// RS = 0 ��巹���������
	PORTC &= 0xf5;    		// RW = 0 ���⵿�� 
	PORTC |= 0xf4;    		// E = 1 ��� enable
	PORTC &= 0xf0;    		// E = 0, LCD ����Ʈ Ÿ�ֿ̹� ���� E = 1 -> 0 �� �����Ŵ 
	_delay_ms(3);            	// �����ð� �ð����� 
}       

void lcd_init()
{
	DDRD = 0xff;
	DDRC = DDRC | 0xff;		
	_delay_ms(15);           	// 15ms �̻� ���          
	
	PORTD = 0b00110000;         // ��� �� : 0x3c, 8-bit ��� ,F=1(5*8dot),2-lines    
	PORTC &= 0b11110000;    	// E,RW,RS = 0,0,0
	_delay_us(4100);         	// 4.1ms �̻� ���  
	PORTC &= 0b11110000;    	// E,RW,RS = 0,0,0
	_delay_us(100);          	// 100us �̻� ���    
	PORTC &= 0b11110000;    	// E,RW,RS = 0,0,0

//PORTC�� 0xf0�� ����4��Ʈ�� set��Ű�� command�� PORTF�� ������ ����Ÿ�� �ٲ��ʿ����.    

	lcd_command(0b00111000);	// ��� �� : 8-bit ��� ,F=1(5*7dot),2line,

	lcd_command(0b00001000);	//0x0f,display-on,cursor-on, cursor blink-on, ���÷��� ���� ,d=0(nondisplay),c=0(non cursor),b=0(cursor is nonblighting)
    
	lcd_command(0b00000001);	// ���÷��� Ŭ����

	lcd_command(0b00001111);	// ���÷��� ��, Ŀ�� ��, Ŀ�� ������ 
}         

void lcd_data(unsigned char byte)
{
	lcd_busy();
    
	PORTC |= 0b11110001; 		// RS=1
	PORTC &= 0b11110101; 		// RW=0 
	PORTC |= 0b11110100; 		// E=1 
	_delay_us(50); 			// �����ð� �ð����� 
	PORTD = byte;   
	_delay_us(50); 			// �����ð� �ð����� 
	PORTC &= 0b11110011; 		// E=0, LCD ����Ʈ Ÿ�ֿ̹� ���� E=1 -> 0  
    	_delay_ms(3);        
}

void lcd_string(unsigned char *str)
{
int i=0;
     
	while(1) {
		if (str[i] == '\0') break;
		lcd_data(str[i++]);	
	} 
}

void lcd_display_clear()
{
	lcd_command(0b00000001);
}

void lcd_cursor_home()
{
	lcd_command(0b00000010);
}

void lcd_entry_mode(int ID, int S)
{
unsigned char cmd;
	
	cmd = 0b00000100;
	if (ID == 1) cmd = cmd | 0b00000010;
	if (S == 1) cmd = cmd | 0b00000001;
	lcd_command(cmd);
}

void lcd_display_OnOff(int D, int C, int B)
{
unsigned cmd;
	
	cmd = 0b00001000;
	if (D == 1) cmd = cmd | 0b00000100;
	if (C == 1) cmd = cmd | 0b00000010;
	if (B == 1) cmd = cmd | 0b00000001;
	lcd_command(cmd);
}

void lcd_cursordisplay_shift(int SC, int RL)
{
unsigned cmd;
	
	cmd = 0b00010000;
	if (SC == 1) cmd = cmd | 0b00001000;
	if (RL == 1) cmd = cmd | 0b00000100;
	lcd_command(cmd);
}

void lcd_display_position(unsigned char row, unsigned char col)   // LCDǥ�� ��ġ�� DDRAM address�������� ������
{						    //  
unsigned cmd;

	cmd = 0b10000000 | ((row-1)*0x40+(col-1));	    // DB7=1,DB6~DB0(ù��° ��: 00H~27H, �ι�°��:40H~67H)
	lcd_command(cmd);				    // OR�����ϸ� (ù��° ��: 80H~A7H, �ι�°��:C0H~E7H)
}
