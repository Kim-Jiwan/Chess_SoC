module seven_seg(
input	wire			resetn,
input	wire			segclk,
input	wire	[31:0]	data, 
output	reg		[7:0]	seg_en, 
output	reg		[7:0]	seg_data
);

wire	[7:0]	seg0;
wire	[7:0]	seg1;
wire	[7:0]	seg2;
wire	[7:0]	seg3;
wire	[7:0]	seg4;
wire	[7:0]	seg5;
wire	[7:0]	seg6;
wire	[7:0]	seg7;

reg		[14:0]	clk_cnt;


bin2seg		bin2seg_u0		(.bin(data[31:28]),	.seg(seg7)	);
bin2seg		bin2seg_u1		(.bin(data[27:24]),	.seg(seg6)	);
bin2seg		bin2seg_u2		(.bin(data[23:20]),	.seg(seg5)	);
bin2seg		bin2seg_u3		(.bin(data[19:16]),	.seg(seg4)	);
bin2seg		bin2seg_u4		(.bin(data[15:12]),	.seg(seg3)	);
bin2seg		bin2seg_u5		(.bin(data[11: 8]),	.seg(seg2)	);
bin2seg		bin2seg_u6		(.bin(data[ 7: 4]),	.seg(seg1)	);
bin2seg		bin2seg_u7		(.bin(data[ 3: 0]),	.seg(seg0)	);

always @(posedge segclk or negedge resetn)
	begin
		if(!resetn) begin
			clk_cnt <= 15'd0;
		end
		else begin
			if(clk_cnt == 15'd16384) begin
				clk_cnt <= 15'd0;
			end
			else begin
				clk_cnt <= clk_cnt + 15'd1;
			end
		end
	end

always @(posedge segclk or negedge resetn)
	begin
		if(!resetn) begin
			seg_en <= 8'b0000_0001;
		end
		else begin
			if(clk_cnt == 15'd16384) begin
				seg_en <= {seg_en[6:0], seg_en[7]};
				
				// if(seg_en == 8'b1000_0000)
				// 	seg_en <= 8'b0000_0001;
				// else
				// 	seg_en <= seg_en << 1;
			end
			else begin
				seg_en <= seg_en;
			end
		end
	end

always @(seg_en or seg0 or seg1 or seg2 or seg3 or seg4 or seg5 or seg6 or seg7)
	begin
		case(seg_en)
			// Selecting seg_data MUX
			8'b0000_0001 : seg_data = seg0;
			8'b0000_0010 : seg_data = seg1;
			8'b0000_0100 : seg_data = seg2 - 1'b1;
			8'b0000_1000 : seg_data = seg3;
			8'b0001_0000 : seg_data = seg4;
			8'b0010_0000 : seg_data = seg5;
			8'b0100_0000 : seg_data = seg6;
			8'b1000_0000 : seg_data = seg7;
			default : 	   seg_data = 8'hFF;
		endcase
	end

endmodule
