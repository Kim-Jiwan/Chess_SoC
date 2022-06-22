`timescale 1ns / 1ps

//
// TFT-LCD Clock 생성
// 입력 Clock을 2분주 시킴
//

module g2m(
  input clk,
  input rstn,
  output reg opclk);
    
  always@(negedge clk or negedge rstn)
  begin
    if(!rstn)
        opclk <= 0;
    else
        opclk <= ~opclk;
  end
    
endmodule
