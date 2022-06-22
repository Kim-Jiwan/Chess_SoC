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

void SD_read(char *filename)
{
	Res = f_mount(&fatfs, Path, 0);
    if(Res != FR_OK){
    	xil_printf("\nmount_fail\n");
    	return 0;
    }

    Res = f_open(&fil, filename, FA_READ);
    if(Res){
        xil_printf("\nfile_open_fail\n");
        return 0;
    }

    Res = f_lseek(&fil, 0);
    if (Res) {
    	xil_printf("\nfseek_fail\n");
    	return 0;
    }

    Res = f_read(&fil, buffer, data_size, &NumBytesRead);
    if (Res){
        xil_printf("\ndata_read_fail\n");
        return 0;
    }

    Res = f_close(&fil);

	xil_printf("\nfile_read_success\n");
}

void TFTLCD_write_background()
{
	int i, j;

	SD_read(&chess_board);

    for (int i = 0; i < 272; i++){ // loop for vertical
    	for (int j = 0; j < 480 / 2; j++){ // loop for horizontal
    		// 1
			// Below codes are representing oven number pixel
			Data = (int)buffer[j + 240*i] & 0x0000ffff;
			R = (Data >> 11) & 0x0000001f;
			G = Data & 0x000007E0;
			B = Data & 0x0000001f;
			Data = (B<<11)| G | R;
			Xil_Out32(XPAR_TFTLCD_0_S00_AXI_BASEADDR + (2*j + 480*i)*4, Data);

			// 2
			Data = (int)buffer[j + 240*i] >> 16;
			R = (Data >> 11) & 0x0000001f;
			G = Data & 0x000007E0;
			B = Data & 0x0000001f;
			Data = (B<<11)| G | R;
			Xil_Out32(XPAR_TFTLCD_0_S00_AXI_BASEADDR + (1 + 2*j + 480*i)*4, Data);
    	}
    }
}

void TFTLCD_write_turnflag(char *filename)
{
	int i, j;

	SD_read(filename);

    for (int i = 0; i < 272; i++){ // loop for vertical
    	for (int j = 0; j < 208 / 2; j++){ // loop for horizontal
    		// 1
			// Below codes are representing oven number pixel
			Data = (int)buffer[j + 240*i] & 0x0000ffff;
			R = (Data >> 11) & 0x0000001f;
			G = Data & 0x000007E0;
			B = Data & 0x0000001f;
			Data = (B<<11)| G | R;
			Xil_Out32(XPAR_TFTLCD_0_S00_AXI_BASEADDR + (2*j + 480*i)*4, Data);

			// 2
			Data = (int)buffer[j + 240*i] >> 16;
			R = (Data >> 11) & 0x0000001f;
			G = Data & 0x000007E0;
			B = Data & 0x0000001f;
			Data = (B<<11)| G | R;
			Xil_Out32(XPAR_TFTLCD_0_S00_AXI_BASEADDR + (1 + 2*j + 480*i)*4, Data);
    	}
    }
}

void TFTLCD_write_sector(char horizontal, int vertical)
{
	int i, j;

	horizontal = (int)'H' - (int)horizontal + 1;

	int horizontal_start 	= horizontal_formation[horizontal - 1];
	int horizontal_end 		= horizontal_formation[horizontal];
	int vertical_start 		= vertical_formation[vertical - 1];
	int vertical_end 		= vertical_formation[vertical];

    for (int i = vertical_start + 1; i < vertical_end + 1; i++){ // loop for vertical
    	for (int j = horizontal_start / 2 + 1; j < horizontal_end / 2 + 1; j++){ // loop for horizontal
    		// 1
			Data = (int)buffer[j + 240 * i] & 0x0000ffff;
			R = (Data >> 11) & 0x0000001f;
			G = Data & 0x000007E0;
			B = Data & 0x0000001f;
			Data = (B<<11)| G | R;
			Xil_Out32(XPAR_TFTLCD_0_S00_AXI_BASEADDR + (2*j + 480*i)*4, Data);

			// 2
			Data = (int)buffer[j + 240 * i] >> 16;
			R = (Data >> 11) & 0x0000001f;
			G = Data & 0x000007E0;
			B = Data & 0x0000001f;
			Data = (B<<11)| G | R;
			Xil_Out32(XPAR_TFTLCD_0_S00_AXI_BASEADDR + (1 + 2*j + 480*i)*4, Data);
    	}
    }
}

