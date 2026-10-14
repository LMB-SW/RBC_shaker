#include <c8051f350.h>
#include "ADC.h"
#include "display.h"
								// za sad ideja da ga pokrece PC posto je poslao kalibracione podatke (ako se ide na tu opciju tj ne na flash)
bit ADC_canrun = 0; 			// kako za system calib registre (offs i gain), tako i za aplikacione calib (X0 i X1) i tada postaviti ADC_canrun= 1 i ADC_Loop_Start= 1
bit ADC_Loop_Start = 0;			// startuje prvu ADC konverziju
bit Wght_Meas_canrun = 0;

int  sampCnt= 0;
unsigned long  ADC_samps[MAXSAMPS];
	unsigned long ADC_avg;
unsigned long Wght= 0L;
unsigned long X0, X1;			// za nulu i za 500 gr, dobijeni komunikacijom
	unsigned long REF_value = 500L;

	/*unsigned*/ long Sigma = 0L;
	volatile long ADC_OutputVal = 0L;  /* Concatenated ADC output value */

	unsigned int unit = 500; //50000;		///Privremeno

	unsigned char FiltDivisor = 8;

bit InternCalib_Done = 0;
bit AppCalibOffset_Done = 0;
bit AppCalibGain_Done = 0;

		bit Gres = 0;
		bit filtriraj = 0;

union{ 
	unsigned long  ul;
	unsigned char  uch[4];
}ULONG;

//extern Dspstruct  dspvar;



long ADC2Blong(void){		    // bipolarna verzija
long poml;

  ULONG.uch[3]= ADC0L; 	
  ULONG.uch[2]= ADC0M;	
  ULONG.uch[1]= ADC0H;	
  ULONG.uch[0]= 0;		

  if (ULONG.ul & 0x00800000){
	poml= 0- (0x01000000- ULONG.ul);
  }
  else poml= ULONG.ul;

  return poml >> CUT;
}

unsigned long ADC2Ulong(void){	// unipolarna verzija

  ULONG.uch[3]= ADC0L; 	
  ULONG.uch[2]= ADC0M;	
  ULONG.uch[1]= ADC0H;	
  ULONG.uch[0]= 0;		

  return ULONG.ul >> CUT;
}

/////////////////////////////////////////////////////////////////////

long ADC_Filter(long new_value)
{
    static long filtered = 0;
    static bit first = 1;

    if (first)
    {
        filtered = new_value;
        first = 0;
    }
    else
    {
        filtered = filtered + (new_value - filtered) / FiltDivisor;	// delilac je bio 8

//		if (filtered - new_value < (X1 - X0) / 10)
//			filtered = X0;
    }

    return filtered;
}

/////////////////////////////////////////////////////////////////////
void InternCalibFull(void)
{
	ADC0MD= 0x80;  // idle

	ADC0MD= 0x81;  // startuj intern offs
	while (!AD0INT) {}

	ADC0STA &= ~0x20;	//ADC0STA |= ~0x20;
}
/////////////////////////////////////////////////////////////////////

/*
void InternCalibO(void)
{
	ADC0MD= 0x80;  // idle

	ADC0MD= 0x84;  // startuj intern offs
	while (!AD0INT) {}

	ADC0STA &= ~0x20;	//ADC0STA |= ~0x20;
}
/////////////////////////////////////////////////////////////////////

void InternCalibG(void)
{
	ADC0MD= 0x80;  // idle

	ADC0MD= 0x85;  // startuj intern offs
	while (!AD0INT) {}

	ADC0STA &= ~0x20;	//ADC0STA |= ~0x20;
}
/////////////////////////////////////////////////////////////////////
*/

void SysCalibO(void){			// inicira ga serijska komunikacija 

  ADC0MD= 0x80;  // idle

//  	ADC0STA = 0x00;			//ZA PROBU
  
  ADC0MD= 0x86;  // startuj sys offs
  while (!AD0INT) {}
   
  //ADC0MD= 0x80; vratice se automatski posle calib

  ADC0STA &= ~0x20;	//ADC0STA |= ~0x20;

  ULONG.uch[3]= ADC0COL; 	
  ULONG.uch[2]= ADC0COM;	
  ULONG.uch[1]= ADC0COH;	
  ULONG.uch[0]= 0;			

  //save ULONG.ul u flash ili salji na UART, ali verovatno u flash uz kontrolne karaktere (npr CS) 
}

