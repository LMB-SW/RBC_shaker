void Delay_us(unsigned int us);
void Delay_ms(unsigned int ms);

void putc0(unsigned char dat);
unsigned char SetBoardAddress(void);			// na osnovu jumper-a JP1 na plocici
void SendMotorSpeed(unsigned char duty_cycle);