void move(int start[2], int end[2], char *piece_moved)
{
	SD_read(&chess_board);
	TFTLCD_write_sector(start[0], start[1]);

	SD_read(piece_moved);
	TFTLCD_write_sector(end[0], end[1]);
}

void chess_default()
{
	TFTLCD_write_background();

	SD_read(&rook_white);
	TFTLCD_write_sector('H', 1);
	TFTLCD_write_sector('A', 1);

	SD_read(&night_white);
	TFTLCD_write_sector('G', 1);
	TFTLCD_write_sector('B', 1);

	SD_read(&bishop_white);
	TFTLCD_write_sector('F', 1);
	TFTLCD_write_sector('C', 1);

	SD_read(&king_white);
	TFTLCD_write_sector('E', 1);

	SD_read(&queen_white);
	TFTLCD_write_sector('D', 1);

	SD_read(&pawn_white);
	for (char c = 'H'; c >= 'A'; c--)
		TFTLCD_write_sector(c, 2);

	SD_read(&pawn_black);
	for (char c = 'H'; c >= 'A'; c--)
	 	TFTLCD_write_sector(c, 7);

	SD_read(&rook_black);
	TFTLCD_write_sector('H', 8);
	TFTLCD_write_sector('A', 8);

	SD_read(&night_black);
	TFTLCD_write_sector('G', 8);
	TFTLCD_write_sector('B', 8);

	SD_read(&bishop_black);
	TFTLCD_write_sector('F', 8);
	TFTLCD_write_sector('C', 8);

	SD_read(&king_black);
	TFTLCD_write_sector('E', 8);

	SD_read(&queen_black);
	TFTLCD_write_sector('D', 8);
}

///////////////////////////////////////////////////////
void InitMsg(void)
{
	PrintChar("==Welcome to the Zynq based Chess game!!!==\n\n");
}

void PrintChar(u8 *str)
{
	XUartPs_SendByte(XPAR_PS7_UART_1_BASEADDR, CR);

	while(*str != 0)
	{
		XUartPs_SendByte(XPAR_PS7_UART_1_BASEADDR, *str++);
	}
}

void PrintMsg(u8 *str) // Doesn't need in TFT
{
	while(*str != 0)
	{
		XUartPs_SendByte(XPAR_PS7_UART_1_BASEADDR, *str++);
	}
}

void GetPieceFormation(u8 *piece_formation)
{
	int i = 0;
	char tmp;

	do
	{
		tmp = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);

		if( ((tmp >= '1') && (tmp <= '8')) || ((tmp >= 'A') && (tmp <= 'Z')) || ((tmp >= 'a') && (tmp <= 'z')) || tmp == ' ')
		{
			XUartPs_SendByte(XPAR_PS7_UART_1_BASEADDR, tmp);
			piece_formation[i] = tmp;
			i++;
		}
		else
		{
			continue;
		}
	}
	while( !((tmp == CR) || (i >= NAME_MAX-1)) );

	while(tmp != CR)
		tmp = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);
}

void SetPiece(u8 *start_piece)
{
	if 		(strcmp(start_piece, "rw") == 0)		piece = &rook_white;
	else if (strcmp(start_piece, "nw") == 0)		piece = &night_white;
	else if (strcmp(start_piece, "bw") == 0)		piece = &bishop_white;
	else if (strcmp(start_piece, "kw") == 0)		piece = &king_white;
	else if (strcmp(start_piece, "qw") == 0)		piece = &queen_white;
	else if (strcmp(start_piece, "pw") == 0)		piece = &pawn_white;

	else if (strcmp(start_piece, "rb") == 0)		piece = &rook_black;
	else if (strcmp(start_piece, "nb") == 0)		piece = &night_black;
	else if (strcmp(start_piece, "bb") == 0)		piece = &bishop_black;
	else if (strcmp(start_piece, "kb") == 0)		piece = &king_black;
	else if (strcmp(start_piece, "qb") == 0)		piece = &queen_black;
	else if (strcmp(start_piece, "pb") == 0)		piece = &pawn_black;
}