void SysCalibG(void){					//19.52mV za PGA= 128

  ADC0MD= 0x80;  // idle

//  	ADC0STA = 0x00;			//ZA PROBU
  
  ADC0MD= 0x87;  // startuj sys gain
  while (!AD0INT) {}
   
  //ADC0MD= 0x80; vratice se automatski posle calib 

  ADC0STA &= ~0x20;	//ADC0STA |= ~0x20;

  ULONG.uch[3]= ADC0CGL; 	
  ULONG.uch[2]= ADC0CGM;	
  ULONG.uch[1]= ADC0CGH;	
  ULONG.uch[0]= 0;			

  //save ULONG.ul u flash ili salji na UART, ali verovatno u flash uz kontrolne karaktere (npr CS)
}
/*
unsigned char AppCalibO(void){			// inicira ga serijska komunikacija  - daje X0
long pomL;
 
  	EA = 0;
  
  ADC0MD= 0x80;  // idle

//  	ADC0STA = 0x00;			//ZA PROBU
  
  ADC0MD= 0x82;  // start conv
  while (!AD0INT) {}
   
  //ADC0MD= 0x80; vratice se automatski posle cit

  ADC0STA &= ~0x20;	//ADC0STA |= ~0x20;
  
  ULONG.uch[3]= ADC0L; 	//ULONG.uch[3]= 0;
  ULONG.uch[2]= ADC0M;	//ULONG.uch[2]= ADC0H;
  ULONG.uch[1]= ADC0H;	//ULONG.uch[1]= ADC0M;
  ULONG.uch[0]= 0;		//ULONG.uch[0]= ADC0L;

  #ifdef BIPOLAR  
	  if (ULONG.ul & 0x00800000){
		pomL= 0- (0x01000000- ULONG.ul);
	  }
	  else pomL= ULONG.ul;
  #else	
  	  pomL= ULONG.ul;		// mozda moze i sa 3 bita da prodje
  #endif

  pomL= pomL >> CUT;
  //save pomL u flash ili salji na UART kao X0

  	  	  	X0 = pomL;	// OVO SAMO RADI TESTA - KASNIJE TO TREBA DA SE SALJE APP-U

			if(X0 > 0)
			{
				AppCalibOffset_Done = 1;

				EA = 1;
				return 1;
			}
			else
			{
				AppCalibOffset_Done = 0;

				EA = 1;
				return 0;
			}

}
*/

unsigned char AppCalibO(void){			// inicira ga serijska komunikacija  - daje X0
//long pomL;
//unsigned long suma;
//int i;
//unsigned int cnt;
 
  	EA = 0;

/* 	
	Sigma   = 0L;
	ADC_avg = 0L;
	for(sampCnt = 0; sampCnt < MAXSAMPS; sampCnt++)
		ADC_samps[sampCnt] = 0L;
	//sampCnt = 0;


	for(sampCnt = 0; sampCnt <= MAXSAMPS-1; sampCnt++) 		// ovde bio cnt umesto sampCnt
  	{
 		ADC0MD= 0x80;  // idle  
  		ADC0MD= 0x82;  // start conv
  		while (!AD0INT) {}

  		ADC0STA &= ~0x20;	//ADC0STA |= ~0x20;
  
  		ULONG.uch[3]= ADC0L; 	
  		ULONG.uch[2]= ADC0M;	
  		ULONG.uch[1]= ADC0H;	
  		ULONG.uch[0]= 0;		

  		#ifdef BIPOLAR  
	  		if (ULONG.ul & 0x00800000){
				pomL= 0- (0x01000000- ULONG.ul);
	  		}
	  		else pomL= ULONG.ul;
 		 #else	
  	 		 pomL= ULONG.ul;		// mozda moze i sa 3 bita da prodje
 		 #endif

  		pomL= pomL >> CUT;


	///	Sigma = Sigma - ADC_samps[sampCnt] + pomL;

		Sigma = Sigma + 0.2*(pomL - Sigma);	//Sigma = Sigma + 0.5*(pomL - Sigma);

		ADC_samps[sampCnt] = Sigma;

		//sampCnt++;
	}
		
//	ADC_avg = Sigma/MAXSAMPS;

/// 	X0 = Sigma;	//ADC_avg;	//pomL;	

		suma = 0;

		for (i= 0; i< 16MAXSAMPS; i++)
		{
	  	 #ifdef BIPOLAR
	   		suma+= (long)ADC_samps[i];
	  	 #else 
	    	suma+= ADC_samps[i];
	   	#endif
		}

		X0 = suma/16MAXSAMPS;


	Sigma   = 0L;
	ADC_avg = 0L;
	for(sampCnt = 0; sampCnt < MAXSAMPS; sampCnt++)
		ADC_samps[sampCnt] = 0L;
	sampCnt = 0;

*/
	////////////////////////////
		GetADCvalue();

		X0 = ADC_avg;
	////////////////////////////

			if(X0 > 0)
			{
				AppCalibOffset_Done = 1;

				EA = 1;
				return 1;
			}
			else
			{
				AppCalibOffset_Done = 0;

				EA = 1;
				return 0;
			}

}
/*
unsigned char AppCalibG(void){			// - daje X1
unsigned long adrd;
 
 	EA = 0;
 
  ADC0MD= 0x80;  // idle

//  	ADC0STA = 0x00;			//ZA PROBU
  
  ADC0MD= 0x82;  // start conv
  while (!AD0INT) {}
   
  //ADC0MD= 0x80; vratice se automatski posle cit

  ADC0STA &= ~0x20;	//ADC0STA |= ~0x20;
  
  ULONG.uch[3]= ADC0L; 	//ULONG.uch[3]= 0;
  ULONG.uch[2]= ADC0M;	//ULONG.uch[2]= ADC0H;
  ULONG.uch[1]= ADC0H;	//ULONG.uch[1]= ADC0M;
  ULONG.uch[0]= 0;		//ULONG.uch[0]= ADC0L;
  	
  adrd= ULONG.ul >> CUT;

  //save u flash ili salji na UART kao X1

  		X1 = adrd;	// OVO SAMO RADI TESTA - KASNIJE TO TREBA DA SE SALJE APP-U

		if(X1 > X0)
		{
			AppCalibGain_Done = 1;
			Wght_Meas_canrun = 1; 	//ADC_canrun = 1;

			EA = 1;
			return 1;
		}
		else
		{
			AppCalibGain_Done = 0;
			Wght_Meas_canrun = 0;	//ADC_canrun = 0;

			EA = 1;
			return 0;
		}
}
*/

