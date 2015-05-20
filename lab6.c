#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "hps_pio.h"
//#include "neural_network_float.h"		// Neural network 
//#include "neural_network_float_20x20.h"		// Neural network 
//#include "neural_network_double.h"	// Neural network 

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
	//int digitArr[400] = { 0 };
	
	
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
		//for (i = 0; i < 640; i++) colsSumArr[i] = 0;
		//for (i = 0; i < 480; i++) rowsSumArr[i] = 0;
		
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
		
		// -----------------------------------------------------------------------------------
		// 
		// Read SDRAM -> imgArr[480][640] array of 1's and 0's
		//	The image is made either 255 or 0 in the verilog code
		//  Convert to 0's and 1's in C
		//
		// -----------------------------------------------------------------------------------
		
		*oState = 3;				// State 2 - Reading image
		if (RECORD_TIME) time = getCycles();
			
		for (rows = 0; rows < 480; rows++)	// 640x480
		{	
			for(cols = 0; cols < 640; cols += 16)
			{
				Clock();
				imgArr[rows][cols] 		= *iImgData0;
				imgArr[rows][cols+1] 	= *iImgData1;
				imgArr[rows][cols+2] 	= *iImgData2;
				imgArr[rows][cols+3] 	= *iImgData3;
				imgArr[rows][cols+4] 	= *iImgData4;
				imgArr[rows][cols+5] 	= *iImgData5;
				imgArr[rows][cols+6] 	= *iImgData6;
				imgArr[rows][cols+7] 	= *iImgData7;
				
				imgArr[rows][cols+8] 	= *iImgData8;
				imgArr[rows][cols+9] 	= *iImgData9;
				imgArr[rows][cols+10] 	= *iImgData10;
				imgArr[rows][cols+11] 	= *iImgData11;
				imgArr[rows][cols+12] 	= *iImgData12;
				imgArr[rows][cols+13] 	= *iImgData13;
				imgArr[rows][cols+14] 	= *iImgData14;
				imgArr[rows][cols+15] 	= *iImgData15;
				/*
				imgArr[rows][cols+16] 	= *iImgData16;
				imgArr[rows][cols+17] 	= *iImgData17;
				imgArr[rows][cols+18] 	= *iImgData18;
				imgArr[rows][cols+19] 	= *iImgData19;
				imgArr[rows][cols+20] 	= *iImgData20;
				imgArr[rows][cols+21] 	= *iImgData21;
				imgArr[rows][cols+22] 	= *iImgData22;
				imgArr[rows][cols+23] 	= *iImgData23;
				
				imgArr[rows][cols+24] 	= *iImgData24;
				imgArr[rows][cols+25] 	= *iImgData25;
				imgArr[rows][cols+26] 	= *iImgData26;
				imgArr[rows][cols+27] 	= *iImgData27;
				imgArr[rows][cols+28] 	= *iImgData28;
				imgArr[rows][cols+29] 	= *iImgData29;
				imgArr[rows][cols+30] 	= *iImgData30;
				imgArr[rows][cols+31] 	= *iImgData31;
				*/
			}
		}
		
		if (RECORD_TIME) printf("Cycles: %d\n\n", getCycles() - time);
				
		// Restart Clock because we're done with the SDRAM
		*oStart = 1;
		
		// DEBUG - Print out entire image array with buffer
		//*
		printf("Printing out full image array with buffers\n");
		for (rows = 48; rows < 432; rows++)	// 640x480
		{
			for(cols = 64; cols < 576; cols++)
			{	
				if (imgArr[rows][cols])
					printf(" ");
				else
					printf("0");
			}
			printf("\n");
		}
		//*/
	} 
	
	return 0;
}