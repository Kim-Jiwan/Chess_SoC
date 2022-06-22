`timescale 1ns / 1ps
module vertical
(
    input wire rstn,
    input wire clk,
    input wire [9:0] HsyncCount,
    
    output reg [8:0] VsyncCount,
    output reg Vsync,
    output wire vDE
);

assign vDE = (VsyncCount > 9'd11) && (VsyncCount <= 9'd283) ? 1'b1 : 1'b0;

always @(negedge clk or negedge rstn) begin
	if(!rstn) begin
		VsyncCount	<=	9'd0;
	end
	else begin
		if(HsyncCount == 10'd524) begin
			if(VsyncCount < 9'd285) begin
				VsyncCount	<=	VsyncCount + 9'd1;
			end
			else begin
				VsyncCount	<=	9'd0;
			end
		end
		else begin
			VsyncCount	<=	VsyncCount;
		end
	end
end


always @(negedge clk or negedge rstn) begin
	if(!rstn) begin
		Vsync	<=	1'b0;
	end
	else begin
		if(VsyncCount <= 9'd9) begin
			Vsync	<=	1'b0;
		end
		else begin
			Vsync	<=	1'b1;
		end
	end
end

endmodule

