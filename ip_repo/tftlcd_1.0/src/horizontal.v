`timescale 1ns / 1ps

module horizontal
(
    input wire rstn,
    input wire clk,
    
    output reg [9:0] HsyncCount,
    output reg Hsync,
    output wire hDE
);

assign hDE = (HsyncCount > 10'd42) && (HsyncCount <= 10'd522) ? 1'b1 : 1'b0;

always @(negedge clk or negedge rstn) begin
	if(!rstn) begin
		HsyncCount	<=	10'd0;
	end
	else begin
		if(HsyncCount < 10'd524) begin
			HsyncCount	<=	HsyncCount + 10'd1;
		end
		else begin
			HsyncCount	<=	10'd0;
		end
	end
end

always @(negedge clk or negedge rstn) begin
	if(!rstn) begin
		Hsync	<=	1'b0;
	end
	else begin
		if(HsyncCount <= 10'd40) begin
			Hsync	<=	1'b0;
		end
		else begin
			Hsync	<=	1'b1;
		end
	end
end

endmodule