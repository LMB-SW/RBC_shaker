#define CUT 	4
#define BIPOLAR
#define MAXSAMPS	32	//20
/*
typedef enum{

	MEASURING			= 1,
	NOT_CALIBRATED		= 2,
	UNDER_CALIBATION	= 3,	
	ERROR				= 4
		
}BalStat;
*/

void InternCalibO(void);
void InternCalibG(void);
void InternCalibFull(void);

void SysCalibO(void);
void SysCalibG(void);
unsigned char AppCalibO(void);
unsigned char AppCalibG(void);
void ADCLoop(void);

long ADC_Filter(long new_value);