void GetCmd(u8 *sel)
{
	char tmp;

	//***************************************************
	//
	//Coding here !
	//
	//***************************************************

	while(tmp != CR)
		tmp = XUartPs_RecvByte(XPAR_PS7_UART_1_BASEADDR);
}

void InitValue(u8 *formation, u8 *piece)
{
	int i = 0;

	for (i = 0; i <= FORM_MAX -1; i++)
	{
		formation[i] = 0;
	}

	for (i = 0; i <= NAME_MAX -1; i++)
	{
		piece[i] = 0;
	}
}

int GicConfigure(u16 DeviceId)
{
	int Status;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	GicConfig = XScuGic_LookupConfig(DeviceId);
	if (NULL == GicConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, GicConfig,
					GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			&InterruptController);

	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler performs
	 * the specific interrupt processing for the device
	 */
	Status = XScuGic_Connect(&InterruptController, INTC_DEVICE_INT_ID,
			   (Xil_ExceptionHandler)ServiceRoutine,
			   (void *)&InterruptController);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the device and then cause (simulate) an
	 * interrupt so the handlers will be called
	 */
	XScuGic_Enable(&InterruptController, INTC_DEVICE_INT_ID);

	return XST_SUCCESS;
}

void ServiceRoutine(void *CallbackRef)
{
	char pb;

	pb = PUSHBUTTON_mReadReg(XPAR_PUSHBUTTON_0_S00_AXI_BASEADDR, 0);

	PUSHBUTTON_mWriteReg(XPAR_PUSHBUTTON_0_S00_AXI_BASEADDR, 0, 0);

	// Coding here!!!!

	if ((pb & 1) == 1){
		chess_default();
	}
	else if ((pb & 2) == 2){
		if ((turn_flag == 3)&&(flag == 1)){
			move(ep, sp, piece);
			turn_flag = 1; 		// Since "White turn(flag==1) and after move(turn_flag==3)", turn_flag = 1.
		}
		else if ((turn_flag == 3)&&(flag == 0)) {
			move(ep, sp, piece);
			turn_flag = 0;	// Since "Black turn(flag==0) and after move(turn_flag==3)", turn_flag = 0.
		}
	}
	else if ((pb & 4) == 4){
		if (flag == 1)	turn_flag = 0;		// White turn -> Black turn.
		else if (flag == 0) turn_flag = 1;	// Black turn -> White turn.
	}
	else if ((pb & 8) == 8){
		xil_printf("S4 Switch is pushed\r\n");
	}

	return XST_SUCCESS;
}

/*
void GetTextLine(char *ReadReg, unsigned int TextLine)
{
	int				nReg = 0;
	int				nOffset = 0;
	unsigned int	temp = 0;

	for (nReg = 0; nReg < 4; nReg++)
	{
		temp = TEXTLCD_mReadReg(XPAR_TEXTLCD_0_S00_AXI_BASEADDR, TextLine * 16 + nReg * 4);
		do
		{
			ReadReg[nReg * 4 + nOffset] = (temp >> (8 * (3 - nOffset))) & 0xff;

			nOffset++;
		} while (!(nOffset == 4));
		nOffset = 0;
	}

}

void ReadTLCDReg(void)
{
	char ReadReg[MAX] = { 0, };
	PrintChar("------------------------------------------------------------------ ");
	PrintChar("Address 0x43C9_0000 ~ 0x43C0_000F --- ");
	GetTextLine(ReadReg, TextLine1);
	PrintChar(ReadReg);

	PrintChar("Address 0x43C9_0010 ~ 0x43C0_001F --- ");
	GetTextLine(ReadReg, TextLine2);
	PrintChar(ReadReg);
	PrintChar("------------------------------------------------------------------ ");
}
*/
