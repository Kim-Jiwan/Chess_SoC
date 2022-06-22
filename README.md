# Chess_SoC
수강정보 : ITEC0412-001/SoC 설계 및 프로그래밍/문병인 교수님
SoC 설계 및 프로그래밍 강의를 수강하면서 진행한 텀프로젝트를 정리하여 공유한 포트폴리오입니다.

## 1. 프로젝트 개요
본 강의에서 학습한 verilog HDL, C 프로그래밍, FPGA 및 xilinx vivado를 사용하여 제작한 프로젝트입니다.\
사용한 HW는 7segment, text lcd, TFT lcd(4.3inch), pushbutton 입니다.\
\
본 repo에 PS영역에서 사용한 main.c와 ip repo를 업로드하였습니다.
### Chess on Zynq
본 강의에서 사용하는 보드인 Zynq-7000을 사용해서 구현한 체스게임입니다.\
[시연 동영상 링크](https://youtu.be/7JcUij9pdh4)

- UART 통신을 통해 좌표, 움직일 말을 입력받 을수 있습니다. (pw A2 A3) 형식
- PS의 알고리즘으로 선택한 좌표로 말을 이동할 수 있습니다.
- 만약 잘못된 turn에 이동하려고 시도하면(white turn에 black piece 이동) 경고 메세지를 출력하면서 다시 입력받습니다.
- push button을 눌러서 interrupt service routine을 호출할 수 있고, 이동 확정과 되돌리기 기능을 구현했습니다.
- turn이 바뀔 때 마다 99.99초 timer가 초기화됩니다.

## 2. Design overview
### 2-1. Block diagram
