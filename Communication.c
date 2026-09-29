#include "Communication.h"
#include "functions.h"
#include "CPU_setup.h"
#include "shaker.h"
#include "ADC.h"


bit MessageReceived = 0;
bit MessageSending = 0;
unsigned char BoardAddress;
unsigned char Command;
unsigned char bufT[MESSAGE_SIZE];
unsigned char bufR[MESSAGE_SIZE];

extern unsigned long Wght;
extern unsigned long ADC_avg;
extern unsigned long X0, X1;
extern unsigned long REF_value;
extern unsigned char ShakingTime;
extern xdata unsigned char ComData[UART0_BUFFER_SIZE];
extern bit ADC_canrun;
extern bit AppCalibOffset_Done;
extern bit AppCalibGain_Done;
extern bit Shaker_Run;

extern ShakStat ShakerStatus;

void SendMessage(unsigned long Data)
{
	unsigned char cnt;
	
	bufT[0] = START;
	bufT[1] = BoardAddress;
	bufT[2] = Command;

	bufT[3] = (Data & 0xFF000000) >> 24;
	bufT[4] = (Data & 0x00FF0000) >> 16;
	bufT[5] = (Data & 0x0000FF00) >> 8;
	bufT[6] = (Data & 0x000000FF);

	bufT[7] = END;
	bufT[8] = GenCheckSum();

	//sending
	MessageSending = 1;
	ComDir = 1;
	ES0= 0;	//disable UART interrupt
	
	for(cnt = 0; cnt < 9; cnt++)
		putc0(bufT[cnt]);

	MessageSending = 0;
	ComDir = 0;
	ES0= 1;	//enable UART interrupt
	
}

unsigned char GenCheckSum(void)
{
	unsigned char cnt, chck=0;

   	for(cnt = 0; cnt < MESSAGE_SIZE-1; cnt++)
    	chck = chck ^ bufT[cnt];
 
	return chck;	
}


unsigned char CalcCheckSum(unsigned char *buf)
{
	unsigned char cnt, chck=0;

   	for(cnt = 0; cnt < MESSAGE_SIZE-1; cnt++)
    	chck = chck ^ buf[cnt];
 
	return chck;	
}


unsigned long ConvertBytesToLong(unsigned char *command)
{
	unsigned long val = 0;

	val = val + (long) command[3];
	val = val * 256L + (long) command[4];
	val = val * 256L + (long) command[5];
	val = val * 256L + (long) command[6];

	return val;
}


unsigned char AnalyzeReceivedMessage(unsigned char *message)
{
	unsigned long X0_temp=0, X1_temp=0;
		
	if(message[1] != BoardAddress)
		return 0;							// neodgovarajuca adresa
	
	if(message[8] != CalcCheckSum(message))
		return 2;							// pogresna cek-suma


	Command = message[2];
	//CommandType = Command;	

	switch (Command)
	{
		case COM_TEST:	

			SendMessage(1);			
			break;


		case REQUEST_FOR_WEIGHT:	

			SendMessage(Wght);			
			break;

		case REQUEST_FOR_AD_VALUE:	 

			SendMessage(ADC_avg); 
			break;
			
		case REQUEST_FOR_X0_CURRENT_VALUE:
			
			SendMessage(X0);
			break;

		case REQUEST_FOR_X1_CURRENT_VALUE:

			SendMessage(X1);
			break;

		case TAKE_REF_POINT_FOR_ZER0:

			if(AppCalibO())
				SendMessage(1);
			else
				SendMessage(2);	

			break;

		case TAKE_REF_POINT_FOR_REF_VALUE:

			if(!AppCalibOffset_Done)	
				SendMessage(3);	// nedostaje prethodno odradjena AppCalib za ofset
			else 
			{
				REF_value = ConvertBytesToLong(message);	// vidi sta ces sa ovim REF_value
				
				if(AppCalibG())
					SendMessage(1);	// uspesna
				else 
					SendMessage(2);	// neuspesna
			}
	
			break;

		case WRITE_X0_VALUE:

			X0_temp = ConvertBytesToLong(message);

			if(X0_temp > 0  &&  X0_temp < 1048576)
			{
				X0 = X0_temp;
				AppCalibOffset_Done = 1;
				SendMessage(1);
			}
			else
			{
				AppCalibOffset_Done = 0;
				SendMessage(2);
			}

			break;
	
		case WRITE_X1_VALUE:	
		
			X1_temp = ConvertBytesToLong(message);

			if(!AppCalibOffset_Done)
			{	
				ADC_canrun = 0;
				AppCalibGain_Done = 0;
				SendMessage(3);	// nedostaje prethodno upisana vrednost za X0
			}
			else if(X1_temp > X0  &&  X1_temp < 1048576)
			{
				X1 = X1_temp;
				ADC_canrun = 1;
				AppCalibGain_Done = 1;
				SendMessage(1);
			}
			else
			{
				ADC_canrun = 0;
				AppCalibGain_Done = 0;
				SendMessage(2);
			}

			break;

		case RUN_SHAKER:	
		
			//ShakingTime = message[6];	//ShakingTime = ConvertBytesToLong(message);

			if(message[6] /*ShakingTime*/ > 0  &&  ShakerStatus == IDLE)
			{
				ShakingTime = message[6];
				Shaker_Run = 1;	//ShakerStatus = RUNNING; 
				SendMessage(1);
			}
			else
			{
				Shaker_Run = 0;	//ShakerStatus = IDLE; 	
				SendMessage(2);
			}
			break;
	}

	return 1;
		
}



void CommunicationLoop(void)
{
	unsigned char i;

	if(MessageReceived)		// stigla poruka od masine
	{
		for(i=0; i<MESSAGE_SIZE; i++)
    		bufR[i] = ComData[i];

		AnalyzeReceivedMessage(bufR);

		MessageReceived = 0;			
	}

}