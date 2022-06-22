module clk_divider_ms(
input	wire			resetn,
input	wire			segclk,
input	wire	[1:0]		reg_a,
input   wire            rstn,

output  reg     [3:0]   cnt_msec1,
output  reg     [3:0]   cnt_msec10,
output	reg		[3:0]	cnt_sec1,
output	reg		[3:0]	cnt_sec10
);

reg     [20:0]  cnt_onemsec;
reg             clk_onemsec;
reg           rst;


/////////////////////////////////////////////
//       below are fundamental code        //



always @(posedge segclk or negedge resetn)
begin
	if(!resetn) begin
		cnt_onemsec <= 21'd0;
	end
	else begin
		if(cnt_onemsec < 21'd124999) begin // Intuition : 1sec보다 분주�??????????? 10�??????????? ?��?���??????????? 1ms?�� 것이?��
			cnt_onemsec <= cnt_onemsec + 21'd1;
		end
		else begin
			cnt_onemsec <= 21'd0;
		end
	end
end

always @(posedge segclk or negedge resetn)
begin 
        if(!resetn) begin
            clk_onemsec <= 1'b1;
        end
        else begin
                if(cnt_onemsec == 21'd124999) begin
                        clk_onemsec <= ~clk_onemsec;
                end
                else begin 
                        clk_onemsec <= clk_onemsec;
                end
        end
end

always @(reg_a) begin
    if(reg_a[0] || reg_a[1] == 1'b1) begin
        rst <= 1'b1;
    end
    else begin
        rst <= 1'b0;
    end
end

always @(posedge clk_onemsec or negedge resetn or posedge rst)
begin
	if(!resetn || rst) begin
		cnt_msec1 <= 4'd9;
	end
	else begin
		if(cnt_msec1 > 4'd0) begin
			cnt_msec1 <= cnt_msec1 - 4'd1;
		end
		else if({cnt_sec10, cnt_sec1, cnt_msec1} == 12'd0) begin
				cnt_msec1 <= 4'd0;
			end
		else begin
			cnt_msec1 <= 4'd9;
		end
	end
end



always @(posedge clk_onemsec or negedge resetn or posedge rst)
begin
	if(!resetn || rst) begin
		cnt_msec10 <= 4'd9;
	end
	else begin
		if(cnt_msec1 == 4'd0) begin
			if(cnt_msec10 > 4'd0) begin
				cnt_msec10 <= cnt_msec10 - 4'd1;
			end
			else if({cnt_sec10, cnt_sec1} == 8'd0) begin
				cnt_msec10 <= 4'd0;
			end
			else begin
				cnt_msec10 <= 4'd9;
			end
		end
		else begin
			cnt_msec10 <= cnt_msec10;
		end
	end
end

// ?��기서 resetn ???��?�� interrupt�??????????? 받아?��겠다!!
always @(posedge clk_onemsec or negedge resetn or posedge rst)
begin
	if(!resetn || rst) begin
		cnt_sec1 <= 4'd9;
	end
	else begin
        if ({cnt_msec1, cnt_msec10} == 8'd0) begin
            if(cnt_sec1 > 4'd0) begin
                cnt_sec1 <= cnt_sec1 - 4'd1;
            end
			else if (cnt_sec10 == 4'd0) begin
				cnt_sec1 <= 4'd0;
			end
            else begin
                cnt_sec1 <= 4'd9;
            end
        end
		else begin
			cnt_sec1 <= cnt_sec1;
		end
	end
end

always @(posedge clk_onemsec or negedge resetn or posedge rst)
begin
	if(!resetn || rst) begin
		cnt_sec10 <= 4'd9;
	end
	else begin
		if({cnt_msec1, cnt_msec10, cnt_sec1} == 12'd0) begin
			if(cnt_sec10 > 4'd0) begin
				cnt_sec10 <= cnt_sec10 - 4'd1;
			end
			else begin
				cnt_sec10 <= 4'd0;
				// After turn, Can I invoke interrupt in 7-seg IP ?
			end
		end
		else begin
			cnt_sec10 <= cnt_sec10;
		end
	end
end

endmodule

