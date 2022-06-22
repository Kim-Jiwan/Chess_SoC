# Chess_SoC
수강정보 : ITEC0412-001/SoC 설계 및 프로그래밍/문병인 교수님\
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
![block_diagram](https://user-images.githubusercontent.com/65444464/174958474-436cfc02-0714-4e07-8a34-ce2e34289909.jpg)
UART로 부터 input 정보를 받고, 각 PS에서 처리 후 각 IP에 뿌려주는 방식입니다. 그리고 push button을 통해 ISR에 진입합니다.
### 2-2. Flow chart
![flow_chart](https://user-images.githubusercontent.com/65444464/174958592-47bd21c2-d578-4d1b-ad87-e5c8434d682c.jpg)
## 3. main.c
```C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"
#include "xil_printf.h"
#include "xil_io.h"

#include "xil_exception.h"
#include "xparameters.h"
#include "xscugic.h"
#include "pushbutton.h"
#include "textlcd.h"
#include "timer_seven_seg.h"
// Above are header file for interrupt

/////
#include "xil_types.h"
#include "xuartps_hw.h"
/////

/////
#define		CR				0x0D						// carriage return
#define 	FORM_MAX		20							// number maximum
#define 	NAME_MAX		10							// name maximum
/////

#define		TextLine1			0				// Text LCD line1 indicator
#define 	TextLine2			1				// Text LCD line2 indicator
#define		SP					0x20			// Space
#define		MAX					17				// Number maximum

#define		TextLine1			0				// Text LCD line1 indicator
#define 	TextLine2			1				// Text LCD line2 indicator
#define		SP					0x20			// Space
#define		MAX					17				// Number maximum

#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#define INTC_DEVICE_INT_ID	31

static FATFS fatfs;
static FIL fil;

static char white_turn[32] = "wt.bin";
static char black_turn[32] = "bt.bin";
static char chess_board[32] = "cb.bin";		// cb : chess board
static char king_white[32] = "kw.bin";		// kw : king white
static char king_black[32] = "kb.bin"; 		// kb : king black
static char queen_white[32] = "qw.bin";		// qw : queen white
static char queen_black[32] = "qb.bin";		// qb : queen black
static char rook_white[32] = "rw.bin";		// rw : rook white
static char rook_black[32] = "rb.bin";		// rb : rook black
static char bishop_white[32] = "bw.bin";
static char bishop_black[32] = "bb.bin";
static char night_white[32] = "nw.bin";
static char night_black[32] = "nb.bin";
static char pawn_white[32] = "pw.bin";
static char pawn_black[32] = "pb.bin";

static int vertical_formation[9] = {0, 34, 68, 102, 136, 170, 204, 238, 272};
static int horizontal_formation[9] = {208, 242, 276, 310, 344, 378, 412, 446, 480};

static int Data;
static int R;
static int G;
static int B;

FRESULT Res;
TCHAR *Path = "0:/";
u32 * buffer[65280];
u32 data_size = 4 * 65280; // 4byte * buffer_size because integer(u32) is 4byte
u32 NumBytesRead;

XScuGic InterruptController; 	     // Instance of the Interrupt Controller
static XScuGic_Config *GicConfig;    // The configuration parameters of the controller

void 	SD_read(char *filename);
void 	TFTLCD_write_background();
void 	TFTLCD_write_sector(char horizontal, int vertical);
void 	TFTLCD_write_turnflag(char *filename);
void 	move(int start[2], int end[2], char *piece_moved);
void 	chess_default();

void	InitMsg(void);
void	PrintChar(u8 *str);
void	PrintMsg(u8 *str);
void 	GetPieceFormation(u8 *piece_formation);
void	SetPiece(u8 *start_piece);
void 	InitValue(u8 *formation, u8 *piece);

int 	GicConfigure(u16 DeviceId);
void 	ServiceRoutine(void *CallbackRef);

//void 	GetTextLine(char *ReadReg, unsigned int TextLine);
//void	ReadTLCDReg(void);

char	*piece;
int		sp[2];
int		ep[2];
int 	turn_flag = 1;
int 	flag = 0;

int main()
{
	// TRUE : white turn
	// FALSE : black turn
	int Status;

	//////
	u32	CntrlRegister;
	u8  piece_formation[FORM_MAX]	=	{0, };
	char  start_piece[2];
	char  start_point[2];
	char  end_point[2];

	char  *ptr;
	//////

	int 	i = 0;
	char	line1[17];//line1[16]; ZYNQ CHESS
	char	line2[17];//line2[17]; Player1 - WHITE
	char	line3[17];//line3[17]; Player2 - BLACK
	char	lineW[17];//lineW[17]; PUSH THE BUTTON
	char	lineB[17];//lineB[17]; PB2:UNDO|PB3:DO

	sprintf(line1, "    ZYNQ CHESS  ");
	sprintf(lineW, " PLAYER1 - WHITE");
	sprintf(lineB, " PLAYER2 - BLACK");

	sprintf(line2, " PUSH THE BUTTON");
	sprintf(line3, " PB2:UNDO|PB3:DO");

	CntrlRegister = XUartPs_ReadReg(XPAR_PS7_UART_1_BASEADDR, XUARTPS_CR_OFFSET);

	XUartPs_WriteReg( XPAR_PS7_UART_1_BASEADDR, XUARTPS_CR_OFFSET,
					  ((CntrlRegister & ~XUARTPS_CR_EN_DIS_MASK) | XUARTPS_CR_TX_EN | XUARTPS_CR_RX_EN) );

	NumBytesRead = 0;

	InitMsg();
	chess_default();

	while(TRUE)
	{
		if((turn_flag==1)) {
			TIMER_SEVEN_SEG_mWriteReg(XPAR_TIMER_SEVEN_SEG_0_S00_AXI_BASEADDR, 4, 1);
			TIMER_SEVEN_SEG_mWriteReg(XPAR_TIMER_SEVEN_SEG_0_S00_AXI_BASEADDR, 4, 0);
			do {
				for(i = 0; i < 4; i++)
				{
					TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, i*4, line1[i*4+3] + (line1[i*4+2]<<8) + (line1[i*4+1]<<16) + (line1[i*4]<<24));
					TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, i*4+16, lineW[i*4+3] + (lineW[i*4+2]<<8) + (lineW[i*4+1]<<16) + (lineW[i*4]<<24));
				}
				PrintChar("Enter the piece and start, end formation(format : pw A2 A3) : ");

				GetPieceFormation(piece_formation);

				ptr = strtok(piece_formation, " ");
				strcpy(start_piece, ptr);

				ptr = strtok(NULL, " ");
				strcpy(start_point, ptr);

				ptr = strtok(NULL, " ");
				strcpy(end_point, ptr);
				if (start_piece[1] == 'b') {
					PrintChar("It is the white turn. Please reenter\n");
					TFTLCD_write_turnflag(&white_turn);
					sleep(4);
					TFTLCD_write_turnflag(&chess_board);
					continue;
				}
			} while(start_piece[1] != 'w');

			SetPiece(start_piece); // Setting pointer of piece wanted to move

			sp[0] = start_point[0];
			sp[1] = (int)start_point[1] - 48;

			ep[0] = end_point[0];
			ep[1] = (int)end_point[1] - 48;

			move(sp, ep, piece);

			for(i = 0; i < 4; i++)
			{
				TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, i*4, line2[i*4+3] + (line2[i*4+2]<<8) + (line2[i*4+1]<<16) + (line2[i*4]<<24));
				TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, i*4+16, line3[i*4+3] + (line3[i*4+2]<<8) + (line3[i*4+1]<<16) + (line3[i*4]<<24));
			}
			turn_flag=3;
			flag = 1;
			// Jump to function that process interrupt(push button)
			Status = GicConfigure(INTC_DEVICE_ID);
			if (Status != XST_SUCCESS) {
				xil_printf("GIC Configure Failed\r\n");
				return XST_FAILURE;
			}
		}
		else if(turn_flag==0)
		{
			TIMER_SEVEN_SEG_mWriteReg(XPAR_TIMER_SEVEN_SEG_0_S00_AXI_BASEADDR, 4, 2);
			TIMER_SEVEN_SEG_mWriteReg(XPAR_TIMER_SEVEN_SEG_0_S00_AXI_BASEADDR, 4, 0);
			for(i = 0; i < 4; i++)
			{
				TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, i*4, line1[i*4+3] + (line1[i*4+2]<<8) + (line1[i*4+1]<<16) + (line1[i*4]<<24));
				TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, i*4+16, lineB[i*4+3] + (lineB[i*4+2]<<8) + (lineB[i*4+1]<<16) + (lineB[i*4]<<24));
			}
			do {
				PrintChar("Enter the piece and start, end formation(format : pb A7 A6) : ");
				GetPieceFormation(piece_formation);

				ptr = strtok(piece_formation, " ");
				strcpy(start_piece, ptr);

				ptr = strtok(NULL, " ");
				strcpy(start_point, ptr);

				ptr = strtok(NULL, " ");
				strcpy(end_point, ptr);
				if (start_piece[1] == 'w') {
					PrintChar("It is the black turn. Please reenter\n");
					TFTLCD_write_turnflag(&black_turn);
					sleep(4);
					TFTLCD_write_turnflag(&chess_board);
					continue;
				}
			} while(start_piece[1] != 'b');

			SetPiece(start_piece); // Setting pointer of piece wanted to move

			sp[0] = start_point[0];
			sp[1] = (int)start_point[1] - 48;

			ep[0] = end_point[0];
			ep[1] = (int)end_point[1] - 48;

			move(sp, ep, piece);
			for(i = 0; i < 4; i++)
			{
				TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, i*4, line2[i*4+3] + (line2[i*4+2]<<8) + (line2[i*4+1]<<16) + (line2[i*4]<<24));
				TEXTLCD_mWriteReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, i*4+16, line3[i*4+3] + (line3[i*4+2]<<8) + (line3[i*4+1]<<16) + (line3[i*4]<<24));
			}
			turn_flag=3;
			flag = 0;
			// Jump to function that process interrupt(push button)
			Status = GicConfigure(INTC_DEVICE_ID);
			if (Status != XST_SUCCESS) {
				xil_printf("GIC Configure Failed\r\n");
				return XST_FAILURE;
			}
		}

	}
}
```
user define function은 repo의 main.c 파일 및 문서를 통해 확인하시면 됩니다.
