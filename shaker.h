#define BAG_WEIGHT_THRESHOLD	0.3

typedef enum{

	IDLE			= 1,
	RUNNING			= 2,
	FINISHED		= 3,
	ERROR			= 4
		
}ShakStat;


void ShakerLoop(void);