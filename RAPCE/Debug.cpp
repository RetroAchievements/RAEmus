/******************************************************************************
Ootake

 [Debug.c]
    Function to debug software.
	デバッグ用ウィンドウの表示。ディスアセンブルなどを行う。

  -- How to use --
  [D]button -> DisAssembling of one instruction & Paused.
  [P]button -> Pause is released.
  - Possible to operate it by the Keyboard.

  * This Debugger is incomplete. it is developing.

Copyright(C)2006-2010 Kitao Nakamura.
    Attach the source code when you open the remodeling version and the
    succession version to the public. and, please contact me by E-mail.
    Business use is prohibited.
	Additionally, it applies to "GNU General Public License". 
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Debug.h"
#include "resource.h"
#include "MainBoard.h"
#include "WinMain.h"
#include "App.h"
#include "AudioOut.h"
#include "CDROM.h"

#define LINE_LEN	40

#define EDIT_MEMO			1
#define BUTTON_DISASSEMBLE	2
#define BUTTON_PAUSE		3
#define BUTTON_WRITE		4


static HBRUSH		_hMyb; //自作ブラシ色
static HFONT		_hFontB; //ボタン用フォント

static Uint32		_FontWidth;
static Uint32		_FontHeight;
static const char*	_pCaption = "Ootake Debugger";
static HINSTANCE	_hInstance = NULL;
static HWND 		_hWnd;

static HANDLE		_hThread = INVALID_HANDLE_VALUE;
static DWORD		_dwThreadID;

#define MEMO_LENGTH	65536
static char			_Memo[MEMO_LENGTH] = ""; //メモ表示用バッファ

static BOOL			_bPauseLong;
static BOOL			_bPause;


//フォントの高さを取得する
static Uint32
get_font_height(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	TEXTMETRIC		tm;

	hDC 	 = GetDC(hWnd);
	hFont	 = (HFONT)GetStockObject(OEM_FIXED_FONT);
	hFontOld = (HFONT)SelectObject(hDC, hFont);

	GetTextMetrics(hDC, &tm);

	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);

	return (Uint32)(tm.tmHeight);
}

//フォントの横幅を取得する
static Uint32
get_font_width(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	TEXTMETRIC		tm;

	hDC 	 = GetDC(hWnd);
	hFont	 = (HFONT)GetStockObject(OEM_FIXED_FONT);
	hFontOld = (HFONT)SelectObject(hDC, hFont);

	GetTextMetrics(hDC, &tm);

	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);

	return (Uint32)tm.tmAveCharWidth;
}


static void
set_window_size(
	HWND			hWnd)
{
	RECT		rc;
	Uint32		wndW = _FontWidth  * LINE_LEN;

	SetRect(&rc, 0, 0, wndW, 16);
	AdjustWindowRectEx(&rc, GetWindowLong(hWnd, GWL_STYLE),
						GetMenu(hWnd) != NULL, GetWindowLong(hWnd, GWL_EXSTYLE));
	wndW = rc.right - rc.left;
	GetWindowRect(WINMAIN_GetHwnd(), &rc);
	if ((int)(rc.right + wndW) > GetSystemMetrics(SM_CXSCREEN))
		rc.right = rc.left - wndW;
	if (rc.right<0) rc.right=0;
	MoveWindow(hWnd, rc.right, rc.top, wndW, rc.bottom - rc.top, TRUE);
}


static void
update_window(
	HWND			hWnd)
{
	HDC 			hDC;
	PAINTSTRUCT 	ps;
	RECT			rect = {0, 0, _FontWidth*LINE_LEN, _FontHeight};

	//描画準備
	hDC = BeginPaint(hWnd, &ps);

	//描画
	FillRect(hDC, &rect, (HBRUSH)GetStockObject(LTGRAY_BRUSH));

	//終了処理
	EndPaint(hWnd, &ps);
	ReleaseDC(hWnd, hDC);
}


static void
add_memo(
	char*	pString)
{
	HWND	hWnd = GetDlgItem(_hWnd,EDIT_MEMO);

	if (strlen(_Memo) >= MEMO_LENGTH-256) //バッファが一杯になった場合、前半部分は削除。
		strcpy(_Memo, _Memo+(MEMO_LENGTH/2));
	if (*_Memo != 0) //開始行以外なら
		strcat(_Memo, "\r\n"); //最下段までみっちり表示できる世に、改行コードは行の追加直前で付ける。
	strcat(_Memo, pString);
	
	//表示を更新
	SetWindowText(hWnd, _Memo);
	//（表示位置を最下段に）
	SendMessage(hWnd, EM_SETSEL, 65535, 65535);
	SendMessage(hWnd, EM_SCROLLCARET, 0, 0);
}


#define ADD_next0	strcat(s, "        ")
#define ADD_next1	sprintf(t, "%02X      ", next1); strcat(s, t)
#define ADD_next12	sprintf(t, "%02X %02X   ", next1,next2); strcat(s, t)
#define ADD_next123	sprintf(t, "%02X %02X %02X ", next1,next2,next3); strcat(s, t)
#define ADD_next123456	sprintf(t, "%02X %02X %02X %02X %02X %02X ", next1,next2,next3,next4,next5,next6); strcat(s, t)
static void
disassemble()
{
	char	s[256];
	char	t[256];
	char	res[256];
	Uint16	pc;
	Uint8	opcode;
	Uint8	flags;
	Uint8	CF;
	Uint8	ZF;
	Uint8	IF;
	Uint8	DF;
	Uint8	BF;
	Uint8	TF;
	Uint8	VF;
	Uint8	NF;
	Uint8	next1, next2, next3, next4, next5, next6;
	Uint16	addr;

	pc = CPU_GetPrevPC();
	sprintf(s, "%04X: ", pc);

	opcode = (Uint8)CPU_GetOpCode();
	sprintf(t, "%02X ", opcode);
	strcat(s, t);

	flags = CPU_GetPrevFlags();
	CF = flags & CPU_CF;
	ZF = ~(flags >> 1) & 1;
	IF = flags & CPU_IF;
	DF = flags & CPU_DF;
	BF = flags & CPU_BF;
	TF = flags & CPU_TF;
	VF = flags;
	NF = flags;

	next1 = CPU_ReadCode(pc+1);
	next2 = CPU_ReadCode(pc+2);
	next3 = CPU_ReadCode(pc+3);
	next4 = CPU_ReadCode(pc+4);
	next5 = CPU_ReadCode(pc+5);
	next6 = CPU_ReadCode(pc+6);

	strcpy(res, "");

	switch (opcode)
	{
		//-------------------------------------------------------------------
		// ALU instructions
		//-------------------------------------------------------------------

		//---- ADC ----------------------------------------------------------
		case CPU_INST_ADC_IMM:
			ADD_next1;
			if (TF)
				sprintf(res, "ADC #$%02X[T]", next1);
			else
				sprintf(res, "ADC #$%02X", next1);
			break;

		case CPU_INST_ADC_ZP:
			ADD_next1;
			if (TF)
				sprintf(res, "ADC $%02X[T]", next1);
			else
				sprintf(res, "ADC $%02X", next1);
			break;

		case CPU_INST_ADC_ZP_X:
			ADD_next1;
			if (TF)
				sprintf(res, "ADC $%02X,X[T]", next1);
			else
				sprintf(res, "ADC $%02X,X", next1);
			break;

		case CPU_INST_ADC_IND:
			ADD_next1;
			if (TF)
				sprintf(res, "ADC ($%02X)[T]", next1);
			else
				sprintf(res, "ADC ($%02X)", next1);
			break;

		case CPU_INST_ADC_IND_X:
			ADD_next1;
			if (TF)
				sprintf(res, "ADC ($%02X,X)[T]", next1);
			else
				sprintf(res, "ADC ($%02X,X)", next1);
			break;

		case CPU_INST_ADC_IND_Y:
			ADD_next1;
			if (TF)
				sprintf(res, "ADC ($%02X),Y[T]", next1);
			else
				sprintf(res, "ADC ($%02X),Y", next1);
			break;

		case CPU_INST_ADC_ABS:
			ADD_next12;
			if (TF)
				sprintf(res, "ADC $%02X%02X[T]", next2,next1);
			else
				sprintf(res, "ADC $%02X%02X", next2,next1);
			break;

		case CPU_INST_ADC_ABS_X:
			ADD_next12;
			if (TF)
				sprintf(res, "ADC $%02X%02X,X[T]", next2,next1);
			else
				sprintf(res, "ADC $%02X%02X,X", next2,next1);
			break;

		case CPU_INST_ADC_ABS_Y:
			ADD_next12;
			if (TF)
				sprintf(res, "ADC $%02X%02X,Y[T]", next2,next1);
			else
				sprintf(res, "ADC $%02X%02X,Y", next2,next1);
			break;

		//---- SBC ----------------------------------------------------------
		case CPU_INST_SBC_IMM:
			ADD_next1;
			if (TF)
				sprintf(res, "SBC #$%02X[T]", next1); //SBCでTフラグが有効かどうか不明
			else
				sprintf(res, "SBC #$%02X", next1);
			break;

		case CPU_INST_SBC_ZP:
			ADD_next1;
			if (TF)
				sprintf(res, "SBC $%02X[T]", next1);
			else
				sprintf(res, "SBC $%02X", next1);
			break;

		case CPU_INST_SBC_ZP_X:
			ADD_next1;
			if (TF)
				sprintf(res, "SBC $%02X,X[T]", next1);
			else
				sprintf(res, "SBC $%02X,X", next1);
			break;

		case CPU_INST_SBC_IND:
			ADD_next1;
			if (TF)
				sprintf(res, "SBC ($%02X)[T]", next1);
			else
				sprintf(res, "SBC ($%02X)", next1);
			break;

		case CPU_INST_SBC_IND_X:
			ADD_next1;
			if (TF)
				sprintf(res, "SBC ($%02X,X)[T]", next1);
			else
				sprintf(res, "SBC ($%02X,X)", next1);
			break;

		case CPU_INST_SBC_IND_Y:
			ADD_next1;
			if (TF)
				sprintf(res, "SBC ($%02X),Y[T]", next1);
			else
				sprintf(res, "SBC ($%02X),Y", next1);
			break;

		case CPU_INST_SBC_ABS:
			ADD_next12;
			if (TF)
				sprintf(res, "SBC $%02X%02X[T]", next2,next1);
			else
				sprintf(res, "SBC $%02X%02X", next2,next1);
			break;

		case CPU_INST_SBC_ABS_X:
			ADD_next12;
			if (TF)
				sprintf(res, "SBC $%02X%02X,X[T]", next2,next1);
			else
				sprintf(res, "SBC $%02X%02X,X", next2,next1);
			break;

		case CPU_INST_SBC_ABS_Y:
			ADD_next12;
			if (TF)
				sprintf(res, "SBC $%02X%02X,Y[T]", next2,next1);
			else
				sprintf(res, "SBC $%02X%02X,Y", next2,next1);
			break;

		//---- AND ----------------------------------------------------------
		case CPU_INST_AND_IMM:
			ADD_next1;
			if (TF)
				sprintf(res, "AND #$%02X[T]", next1);
			else
				sprintf(res, "AND #$%02X", next1);
			break;

		case CPU_INST_AND_ZP:
			ADD_next1;
			if (TF)
				sprintf(res, "AND $%02X[T]", next1);
			else
				sprintf(res, "AND $%02X", next1);
			break;

		case CPU_INST_AND_ZP_X:
			ADD_next1;
			if (TF)
				sprintf(res, "AND $%02X,X[T]", next1);
			else
				sprintf(res, "AND $%02X,X", next1);
			break;

		case CPU_INST_AND_IND:
			ADD_next1;
			if (TF)
				sprintf(res, "AND ($%02X)[T]", next1);
			else
				sprintf(res, "AND ($%02X)", next1);
			break;

		case CPU_INST_AND_IND_X:
			ADD_next1;
			if (TF)
				sprintf(res, "AND ($%02X,X)[T]", next1);
			else
				sprintf(res, "AND ($%02X,X)", next1);
			break;

		case CPU_INST_AND_IND_Y:
			ADD_next1;
			if (TF)
				sprintf(res, "AND ($%02X),Y[T]", next1);
			else
				sprintf(res, "AND ($%02X),Y", next1);
			break;

		case CPU_INST_AND_ABS:
			ADD_next12;
			if (TF)
				sprintf(res, "AND $%02X%02X[T]", next2,next1);
			else
				sprintf(res, "AND $%02X%02X", next2,next1);
			break;

		case CPU_INST_AND_ABS_X:
			ADD_next12;
			if (TF)
				sprintf(res, "AND $%02X%02X,X[T]", next2,next1);
			else
				sprintf(res, "AND $%02X%02X,X", next2,next1);
			break;

		case CPU_INST_AND_ABS_Y:
			ADD_next12;
			if (TF)
				sprintf(res, "AND $%02X%02X,Y[T]", next2,next1);
			else
				sprintf(res, "AND $%02X%02X,Y", next2,next1);
			break;

		//---- EOR ----------------------------------------------------------
		case CPU_INST_EOR_IMM:
			ADD_next1;
			if (TF)
				sprintf(res, "EOR #$%02X[T]", next1);
			else
				sprintf(res, "EOR #$%02X", next1);
			break;

		case CPU_INST_EOR_ZP:
			ADD_next1;
			if (TF)
				sprintf(res, "EOR $%02X[T]", next1);
			else
				sprintf(res, "EOR $%02X", next1);
			break;

		case CPU_INST_EOR_ZP_X:
			ADD_next1;
			if (TF)
				sprintf(res, "EOR $%02X,X[T]", next1);
			else
				sprintf(res, "EOR $%02X,X", next1);
			break;

		case CPU_INST_EOR_IND:
			ADD_next1;
			if (TF)
				sprintf(res, "EOR ($%02X)[T]", next1);
			else
				sprintf(res, "EOR ($%02X)", next1);
			break;

		case CPU_INST_EOR_IND_X:
			ADD_next1;
			if (TF)
				sprintf(res, "EOR ($%02X,X)[T]", next1);
			else
				sprintf(res, "EOR ($%02X,X)", next1);
			break;

		case CPU_INST_EOR_IND_Y:
			ADD_next1;
			if (TF)
				sprintf(res, "EOR ($%02X),Y[T]", next1);
			else
				sprintf(res, "EOR ($%02X),Y", next1);
			break;

		case CPU_INST_EOR_ABS:
			ADD_next12;
			if (TF)
				sprintf(res, "EOR $%02X%02X[T]", next2,next1);
			else
				sprintf(res, "EOR $%02X%02X", next2,next1);
			break;

		case CPU_INST_EOR_ABS_X:
			ADD_next12;
			if (TF)
				sprintf(res, "EOR $%02X%02X,X[T]", next2,next1);
			else
				sprintf(res, "EOR $%02X%02X,X", next2,next1);
			break;

		case CPU_INST_EOR_ABS_Y:
			ADD_next12;
			if (TF)
				sprintf(res, "EOR $%02X%02X,Y[T]", next2,next1);
			else
				sprintf(res, "EOR $%02X%02X,Y", next2,next1);
			break;

		//---- ORA ----------------------------------------------------------
		case CPU_INST_ORA_IMM:
			ADD_next1;
			if (TF)
				sprintf(res, "ORA #$%02X[T]", next1);
			else
				sprintf(res, "ORA #$%02X", next1);
			break;

		case CPU_INST_ORA_ZP:
			ADD_next1;
			if (TF)
				sprintf(res, "ORA $%02X[T]", next1);
			else
				sprintf(res, "ORA $%02X", next1);
			break;

		case CPU_INST_ORA_ZP_X:
			ADD_next1;
			if (TF)
				sprintf(res, "ORA $%02X,X[T]", next1);
			else
				sprintf(res, "ORA $%02X,X", next1);
			break;

		case CPU_INST_ORA_IND:
			ADD_next1;
			if (TF)
				sprintf(res, "ORA ($%02X)[T]", next1);
			else
				sprintf(res, "ORA ($%02X)", next1);
			break;

		case CPU_INST_ORA_IND_X:
			ADD_next1;
			if (TF)
				sprintf(res, "ORA ($%02X,X)[T]", next1);
			else
				sprintf(res, "ORA ($%02X,X)", next1);
			break;

		case CPU_INST_ORA_IND_Y:
			ADD_next1;
			if (TF)
				sprintf(res, "ORA ($%02X),Y[T]", next1);
			else
				sprintf(res, "ORA ($%02X),Y", next1);
			break;

		case CPU_INST_ORA_ABS:
			ADD_next12;
			if (TF)
				sprintf(res, "ORA $%02X%02X[T]", next2,next1);
			else
				sprintf(res, "ORA $%02X%02X", next2,next1);
			break;

		case CPU_INST_ORA_ABS_X:
			ADD_next12;
			if (TF)
				sprintf(res, "ORA $%02X%02X,X[T]", next2,next1);
			else
				sprintf(res, "ORA $%02X%02X,X", next2,next1);
			break;

		case CPU_INST_ORA_ABS_Y:
			ADD_next12;
			if (TF)
				sprintf(res, "ORA $%02X%02X,Y[T]", next2,next1);
			else
				sprintf(res, "ORA $%02X%02X,Y", next2,next1);
			break;

		//---- ASL ----------------------------------------------------------
		case CPU_INST_ASL_ACCUM:
			ADD_next0;
			sprintf(res, "ASL A");
			break;

		case CPU_INST_ASL_ZP:
			ADD_next1;
			sprintf(res, "ASL $%02X", next1);
			break;

		case CPU_INST_ASL_ZP_X:
			ADD_next1;
			sprintf(res, "ASL $%02X,X", next1);
			break;

		case CPU_INST_ASL_ABS:
			ADD_next12;
			sprintf(res, "ASL $%02X%02X", next2,next1);
			break;

		case CPU_INST_ASL_ABS_X:
			ADD_next12;
			sprintf(res, "ASL $%02X%02X,X", next2,next1);
			break;

		//---- LSR ----------------------------------------------------------
		case CPU_INST_LSR_ACCUM:
			ADD_next0;
			sprintf(res, "LSR A");
			break;

		case CPU_INST_LSR_ZP:
			ADD_next1;
			sprintf(res, "LSR $%02X", next1);
			break;

		case CPU_INST_LSR_ZP_X:
			ADD_next1;
			sprintf(res, "LSR $%02X,X", next1);
			break;

		case CPU_INST_LSR_ABS:
			ADD_next12;
			sprintf(res, "LSR $%02X%02X", next2,next1);
			break;

		case CPU_INST_LSR_ABS_X:
			ADD_next12;
			sprintf(res, "LSR $%02X%02X,X", next2,next1);
			break;

		//---- CMP ----------------------------------------------------------
		case CPU_INST_CMP_IMM:
			ADD_next1;
			sprintf(res, "CMP #$%02X", next1);
			break;

		case CPU_INST_CMP_ZP:
			ADD_next1;
			sprintf(res, "CMP $%02X", next1);
			break;

		case CPU_INST_CMP_ZP_X:
			ADD_next1;
			sprintf(res, "CMP $%02X,X", next1);
			break;

		case CPU_INST_CMP_IND:
			ADD_next1;
			sprintf(res, "CMP ($%02X)", next1);
			break;

		case CPU_INST_CMP_IND_X:
			ADD_next1;
			sprintf(res, "CMP ($%02X,X)", next1);
			break;

		case CPU_INST_CMP_IND_Y:
			ADD_next1;
			sprintf(res, "CMP ($%02X),Y", next1);
			break;

		case CPU_INST_CMP_ABS:
			ADD_next12;
			sprintf(res, "CMP $%02X%02X", next2,next1);
			break;

		case CPU_INST_CMP_ABS_X:
			ADD_next12;
			sprintf(res, "CMP $%02X%02X,X", next2,next1);
			break;

		case CPU_INST_CMP_ABS_Y:
			ADD_next12;
			sprintf(res, "CMP $%02X%02X,Y", next2,next1);
			break;

		//---- CPX ----------------------------------------------------------
		case CPU_INST_CPX_IMM:
			ADD_next1;
			sprintf(res, "CPX #$%02X", next1);
			break;

		case CPU_INST_CPX_ZP:
			ADD_next1;
			sprintf(res, "CPX $%02X", next1);
			break;

		case CPU_INST_CPX_ABS:
			ADD_next12;
			sprintf(res, "CPX $%02X%02X", next2,next1);
			break;

		//---- CPY ----------------------------------------------------------
		case CPU_INST_CPY_IMM:
			ADD_next1;
			sprintf(res, "CPY #$%02X", next1);
			break;

		case CPU_INST_CPY_ZP:
			ADD_next1;
			sprintf(res, "CPY $%02X", next1);
			break;

		case CPU_INST_CPY_ABS:
			ADD_next12;
			sprintf(res, "CPY $%02X%02X", next2,next1);
			break;

		//---- DEC ----------------------------------------------------------
		case CPU_INST_DEC_ACCUM:
			ADD_next0;
			sprintf(res, "DEC A");
			break;

		case CPU_INST_DEC_ZP:
			ADD_next1;
			sprintf(res, "DEC $%02X", next1);
			break;

		case CPU_INST_DEC_ZP_X:
			ADD_next1;
			sprintf(res, "DEC $%02X,X", next1);
			break;

		case CPU_INST_DEC_ABS:
			ADD_next12;
			sprintf(res, "DEC $%02X%02X", next2,next1);
			break;

		case CPU_INST_DEC_ABS_X:
			ADD_next12;
			sprintf(res, "DEC $%02X%02X,X", next2,next1);
			break;

		//---- DEX ----------------------------------------------------------
		case CPU_INST_DEX:
			ADD_next0;
			sprintf(res, "DEX");
			break;

		//---- DEY ----------------------------------------------------------
		case CPU_INST_DEY:
			ADD_next0;
			sprintf(res, "DEY");
			break;

		//---- INC ----------------------------------------------------------
		case CPU_INST_INC_ACCUM:
			ADD_next0;
			sprintf(res, "INC A");
			break;

		case CPU_INST_INC_ZP:
			ADD_next1;
			sprintf(res, "INC $%02X", next1);
			break;

		case CPU_INST_INC_ZP_X:
			ADD_next1;
			sprintf(res, "INC $%02X,X", next1);
			break;

		case CPU_INST_INC_ABS:
			ADD_next12;
			sprintf(res, "INC $%02X%02X", next2,next1);
			break;

		case CPU_INST_INC_ABS_X:
			ADD_next12;
			sprintf(res, "INC $%02X%02X,X", next2,next1);
			break;

		//---- INX ----------------------------------------------------------
		case CPU_INST_INX:
			ADD_next0;
			sprintf(res, "INX");
			break;
					
		//---- INY ----------------------------------------------------------
		case CPU_INST_INY:
			ADD_next0;
			sprintf(res, "INY");
			break;
					
		//---- ROL ----------------------------------------------------------
		case CPU_INST_ROL_ACCUM:
			ADD_next0;
			sprintf(res, "ROL A");
			break;

		case CPU_INST_ROL_ZP:
			ADD_next1;
			sprintf(res, "ROL $%02X", next1);
			break;

		case CPU_INST_ROL_ZP_X:
			ADD_next1;
			sprintf(res, "ROL $%02X,X", next1);
			break;

		case CPU_INST_ROL_ABS:
			ADD_next12;
			sprintf(res, "ROL $%02X%02X", next2,next1);
			break;

		case CPU_INST_ROL_ABS_X:
			ADD_next12;
			sprintf(res, "ROL $%02X%02X,X", next2,next1);
			break;

		//---- ROR ----------------------------------------------------------
		case CPU_INST_ROR_ACCUM:
			ADD_next0;
			sprintf(res, "ROR A");
			break;

		case CPU_INST_ROR_ZP:
			ADD_next1;
			sprintf(res, "ROR $%02X", next1);
			break;

		case CPU_INST_ROR_ZP_X:
			ADD_next1;
			sprintf(res, "ROR $%02X,X", next1);
			break;

		case CPU_INST_ROR_ABS:
			ADD_next12;
			sprintf(res, "ROR $%02X%02X", next2,next1);
			break;

		case CPU_INST_ROR_ABS_X:
			ADD_next12;
			sprintf(res, "ROR $%02X%02X,X", next2,next1);
			break;

		//---- CLA ----------------------------------------------------------
		case CPU_INST_CLA:
			ADD_next0;
			sprintf(res, "CLA");
			break;

		//---- CLX ----------------------------------------------------------
		case CPU_INST_CLX:
			ADD_next0;
			sprintf(res, "CLX");
			break;

		//---- CLY ----------------------------------------------------------
		case CPU_INST_CLY:
			ADD_next0;
			sprintf(res, "CLY");
			break;

		//-------------------------------------------------------------------
		// flag instructions
		//-------------------------------------------------------------------
		//---- CLC ----------------------------------------------------------
		case CPU_INST_CLC:
			ADD_next0;
			sprintf(res, "CLC");
			break;

		//---- CLD ----------------------------------------------------------
		case CPU_INST_CLD:
			ADD_next0;
			sprintf(res, "CLD");
			break;

		//---- CLI ----------------------------------------------------------
		case CPU_INST_CLI:
			ADD_next0;
			sprintf(res, "CLI");
			break;

		//---- CLV ----------------------------------------------------------
		case CPU_INST_CLV:
			ADD_next0;
			sprintf(res, "CLV");
			break;

		//---- SEC ----------------------------------------------------------
		case CPU_INST_SEC:
			ADD_next0;
			sprintf(res, "SEC");
			break;

		//---- SED ----------------------------------------------------------
		case CPU_INST_SED:
			ADD_next0;
			sprintf(res, "SED");
			break;

		//---- SEI ----------------------------------------------------------
		case CPU_INST_SEI:
			ADD_next0;
			sprintf(res, "SEI");
			break;

		//---- SET ----------------------------------------------------------
		case CPU_INST_SET:
			ADD_next0;
			sprintf(res, "SET");
			break;

		//-------------------------------------------------------------------
		// data transfer instructions
		//-------------------------------------------------------------------
		//---- LDA ----------------------------------------------------------
		case CPU_INST_LDA_IMM:
			ADD_next1;
			sprintf(res, "LDA #$%02X", next1);
			break;

		case CPU_INST_LDA_ZP:
			ADD_next1;
			sprintf(res, "LDA $%02X", next1);
			break;

		case CPU_INST_LDA_ZP_X:
			ADD_next1;
			sprintf(res, "LDA $%02X,X", next1);
			break;

		case CPU_INST_LDA_IND:
			ADD_next1;
			sprintf(res, "LDA ($%02X)", next1);
			break;

		case CPU_INST_LDA_IND_X:
			ADD_next1;
			sprintf(res, "LDA ($%02X,X)", next1);
			break;

		case CPU_INST_LDA_IND_Y:
			ADD_next1;
			sprintf(res, "LDA ($%02X),Y", next1);
			break;

		case CPU_INST_LDA_ABS:
			ADD_next12;
			sprintf(res, "LDA $%02X%02X", next2,next1);
			break;

		case CPU_INST_LDA_ABS_X:
			ADD_next12;
			sprintf(res, "LDA $%02X%02X,X", next2,next1);
			break;

		case CPU_INST_LDA_ABS_Y:
			ADD_next12;
			sprintf(res, "LDA $%02X%02X,Y", next2,next1);
			break;

		//---- LDX ----------------------------------------------------------
		case CPU_INST_LDX_IMM:
			ADD_next1;
			sprintf(res, "LDX #$%02X", next1);
			break;

		case CPU_INST_LDX_ZP:
			ADD_next1;
			sprintf(res, "LDX $%02X", next1);
			break;

		case CPU_INST_LDX_ZP_Y:
			ADD_next1;
			sprintf(res, "LDX $%02X,Y", next1);
			break;

		case CPU_INST_LDX_ABS:
			ADD_next12;
			sprintf(res, "LDX $%02X%02X", next2,next1);
			break;

		case CPU_INST_LDX_ABS_Y:
			ADD_next12;
			sprintf(res, "LDX $%02X%02X,Y", next2,next1);
			break;

		//---- LDY ----------------------------------------------------------
		case CPU_INST_LDY_IMM:
			ADD_next1;
			sprintf(res, "LDY #$%02X", next1);
			break;

		case CPU_INST_LDY_ZP:
			ADD_next1;
			sprintf(res, "LDY $%02X", next1);
			break;

		case CPU_INST_LDY_ZP_X:
			ADD_next1;
			sprintf(res, "LDY $%02X,X", next1);
			break;

		case CPU_INST_LDY_ABS:
			ADD_next12;
			sprintf(res, "LDY $%02X%02X", next2,next1);
			break;

		case CPU_INST_LDY_ABS_X:
			ADD_next12;
			sprintf(res, "LDY $%02X%02X,X", next2,next1);
			break;

		//---- STA ----------------------------------------------------------
		case CPU_INST_STA_ZP:
			ADD_next1;
			sprintf(res, "STA $%02X", next1);
			break;

		case CPU_INST_STA_ZP_X:
			ADD_next1;
			sprintf(res, "STA $%02X,X", next1);
			break;

		case CPU_INST_STA_IND:
			ADD_next1;
			sprintf(res, "STA ($%02X)", next1);
			break;

		case CPU_INST_STA_IND_X:
			ADD_next1;
			sprintf(res, "STA ($%02X,X)", next1);
			break;

		case CPU_INST_STA_IND_Y:
			ADD_next1;
			sprintf(res, "STA ($%02X),Y", next1);
			break;

		case CPU_INST_STA_ABS:
			ADD_next12;
			sprintf(res, "STA $%02X%02X", next2,next1);
			break;

		case CPU_INST_STA_ABS_X:
			ADD_next12;
			sprintf(res, "STA $%02X%02X,X", next2,next1);
			break;

		case CPU_INST_STA_ABS_Y:
			ADD_next12;
			sprintf(res, "STA $%02X%02X,Y", next2,next1);
			break;

		//---- STX ----------------------------------------------------------
		case CPU_INST_STX_ZP:
			ADD_next1;
			sprintf(res, "STX $%02X", next1);
			break;

		case CPU_INST_STX_ZP_Y:
			ADD_next1;
			sprintf(res, "STX $%02X,Y", next1);
			break;

		case CPU_INST_STX_ABS:
			ADD_next12;
			sprintf(res, "STX $%02X%02X", next2,next1);
			break;

		//---- STY ----------------------------------------------------------
		case CPU_INST_STY_ZP:
			ADD_next1;
			sprintf(res, "STY $%02X", next1);
			break;

		case CPU_INST_STY_ZP_X:
			ADD_next1;
			sprintf(res, "STY $%02X,X", next1);
			break;

		case CPU_INST_STY_ABS:
			ADD_next12;
			sprintf(res, "STY $%02X%02X", next2,next1);
			break;

		//---- SAX ----------------------------------------------------------
		case CPU_INST_SAX:
			ADD_next1;
			sprintf(res, "SAX");
			break;

		//---- SAY ----------------------------------------------------------
		case CPU_INST_SAY:
			ADD_next1;
			sprintf(res, "SAY");
			break;

		//---- SXY ----------------------------------------------------------
		case CPU_INST_SXY:
			ADD_next1;
			sprintf(res, "SXY");
			break;

		//---- ST0 ----------------------------------------------------------
		case CPU_INST_ST0_IMM:
			ADD_next1;
			sprintf(res, "ST0 #$%02X", next1);
			break;

		//---- ST1 ----------------------------------------------------------
		case CPU_INST_ST1_IMM:
			ADD_next1;
			sprintf(res, "ST1 #$%02X", next1);
			break;

		//---- ST2 ----------------------------------------------------------
		case CPU_INST_ST2_IMM:
			ADD_next1;
			sprintf(res, "ST2 #$%02X", next1);
			break;

		//---- STZ ----------------------------------------------------------
		case CPU_INST_STZ_ZP:
			ADD_next1;
			sprintf(res, "STZ $%02X", next1);
			break;

		case CPU_INST_STZ_ZP_X:
			ADD_next1;
			sprintf(res, "STZ $%02X,X", next1);
			break;

		case CPU_INST_STZ_ABS:
			ADD_next12;
			sprintf(res, "STZ $%02X%02X", next2,next1);
			break;

		case CPU_INST_STZ_ABS_X:
			ADD_next12;
			sprintf(res, "STZ $%02X%02X,X", next2,next1);
			break;

		//---- TAI ----------------------------------------------------------
		case CPU_INST_TAI:
			ADD_next123456;
			sprintf(res, "TAI");
			break;

		//---- TDD ----------------------------------------------------------
		case CPU_INST_TDD:
			ADD_next123456;
			sprintf(res, "TDD");
			break;

		//---- TIA ----------------------------------------------------------
		case CPU_INST_TIA:
			ADD_next123456;
			sprintf(res, "TIA");
			break;

		//---- TII ----------------------------------------------------------
		case CPU_INST_TII:
			ADD_next123456;
			sprintf(res, "TII");
			break;

		//---- TIN ----------------------------------------------------------
		case CPU_INST_TIN:
			ADD_next123456;
			sprintf(res, "TIN");
			break;

		//---- TAMi ---------------------------------------------------------
		case CPU_INST_TAM:
			ADD_next1;
			sprintf(res, "TAM #$%02X,X", next1);
			break;

		//---- TMAi ---------------------------------------------------------
		case CPU_INST_TMA:
			ADD_next1;
			sprintf(res, "TMA #$%02X,X", next1);
			break;

		//---- TAX ----------------------------------------------------------
		case CPU_INST_TAX:
			ADD_next0;
			sprintf(res, "TAX");
			break;

		//---- TAY ----------------------------------------------------------
		case CPU_INST_TAY:
			ADD_next0;
			sprintf(res, "TAY");
			break;

		//---- TSX ----------------------------------------------------------
		case CPU_INST_TSX:
			ADD_next0;
			sprintf(res, "TSX");
			break;

		//---- TXA ----------------------------------------------------------
		case CPU_INST_TXA:
			ADD_next0;
			sprintf(res, "TXA");
			break;

		//---- TXS ----------------------------------------------------------
		case CPU_INST_TXS:
			ADD_next0;
			sprintf(res, "TXS");
			break;

		//---- TYA ----------------------------------------------------------
		case CPU_INST_TYA:
			ADD_next0;
			sprintf(res, "TYA");
			break;

	//-------------------------------------------------------------------
	// branch / jump instructions
	//-------------------------------------------------------------------
		//---- BBRi ---------------------------------------------------------
		case CPU_INST_BBR0_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBR0 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBR1_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBR1 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBR2_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBR2 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBR3_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBR3 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBR4_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBR4 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBR5_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBR5 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBR6_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBR6 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBR7_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBR7 $%02X,$%04X", next1,addr);
			break;

		//---- BBSi ---------------------------------------------------------
		case CPU_INST_BBS0_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBS0 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBS1_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBS1 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBS2_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBS2 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBS3_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBS3 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBS4_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBS4 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBS5_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBS5 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBS6_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBS6 $%02X,$%04X", next1,addr);
			break;

		case CPU_INST_BBS7_ZP_REL:
			ADD_next12;
			addr = pc + 3 + (Sint8)next2;
			sprintf(res, "BBS7 $%02X,$%04X", next1,addr);
			break;

		//---- BCC ----------------------------------------------------------
		case CPU_INST_BCC_REL:
			ADD_next1;
			addr = pc + 2 + (Sint8)next1;
			sprintf(res, "BCC $%04X", addr);
			break;

		//---- BNE ----------------------------------------------------------
		case CPU_INST_BNE_REL:
			ADD_next1;
			addr = pc + 2 + (Sint8)next1;
			sprintf(res, "BNE $%04X", addr);
			break;

		//---- BVC ----------------------------------------------------------
		case CPU_INST_BVC_REL:
			ADD_next1;
			addr = pc + 2 + (Sint8)next1;
			sprintf(res, "BEL $%04X", addr);
			break;

		//---- BPL ----------------------------------------------------------
		case CPU_INST_BPL_REL:
			ADD_next1;
			addr = pc + 2 + (Sint8)next1;
			sprintf(res, "BPL $%04X", addr);
			break;

		//---- BCS ----------------------------------------------------------
		case CPU_INST_BCS_REL:
			ADD_next1;
			addr = pc + 2 + (Sint8)next1;
			sprintf(res, "BCS $%04X", addr);
			break;

		//---- BEQ ----------------------------------------------------------
		case CPU_INST_BEQ_REL:
			ADD_next1;
			addr = pc + 2 + (Sint8)next1;
			sprintf(res, "BEQ $%04X", addr);
			break;

		//---- BVS ----------------------------------------------------------
		case CPU_INST_BVS_REL:
			ADD_next1;
			addr = pc + 2 + (Sint8)next1;
			sprintf(res, "BVS $%04X", addr);
			break;

		//---- BMI ----------------------------------------------------------
		case CPU_INST_BMI_REL:
			ADD_next1;
			addr = pc + 2 + (Sint8)next1;
			sprintf(res, "BMI $%04X", addr);
			break;

		//---- BRA ----------------------------------------------------------
		case CPU_INST_BRA_REL:
			ADD_next1;
			addr = pc + 2 + (Sint8)next1;
			sprintf(res, "BRA $%04X", addr);
			break;

		//---- JMP ----------------------------------------------------------
		case CPU_INST_JMP_ABS:
			ADD_next12;
			sprintf(res, "JMP $%02X%02X", next2,next1);
			break;

		case CPU_INST_JMP_INDIR:
			ADD_next12;
			sprintf(res, "JMP ($%02X%02X)", next2,next1);
			break;

		case CPU_INST_JMP_INDIRX:
			ADD_next12;
			sprintf(res, "JMP $%02X%02X,X", next2,next1);
			break;

		//-------------------------------------------------------------------
		// subroutine instructions
		//-------------------------------------------------------------------
		//---- BSR ----------------------------------------------------------
		case CPU_INST_BSR_REL:
			ADD_next1;
			addr = pc + 2 + (Sint8)next1;
			sprintf(res, "BSR $%04X", addr);
			break;

		//---- JSR ----------------------------------------------------------
		case CPU_INST_JSR_ABS:
			ADD_next12;
			sprintf(res, "JSR $%02X%02X,X", next2,next1);
			break;

		//---- PHA ----------------------------------------------------------
		case CPU_INST_PHA:
			ADD_next0;
			sprintf(res, "PHA");
			break;

		//---- PHP ----------------------------------------------------------
		case CPU_INST_PHP:
			ADD_next0;
			sprintf(res, "PHP");
			break;

		//---- PHX ----------------------------------------------------------
		case CPU_INST_PHX:
			ADD_next0;
			sprintf(res, "PHX");
			break;

		//---- PHY ----------------------------------------------------------
		case CPU_INST_PHY:
			ADD_next0;
			sprintf(res, "PHY");
			break;

		//---- PLA ----------------------------------------------------------
		case CPU_INST_PLA:
			ADD_next0;
			sprintf(res, "PLA");
			break;

		//---- PLP ----------------------------------------------------------
		// PLP 命令実行直後は：
		// - T フラグをリセットしない 
		case CPU_INST_PLP:
			ADD_next0;
			sprintf(res, "PLP");
			break;

		//---- PLX ----------------------------------------------------------
		case CPU_INST_PLX:
			ADD_next0;
			sprintf(res, "PLX");
			break;

		//---- PLY ----------------------------------------------------------
		case CPU_INST_PLY:
			ADD_next0;
			sprintf(res, "PLY");
			break;

		//---- RTI ----------------------------------------------------------
		// RTI 命令実行直後は
		// - T フラグをリセットしない 
		case CPU_INST_RTI:
			ADD_next0;
			sprintf(res, "RTI");
			break;

		//---- RTS ----------------------------------------------------------
		case CPU_INST_RTS:
			ADD_next0;
			sprintf(res, "RTS");
			break;

		//-------------------------------------------------------------------
		// test instructions
		//-------------------------------------------------------------------
		//---- BIT ----------------------------------------------------------
		case CPU_INST_BIT_IMM:
			ADD_next1;
			sprintf(res, "BIT #$%02X", next1);
			break;

		case CPU_INST_BIT_ZP:
			ADD_next1;
			sprintf(res, "BIT $%02X", next1);
			break;

		case CPU_INST_BIT_ZP_X:
			ADD_next1;
			sprintf(res, "BIT $%02X,X", next1);
			break;

		case CPU_INST_BIT_ABS:
			ADD_next12;
			sprintf(res, "BIT $%02X%02X", next2,next1);
			break;

		case CPU_INST_BIT_ABS_X:
			ADD_next12;
			sprintf(res, "BIT $%02X%02X,X", next2,next1);
			break;

		//---- TRB ----------------------------------------------------------
		case CPU_INST_TRB_ZP:
			ADD_next1;
			sprintf(res, "TRB $%02X", next1);
			break;

		case CPU_INST_TRB_ABS:
			ADD_next12;
			sprintf(res, "TRB $%02X%02X", next2,next1);
			break;

		//---- TSB ----------------------------------------------------------
		case CPU_INST_TSB_ZP:
			ADD_next1;
			sprintf(res, "TSB $%02X", next1);
			break;

		case CPU_INST_TSB_ABS:
			ADD_next12;
			sprintf(res, "TSB $%02X%02X", next2,next1);
			break;

		//---- TST ----------------------------------------------------------
		case CPU_INST_TST_IMM_ZP:
			ADD_next12;
			sprintf(res, "TST #$%02X,$%02X", next1,next2);
			break;

		case CPU_INST_TST_IMM_ZP_X:
			ADD_next12;
			sprintf(res, "TST #$%02X,$%02X,X", next1,next2);
			break;

		case CPU_INST_TST_IMM_ABS:
			ADD_next123;
			sprintf(res, "TST #$%02X,$%02X%02X", next1,next3,next2);
			break;

		case CPU_INST_TST_IMM_ABS_X:
			ADD_next123;
			sprintf(res, "TST #$%02X,$%02X%02X,X", next1,next3,next2);
			break;

		//-------------------------------------------------------------------
		// control instructions
		//-------------------------------------------------------------------
		//---- CSL ----------------------------------------------------------
		case CPU_INST_CSL:
			ADD_next0;
			sprintf(res, "CSL");
			break;

		//---- CSH ----------------------------------------------------------
		case CPU_INST_CSH:
			ADD_next0;
			sprintf(res, "CSH");
			break;

		//---- BRK ----------------------------------------------------------
		case CPU_INST_BRK:
			ADD_next0;
			sprintf(res, "BRK");
			break;

		//---- NOP ----------------------------------------------------------
		case CPU_INST_NOP:
			ADD_next0;
			sprintf(res, "NOP");
			break;

		//---- RMBi ---------------------------------------------------------
		case CPU_INST_RMB0_ZP:
			ADD_next1;
			sprintf(res, "RMB0 $%02X", next1);
			break;

		case CPU_INST_RMB1_ZP:
			ADD_next1;
			sprintf(res, "RMB1 $%02X", next1);
			break;

		case CPU_INST_RMB2_ZP:
			ADD_next1;
			sprintf(res, "RMB2 $%02X", next1);
			break;

		case CPU_INST_RMB3_ZP:
			ADD_next1;
			sprintf(res, "RMB3 $%02X", next1);
			break;

		case CPU_INST_RMB4_ZP:
			ADD_next1;
			sprintf(res, "RMB4 $%02X", next1);
			break;

		case CPU_INST_RMB5_ZP:
			ADD_next1;
			sprintf(res, "RMB5 $%02X", next1);
			break;

		case CPU_INST_RMB6_ZP:
			ADD_next1;
			sprintf(res, "RMB6 $%02X", next1);
			break;

		case CPU_INST_RMB7_ZP:
			ADD_next1;
			sprintf(res, "RMB7 $%02X", next1);
			break;

		//---- SMBi ---------------------------------------------------------
		case CPU_INST_SMB0_ZP:
			ADD_next1;
			sprintf(res, "SMB0 $%02X", next1);
			break;

		case CPU_INST_SMB1_ZP:
			ADD_next1;
			sprintf(res, "SMB1 $%02X", next1);
			break;

		case CPU_INST_SMB2_ZP:
			ADD_next1;
			sprintf(res, "SMB2 $%02X", next1);
			break;

		case CPU_INST_SMB3_ZP:
			ADD_next1;
			sprintf(res, "SMB3 $%02X", next1);
			break;

		case CPU_INST_SMB4_ZP:
			ADD_next1;
			sprintf(res, "SMB4 $%02X", next1);
			break;

		case CPU_INST_SMB5_ZP:
			ADD_next1;
			sprintf(res, "SMB5 $%02X", next1);
			break;

		case CPU_INST_SMB6_ZP:
			ADD_next1;
			sprintf(res, "SMB6 $%02X", next1);
			break;

		case CPU_INST_SMB7_ZP:
			ADD_next1;
			sprintf(res, "SMB7 $%02X", next1);
			break;

		default: //invalid instructions
			ADD_next0;
			sprintf(res, "*Invalid*");
			break;
	}

	strcat(s, res);
	add_memo(s);
}
#undef ADD_next0
#undef ADD_next1
#undef ADD_next12
#undef ADD_next123
#undef ADD_next123456


static void
click_disassemble()
{
//test
//while (1)
{
	_bPauseLong = TRUE;
	_bPause = FALSE; //※メインウィンドウが止まっているときにDEBUG_SetPause()を使うとフリーズする。
	AOUT_Play(FALSE);//音を止める
	disassemble();

//	if ((CPU_ReadCode(CPU_GetPrevPC()+0) != 0x93)&&
//		(CPU_ReadCode(CPU_GetPrevPC()+1) == 0x00)&&
//		(CPU_ReadCode(CPU_GetPrevPC()+2) == 0x18))
//		break;
}
}

static void
click_pauseRelease()
{
	_bPauseLong = FALSE;
	DEBUG_SetPause(FALSE);
	if (!MAINBOARD_GetPause())
		AOUT_Play(TRUE);//音を再開
	add_memo("-- pause released --");
}

static void
click_showCDPort()
{
	char	buf[256];

	sprintf(buf, "CD Port[0] = %X", CDROM_GetPort(0));
	add_memo(buf);
	sprintf(buf, "CD Port[1] = %X", CDROM_GetPort(1));
	add_memo(buf);
	sprintf(buf, "CD Port[2] = %X", CDROM_GetPort(2));
	add_memo(buf);
}

//開発中。テスト用
static void
click_writeValue()
{
	if (APP_ShowWriteMemoryForm(_hWnd, _hInstance))
		add_memo("-- test write done --");
}

//開発中。テスト用
static void
click_writeValue2()
{
	CDROM_SetPort(0,0xF8);
	//CPU_WriteMemoryZero(0x59,0xFF);//test
	add_memo("-- test write done --");
}

//開発中。テスト用
static void
click_writeValue3()
{
	CDROM_SetPort(0,CDROM_GetPort(0) & ~0x80);
	add_memo("-- test write done --");
}

//開発中。テスト用
static void
click_writeMemToFile()
{
	char	fileName[MAX_PATH+1];
	FILE*	fp;

return;
	//CPU_WriteMemoryMpr(0xF8, 0x005F, 0x01);

	strcpy(fileName, APP_GetAppPath());
	strcat(fileName, "save");
	CreateDirectory(fileName, NULL);//saveディレクトリがない場合作る
	strcat(fileName, "\\DebugTest.dat");

	fp = fopen(fileName, "wb");
	if (fp != NULL)
	{
		fwrite(MAINBOARD_GetpMainRam(), 0x8000, 1, fp);
		fclose(fp);
	}		
	add_memo("-- saved \"\\save\\DebugTest.dat\" --");
}

static LRESULT CALLBACK
debug_wnd_proc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		_hFontB = CreateFont(  0,						// 高さ。0 = デフォルト
		                       0,						// 幅。0なら高さに合った幅
    		                   0,						// 角度
        		               0,						// ベースラインとの角度
            		           FW_NORMAL,				// 太さ
                		       FALSE,					// イタリック
	                    	   FALSE,					// アンダーライン
		                       FALSE,					// 打ち消し線
    		                   0,						// 日本語を取り扱うときはSHIFTJIS_CHARSETにする。
        		               0,						// 出力精度
            		           0,						// クリッピング精度
                		       0,						// 出力品質
                    		   0,						// ピッチとファミリー
		                       ""						// 書体名
							); //英語のデフォルトフォントに設定
		_FontWidth	= get_font_width(hWnd);
		_FontHeight = get_font_height(hWnd);
		set_window_size(hWnd);
		break;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == GetDlgItem(_hWnd, EDIT_MEMO))
		{
			SetBkMode((HDC)wParam, OPAQUE); //背景を塗りつぶし
			SetTextColor((HDC)wParam, RGB(64,0,128)); //テキストの色
			SetBkColor((HDC)wParam, RGB(192,240,208)); //テキストが書かれている部分の背景色
			return (LRESULT)_hMyb; //テキストが書かれていない部分の背景色 ※ここでGetStockObject()を使うリークが起こる
		}
		break;

	case WM_PAINT:
		update_window(hWnd);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case BUTTON_DISASSEMBLE: //１命令ディスアセンブル＆ポーズ
				SetFocus(_hWnd);
				click_disassemble();
				break;
			case BUTTON_PAUSE: //ポーズ解除
				SetFocus(_hWnd);
				click_pauseRelease();
				break;
			case BUTTON_WRITE: //ライトメモリ
				SetFocus(_hWnd);
				click_writeValue();
				break;
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		if (wParam == 'D') //「D」キー
			click_disassemble();
		if (wParam == 'P') //「P」キー
			click_pauseRelease();
		if (wParam == 'C') //「C」キー
			click_showCDPort();
		if (wParam == 'W') //「W」キー
			click_writeValue();
		if (wParam == 'Q') //「Q」キー
			click_writeValue2();
		if (wParam == 'E') //「E」キー
			click_writeValue3();
		if (wParam == 'F') //「F」キー
			click_writeMemToFile();
		break;

	case WM_CLOSE:
		DEBUG_Deinit();
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static DWORD WINAPI
debug_thread(
	LPVOID	param)
{
	WNDCLASS	wc;
	HWND		hWnd;
	MSG 		msg;
	Uint32		x;
	HWND		hWndTmp;
	RECT		rc;

	ZeroMemory(&wc, sizeof(wc));
	wc.style		 = 0;
	wc.lpfnWndProc	 = debug_wnd_proc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = _hInstance;
	wc.hIcon		 = LoadIcon(_hInstance, MAKEINTRESOURCE(OOTAKEICON)); //アイコンを読み込み。v2.00更新
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName  = "";
	wc.lpszClassName = _pCaption;

	if (RegisterClass(&wc) == 0)
		return 0;

	hWnd = CreateWindow(
		_pCaption,
		_pCaption,
		WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION /*| WS_SIZEBOX*/,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		NULL,
		NULL,
		_hInstance,
		NULL
	);

	if (hWnd == NULL)
		return 0;

	_hWnd	   = hWnd;

	//DisAssembleボタンを作成
	x = 0;
	hWndTmp = CreateWindow(
		"BUTTON", "D",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, 0, _FontWidth*5, _FontHeight+2,
		_hWnd, (HMENU)BUTTON_DISASSEMBLE, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//文字のフォントを設定

	//Pause解除ボタンを作成
	x += _FontWidth*5;
	hWndTmp = CreateWindow(
		"BUTTON", "P",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, 0, _FontWidth*5, _FontHeight+2,
		_hWnd, (HMENU)BUTTON_PAUSE, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//文字のフォントを設定

	//ライトメモリボタンを作成
	x += _FontWidth*5;
	hWndTmp = CreateWindow(
		"BUTTON", "W",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, 0, _FontWidth*5, _FontHeight+2,
		_hWnd, (HMENU)BUTTON_WRITE, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//文字のフォントを設定

	//EDITメモを作成
	GetWindowRect(WINMAIN_GetHwnd(), &rc);
	hWndTmp = CreateWindow(
		"EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL,
		_FontWidth/2, _FontHeight+_FontHeight/4, _FontWidth*(LINE_LEN-1), rc.bottom-rc.top-(_FontHeight*3+_FontHeight/4),
		_hWnd, (HMENU)EDIT_MEMO, _hInstance, NULL
	);
	_hMyb = CreateSolidBrush(RGB(192,240,208)); //背景色用のブラシを作る
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)GetStockObject(OEM_FIXED_FONT), MAKELPARAM(TRUE, 0));//文字のフォントを設定
	strcpy(_Memo, "");

	ShowWindow(_hWnd, SW_SHOWNORMAL);
	UpdateWindow(_hWnd);
	GetWindowRect(_hWnd, &rc);
	SetWindowPos(_hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED); //手前に表示
	ImmAssociateContext(_hWnd, 0); //Kitao追加。IMEを無効にする。v0.79

	// メッセージループ 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}


BOOL
DEBUG_Init(
	HINSTANCE	hInstance)
{
	if (_hInstance != NULL)
		DEBUG_Deinit();

	_hInstance = hInstance;

	// スレッドを作成して実行する	
	_hThread = CreateThread(NULL, 0, debug_thread, NULL, 0, &_dwThreadID);
	if (_hThread == NULL)
		return FALSE;

	_bPauseLong = FALSE;
	_bPause = FALSE;
	CPU_SetDebug(TRUE);

	return TRUE;
}


void
DEBUG_Deinit()
{
	if (_hInstance != NULL)
	{
		//スレッド内でWindowを作ったので、スレッド終了の前にWindowを破棄する必要がある。
		DestroyWindow(_hWnd);
		_hWnd = NULL;
		UnregisterClass(_pCaption, _hInstance);
		_hInstance = NULL;
		DeleteObject(_hFontB); //ボタン用フォントを開放
		DeleteObject(_hMyb); //ブラシを開放

		//メインウィンドウにフォーカスを戻し前面に。
		EnableWindow(WINMAIN_GetHwnd(), TRUE);
		APP_SetForegroundWindowOotake(); //確実にアクティブにする

		//メインスレッドを再開。スレッド終了前にやる必要がある。
		CPU_SetDebug(FALSE);
		_bPauseLong = FALSE;
		DEBUG_SetPause(FALSE); //デバッグポーズ中だった場合、この瞬間にメインウィンドウは動き出す。

		PostThreadMessage(_dwThreadID, WM_QUIT, 0, 0);
		if (_hThread != INVALID_HANDLE_VALUE)
		{
			WaitForSingleObject(_hThread, INFINITE); //スレッドの終了を待つ
			CloseHandle(_hThread);
			_hThread = INVALID_HANDLE_VALUE;
		}
		_dwThreadID = 0;
	}
}


const HWND
DEBUG_GetHwnd()
{
	return _hWnd;
}


void
DEBUG_SetPause(
	BOOL	pause)
{
	HWND	hWnd = WINMAIN_GetHwnd();

	_bPause = pause;
	if (pause) //ポーズ中はメインウィンドウの操作が効かなくなるので、誤動作させないように閉じるボタンを隠しておく。
		SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_SYSMENU);
	else
	{
		if (!MAINBOARD_GetPause())
			AOUT_Play(TRUE);//音を再開
		SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | WS_SYSMENU);
	}
}

BOOL
DEBUG_GetPause()
{
	return _bPause;
}

BOOL
DEBUG_GetPauseLong()
{
	return _bPauseLong;
}

