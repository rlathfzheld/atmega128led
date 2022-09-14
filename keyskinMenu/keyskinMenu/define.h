
#define 	BYTE		unsigned char
#define 	WORD		unsigned int 
#define 	LONG		unsigned long
#define 	L_LONG 		unsigned long long

#define 	SBYTE		signed char
#define 	SWORD		signed int 
#define 	SLONG		signed long
#define 	SL_LONG		signed long long

////////////////////////////////////////////////////////////////////////////////
/* Bit Flag */
#define   BIT0  		0x01	//00000001
#define   BIT1  		0x02	//00000010
#define   BIT2  		0x04	//00000100
#define   BIT3  		0x08	//00001000
#define   BIT4  		0x10	//00010000
#define   BIT5  		0x20	//00100000
#define   BIT6  		0x40	//01000000
#define   BIT7  		0x80	//10000000

////////////////////////////////////////////////////////////////////////////////

#define		SEG71_COM1	BIT0	//
#define		SEG71_COM2	BIT1	//
#define		SEG71_COM3	BIT2	//
#define		SEG71_COM4	BIT3	//

#define		SEG72_COM1	BIT4	//
#define		SEG72_COM2	BIT5	//
#define		SEG72_COM3	BIT6	//
#define		SEG72_COM4	BIT7	//

////////////////////////////////////////////////////////////////////////////////

#define		LCD_RS		BIT3	//
#define		LCD_RW		BIT4	//
#define		LCD_EN		BIT5	//

#define		LCD_DATA	PORTA	//

#define		LINE1_HEAD	0x80	//Line1 start address
#define		LINE2_HEAD	0xC0	//Line2 start address


////////////////////////////////////////////////////////////////////////////////

#define		CHAT_COUNT	5		//4x4 KEY chattering count

////////////////////////////////////////////////////////////////////////////////
