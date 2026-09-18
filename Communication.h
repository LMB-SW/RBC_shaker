//////////// Communication parameters ///////////
/// Baud rate: 	115200
/// Data bits: 	8
/// Parity: 	None
/// Stop bits:	1
/////////////////////////////////////////////////

#define UART0_BUFFER_SIZE		10 		//velicina bufera za UART0
#define MESSAGE_SIZE			9		//velicina komunikacione poruke

#define START	0x21
#define END		0x3B

/////////////////////////////////////////////////
#define	REQUEST_FOR_WEIGHT				 0x31
#define	TAKE_REF_POINT_FOR_ZER0			 0x32
#define	REQUEST_FOR_AD_VALUE			 0x33
#define	TAKE_REF_POINT_FOR_REF_VALUE	 0x34
#define	REQUEST_FOR_X0_CURRENT_VALUE	 0x35
#define	REQUEST_FOR_X1_CURRENT_VALUE	 0x36
#define	WRITE_X1_VALUE					 0x39
#define	WRITE_X0_VALUE					 0x3a
/////////////////////////////////////////////////
/*
enum{

	REQUEST_FOR_WEIGHT				= 0x31,
	TAKE_REF_POINT_FOR_ZER0			= 0x32,
	REQUEST_FOR_AD_VALUE			= 0x33,
	TAKE_REF_POINT_FOR_REF_VALUE	= 0x34,
	REQUEST_FOR_X0_CURRENT_VALUE	= 0x35,
	REQUEST_FOR_X1_CURRENT_VALUE	= 0x36,
	WRITE_X1_VALUE					= 0x39,
	WRITE_X0_VALUE					= 0x3a
		
}CommandType;
*/

unsigned char GenCheckSum(void);
unsigned char CalcCheckSum(unsigned char *buf);
void SendMessage(unsigned long Data);