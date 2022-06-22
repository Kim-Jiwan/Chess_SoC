module top(
input	wire			resetn,
input	wire			segclk,
input	wire	[31:0]		reg_a,

output	wire	[7:0]	seg_en,
output	wire	[7:0]	seg_data
);

// dip signal?„ pulseë¡? ë°”ê¿”ì£¼ëŠ”ê²? ?•„?š”?•˜?‹¤!!

wire	[31:0]	segdata;

digital_clock digi_clk(		.resetn			(resetn),
							.segclk			(segclk),
							.reg_a(reg_a),
							.segdata		(segdata)		);
							
seven_seg sv_seg(			.resetn			(resetn),
							.segclk			(segclk),
							.data			(segdata),
							
							.seg_en			(seg_en),
							.seg_data		(seg_data)		);

endmodule

