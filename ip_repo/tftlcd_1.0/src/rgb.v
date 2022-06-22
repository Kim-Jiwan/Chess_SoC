`timescale 1ns / 1ps

module rgb(clk,rstn,R,G,B,HsyncCount,VsyncCount,DE,Move_Left,Move_Up,Move_Down,Move_Right);
    input       clk;
    input       rstn;
    input [9:0] HsyncCount;
    input [9:0] VsyncCount;
    input DE;
    input Move_Left;
    input Move_Up;
    input Move_Down;
    input Move_Right;
    
    output reg [4:0] R;
    output reg [5:0] G;
    output reg [4:0] B;
    
    wire Left_oneshot;
    wire Up_oneshot;
    wire Down_oneshot;
    wire Right_oneshot;
    
    reg Left_sig;
    reg Up_sig;
    reg Down_sig;
    reg Right_sig;
    
    reg     [9:0]   Left_cnt;
    reg     [9:0]   Up_cnt;
    reg     [9:0]   Down_cnt;
    reg     [9:0]   Right_cnt;
    
    assign Up_oneshot    =   (~Up_sig)&(~Move_Up);
    assign Down_oneshot    =   (~Down_sig)&(~Move_Down);
    assign Right_oneshot    =   (~Right_sig)&(~Move_Right); 
       
    assign Left_oneshot    =   (~Left_sig)&(~Move_Left);
    
    always@(negedge clk or negedge rstn)begin
    if(!rstn)begin
        Left_sig    <=  1'b0;
    end
    else begin
        Left_sig    <=  ~Move_Left;
    end
    end
    
    always@(negedge clk or negedge rstn)begin
    if(!rstn)begin
        Up_sig    <=  1'b0;
    end
    else begin
        Up_sig    <=  ~Move_Up;
    end
    end
    
    always@(negedge clk or negedge rstn)begin
    if(!rstn)begin
        Down_sig    <=  1'b0;
    end
    else begin
        Down_sig    <=  ~Move_Down;
    end
    end
    
    always@(negedge clk or negedge rstn)begin
    if(!rstn)begin
        Right_sig    <=  1'b0;
    end
    else begin
        Right_sig    <=  ~Move_Right;
    end
    end
    
    always@(negedge clk or negedge rstn)begin
    if(!rstn)
        Left_cnt    <=  10'd0;
    else begin
        if(Left_oneshot)
            Left_cnt    <=  Left_cnt    +   10'd8;
        else
            Left_cnt    <=  Left_cnt;
    end
    end
    
    // 우리가 하는 project에서는 8pixel 이상의 pixel을 찾아서 이동시켜야 할듯!
    // 8 x 8 에서 한번에 한칸씩 움직이는 pixel 찾아보기
    always@(negedge clk or negedge rstn)begin
    if(!rstn)
        Up_cnt    <=  10'd0;
    else begin
        if(Up_oneshot)
            Up_cnt    <=  Up_cnt    +   10'd8;
        else
            Up_cnt    <=  Up_cnt;
    end
    end
    
    always@(negedge clk or negedge rstn)begin
    if(!rstn)
        Down_cnt    <=  10'd0;
    else begin
        if(Down_oneshot)
            Down_cnt    <=  Down_cnt    +   10'd8;
        else
            Down_cnt    <=  Down_cnt;
    end
    end

    always@(negedge clk or negedge rstn)begin
    if(!rstn)
        Right_cnt    <=  10'd0;
    else begin
        if(Right_oneshot)
            Right_cnt    <=  Right_cnt    +   10'd8;
        else
            Right_cnt    <=  Right_cnt;
    end
    end
    

    always@(DE,HsyncCount,VsyncCount,rstn,Right_cnt,Left_cnt,Up_cnt,Down_cnt)
    begin
        if(!rstn)
        begin
            R = 0;
            G = 0;
            B = 0;
        end
        
        else if(DE == 1)
        begin
            if((HsyncCount >= (10'd43 + (Right_cnt) - (Left_cnt)))&&(HsyncCount <= (10'd103 + (Right_cnt) - (Left_cnt))))
            begin
                if ((VsyncCount > 10'd11 - (Up_cnt) + (Down_cnt)) && (VsyncCount <= 10'd51 - (Up_cnt) + (Down_cnt)))
                begin
                    R = 5'b1111;
                    G = 6'b000000;
                    B = 5'b11111;
                end
                
                else 
                begin
                    R = 0;
                    G = 0;
                    B = 0;
                end
            end
            else 
            begin
                R = 0;
                G = 0;
                B = 0;
            end
        end
        else
        begin
            R = 0;
            G = 0;
            B = 0;
        end
    end
    
endmodule
