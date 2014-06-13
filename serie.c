#define SERIAL_C
#include "serie.h"
#include <at89c51ic2.h>

/* Function to initialize the serial port and the UART baudrate.
*------------------------------------------------------------------------------*/
void com_initialize (int baudrate){	
// Configure timer 1 as a baud rate generator
//	PCON |= 0x80; // 0x80=SMOD: set serial baudrate doubler
//	TMOD |= 0x20; // put timer 1 into MODE 2
//	TH1 = (unsigned char) (256 - (XTAL / (16L * 12L * baudrate)));
//	TL1 = TH1;
//	TR1 = 1; // start timer 1
//	SCON = 0x50; // serial port MODE 1, enable serial receiver
//	ES = 1; // enable serial interrupts

	CKCON0 = 0x7F;
	SCON = 0x50; /* uart in mode 1 (8 bit), REN=1 */
	BDRCON &= 0xEE; /* BRR=0; SRC=0; */
	BDRCON |= 0x0E; /* TBCK=1;RBCK=1; SPD=1 *///(TBCK=1 e RBCK=1 -> gerador de baudrate interno para Transmitir e Receber)(SPD=1 para baudrate altos)
	//spd = BDRCON & 2;
	BRL = 256 - ((2 * XTAL)/(4L*16L*baudrate));//4L??16L???
	//BRL=0xD9; /* 9600 Bds at 12MHz */
	ES = 1; /* Enable serial interrupt*/
	BDRCON |=0x10; /* Baud rate generator run*/
	 
	istart = 0; // empty transmit buffers
	iend = 0;
	ostart = 0; // empty transmit buffers
	oend = 0;
	sendactive = 0; // transmitter is not active
	sendfull = 0; // clear 'sendfull' flag

}

// Serial Interrupt Service Routine
static void com_isr (void) interrupt 4 using 1 
{
	char c;	  
	//----- Received data interrupt. -----
	if (RI) {
		c = SBUF; // read character
		RI = 0; // clear interrupt request flag

		if (istart + ILEN != iend) {
			inbuf[iend++ & (ILEN-1)] = c; // put character into buffer
		}
		//putchar(c);
	}
	//----- Transmitted data interrupt. -----
	if (TI) 
	{
		TI = 0; // clear interrupt request flag
		if (ostart != oend) 
		{ // if characters in buffer and
			SBUF = outbuf[ostart++ & (OLEN-1)]; // transmit character
			sendfull = 0; // clear 'sendfull' flag
		}
		else 
		{ // if all characters transmitted
			sendactive = 0; // clear 'sendactive'
		}
	}
}
 /*--------------------------------------------------------------------------------
* Checks if input buffer is empty
*------------------------------------------------------------------------------*/
 bit inb_empty()
 {
 	return ( iend == istart);
 }
 /*--------------------------------------------------------------------------------
* Checks if output buffer is empty
*------------------------------------------------------------------------------*/
 bit outb_empty()
 {
 	return ( oend == ostart);
 }
/*--------------------------------------------------------------------------------
* Replacement routine for the standard library _getkey routine.
* The getchar and gets functions uses _getkey to read a character.
*------------------------------------------------------------------------------*/
char _getkey (void) {
	char c;

	while (iend == istart); // wait until there are characters

	ES = 0; // disable serial interrupts during buffer update
	c = inbuf[istart++ & (ILEN-1)];
	ES = 1; // enable serial interrupts again
	return (c);
}

/*--------------------------------------------------------------------------------
* Replacement routine for the standard library putchar routine.
* The printf function uses putchar to output a character.
*------------------------------------------------------------------------------*/
char putchar (char c) 
{
	if (c == '\n') { // expand new line character:
		while (sendfull); // wait until there is space in buffer
		putbuf (0x0D); // send CR before LF for <new line>
	}
	while (sendfull); // wait until there is space in buffer
	putbuf (c); // place character into buffer
	return (c);
}

 /* putbuf: write a character to SBUF or transmission buffer */
void putbuf (char c) 
{
	if (!sendfull) { // transmit only if buffer not full
		if (!sendactive) { // if transmitter not active:
			sendactive = 1; // transfer first character direct
			SBUF = c; // to SBUF to start transmission
		}
		else {
			ES = 0; // disable serial interrupts during buffer update
			outbuf[oend++ & (OLEN-1)] = c; // put char to transmission buffer
			if (((oend ^ ostart) & (OLEN-1)) == 0) {
			sendfull = 1;
			} // set flag if buffer is full
			ES = 1; // enable serial interrupts again
		}
	}
}