unsigned char AppCalibG(void){			// - daje X1
//unsigned long adrd;
//unsigned int cnt = 0;
 
 	EA = 0;

/*
	Sigma   = 0L;
	ADC_avg = 0L;
	for(sampCnt = 0; sampCnt < MAXSAMPS; sampCnt++)
		ADC_samps[sampCnt] = 0L;
	//sampCnt = 0;
 

	for(sampCnt = 0; sampCnt <= MAXSAMPS-1; sampCnt++)	// bio cnt umesto sampCnt 
  	{
 		ADC0MD= 0x80;  // idle
  
  		ADC0MD= 0x82;  // start conv
  		while (!AD0INT) {}

  		ADC0STA &= ~0x20;	//ADC0STA |= ~0x20;
  
 		ULONG.uch[3]= ADC0L; 	
  		ULONG.uch[2]= ADC0M;	
  		ULONG.uch[1]= ADC0H;	
  		ULONG.uch[0]= 0;		
  	
  		adrd= ULONG.ul >> CUT;

  		//save u flash ili salji na UART kao X1

		
		Sigma = Sigma - ADC_samps[sampCnt] + adrd;

		ADC_samps[sampCnt] = adrd;

		//sampCnt++;
	}
		
	ADC_avg = Sigma/MAXSAMPS;


  		X1 = ADC_avg;	//adrd;	// OVO SAMO RADI TESTA - KASNIJE TO TREBA DA SE SALJE APP-U


	Sigma   = 0L;
	ADC_avg = 0L;
	for(sampCnt = 0; sampCnt < MAXSAMPS; sampCnt++)
		ADC_samps[sampCnt] = 0L;
	sampCnt = 0;

*/

	////////////////////////////
		GetADCvalue();

		X1 = ADC_avg;
	////////////////////////////

		if(X1 > X0)
		{
			AppCalibGain_Done = 1;
			Wght_Meas_canrun = 1; 	//ADC_canrun = 1;

			EA = 1;
			return 1;
		}
		else
		{
			AppCalibGain_Done = 0;
			Wght_Meas_canrun = 0;	//ADC_canrun = 0;

			EA = 1;
			return 0;
		}
}

