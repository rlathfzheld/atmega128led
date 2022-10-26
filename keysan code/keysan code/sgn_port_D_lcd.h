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
	
    } while(PIND & 0x80);	//'0'면 not_busy로 escape loop
     
	DDRD = 0xff;	// confirm the output direction

	_delay_ms(20);
}

void lcd_command(unsigned char cmd)	// cmd=0x40 : DB7=0,DB6=1,DB5~DB0(CGRAM address)
{
	lcd_busy();			// if not busy,
	PORTD = cmd;     		// 주소값을 D_port로 내보내고
	PORTC &= 0xf6;    		// RS = 0 어드레스설정모드
	PORTC &= 0xf5;    		// RW = 0 쓰기동작 
	PORTC |= 0xf4;    		// E = 1 모듈 enable
	PORTC &= 0xf0;    		// E = 0, LCD 라이트 타이밍에 따라 E = 1 -> 0 로 변경시킴 
	_delay_ms(3);            	// 일정시간 시간지연 
}       

void lcd_init()
{
	DDRD = 0xff;
	DDRC = DDRC | 0xff;		
	_delay_ms(15);           	// 15ms 이상 대기          
	
	PORTD = 0b00110000;         // 평션 셋 : 0x3c, 8-bit 모드 ,F=1(5*8dot),2-lines    
	PORTC &= 0b11110000;    	// E,RW,RS = 0,0,0
	_delay_us(4100);         	// 4.1ms 이상 대기  
	PORTC &= 0b11110000;    	// E,RW,RS = 0,0,0
	_delay_us(100);          	// 100us 이상 대기    
	PORTC &= 0b11110000;    	// E,RW,RS = 0,0,0

//PORTC만 0xf0로 상위4비트만 set시키고 command는 PORTF로 나가는 데이타로 바꿀필요업슴.    

	lcd_command(0b00111000);	// 평션 셋 : 8-bit 모드 ,F=1(5*7dot),2line,

	lcd_command(0b00001000);	//0x0f,display-on,cursor-on, cursor blink-on, 디스플레이 오프 ,d=0(nondisplay),c=0(non cursor),b=0(cursor is nonblighting)
    
	lcd_command(0b00000001);	// 디스플레이 클리어

	lcd_command(0b00001111);	// 디스플레이 온, 커셔 온, 커셔 깜박임 
}         

void lcd_data(unsigned char byte)
{
	lcd_busy();
    
	PORTC |= 0b11110001; 		// RS=1
	PORTC &= 0b11110101; 		// RW=0 
	PORTC |= 0b11110100; 		// E=1 
	_delay_us(50); 			// 일정시간 시간지연 
	PORTD = byte;   
	_delay_us(50); 			// 일정시간 시간지연 
	PORTC &= 0b11110011; 		// E=0, LCD 라이트 타이밍에 따라 E=1 -> 0  
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

void lcd_display_position(unsigned char row, unsigned char col)   // LCD표시 위치는 DDRAM address설정으로 결정됨
{						    //  
unsigned cmd;

	cmd = 0b10000000 | ((row-1)*0x40+(col-1));	    // DB7=1,DB6~DB0(첫번째 행: 00H~27H, 두번째행:40H~67H)
	lcd_command(cmd);				    // OR연산하면 (첫번째 행: 80H~A7H, 두번째행:C0H~E7H)
}
