#include <at89c51ic2.h>

int twiData;

unsigned char vec[] = {12,0xcf,0x54,0x46,0x87,0x26,0x24,0x4f,0x04, 0x07};

void main()
{
   	IEN1 |= 0x02; //enable interrupt
	SSADR = 0x55 << 1; //slave adress
	SSCON |= 0x44;	//activate TWI & AA
	EA = 1;	    //enable all

	while(1);
}

void twi_slave_receiver() interrupt 8
{
	if(SSCS == 0x60){	 //Own SLA+W has been received; ACK has been returned
	   					 //Data byte will be received and ACK will be returned
	   SSCON |= 0x04;	 //set AA
	   //P1 = vec[0];
	}
	else if(SSCS == 0x80){	//Previously addressed with own SLA+W; data has been received; ACK has been returned
	   twiData = SSDAT;
	   SSCON |= 0x04; 		//Data byte will be received and ACK will be returned
		//P1 = vec[9];
		P1 = twiData;
	}
	else if(SSCS == 0x88){	//Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
	   twiData = SSDAT;		//Switched to the not addressed slave mode; own SLA will be recognised; GCA will be recognised if GC=logic 
	   P1 = twiData;
	   SSCON |= 0x04;
		//P1 = vec[8];
	}
	else if(SSCS == 0xA0){	//A STOP condition or repeated START condition has been received while still addressed as slave
	   SSCON |= 0x04;
	    b_TWI_busy = 0;
		//P1 = vec[3];
	}
	SSCON &= ~0x08;                  /* clear flag */
}
