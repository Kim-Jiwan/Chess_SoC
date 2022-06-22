module bin2seg(
input	wire	[3:0]	bin,

output	reg		[7:0]	seg
);

always @(bin)
	begin
		case(bin)
			4'h0 : 		seg <= 8'h03; // 0
			4'h1 : 		seg <= 8'h9F; // 1
			4'h2 : 		seg <= 8'h25; // 2
			4'h3 : 		seg <= 8'h0D; // 3
			4'h4 : 		seg <= 8'h99; // 4
			4'h5 : 		seg <= 8'h49; // 5
			4'h6 : 		seg <= 8'h41; // 6
			4'h7 : 		seg <= 8'h1B; // 7
			4'h8 : 		seg <= 8'h01; // 8
			4'h9 : 		seg <= 8'h19; // 9
			4'hA : 		seg <= 8'hFD; // -

			4'hB :		seg <= 8'hD5; // n
			4'hC :		seg <= 8'hC5; // o
			4'hD :		seg <= 8'h61; // E
			4'hE :		seg <= 8'h31; // P
			4'hF :		seg <= 8'hFF; // space

			default : 	seg = 8'hFF;
		endcase
	end

endmodule
