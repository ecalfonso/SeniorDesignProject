#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//#include "neural_network_float.h"		// Neural network 
//#include "neural_network_double.h"	// Neural network 


volatile int * oStart			= (int *) 0xFF200170;
volatile int * oClock			= (int *) 0xFF200100;		// Increments counter register in verilog
volatile int * oState			= (int *) 0xFF200120;		// Used to show the state with LEDs
volatile int * oDigits			= (int *) 0xFF200130;		// Displays proposed digits to HEX modules

volatile int * oRowAddr			= (int *) 0xFF200140;
volatile int * oColAddr			= (int *) 0xFF200110;

volatile int * iRowData			= (int *) 0xFF200160;
volatile int * iColData			= (int *) 0xFF2000F0;

volatile int * iImgData0			= (int *) 0xFF200150;
volatile int * iImgData1			= (int *) 0xFF2000E0;
volatile int * iImgData2			= (int *) 0xFF2000D0;
volatile int * iImgData3			= (int *) 0xFF2000C0;
volatile int * iImgData4			= (int *) 0xFF2000B0;
volatile int * iImgData5			= (int *) 0xFF2000A0;
volatile int * iImgData6			= (int *) 0xFF200090;
volatile int * iImgData7			= (int *) 0xFF200080;

volatile int * iImgData8			= (int *) 0xFF200070;
volatile int * iImgData9			= (int *) 0xFF200060;
volatile int * iImgData10			= (int *) 0xFF200050;
volatile int * iImgData11			= (int *) 0xFF200040;
volatile int * iImgData12			= (int *) 0xFF200030;
volatile int * iImgData13			= (int *) 0xFF200020;
volatile int * iImgData14			= (int *) 0xFF200010;
volatile int * iImgData15			= (int *) 0xFF200000;

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

int myMin(int n)
{
	if (n > 5) 	return 5;
	else		return n;
}

float mySigmoid(float x)
{
	if (x > 5) 
		return 0.001;
	else if (x < -5) 
		return 0.999;
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

	// ROI variables
	int rowsSumArr[480] = { 0 };	// Holds summation of all rows
	int colsSumArr[640] = { 0 };	// Holds summation of all cols
	
	int rowsSumMax = 0;			// These holds values for the light level
	int colsSumMax = 0;
	int rowsLevel = 0;
	int colsLevel = 0;
	
	int binaryRowsSumArr[480] = { 0 };	// We transform these values into binary values
	int binaryColsSumArr[640] = { 0 };	//    according to the calculated level
	
	int projTop = 0, projBottom = 0;	// Row/col indexes for projector space
	int projLeft = 0, projRight = 0;
	
	int roiTop = 0, roiBottom = 0;		// Row/col indexes for ROI
	int roiLeft = 0, roiRight = 0;
	
	// Segmentation and Resize variables
	int numDigits = 0;
	int currentDigit = 0;
	int digitWidth = 0;
	int digitHeight = 0;
	int segmentIntensity = 0;			// Accumulator to check if the segment isn't all white pixels
	int digitArr[784] = { 0 };
	
	// Neural network variables
	float sum;
	float Z1[200];
	float Z2[200];
	int max = 0;
	int pos = 0;
	
	int answer = 0;
	
	// Timing variables
	int RECORD_TIME = 0;				// Variable that decides if we print (1) or not (0)
	
	// -----------------------------------------------------------------------------------
	// 
	// Begin system
	//
	// -----------------------------------------------------------------------------------
	printf("System restart\n");
	*oStart = 1;				// Initiate clock system
	*oClock = 0;				// Set HPS simulated clock to 0
	*oDigits = 0;

	initCounters(); 
	volatile unsigned int time;

	while(1)
	{	
		// -----------------------------------------------------------------------------------
		// 
		// Reset the variables for next iteration
		//
		// -----------------------------------------------------------------------------------
		memset(colsSumArr, 0, sizeof(colsSumArr));
		memset(rowsSumArr, 0, sizeof(rowsSumArr));
		
		max = -100000000;
		answer = 0;
		
		rowsSumMax = 0;
		colsSumMax = 0;

		// -----------------------------------------------------------------------------------
		// 
		// Prompt user to begin
		//
		// -----------------------------------------------------------------------------------
		*oState = 1;				// State 1 - Ready
			
		if (RECORD_TIME != 2)
		{
			printf("Enter (2) for infinite loop, (1) for timing run, (0) just to run: ");
			scanf("%d", &RECORD_TIME);
		}
		
		switch (RECORD_TIME)
		{
			case (0):	break;
			case (1):	break;
			case (2):	break;
			default:	RECORD_TIME = 0;
		}
		
		*oStart = 0;				// Stop camera, begin system
		if (RECORD_TIME) time = getCycles();
		
		// -----------------------------------------------------------------------------------
		// 
		// Read SDRAM -> imgArr[480][640] array of 1's and 0's
		//	The image is made either 255 or 0 in the verilog code
		//  Convert to 0's and 1's in C
		//
		// -----------------------------------------------------------------------------------
		
		*oState = 3;				// State 2 - Reading image
			
		for (rows = 0; rows < 480; rows++)	// 640x480
		{	
			for(cols = 0; cols < 640; cols += 16)
			{
				Clock();
				imgArr[rows][cols] = *iImgData0;
				imgArr[rows][cols+1] = *iImgData1;
				imgArr[rows][cols+2] = *iImgData2;
				imgArr[rows][cols+3] = *iImgData3;
				imgArr[rows][cols+4] = *iImgData4;
				imgArr[rows][cols+5] = *iImgData5;
				imgArr[rows][cols+6] = *iImgData6;
				imgArr[rows][cols+7] = *iImgData7;
				imgArr[rows][cols+8] = *iImgData8;
				imgArr[rows][cols+9] = *iImgData9;
				imgArr[rows][cols+10] = *iImgData10;
				imgArr[rows][cols+11] = *iImgData11;
				imgArr[rows][cols+12] = *iImgData12;
				imgArr[rows][cols+13] = *iImgData13;
				imgArr[rows][cols+14] = *iImgData14;
				imgArr[rows][cols+15] = *iImgData15;
			}
		}
		
		// Restart Clock because we're done with the SDRAM
		*oStart = 1;
		
		if (RECORD_TIME) printf("Cycles: %d\n\n", getCycles() - time);
		
		// DEBUG - Print out entire image array with buffer
		//*
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