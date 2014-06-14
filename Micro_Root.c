#include <at89c51ic2.h>
#include <stdio.h>
#include "serie.h"

#define PB1 P3_3
#define PB2 P3_5

/*SPI*/
bit spi_transmit_completed;
unsigned char SPI_data;

/* TWI */ 
bit rw = 0;
bit b_TWI_busy;
signed char slave_adr = 0x55;
unsigned char TWI_data;

/*GAME*/
bit b_game_running = 0;
int P1_Counter = 0;
int P2_Counter = 0;
void EndGame();
/* MISC */
unsigned char vec[] = {12,0xcf,0x54,0x46,0x87,0x26,0x24,0x4f,0x04, 0x07};  //hexa para Disp = 1,2,...

void init();
void init_SPI();
void init_I2C();
void set_GameTimer();

void main()
{			
	init();
	while(1){
		printf("\nREADY?!\n");
		_getkey();
		printf("\nGAME ON!!\n\n");
		TR2 = b_game_running = 1;
	
		while(b_game_running)	     					   
		{
			if(spi_transmit_completed)
			{
			 	putchar(SPI_data);
				spi_transmit_completed = 0;
				P2_Counter++;
			}				
			if(!PB1)
			{
				putchar('-');
				P1_Counter++;
				while(!PB1);
			}
	
			else if(!PB2)
			{
				putchar('+');
				P1_Counter++;
				while(!PB2);
			}	
		}
		EndGame();	
	}
}

void init()
{
	com_initialize(9600);
    init_SPI();
	init_I2C();
	set_GameTimer();
	EA=1;                         /* enable interrupts */
}

void init_SPI()
{
	SPCON |= 0x20;                /* P1.1 is available as standard I/O pin */
	SPCON &= ~0x10;               /* slave mode */
//	SPCON |= 0x20;                /* CPOL=0; transmit mode example*/
	SPCON &= ~0x08;               /* CPHA=1; transmit mode example*/
	IEN1 |= 0x04; 				  /* enable spi interrupt */     
	SPCON |= 0x40;                /* spi run */
	spi_transmit_completed = 0;   /* clear software transfert flag */
}

void init_I2C()
{
  	SSCON |= 0x40;                /* enable TWI */                           
	IEN1 |=0x02;   				  /* interrupt enable */
}

void set_GameTimer()
{
	TL2 = -50000 & 0xFF;
	TH2 = -50000 >> 8;
	RCAP2H = TH2;
	RCAP2L = TL2;
	ET2 = 1;
}

void EndGame()
{
	int winner;
	SPCON &= ~0x40; //cut controller connection
	if( P1_Counter != P2_Counter){
		winner = (P1_Counter > P2_Counter) ? 1 : 2;
		printf("\nGanhou P%d!!", winner);
	} else{
		printf("\nEmpate!");
		winner = 0;
	}

	TWI_data =	vec[winner];
	b_TWI_busy = 1;
	SSCON |= 0x20; //Start Condition
	while(b_TWI_busy);

	SPCON |= 0x40; //reconnect controller
}


void it_SPI(void) interrupt 9     /* interrupt address is 0x004B */
{
	switch	( SPSTA )             /* read and clear spi status register */
	{
		case 0x80:
		{
         	SPI_data=SPDAT;    /* read receive data */
         	spi_transmit_completed=1;/* set software flag */		
		}break;
	}
}

void it_TWI(void) interrupt 8
{
	switch(SSCS)                     /* TWI status tasking */
	{
      case(0x08):                /* A start condition has been sent */
      {                          /* SLR+R/W are transmitted, ACK bit received */
	      SSCON &= ~0x20;            /* clear start condition */
	      SSDAT = (slave_adr<<1)|rw; /* send slave adress and read/write bit */
		  //P1 = vec[0];
	      break;
      }

      case(0x10):                /* A repeated start condition has been sent */
      {                          /* SLR+R/W are transmitted, ACK bit received */
	      SSCON &= ~0x20;            /* clear start condition */
	      SSDAT = (slave_adr<<1)|rw; /* send slave adress and read/write bit */
		  //P1 = vec[1];
	      break;
      }

      case(0x18):                /* SLR+W was transmitted, ACK bit received */
      {
	      SSDAT = TWI_data;          /* Transmit data byte, ACK bit received */
	      //P1 = vec[2];
	      break;
      }

      case(0x20):                /* SLR+W was transmitted, NOT ACK bit received */
      {
	      SSCON |= 0x10;             /* Transmit STOP */
	      b_TWI_busy=0;              /* TWI is free */
		  //P1 = vec[3];
	      break;
      }

      case(0x28):                /* DATA was transmitted, ACK bit received */
      {
	      SSCON |= 0x10;             /* send STOP */
	      b_TWI_busy=0;              /* TWI is free */
		  //P1 = vec[4];
	      break;
      }

      case(0x30):                /* DATA was transmitted, NOT ACK bit received */
      {
	      SSCON |= 0x10;             /* Transmit STOP */
	      b_TWI_busy=0;              /* TWI is free */
		  //P1 = vec[5];
	      break;
      }

      case(0x38):                /* Arbitration lost in SLA+W or DATA.  */
      {
	      SSCON |= 0x10;             /* Transmit STOP */
	      b_TWI_busy=0;              /* TWI is free */
		  //P1 = vec[6];
	      break;
      }
	}
	SSCON &= ~0x08;                  /* clear flag */
}

void GameTimer_isr() interrupt 5
{
	static counter = 0;
	
	if(counter++ == 20 * 1){   //5sec
		 counter = 0;
		 b_game_running = 0;
		 TR2 = 0;
	}
    TF2 = 0;
}
