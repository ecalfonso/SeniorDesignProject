module rightShiftReg32b (
	input 				iData,
	input					iCLK,
	input 				iRST,
	output reg [31:0] oData
	);
	
	
always@(posedge iCLK or negedge iRST)
begin
	if (!iRST) 	oData <= 32'b0;
	else			oData <= {iData, oData[31:1]};
end 

endmodule 