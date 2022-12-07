void lcd_busy(void)
{

	DDRB = 0x00;
	do{
		PORTC &= 0b00000110; // RS = 0
		PORTC |= 0b00000010; // RW = 1
		PORTC |= 0b00000100; // E = 1
	} while(PINB & 0x80);

	DDRB = 0xff;

	_delay_ms(50);
}

void lcd_command(unsigned char cmd)
{
	lcd_busy();
	_delay_us(10);
	PORTB = cmd;
	PORTC &= 0b00000110; // RS = 0
	PORTC &= 0b00000101; // RW = 0
	PORTC |= 0b00000100; // E = 1
	PORTC &= 0b00000011; // E = 0, LCD라이트 타이밍에 따라 E=1 -> 0
	_delay_us(1);		 // 일정시간 지연
}

void lcd_init()
{
	_delay_ms(150);
	DDRB = 0xff;
	DDRC = DDRC | 0x0f;

	_delay_ms(150);		 // 15ms 이상 대기
	
	PORTB = 0b00110000;
	PORTC &= 0b00000000; // E,RW,RS = 0,0,0
	_delay_us(4100);	 // 4.1ms 이상 대기
	PORTC &= 0b00000000; // E,RW,RS = 0,0,0
	_delay_us(100);		 // 100us 이상 대기
	PORTC &= 0b00000000; // E,RW,RS = 0,0,0

	lcd_command(0b00111000); // 평션 셋 : 8-bit 모드
	_delay_ms(5);
	lcd_command(0b00001000); // 디스플레이 오프
	_delay_ms(5);
	lcd_command(0b00000001); // 디스플레이 클리어
	_delay_ms(5);	
	lcd_command(0b00000110); // 엔트리 모드
	_delay_ms(5);
	lcd_command(0b00001111); // 디스플레이 온, 커서 온, 커서 깜박임
}

void lcd_data(unsigned char byte)
{
	lcd_busy();
	_delay_us(10);
	PORTC |= 0b00000001;	// RS=1
	PORTC &= 0b00000101;	// RW=0
	PORTC |= 0b00000100;	// E=1
	_delay_us(50);			// 일정 시간 시간지연
	PORTB = byte;
	_delay_us(50);			// 일정 시간 시간지연
	PORTC &= 0b00000011;	// E=0, LCD 라이트 타이밍에 따라 E=1 -> 0
}

void lcd_string(char *str)
{
	int i = 0;

	while(1)
	{
		if(str[i] == '\0') break;
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

void lcd_display_position(unsigned char row, unsigned char col)
{
	unsigned cmd;

	cmd = 0b10000000 | ((row-1)*0x40+(col-1));
	lcd_command(cmd);
}
