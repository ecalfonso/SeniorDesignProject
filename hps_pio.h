volatile int * oStart			= (int *) 0xFF200270;
volatile int * oClock			= (int *) 0xFF200200;		// Increments counter register in verilog

volatile int * oState			= (int *) 0xFF200220;		// Used to show the state with LEDs
volatile int * oDigits			= (int *) 0xFF200230;		// Displays proposed digits to HEX modules

volatile int * oRowAddr			= (int *) 0xFF200240;
volatile int * oColAddr			= (int *) 0xFF200210;

volatile int * iRowData			= (int *) 0xFF200260;
volatile int * iColData			= (int *) 0xFF2001F0;

volatile int * iImgData0			= (int *) 0xFF200250;
volatile int * iImgData1			= (int *) 0xFF2001E0;
volatile int * iImgData2			= (int *) 0xFF2001D0;
volatile int * iImgData3			= (int *) 0xFF2001C0;
volatile int * iImgData4			= (int *) 0xFF2001B0;
volatile int * iImgData5			= (int *) 0xFF2001A0;
volatile int * iImgData6			= (int *) 0xFF200190;
volatile int * iImgData7			= (int *) 0xFF200180;

volatile int * iImgData8			= (int *) 0xFF200170;
volatile int * iImgData9			= (int *) 0xFF200160;
volatile int * iImgData10			= (int *) 0xFF200150;
volatile int * iImgData11			= (int *) 0xFF200140;
volatile int * iImgData12			= (int *) 0xFF200130;
volatile int * iImgData13			= (int *) 0xFF200120;
volatile int * iImgData14			= (int *) 0xFF200110;
volatile int * iImgData15			= (int *) 0xFF200100;

volatile int * iImgData16			= (int *) 0xFF2000F0;
volatile int * iImgData17			= (int *) 0xFF2000E0;
volatile int * iImgData18			= (int *) 0xFF2000D0;
volatile int * iImgData19			= (int *) 0xFF2000C0;
volatile int * iImgData20			= (int *) 0xFF2000B0;
volatile int * iImgData21			= (int *) 0xFF2000A0;
volatile int * iImgData22			= (int *) 0xFF200090;
volatile int * iImgData23			= (int *) 0xFF200080;

volatile int * iImgData24			= (int *) 0xFF200070;
volatile int * iImgData25			= (int *) 0xFF200060;
volatile int * iImgData26			= (int *) 0xFF200050;
volatile int * iImgData27			= (int *) 0xFF200040;
volatile int * iImgData28			= (int *) 0xFF200030;
volatile int * iImgData29			= (int *) 0xFF200020;
volatile int * iImgData30			= (int *) 0xFF200010;
volatile int * iImgData31			= (int *) 0xFF200000;
