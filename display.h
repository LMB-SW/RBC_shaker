
#define	DISPLAY_REFRESH_PERID	3	// 3ms is refresh period for displays (3ms per digit)


typedef struct {
  unsigned long oldweight;
  unsigned long newweight;
  unsigned char oldTime;
  unsigned char newTime;
  unsigned char Cif[3];
  char Dps[3];
}Dspstruct;



void DisplayInit(void);
void DisplayLoop(void);
