#ifdef SERIAL_C

#define XTAL 12000000 // CPU Oscillator Frequency
//#define baudrate 9600 // 9600 bps communication baudrate

#define OLEN 8 // size of serial transmission buffer
unsigned char ostart; // transmission buffer start index
unsigned char oend; // transmission buffer end index
char idata outbuf[OLEN]; // storage for transmission buffer

#define ILEN 8 // size of serial receiving buffer
unsigned char istart; // receiving buffer start index
unsigned char iend; // receiving buffer end index
char idata inbuf[ILEN]; // storage for receiving buffer

bit sendfull; // flag: marks transmit buffer full
bit sendactive; // flag: marks transmitter active

#endif

void com_initialize (int baudrate);

char _getkey (void);

char putchar (char c);

void putbuf (char c);
