#include "CPU_setup.h"
#include "display.h"
#include "communication.h"
#include "shaker.h"


#define TIMER2_PERIOD	1	// 1ms

extern bit MessageReceived;
extern unsigned char BoardAddress;

extern bit DPoint;


xdata unsigned char ComData[UART0_BUFFER_SIZE];
unsigned int CntRX0 = 0;

unsigned char ShakingTime = 0;

unsigned char sendByteDelay = 0;

unsigned char Hundred_ms = 100 * TIMER2_PERIOD;	// 100counts x 1ms = 100ms
unsigned int OneSecond = 1000 * TIMER2_PERIOD;		// 1000counts x 1ms = 1s
unsigned int OneMin = 60000 * TIMER2_PERIOD;		// 60000counts x 1ms = 1min

unsigned char disp_cnt = DISPLAY_REFRESH_PERID - 1;
bit disp_show_flag = 0;

	unsigned char ADCsemp = 10;
	unsigned char ADCcnt = 10;

	extern char ADC_Loop_Start;

	extern ShakStat ShakerStatus;

//	bit Gres=0;


void Timer2_ISR(void) interrupt 5{			// na 1ms (PROVERITI !!!  --  jeste na 1ms, provereno osciloskopom)

	
	TF2H = 0;         /* clear Timer2 interrupt flag */


	//////////////////// on every 1 ms ///////////////////////

	if(disp_cnt)
		disp_cnt--;
	else
	{
		disp_cnt = DISPLAY_REFRESH_PERID - 1;

		disp_show_flag = 1;

	}

			if(ADCcnt)
				ADCcnt--;
			else
			{
				ADCcnt = ADCsemp;
				ADC_Loop_Start = 1;		//PRIVREMENO, ZA TEST
			}
	//////////////////////////////////////////////////////////

	//////////////////// on every 100 ms /////////////////////

	if(Hundred_ms)
		Hundred_ms--;
	else
	{
		Hundred_ms = 100 * TIMER2_PERIOD;

		if(sendByteDelay)
			sendByteDelay--;
		
		///P1 = ~P1;

		///ADC_Loop_Start = 1;		//PRIVREMENO, ZA TEST

	}
	//////////////////////////////////////////////////////////

	//////////////////// on every 1 s ////////////////////////

	if(OneSecond)
		OneSecond--;
	else
	{
		OneSecond = 1000 * TIMER2_PERIOD;
		
		///P1 = ~P1;

		if(ShakingTime  &&  ShakerStatus == RUNNING)
		{
			ShakingTime--;
			DPoint = ~DPoint;
		}

	}

	//////////////////////////////////////////////////////////

}

void Uart0_ISR(void) interrupt 4{  

	if(RI0 && !MessageReceived)		//Receiving
	{
		ComData[CntRX0]=SBUF0;				//Iz registra UART-a osmobitni podatak se smesta u elemente niza ComData		

		if(CntRX0 == 1 && (ComData[CntRX0] != BoardAddress))	//stigo bajt koji sadrzi adresu, provera pristigle adrese 
		{
			CntRX0 = 0;
			RI0 = 0;
			return;	
		}
		
		CntRX0++;
		
		if(CntRX0 >= MESSAGE_SIZE)
		{	//kraj prijema - primljena kompletna poruka
			MessageReceived = 1;
			CntRX0 = 0; 
		}

//		if(CntRX0 > (UART0_BUFFER_SIZE-1))
//			CntRX0 = 0; 

	}
	
	RI0 = 0;

		
	if(TI0)		//Transmitting
	{
		
		TI0 = 0;	
	}

}

/*
void ADC0_ISR (void) interrupt 10{

}
*/

