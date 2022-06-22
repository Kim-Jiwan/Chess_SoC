`timescale 1ns / 1ps

//
// TFT-LCD에 Color Test Pattern을 display하기 위한 coding
//

module TFTLCDctrl (
    input clk,
    input rstn,
    
    output wire opclk,	// TFT-LCD Clock
    output wire Hsync,	// TFT-LCD HSYNC
    output wire Vsync,	// TFT-LCD VSYNC
    output wire DE, // R, G, B signal이 BRAM에 의해 변경된다!!!!!
    /*
    output wire [4:0] R, // TFT-LCD Red signal 
    output wire [5:0] G, // TFT-LCD Green signal
    output wire [4:0] B, // TFT-LCD Blue signal
    */
    output wire TFTLCD_Tpower,  // TFT-LCD Backlight On signal
    output wire TFTLCD_DE_out,	// TFT-LCD Data enable
    //--------------------------------
    // 여기 Left, Up, Down, Right를
    // Port에 직접 연결하는구나!
    input       Left,
    input       Up,
    input       Down,
    input       Right
    //--------------------------------
    ); //BRAM Data 16bits
   
    wire [9:0] HsyncCount;
    wire [8:0] VsyncCount;
    wire hDE;
    wire vDE;
    wire DE;

    assign TFTLCD_DE_out = 1'b1;
    assign TFTLCD_Tpower = 1'b1;
    assign DE = hDE & vDE;

g2m g2m_u0(
    .rstn(rstn),
    .clk(clk),
    
    .opclk(opclk));

horizontal horizontal_u0(
    .rstn(rstn),
    .clk(opclk),
    
    .HsyncCount(HsyncCount),
    .Hsync(Hsync),
    .hDE(hDE));
    
vertical vertical_u0(
    .rstn(rstn),
    .clk(opclk),
    .HsyncCount(HsyncCount),
    
    .VsyncCount(VsyncCount),
    .Vsync(Vsync),
    .vDE(vDE));

/*
rgb rgb_u0(
    .rstn(rstn),
    .clk(opclk),
    .DE(DE),
    .HsyncCount(HsyncCount),
    .VsyncCount(VsyncCount),
    .Move_Left(Left),
    .Move_Up(Up),
    .Move_Down(Down),
    .Move_Right(Right),
    
	.R(R),
	.G(G), 
	.B(B));
*/
	
endmodule
