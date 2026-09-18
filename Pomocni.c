#define SYSCLK    24500000          /* SYSCLK frequency (Hz) */
#define BAUDRATE  115200            /* UART0 Baudrate (bps) */
#define MDCLK     2457600           /* Modulator Clock (Hz) */
#define OWR       10                /* desired Output Word Rate in Hz */

sbit                C1 = P0 ^ 0;    /* LED='1' means ON */
sbit                C2 = P0 ^ 1;    /* LED='1' means ON */
sbit                C3 = P0 ^ 7;    /* LED='1' means ON */

code char           bcd27s[13] =
{

	/* numbers dp g f e d c b a */
	0x3f, /* 0 */
	0x06, /* 1 */
	0x5b, /* 2 */
	0x4f, /* 3 10 01 20 */
	0x66, /* 4 1+e+g 01+08+82 */
	0x6d, /* 5 -20 02-10 */
	0x7c, /* 6 5+d-f +10-04 */
	0x07, /* 7 */
	0x7f, /* 8 -20 */
	0x6f, /* 9 4+f 04 */

	/* special */
	0x80, /* '.' */
	0x00, /* ' ' */
	0x40, /* '-' */

	
};


/*
=======================================================================================================================
PORT_Init ;
* Configure the Crossbar and GPIO ports. ;
P0.4 - TX0 (push-pull) ;
P0.5 - RX0 ;
P0.6 - dir (push-pull) ;
=======================================================================================================================
*/
void PORT_Init(void){
	XBR0 = 0x01;  /* UART0 Selected */
	XBR1 = 0x40;  /* Enable crossbar and weak pull-ups */

	P0MDOUT = 0x50;
}

/*
=======================================================================================================================
SYSCLK_Init ;
* This routine initializes the system clock to use the internal 24.5MHz ;
oscillator as its clock source, with x 2 multiply for ;
49 MHz operation. Also enables missing clock detector reset. ;
=======================================================================================================================
*/
void SYSCLK_Init(void){
	unsigned  i;

	OSCICN = 0x80;    /* enable intosc */
	CLKSEL = 0x00;    /* select intosc as sysclk source */

	/* INTOSC configure */
	OSCICN = 0x83;

	/* PLL configure */
	CLKMUL = 0x00;    /* Reset Clock Multiplier */

	CLKMUL &= ~0x03;  /* select INTOSC / 2 as PLL source */

	CLKMUL |= 0x80;   /* Enable 4x Multipler (MULEN = 1) */

	for(i = 0; i < 125; i++);

	/* Delay for at least 5us */

	CLKMUL |= 0xC0;   /* Initialize Multiplier */

	while(!(CLKMUL & 0x20));

	/* Poll for Multiply Ready */

	/* SYSCLK configure */
	VDM0CN = 0x80;    /* enable VDD monitor */
	RSTSRC = 0x06;    /* enable missing clock detector */

	/* and VDD monitor reset sources */
	CLKSEL = 0x02;    /* select PLL as clock source */
}



/*
=======================================================================================================================
UART0_Init ;
* Configure the UART0 using Timer1, for <BAUDRATE> and 8-N-1. ;
=======================================================================================================================
*/
//#define SYSCLK    49000000          /* SYSCLK frequency (Hz) */
//#define BAUDRATE  115200            /* UART0 Baudrate (bps) */

