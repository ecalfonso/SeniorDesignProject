
module lab3test(
	////////////////////////////////////
	// FPGA Pins
	////////////////////////////////////

	// Clock pins
	input CLOCK_50,
	input CLOCK2_50,
	
	input [3:0] KEY,							//	Pushbutton[3:0]
	
	input [9:0] SW,							//	Toggle Switch[17:0]	
	
	// Seven Segment Displays
	output [6:0] HEX0,
	output [6:0] HEX1,
	output [6:0] HEX2,
	output [6:0] HEX3,
	output [6:0] HEX4,
	output [6:0] HEX5,

	// LEDs
	output [9:0] LEDR,
	
	// SDRAM
	output [11:0] DRAM_ADDR,
	output [1:0] DRAM_BA,
	output DRAM_CAS_N,
	output DRAM_CKE,
	output DRAM_CLK,
	output DRAM_CS_N,
	inout [15:0] DRAM_DQ,
	output DRAM_LDQM,
	output DRAM_RAS_N,
	output DRAM_UDQM,
	output DRAM_WE_N,	
	
	//ddr
	output [14:0] HPS_DDR3_ADDR,
	output [2:0] HPS_DDR3_BA,
	output HPS_DDR3_CAS_N,
	output HPS_DDR3_CKE,
	output HPS_DDR3_CK_N,
	output HPS_DDR3_CK_P,
	output HPS_DDR3_CS_N,
	output [3:0] HPS_DDR3_DM,
	inout [31:0] HPS_DDR3_DQ,
	inout [3:0] HPS_DDR3_DQS_N,
	inout [3:0] HPS_DDR3_DQS_P,
	output HPS_DDR3_ODT,
	output HPS_DDR3_RAS_N,
	output HPS_DDR3_RESET_N,
	input HPS_DDR3_RZQ,
	output HPS_DDR3_WE_N,
	
	// VGA
	output	[7:0] VGA_B,
	output 			VGA_BLANK_N,
	output 			VGA_CLK,
	output 	[7:0] VGA_G,
	output 			VGA_HS,
	output 	[7:0] VGA_R,
	output 			VGA_SYNC_N,
	output 			VGA_VS,
	
	// GPIO
	inout [35:0] GPIO_1							//	GPIO Connection 1
);


//=======================================================
//  REG/WIRE declarations
//=======================================================
//	CCD
wire	[11:0]	CCD_DATA;
wire			CCD_SDAT;
wire			CCD_SCLK;
wire			CCD_FLASH;
wire			CCD_FVAL;
wire			CCD_LVAL;
wire			CCD_PIXCLK;
wire			CCD_MCLK;				//	CCD Master Clock

wire		[15:0]	Read_DATA1;
wire		[15:0]	Read_DATA2;
wire					VGA_CTRL_CLK;
wire		[11:0]	mCCD_DATA;
wire					mCCD_DVAL;
wire					mCCD_DVAL_d;
wire		[15:0]	X_Cont;
wire		[15:0]	Y_Cont;
wire		[9:0]		X_ADDR;
wire		[31:0]	Frame_Cont;
wire					DLY_RST_0;
wire					DLY_RST_1;
wire					DLY_RST_2;
wire					Read;
reg		[11:0]	rCCD_DATA;
reg					rCCD_LVAL;
reg					rCCD_FVAL;
wire		[11:0]	sCCD_R;
wire		[11:0]	sCCD_G;
wire		[11:0]	sCCD_B;
wire					sCCD_DVAL;
reg		[1:0]		rClk;
wire					sdram_ctrl_clk;
wire		[9:0]		oVGA_R;   				//	VGA Red[9:0]
wire		[9:0]		oVGA_G;	 				//	VGA Green[9:0]
wire		[9:0]		oVGA_B;   				//	VGA Blue[9:0]

//hps
wire [9:0] HPS_R;
wire [9:0] HPS_G;
wire [9:0] HPS_B;
wire HPS_Capture_Start;
//=======================================================
//  Structural coding
//=======================================================

