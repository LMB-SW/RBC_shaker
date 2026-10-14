#define BAG_WEIGHT_THRESHOLD	0.3

typedef enum{

	IDLE		= 1,
	RUNNING		= 2,
	PAUSE		= 3,
	FINISHED	= 4,
	ERROR		= 5
		
}ShakStat;


void ShakerLoop(void);