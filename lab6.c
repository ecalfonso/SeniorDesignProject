#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include "neural_network_float.h"		// Neural network 
//#include "neural_network_double.h"		// Neural network 

volatile int * oStart			= (int *) 0xFF200180;

volatile int * oClock			= (int *) 0xFF200100;		// Increments counter register in verilog

volatile int * iImgData0		= (int *) 0xFF200150;
volatile int * iImgData1		= (int *) 0xFF2000E0;
volatile int * iImgData2		= (int *) 0xFF2000D0;
volatile int * iImgData3		= (int *) 0xFF2000C0;
volatile int * iImgData4		= (int *) 0xFF2000B0;
volatile int * iImgData5		= (int *) 0xFF2000A0;
volatile int * iImgData6		= (int *) 0xFF200090;
volatile int * iImgData7		= (int *) 0xFF200080;
volatile int * iImgData8		= (int *) 0xFF200000;
volatile int * iImgData9		= (int *) 0xFF200070;
volatile int * iImgData10		= (int *) 0xFF200060;
volatile int * iImgData11		= (int *) 0xFF200050;
volatile int * iImgData12		= (int *) 0xFF200040;
volatile int * iImgData13		= (int *) 0xFF200030;
volatile int * iImgData14		= (int *) 0xFF200020;
volatile int * iImgData15		= (int *) 0xFF200010;

volatile int * iRowData			= (int *) 0xFF200170;
volatile int * iColData			= (int *) 0xFF2000F0;

volatile int * oRowAddr			= (int *) 0xFF200140;
volatile int * oColAddr			= (int *) 0xFF200110;

volatile int * oState			= (int *) 0xFF200120;		// Used to show the state with LEDs
volatile int * oDigits			= (int *) 0xFF200130;		// Displays proposed digits to HEX modules

void delay(int v)
{
	int c, d;
	int max;
	max = 1000 * v;
	for(c = 1; c <= max; c++)
		for(d = 1; d <= 327; d++)
		{}
}

void Clock(void)
{
	*oClock = 0;
	*oClock = 1;
}

int myPow(int n)
{
	int x[9] = {1, 10, 100, 1000, 10000, 100000, 10000000, 100000000, 1000000000};
	return x[n];
}

int myMod(int n)
{
	int x[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
	return x[n];
}

double mySigmoid(double x)
{
	if (x > 5) 
		return 0;
	else if (x < -5) 
		return 1;
	else 
		return 1/(1 + exp(x));
}

static inline unsigned int getCycles ()
{
  unsigned int cycleCount;
  // Read CCNT register
  asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(cycleCount));  
  return cycleCount;
}
static inline void initCounters ()
{
  // Enable user access to performance counter
  asm volatile ("MCR p15, 0, %0, C9, C14, 0\t\n" :: "r"(1));
  // Reset all counters to zero
  int MCRP15ResetAll = 23; 
  asm volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(MCRP15ResetAll));  
  // Enable all counters:  
  asm volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f));  
  // Disable counter interrupts
  asm volatile ("MCR p15, 0, %0, C9, C14, 2\t\n" :: "r"(0x8000000f));
  // Clear overflows:
  asm volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f));
}

int main(void){
	
	// -----------------------------------------------------------------------------------
	// 
	// Variables
	//
	// -----------------------------------------------------------------------------------
	
	// Image variables
	int imgArr[480][640];				// Array that holds image
	
	// We use a buffer of 10% on each side
	int colsMin		= 64; 		// 640*10%
	int colsMax		= 576; 		// 640 - 640*10%
	int rowsMin		= 48;		// 480*10%
	int rowsMax		= 432; 		// 480 - 480*10%
	
	// Loop indexes
	int rows, cols;					
	int i, j, k, x, y;
	
	// Timing variables
	int RECORD_TIME = 0;				// Variable that decides if we print (1) or not (0)
	
	// -----------------------------------------------------------------------------------
	// 
	// Begin system
	//
	// -----------------------------------------------------------------------------------
	printf("System restart\n");
	*oStart = 1;					// Initiate clock system
	*oClock = 0;				// Set HPS simulated clock to 0
	*oDigits = 0;

	initCounters(); 
	volatile unsigned int time;

	while(1)
	{
		
		// -----------------------------------------------------------------------------------
		// 
		// Prompt user to begin
		//
		// -----------------------------------------------------------------------------------
		*oState = 1;				// State 1 - Ready
		if (RECORD_TIME)
		{
			printf("We will print out timing\n");
			printf("Press Enter to continue or enter (0) to stop timing: ");
		} else {
			printf("We will run without timing\n");
			printf("Press Enter to continue or enter (1) to timing this, and future runs: ");
		}
		
		scanf("%d", &RECORD_TIME);
		
		switch (RECORD_TIME)
		{
			case (0):	break;
			case (1):	break;
			default:	RECORD_TIME = 0;
		}
		
		*oStart = 0;				// Stop camera
		delay(2);					// Delay to allow verilog to settle
		
		// -----------------------------------------------------------------------------------
		// 
		// Read SDRAM -> imgArr[480][640] array of 1's and 0's
		//	The image is made either 255 or 0 in the verilog code
		//  Convert to 0's and 1's in C
		//
		// -----------------------------------------------------------------------------------
		
		*oState = 3;				// State 2 - Reading image
		
		if (RECORD_TIME) time = getCycles();
		
		///*	
		for (rows = 0; rows < 480; rows++)	// 640x480
		{	
			for(cols = 0; cols < 640/8; cols++)
			{
				Clock();
				imgArr[rows][8*cols] = *iImgData0;
				imgArr[rows][8*cols+1] = *iImgData1;
				imgArr[rows][8*cols+2] = *iImgData2;
				imgArr[rows][8*cols+3] = *iImgData3;
				imgArr[rows][8*cols+4] = *iImgData4;
				imgArr[rows][8*cols+5] = *iImgData5;
				imgArr[rows][8*cols+6] = *iImgData6;
				imgArr[rows][8*cols+7] = *iImgData7;
				//imgArr[rows][8*cols+8] = *iImgData8;
				//imgArr[rows][8*cols+9] = *iImgData9;
				//imgArr[rows][8*cols+10] = *iImgData10;
				//imgArr[rows][8*cols+11] = *iImgData11;
				//imgArr[rows][8*cols+12] = *iImgData12;
				//imgArr[rows][8*cols+13] = *iImgData13;
				//imgArr[rows][8*cols+14] = *iImgData14;
				//imgArr[rows][8*cols+15] = *iImgData15;
			}
		}
		//*/
		
		/*
		for (rows = 0; rows < 480; rows++)	// 640x480
		{	
			for(cols = 0; cols < 640/16; cols++)
			{
				Clock();
				printf("%d ", *iRowData);
			}
			printf("\n");
		}
		*/
		
		if (RECORD_TIME) printf("Time to extract image: %d cycles\n", getCycles() - time);
		
		// Restart Clock because we're done with the SDRAM
		*oStart = 1;
		
		// Debug print loop
		///*
		printf("Printing out full image array with buffers\n");
		for (rows = 48; rows < 432; rows++)	// 640x480
		{
			for(cols = 64; cols < 576; cols++)
			{					// Get current value of counter[1]		
				if (imgArr[rows][cols])
					printf(" ");
				else
					printf("0");
			}
			printf("\n");
		}
		//*/
		
	} // While(1)
	
	return 0;
}