#include <at89c51ic2.h>
#include <stdio.h>
#include "serie.h"

#define PB1 !P3_3
#define PB2 !P3_5

bit transmit_completed = 0;
bit send;

/**
 * FUNCTION_PURPOSE: This file set up spi in master mode with 
 * Fclk Periph/128 as baud rate and without slave select pin
 * FUNCTION_INPUTS: P1.5(MISO) serial input  
 * FUNCTION_OUTPUTS: P1.7(MOSI) serial output
 * 
 */
void main(void)
{
	com_initialize(9600);
	SPCON |= 0x10;                /* Master mode */
	SPCON |= 0x82;                /* Fclk Periph/128 */
	SPCON |= 0x20;                /* P1.1 is available as standard I/O pin */
	SPCON &= ~0x08;               /* CPOL=0; transmit mode example */
	SPCON |= 0x04;                /* CPHA=1; transmit mode example */
	IEN1  |= 0x04;                /* enable spi interrupt */
	SPCON |= 0x40;                /* run spi */
	EA=1;                         /* enable interrupts */

	while(1)			   			/* endless  */
   {
   		if(PB1){
			SPDAT = 'B';
			send = 1;
			P1_0 = 0;
			P1_3 = 1;     
		}
		else if(PB2){
			SPDAT = 'C';
			send = 1;
			P1_0 = 1;
			P1_3 = 0;
		}
		if(send){
			while(!transmit_completed);		  /* wait end of transmition */
			transmit_completed = send = 0;    /* clear software transfert flag */
			while(PB1 || PB2);
		}
   }
}


/**
 * FUNCTION_PURPOSE:interrupt
 * FUNCTION_INPUTS: void
 * FUNCTION_OUTPUTS: transmit_complete is software transfert flag
 */
void it_SPI(void) interrupt 9 /* interrupt address is 0x004B */
{
	switch	( SPSTA )         /* read and clear spi status register */
	{
		case 0x80:
         transmit_completed = 1;/* set software flag */
		 //SPDAT = 0;
 		break;

		case 0x10:
         /* put here for mode fault tasking */	
		break;
	
		case 0x40:
         /* put here for overrun tasking */	
		break;
	}
}


