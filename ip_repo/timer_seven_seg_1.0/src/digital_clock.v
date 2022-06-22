module digital_clock(
input	wire			resetn,
input	wire			segclk,		// 25MHz
input	wire	  [31:0]		reg_a,

output	wire	[31:0]	segdata
);

wire	[3:0]	cnt_msec1;
wire	[3:0]	cnt_msec10;
wire	[3:0]	cnt_sec1;
wire	[3:0]	cnt_sec10;


reg		[15:0]	which_player;


assign segdata = {	which_player,
					cnt_sec10,	cnt_sec1,	cnt_msec10,	cnt_msec1	};

always @(posedge segclk or negedge resetn) begin
	if (!resetn) begin
		which_player <= 16'hBCBD; // nonE
	end
	else begin
		if (reg_a[1:0] == 2'b01) begin
			which_player <= 16'hF1EF; // _1P_
			
		end

		else if(reg_a[1:0] == 2'b10) begin
			which_player <= 16'hF2EF; // _2P_
		end
		
	end
end

clk_divider_ms	clk_divider_u0(	.resetn		(resetn),
								.segclk		(segclk),
								.reg_a(reg_a[1:0]),
								
								
								.cnt_msec1	(cnt_msec1),
								.cnt_msec10	(cnt_msec10),

								.cnt_sec1	(cnt_sec1),
								.cnt_sec10	(cnt_sec10)
															);

endmodule