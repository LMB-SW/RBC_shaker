#define CUT 	4
#define BIPOLAR
#define MAXSAMPS	32	//20


void InternCalibO(void);
void InternCalibG(void);

void SysCalibO(void);
void SysCalibG(void);
void AppCalibO(void);
void AppCalibG(void);
void ADCLoop(void);

long ADC_Filter(long new_value);