assign	CCD_DATA[0]	=	GPIO_1[13];
assign	CCD_DATA[1]	=	GPIO_1[12];
assign	CCD_DATA[2]	=	GPIO_1[11];
assign	CCD_DATA[3]	=	GPIO_1[10];
assign	CCD_DATA[4]	=	GPIO_1[9];
assign	CCD_DATA[5]	=	GPIO_1[8];
assign	CCD_DATA[6]	=	GPIO_1[7];
assign	CCD_DATA[7]	=	GPIO_1[6];
assign	CCD_DATA[8]	=	GPIO_1[5];
assign	CCD_DATA[9]	=	GPIO_1[4];
assign	CCD_DATA[10]=	GPIO_1[3];
assign	CCD_DATA[11]=	GPIO_1[1];
assign	GPIO_1[16]	=	CCD_MCLK;
assign	CCD_FVAL	=	GPIO_1[22];
assign	CCD_LVAL	=	GPIO_1[21]; 
assign	CCD_PIXCLK	=	GPIO_1[0]; //PixCLK
assign	GPIO_1[19]	=	1'b1;  // tRIGGER
assign	GPIO_1[17]	=	DLY_RST_1;


assign	VGA_CLK		=	VGA_CTRL_CLK;

assign LEDR			= HPS_State;	// Use LEDs to show curret state of HPS during processing

assign	VGA_R		=	oVGA_R[9:2];
assign	VGA_G		=	oVGA_G[9:2];
assign	VGA_B		=	oVGA_B[9:2];

always@(posedge CCD_PIXCLK)
begin
	rCCD_DATA	<=	CCD_DATA;
	rCCD_LVAL	<=	CCD_LVAL;
	rCCD_FVAL	<=	CCD_FVAL;
end

// Decompress image data 
wire [9:0] VGADataIn;
assign VGADataIn = Read_DATA2[0] ? 10'b1111111111 : 10'b0000000000;

VGA_Controller		u1	(	//	Host Side
							.oRequest(Read),				// Read Request is sent to the SDRAM when the VGA pixel scan is at the correct x and y pixel location in the active area
							
							.iRed(VGADataIn),
							.iGreen(VGADataIn),
							.iBlue(VGADataIn),
							
							//	VGA Side
							.oVGA_R(oVGA_R),
							.oVGA_G(oVGA_G),
							.oVGA_B(oVGA_B),
							.oVGA_H_SYNC(VGA_HS),
							.oVGA_V_SYNC(VGA_VS),
							.oVGA_SYNC(VGA_SYNC_N),
							.oVGA_BLANK(VGA_BLANK_N),
							//	Control Signal
							.iCLK(VGA_CTRL_CLK),
							.iRST_N(DLY_RST_2),
							);

Reset_Delay			u2	(
							.iCLK(CLOCK_50),
							.iRST(KEY[0]),
							.oRST_0(DLY_RST_0),
							.oRST_1(DLY_RST_1),
							.oRST_2(DLY_RST_2)
						);

CCD_Capture			u3	(	
							.oDATA(mCCD_DATA),
							.oDVAL(mCCD_DVAL),
							.oX_Cont(X_Cont),
							.oY_Cont(Y_Cont),
							.oFrame_Cont(Frame_Cont),
							.iDATA(rCCD_DATA),
							.iFVAL(rCCD_FVAL),
							.iLVAL(rCCD_LVAL),
							.iSTART(HPS_Capture_Start),
							.iEND(!HPS_Capture_Start),
							.iCLK(CCD_PIXCLK),
							.iRST(DLY_RST_2)
						);

RAW2RGB				u4	(	
							.iCLK(CCD_PIXCLK),
							.iRST(DLY_RST_1),
							.iDATA(mCCD_DATA),
							.iDVAL(mCCD_DVAL),
							.iThreshold(SW[9:0]),
							.oRed(sCCD_R),
							.oGreen(sCCD_G),
							.oBlue(sCCD_B),
							.oDVAL(sCCD_DVAL),
							.iX_Cont(X_Cont),
							.iY_Cont(Y_Cont)
						);
						
//=======================================================
//  FSM ROI Finder
//=======================================================

parameter IMG_TOP 	= 96;
parameter IMG_BOTTOM = 960-96;
parameter IMG_LEFT	= 128;
parameter IMG_RIGHT 	= 1280-128;

reg [31:0]	img_top;
reg [31:0] 	roi_top;
reg [31:0]	roi_bot;
reg [31:0]	roi_left;
reg [31:0]	roi_right;
reg 			roi_regionL;
reg			roi_regionR;
reg [4:0] 	roi_state;
reg [127:0] line_buff;

