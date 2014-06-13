#include <at89c51ic2.h>
#include <stdio.h>
#include "serie.h"

#define PB1 !P3_3
#define PB2 !P3_5

bit transmit_completed;
unsigned char dados,endereco,serial_data;
bit rw=1;

void init();
void init_SPI();
void init_I2C();

void main()
{		
		
		init();
		_getkey();
		printf("GAME ON!!\n\n");
		while(1)	     					   /* endless  */
		{
			if(transmit_completed)
			{
			 	putchar(serial_data);
				transmit_completed = 0;
			}				
			if(PB1)
			{
				putchar('-');
				while(PB1);
			}

			else if(PB2)
			{
				putchar('+');
				while(PB2);
			}
			
		}
}

void init()
{
	com_initialize(9600);
  void init_SPI();
	void init_I2C();
	EA=1;                         /* enable interrupts */
}

void init_SPI()
{
	SPCON |= 0x20;                /* P1.1 is available as standard I/O pin */
	SPCON &= ~0x10;               /* slave mode */
	SPCON |= 0x20;               /* CPOL=0; transmit mode example*/
	SPCON &= ~0x08;                /* CPHA=1; transmit mode example*/
	IEN1 |= 0x04; 				   /* enable spi interrupt */     
	SPCON |= 0x40;                /* spi run */
	transmit_completed = 0;       /* clear software transfert flag */
}
void init_I2C()
{
  	SSCON |= 0x40;                   /* enable TWI */                           
	IEN1 |=0x02;   /* interrupt enable */
}
void it_SPI(void) interrupt 9 /* interrupt address is 0x004B */
{
	switch	( SPSTA )         /* read and clear spi status register */
	{
		case 0x80:
		{
         	serial_data=SPDAT;   /* read receive data */
         	transmit_completed=1;/* set software flag */
			
		}break;
	}
}

void it_TWI(void) interrupt 8
{
switch(SSCS)                     /* TWI status tasking */
   {
      case(0x00):                /* A start condition has been sent */
      {                          /* SLR+R/W are transmitted, ACK bit received */
	      b_TWI_busy=0;              /* TWI is free */
	      break;
      }
      case(0x08):                /* A start condition has been sent */
      {                          /* SLR+R/W are transmitted, ACK bit received */
	      SSCON &= ~0x20;            /* clear start condition */
	      SSDAT = (slave_adr<<1)|rw; /* send slave adress and read/write bit */
	      SSCON |= 0x04;             /* set AA */
	      break;
      }

      case(0x10):                /* A repeated start condition has been sent */
      {                          /* SLR+R/W are transmitted, ACK bit received */
	      SSCON &= ~0x20;            /* clear start condition */
	      SSDAT = (slave_adr<<1)|rw; /* send slave adress and read/write bit */
	      SSCON |= 0x04;             /* set AA */
	      break;
      }

      case(0x18):                /* SLR+W was transmitted, ACK bit received */
      {
	      SSDAT = TWI_data;          /* Transmit data byte, ACK bit received */
	      SSCON |= 0x04;             /* set AA */
	      break;
      }

      case(0x20):                /* SLR+W was transmitted, NOT ACK bit received */
      {
	      SSCON |= 0x10;             /* Transmit STOP */
	      b_TWI_busy=0;              /* TWI is free */
	      break;
      }

      case(0x28):                /* DATA was transmitted, ACK bit received */
      {
	      SSCON |= 0x10;             /* send STOP */
	      b_TWI_busy=0;              /* TWI is free */
	      break;
      }

      case(0x30):                /* DATA was transmitted, NOT ACK bit received */
      {
	      SSCON |= 0x10;             /* Transmit STOP */
	      b_TWI_busy=0;              /* TWI is free */
	      break;
      }

      case(0x38):                /* Arbitration lost in SLA+W or DATA.  */
      {
	      SSCON |= 0x10;             /* Transmit STOP */
	      b_TWI_busy=0;              /* TWI is free */
	      break;
      }
   }
SSCON &= ~0x08;                  /* clear flag */
}