void GetADCvalue(void)
{
	unsigned long suma;
	int i;

	/////////////////////////////////////////////////////////
		if(!AD0BUSY  &&  ADC0MD == 0x80)
		{
 			ADC0MD = 0x80;  // idle 
  			ADC0MD = 0x82;  // start conv  //verovatno moze samo jedna cmd ili mozda Delay izmedju (ispitati)
			//return;

			//while(!AD0INT);
		}
	/////////////////////////////////////////////////////////
			
/*
	if(ADC_Loop_Start  &&  !AD0BUSY)
	{
    	ADC_Loop_Start = 0;
 		ADC0MD = 0x80;  // idle 
  		ADC0MD = 0x82;  // start conv  //verovatno moze samo jedna cmd ili mozda Delay izmedju (ispitati)
		return;
	}

			if(AD0ERR)		// OVO SAMO RADI TESTA - OBRISI NA KRAJUI
				Gres = 1;
*/	
	if(!AD0INT)
		return;		
 	else 		
	{									
		ADC0STA &= ~0x20;	//ADC0STA |= ~0x20;


		/////////////////////////////////////////////////////////

			#ifdef  BIPOLAR
				ADC_OutputVal = ADC2Blong();	///ADC_samps[sampCnt++] = ADC2Blong();
			#else
				ADC_OutputVal = ADC2Ulong();	///ADC_samps[sampCnt++] = ADC2Ulong();
			#endif

			//ADC_OutputVal = ADC_OutputVal >> 2;			

//			Sigma = Sigma + 0.2*(ADC_OutputVal - Sigma);		//Sigma = Sigma - ADC_samps[sampCnt] + ADC_OutputVal;

			ADC_samps[sampCnt++] = ADC_OutputVal;	//Sigma;	//
/*
			if(sampCnt >= (MAXSAMPS-1))
				sampCnt = 0;
			else
				sampCnt++;

			ADC_avg = Sigma/MAXSAMPS;
*/
///				ADC_avg = Sigma;


			if(sampCnt == MAXSAMPS)
			{
				sampCnt = 0;
				suma = 0;

				for (i= 0; i< MAXSAMPS; i++)
				{
			  	 #ifdef BIPOLAR
			   		suma+= (long)ADC_samps[i];
			  	 #else 
			    	suma+= ADC_samps[i];
			   	#endif
				}

				ADC_avg = suma/MAXSAMPS;
		

				//if(filtriraj)
				//	ADC_avg = ADC_Filter(ADC_avg);	// Dodatno filtriranje ADC vrednosti
		
			} 
  	}		


		/////////////////////////////////////////////////////////

/*
		#ifdef  BIPOLAR
			ADC_samps[sampCnt++] = ADC2Blong();
		#else
			ADC_samps[sampCnt++] = ADC2Ulong();
		#endif


		if(sampCnt == MAXSAMPS)
		{
			sampCnt = 0;
			suma = 0;

			for (i= 0; i< MAXSAMPS; i++)
			{
			   #ifdef BIPOLAR
			   		suma+= (long)ADC_samps[i];
			   #else 
			    	suma+= ADC_samps[i];
			   #endif
			}

			ADC_avg= suma/MAXSAMPS;
		

				if(filtriraj)
					ADC_avg = ADC_Filter(ADC_avg);	// Dodatno filtriranje ADC vrednosti
		
		} */
  						
}


void ADCLoop(void)
{
	if(!ADC_canrun) return;	

	GetADCvalue();
								
	if(Wght_Meas_canrun)		
	{
		if(ADC_avg <= X0  ||  ADC_avg-X0 <= 3*(X1-X0)/REF_value)
			Wght = 0;
						
		else
		{
			Wght = (REF_value * (ADC_avg- X0)) / (X1-X0);

			if(Wght > 999)
				Wght = 999;
		}
	}					
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void ADCLoop(void){
unsigned long suma;
int i;
//unsigned long ADC_avg;	///unsigned int ADC_avg;	//ZA TEST

  
//  if (!ADC_canrun) return;		/// VIDI OVO DA RESIS

  if (ADC_Loop_Start==1){
     ADC_Loop_Start= 0;
 	 ADC0MD= 0x80;  // idle 
  	 ADC0MD= 0x82;  // start conv  //verovatno moze samo jedna cmd ili mozda Delay izmedju (ispitati)
	 return;
  }

 		if( AD0ERR)
			Gres = 1;

  
  if (!AD0INT) return;		
  else {									
	    ADC0STA &= ~0x20;	//ADC0STA |= ~0x20;

		#ifdef  BIPOLAR
			ADC_samps[sampCnt++]= ADC2Blong();
			///ADC_samps[sampCnt++]= ADC_Filter(ADC2Blong());	// PROBAAA
		#else
			ADC_samps[sampCnt++]= ADC2Ulong();
		#endif

		if (sampCnt == MAXSAMPS){
			sampCnt= 0;
			suma= 0;
			for (i= 0; i< MAXSAMPS; i++){
			   #ifdef BIPOLAR
			   		suma+= (long)ADC_samps[i];
			   #else 
			    	suma+= ADC_samps[i];
			   #endif
			}
			ADC_avg= suma/MAXSAMPS;
			//Wght= (unit * (ADC_avg- X0)) / (X1-X0); // u 0.01 gramima			///Wght= (50000 * (ADC_avg- X0)) / (X1-X0); // u 0.01 gramima
		
				if(filtriraj)
					ADC_avg = ADC_Filter(ADC_avg);	// Dodatno filtriranje ADC vrednosti
		
			if (ADC_canrun)		
			{
				if(ADC_avg <= X0)
				{
					Wght=0;	

//					dspvar.Cif[0] = 0;
//					dspvar.Cif[1] = 0;
//					dspvar.Cif[2] = 0;
				}		
				else
					Wght= (REF_value * (ADC_avg- X0)) / (X1-X0);
			}

		//	dspvar.newweight= Wght;
		}

//		ADC0MD= 0x80;   /// PRIVREMENO - ZA TEST
//		ADC0MD= 0x82;  
  	}					
}
*/