void UART0_Init(void){
	SCON0 = 0x10;     /* 8-bit variable bit rate */

	/*
	* level of STOP bit is ignored ;
	* RX enabled ;
	* ninth bits are zeros ;
	* clear RI0 and TI0 bits
	*/

	if(SYSCLK / BAUDRATE / 2 / 256 < 1)  {

		TH1 = -(SYSCLK / BAUDRATE / 2);
		CKCON |= 0x08;  /* T1M
		* 1;
		* SCA1:0 = xx */
	}

	else if(SYSCLK / BAUDRATE / 2 / 256 < 4)  {

		TH1 = -(SYSCLK / BAUDRATE / 2 / 4);
		CKCON &= ~0x0B; /* T1M
		* 0;
		* SCA1:0 = 01 */
		CKCON |= 0x01;
	}
	else if(SYSCLK / BAUDRATE / 2 / 256 < 12)  {
		TH1 = -(SYSCLK / BAUDRATE / 2 / 12);
		CKCON &= ~0x0B; /* T1M
		* 0;
		* SCA1:0 = 00 */
	}
	else
	    {
		TH1 = -(SYSCLK / BAUDRATE / 2 / 48);
		CKCON &= ~0x0B; /* T1M
		* 0;
		* SCA1:0 = 10 */
		CKCON |= 0x02;
	}

	TL1 = TH1;        /* init Timer1 */
	TMOD &= ~0xf0;    /* TMOD: timer 1 in 8-bit autoreload */
	TMOD |= 0x20;
	TR1 = 1;          /* START Timer1 */
	TI0 = 1;          /* Indicate TX0 ready */

	//   TX_Ready = 1;                       // Flag showing that UART can transmit
	//   IP |= 0x10;                         // Make UART high priority
	ES0 = 1;                            // Enable UART0 interrupts


}

/*
===================================================================================================================
Timer2_Init ;
* Configure Timer2 to 16-bit auto-reload and generate an interrupt at ;
interval specified by <counts> using SYSCLK/48 as its time base. ;
===================================================================================================================
*/
void Timer2_Init(int counts)    {
	TMR2CN = 0x00;    /* Stop
	* Timer2;
	* Clear
	* TF2;
	* */

	/* use SYSCLK/12 as timebase */
	CKCON &= ~0x60;   /* Timer2 clocked based on
	* T2XCLK;
	* */

	TMR2RL = -counts; /* Init reload values */
	TMR2 = 0xffff;    /* set to reload immediately */
	ET2 = 1;          /* enable Timer2 interrupts */
	TR2 = 1;          /* start Timer2 */
}

//-----------------------------------------------------------------------------


/*
* Timer2_ISR ;
* This routine changes the state of the LED whenever Timer2 overflows. ;
*/
void Timer2_ISR (void) interrupt 5
{

	TF2H = 0;         /* clear Timer2 interrupt flag */

	P1=0x00;

	C1 = 1;
	C2 = 1;
	C3 = 1;

	if(ci < 2)    ci++;
	else
	    ci = 0;

if(zfl)
	P1 = ~(bcd27s[numbers[ci]]|0x80);
	else 
	P1 = ~(bcd27s[numbers[ci]]);


	if(ci == 0)  {
		C1 = 0;
		C2 = 1;
		C3 = 1;
	}
	else if(ci == 1)  {
		C1 = 1;
		C2 = 0;
		C3 = 1;
	}
	else if(ci == 2)  {
		C1 = 1;
		C2 = 1;
		C3 = 0;
	}
}


/*------------------------------------------------------------------------
Procedure:     convert ID:1
Purpose:       string to integer
Input:
Output:
Errors:
------------------------------------------------------------------------*/
long convert(void) {
	long val;

	val = 0L;
	val = val + (long) command[3];
	val = val * 256L + (long) command[4];
	val = val * 256L + (long) command[5];
	val = val * 256L + (long) command[6];

	return(val);
}

//-----------------------------------------------------------------------------
/*
=======================================================================================================================
MAIN Routine ;
=======================================================================================================================
*/


void main(void){

	/* disable watchdog timer */
	PCA0MD &= ~0x40;  /* WDTE = 0 (clear watchdog timer */

	/* enable) */
	SYSCLK_Init();    /* Initialize system clock to 49 MHz */

	PORT_Init();      /* Initialize crossbar and GPIO */


	UART0_Init();     /* Initialize UART0 */

	Timer2_Init(SYSCLK / 12 / 400); /* Init Timer2 to generate */


	EA = 1;           /* enable global interrupts */

	//... ///

}