initial begin
	img_top 			<= 0;
	roi_top			<= 0;
	roi_bot			<= 0;
	roi_left			<= 0;
	roi_right		<= 0;
	roi_state 		<= 0;
	roi_top 			<= 0;
	roi_regionL 	<= 0;
	roi_regionR 	<= 0;
	line_buff		<= 128'h00000000000000000000000000000000; // Zero out the buffer
end
	
always@(posedge CCD_PIXCLK or negedge DLY_RST_1) begin
	if (!DLY_RST_1) begin
		img_top 			<= 0;
		roi_top			<= 0;
	   roi_bot			<= 0;
	   roi_left			<= 0;
	   roi_right		<= 0;
		roi_state 		<= 0;
		roi_top 			<= 0;
		roi_regionL 	<= 0;
		roi_regionR 	<= 0;
		line_buff		<= 128'h00000000000000000000000000000000; // Zero out the buffer
	end
	else begin
		if (IMG_LEFT <= X_Cont && X_Cont <= IMG_RIGHT && IMG_TOP <= Y_Cont && Y_Cont <= IMG_BOTTOM) begin
			case (roi_state)
				0: begin // Find Proj
					if (X_Cont == IMG_LEFT) line_buff <= 128'h00000000000000000000000000000000;	// Reset line buffer
					else begin
						line_buff <= {line_buff[126:0], sCCD_B[0]};	// Shift the pixels into a buffer
					
						// Look for 128 white pixels, it's most likely the proj
						if (X_Cont < 640 && line_buff == 128'hFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF) begin // If the line_buff == all 1s, we found the whitespace
							roi_state <= 1;
							img_top <= Y_Cont;
						end
					end
					if (Y_Cont == IMG_BOTTOM) roi_state <= 0;
				end // case 0
				
				1: begin // Find TOP
					if (X_Cont == IMG_LEFT) line_buff <= 128'h00000000000000000000000000000000;	// Reset line buffer
					else begin
						line_buff <= {line_buff[126:0], sCCD_B[0]};	// Shift the pixels into a buffer
					
						// Only care about the first half of the screen 0->640
						if (Y_Cont > img_top + 50 && X_Cont < 640 && line_buff == 128'hFFFFFFFFFFFFFFFFFFFFFFF000000000) begin // Try to find the ROI
							roi_top <= Y_Cont;
							roi_state <= 2;
						end
					end
					if (Y_Cont == IMG_BOTTOM) roi_state <= 0;
				end // case 1
				
				2: begin // Find LEFT
					if (X_Cont == IMG_LEFT) line_buff <= 128'h00000000000000000000000000000000;	// Reset line buffer
					else begin
						line_buff <= {line_buff[126:0], sCCD_B[0]};	// Shift the pixels into a buffer
					
						// Only care about the first half of the screen 0->640
						if (Y_Cont > roi_top + 24 && X_Cont < 640 && line_buff == 128'hFFFFFFFFFFFFFFFFFFFFFFF000000000) begin // Try to find the ROI
							roi_left <= X_Cont;
							roi_state <= 3;
						end
					end
					if (Y_Cont == IMG_BOTTOM) roi_state <= 0;
				end // case 2
				
				3: begin // Find RIGHT
					if (X_Cont == IMG_LEFT) line_buff <= 128'hFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF;	// Reset line buffer
					else begin
						line_buff <= {line_buff[126:0], sCCD_B[0]};	// Shift the pixels into a buffer
					
						// Only care about the second half of the screen 640->1280
						if (Y_Cont > roi_top + 24 && X_Cont > 640 && line_buff == 128'h000000000FFFFFFFFFFFFFFFFFFFFFFF) begin // Try to find the ROI
							roi_right <= Y_Cont;
						end
					end
					
					if (X_Cont == IMG_RIGHT) roi_state <= 4;
					if (Y_Cont == IMG_BOTTOM) roi_state <= 0;
				end // case 3
				
				4: begin // find BOTTOM
					if (X_Cont == IMG_LEFT) begin
						line_buff <= 128'h00000000000000000000000000000000;	// Reset line buffer
						roi_regionL 	<= 0;		
						roi_regionR    <= 0;
					end
					else begin
						line_buff <= {line_buff[126:0], sCCD_B[0]};	// Shift the pixels into a buffer

						if (Y_Cont > roi_top && X_Cont < 640 && line_buff == 128'hFFFFFFFFFFFFFFFFFFFFFF0000000000) begin
							roi_regionL <= 1;
						end
						if (Y_Cont > roi_top && X_Cont > 640 && line_buff == 128'h0000000000FFFFFFFFFFFFFFFFFFFFFF) begin
							roi_regionR <= 1; // We're still in the ROI region
						end
					end

					if (X_Cont == IMG_RIGHT) begin
						if (roi_regionL == 0 && roi_regionR == 0) begin
							roi_state <= 5;
							roi_bot <= Y_Cont;
						end
					end
					
					if (Y_Cont == IMG_BOTTOM) roi_state <= 0;
				end // case 4
				
				5: if (Y_Cont == IMG_BOTTOM) roi_state <= 0;
				default: roi_state <= 0;
				
			endcase
		end // if (IMG_LEFT <= X_Cont && X_Cont <= IMG_RIGHT && IMG_TOP <= Y_Cont && Y_Cont <= IMG_BOTTOM)
	end // else, if (!DLY_RST_1)
end // always@(posedge CCD_PIXCLK or negedge DLY_RST_1)

SEG7_LUT_8 			u5	(	
							.oSEG0(HEX0),
							.oSEG1(HEX1),
							.oSEG2(HEX2),
							.oSEG3(HEX3),
							.oSEG4(HEX4),
							.oSEG5(HEX5),
							.oSEG6(),
							.oSEG7(),
							.iDIG ({dig03, dig02, dig01, dig3, dig2, dig1})		// Show the proposed digits on the HEX displays
						);

wire [3:0] dig6;
wire [3:0] dig5;
wire [3:0] dig4;
wire [3:0] dig3;
wire [3:0] dig2;
wire [3:0] dig1;
						
bin2dec 					(
							//.iData (HPS_Digits),
							.iData({1'b0,roi_right[31:1]}),
							.HunThousand(dig6),
							.TenThousand(dig5),
							.Thousands(dig4),
							.Hundreds(dig3),
							.Tens(dig2),
							.Ones(dig1)
						);

wire [3:0] dig06;
wire [3:0] dig05;
wire [3:0] dig04;
wire [3:0] dig03;
wire [3:0] dig02;
wire [3:0] dig01;
						
bin2dec 					(
							//.iData (HPS_Digits),
							.iData({1'b0,roi_left[31:1]}),
							.HunThousand(dig06),
							.TenThousand(dig05),
							.Thousands(dig04),
							.Hundreds(dig03),
							.Tens(dig02),
							.Ones(dig01)
						);
						
//=======================================================
//  Shift register
//=======================================================
parameter SHIFT_WIDTH = 16;
reg 	[SHIFT_WIDTH-1:0] shift;
reg 	[SHIFT_WIDTH-1:0]	shift_DVAL;	
reg	[4:0]					shift_clk;

initial begin
	shift 		<= 0;
	shift_DVAL 	<= 0;
	shift_clk 	<= SHIFT_WIDTH/2;
end

always@(posedge CCD_PIXCLK or negedge DLY_RST_1) begin
	if (!DLY_RST_1) begin
		shift 		<= 0;
		shift_DVAL 	<= 0;
	end
	else begin
		shift 		<= {sCCD_B[0], shift[SHIFT_WIDTH-1:1]};
		shift_DVAL 	<= {sCCD_DVAL, shift_DVAL[SHIFT_WIDTH-1:1]};
	end
end

always@(negedge CCD_PIXCLK or negedge DLY_RST_1) begin
	if (!DLY_RST_1)	shift_clk <= SHIFT_WIDTH/2;
	else					shift_clk <= shift_clk + 1;
end

Sdram_Control_4Port	u7	(	
							//	HOST Side
						   .RESET_N(1),
							.CLK(sdram_ctrl_clk),

							//	FIFO Write Side 1
							//.WR1_DATA({1'b0,sCCD_G[11:7],sCCD_B[11:2]}),
							//.WR1_DATA({15'b000000000000000,sCCD_B[0]}),
							//.WR1(sCCD_DVAL),
							.WR1_DATA(shift),
							.WR1(shift_DVAL[0]),
							.WR1_ADDR(0),					// Memory start for one section of the memory
							//.WR1_MAX_ADDR(640*480),
							.WR1_MAX_ADDR(640*480/(SHIFT_WIDTH/2)),
							.WR1_LENGTH(256),
							//.WR1_LENGTH(1),
							//.WR1_LOAD(!DLY_RST_0),
							.WR1_LOAD(!DLY_RST_0),
							//.WR1_CLK(~CCD_PIXCLK),		// This clock is directly from the CCD Camera Module, the Camera controls the write to memory
							.WR1_CLK(shift_clk[3]),
							// CCD data is written on the falling edge of the CCD_PIXCLK

							//	FIFO Write Side 2
							//.WR2_DATA(	{1'b0,sCCD_G[6:2],sCCD_R[11:2]}),
							.WR2_DATA({15'b000000000000000,sCCD_R[0]}),
							.WR2(sCCD_DVAL),
							.WR2_ADDR(22'h100000),		// Memory start for the second section of memory - why can we not write data into one memory block?
							.WR2_MAX_ADDR(22'h100000+640*480),
							.WR2_LENGTH(256),
							//.WR2_LENGTH(1),
							//.WR2_LOAD(!DLY_RST_0),
							.WR2_LOAD(!DLY_RST_0),
							.WR2_CLK(~CCD_PIXCLK),


							//	FIFO Read Side 1
						   .RD1_DATA(Read_DATA1),
				        	//.RD1(Read),
							.RD1(1),			// Always ready since we control the clock
				        	.RD1_ADDR(0),
							//.RD1_ADDR(roi_top<<6 + roi_top << 4),
							//.RD1_MAX_ADDR(640*480),
							.RD1_MAX_ADDR(640*480/(SHIFT_WIDTH/2)),
							.RD1_LENGTH(256),
							//.RD1_LENGTH(1),
							//.RD1_LOAD(!DLY_RST_0),
							.RD1_LOAD(!DLY_RST_0 || reg_SDRAM_Reset),
							//.RD1_CLK(~VGA_CTRL_CLK),
							.RD1_CLK(reg_HPS_Clk),
						 	
							//	FIFO Read Side 2
						   .RD2_DATA(Read_DATA2),
							.RD2(Read),
							.RD2_ADDR(22'h100000), // Memory start address
							.RD2_MAX_ADDR(22'h100000+640*480),	// Allocate enough space for whole 640 x 480 display
							.RD2_LENGTH(256),	// 8 bits long data storage
							//.RD2_LENGTH(1),
				        	//.RD2_LOAD(!DLY_RST_0),
							.RD2_LOAD(!DLY_RST_0),
							.RD2_CLK(~VGA_CTRL_CLK),
							
							//	SDRAM Side - Initialize the SDRAM - Can only initialize one per design
							// Qsys does not allow the allocation of more than one SDRAM connected to the same DE1-SOC DRAM pin
						   .SA(DRAM_ADDR),
						   .BA(DRAM_BA),
        					.CS_N(DRAM_CS_N),
        					.CKE(DRAM_CKE),
        					.RAS_N(DRAM_RAS_N),
        					.CAS_N(DRAM_CAS_N),
        					.WE_N(DRAM_WE_N),
        					.DQ(DRAM_DQ),
        					.DQM({DRAM_UDQM,DRAM_LDQM})
						);
						

I2C_CCD_Config 		u8	(	
							//	Host Side
							.iCLK(CLOCK_50),
							.iRST_N(DLY_RST_2),
							.iEXPOSURE_ADJ(KEY[1]),
							//.iEXPOSURE_DEC_p(SW[0]),
							//.iZOOM_MODE_SW(SW[9]),
							.iEXPOSURE_DEC_p(1'b0),
							.iZOOM_MODE_SW(1'b0),
							//	I2C Side
							.I2C_SCLK(GPIO_1[24]),
							.I2C_SDAT(GPIO_1[23])
						);		
						
						
// Register for SDRAM Read Reset
wire HPS_SDRAM_Reset;
reg reg_SDRAM_Reset;
initial reg_SDRAM_Reset = 1;
always@(HPS_SDRAM_Reset) reg_SDRAM_Reset <= ~HPS_SDRAM_Reset;

// Register for simulated HPS clock
wire HPS_CLK;
reg reg_HPS_Clk;	
initial reg_HPS_Clk = 0;
always@(HPS_CLK) reg_HPS_Clk <= HPS_CLK;

wire [31:0] HPS_Digits;
wire [9:0] HPS_State;
		  
	mysystem u0 (
        .sdram_clk_clk                (sdram_ctrl_clk),                //             sdram_clk.clk
        .dram_clk_clk                 (DRAM_CLK),                 //              dram_clk.clk
        .d5m_clk_clk                  (CCD_MCLK),                  //               d5m_clk.clk
        .vga_clk_clk                  (VGA_CTRL_CLK),                   //               vga_clk.clk
        .system_pll_0_refclk_clk      (CLOCK_50),      //   system_pll_0_refclk.clk
        .system_pll_0_reset_reset     (1'b0),      //    system_pll_0_reset.reset
        .memory_mem_a       (HPS_DDR3_ADDR),       //      memory.mem_a
        .memory_mem_ba      (HPS_DDR3_BA),      //            .mem_ba
        .memory_mem_ck      (HPS_DDR3_CK_P),      //            .mem_ck
        .memory_mem_ck_n    (HPS_DDR3_CK_N),    //            .mem_ck_n
        .memory_mem_cke     (HPS_DDR3_CKE),     //            .mem_cke
        .memory_mem_cs_n    (HPS_DDR3_CS_N),    //            .mem_cs_n
        .memory_mem_ras_n   (HPS_DDR3_RAS_N),   //            .mem_ras_n
        .memory_mem_cas_n   (HPS_DDR3_CAS_N),   //            .mem_cas_n
        .memory_mem_we_n    (HPS_DDR3_WE_N),    //            .mem_we_n
        .memory_mem_reset_n (HPS_DDR3_RESET_N), //            .mem_reset_n
        .memory_mem_dq      (HPS_DDR3_DQ),      //            .mem_dq
        .memory_mem_dqs     (HPS_DDR3_DQS_P),     //            .mem_dqs
        .memory_mem_dqs_n   (HPS_DDR3_DQS_N),   //            .mem_dqs_n
        .memory_mem_odt     (HPS_DDR3_ODT),     //            .mem_odt
        .memory_mem_dm      (HPS_DDR3_DM),      //            .mem_dm
        .memory_oct_rzqin   (HPS_DDR3_RZQ),   //            .oct_rzqin
        .system_ref_clk_clk       (CLOCK_50),       		// HPS reference clock
        .system_ref_reset_reset   (1'b0),   					// We're not resetting
		  
		  .startsignal_export       (HPS_Capture_Start),       //         startsignal.export
        .hps_clk_out_export       (HPS_CLK),       //         hps_clk_out.export
		  
		  .hps_state_out_export     (HPS_State),     //       hps_state_out.export
        .hps_digits_out_export    (HPS_Digits),    //      hps_digits_out.export
		  
		  .imgdata_in0_export       (Read_DATA1[0]),       //         imgdata_in0.export
		  //.imgdata_in1_export       (Read_DATA1[1]),       //         imgdata_in1.export
        .imgdata_in2_export       (Read_DATA1[2]),       //         imgdata_in2.export
        //.imgdata_in3_export       (Read_DATA1[3]),       //         imgdata_in3.export
        .imgdata_in4_export       (Read_DATA1[4]),       //         imgdata_in4.export
        //.imgdata_in5_export       (Read_DATA1[5]),       //         imgdata_in5.export
        .imgdata_in6_export       (Read_DATA1[6]),       //         imgdata_in6.export
        //.imgdata_in7_export       (Read_DATA1[7]),       //         imgdata_in7.export
        .imgdata_in8_export       (Read_DATA1[8]),       //         imgdata_in8.export
        //.imgdata_in9_export       (Read_DATA1[9]),       //         imgdata_in9.export
        .imgdata_in10_export      (Read_DATA1[10]),      //        imgdata_in10.export
        //.imgdata_in11_export      (Read_DATA1[11]),      //        imgdata_in11.export
        .imgdata_in12_export      (Read_DATA1[12]),      //        imgdata_in12.export
        //.imgdata_in13_export      (Read_DATA1[13]),      //        imgdata_in13.export
        .imgdata_in14_export      (Read_DATA1[14]),      //        imgdata_in14.export
        //.imgdata_in15_export      (Read_DATA1[15]),      //        imgdata_in15.export

		  .roi_top_in_export        ({1'b0,roi_top[31:1]} + 1),        //          roi_top_in.export
		  .roi_left_in_export       ({1'b0,roi_left[31:1]}),       //         roi_left_in.export
        .roi_right_in_export      ({1'b0,roi_right[31:1]} - 38),       //        roi_right_in.export
		  .roi_bottom_in_export     ({1'b0,roi_bot[31:1]} - 1),     //       roi_bottom_in.export
		  .hps_sdram_reset_export   (HPS_SDRAM_Reset)
    );
	
endmodule