/****************************************************************************
File    : KeysetGKDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CKeysetGKDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 19.10.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "KeysetGKDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// Static objects

static char s_szLabels[ NUM_KBJOY_KEYS ][ LOADSTRING_SIZE_S + 1 ];

static char *s_apszGetkeyTexts[ NUM_KBJOY_KEYS ] =
{
	_LoadStringSx( IDS_KEYSET_NW,     s_szLabels[ 0 ] ),
	_LoadStringSx( IDS_KEYSET_N,      s_szLabels[ 1 ] ),
	_LoadStringSx( IDS_KEYSET_NE,     s_szLabels[ 2 ] ),
	_LoadStringSx( IDS_KEYSET_E,      s_szLabels[ 3 ] ),
	_LoadStringSx( IDS_KEYSET_SE,     s_szLabels[ 4 ] ),
	_LoadStringSx( IDS_KEYSET_S,      s_szLabels[ 5 ] ),
	_LoadStringSx( IDS_KEYSET_SW,     s_szLabels[ 6 ] ),
	_LoadStringSx( IDS_KEYSET_W,      s_szLabels[ 7 ] ),
	_LoadStringSx( IDS_KEYSET_CENTRE, s_szLabels[ 8 ] ),
	_LoadStringSx( IDS_KEYSET_FIRE,   s_szLabels[ 9 ] )
};

#ifndef VK_OEM_1

#define VK_OEM_1          0xBA  // ';:' for US
#define VK_OEM_PLUS       0xBB  // '+'  any country
#define VK_OEM_COMMA      0xBC  // ','  any country
#define VK_OEM_MINUS      0xBD  // '-'  any country
#define VK_OEM_PERIOD     0xBE  // '.'  any country
#define VK_OEM_2          0xBF  // '/?' for US
#define VK_OEM_3          0xC0  // '`~' for US
#define VK_OEM_4          0xDB  // '[{' for US
#define VK_OEM_5          0xDC  // '\|' for US
#define VK_OEM_6          0xDD  // ']}' for US
#define VK_OEM_7          0xDE  // ''"' for US

#endif

BYTE s_anRegKeys[ 256 ] = /* Regular keys table */
{
/*000*/ 0, 0, 0, 0, 0, 0, 0, 0, VK_BACK, VK_TAB, 0, 0, 0, VK_RETURN, 0, 0,
/*016*/ VK_SHIFT, VK_CONTROL, VK_MENU, VK_PAUSE, VK_CAPITAL, 0, 0, 0, 0, 0, 0, VK_ESCAPE, 0, 0, 0, 0,
/*032*/ VK_SPACE, 0, 0, 0, 0, 0, 0, 0, 0, 0, VK_PRINT, 0, 0, 0, 0, VK_HELP,
/*048*/ 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 0, 0, 0, 0, 0, 0,		// 0 1 2 3 4 5 6 7 8 9
/*064*/ 0, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,	// A B C D E F G H I J K L M N O
/*080*/ 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 0, 0, 0, 0, 0,		// P Q R S T U V W X Y Z
/*096*/ VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9, VK_MULTIPLY, VK_ADD, 0, VK_SUBTRACT, VK_DECIMAL, 0,
/*112*/ VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12, VK_F13, VK_F14, VK_F15, VK_F16,
/*128*/ VK_F17, VK_F18, VK_F19, VK_F20, VK_F21, VK_F22, VK_F23, VK_F24, 0, 0, 0, 0, 0, 0, 0, 0,
/*144*/ 0, VK_SCROLL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*160*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*176*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VK_OEM_1, VK_OEM_PLUS, VK_OEM_COMMA, VK_OEM_MINUS, VK_OEM_PERIOD, VK_OEM_2,	// ; = , - . /
/*192*/ VK_OEM_3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,						// `
/*208*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VK_OEM_4, VK_OEM_5, VK_OEM_6, VK_OEM_7, 0,	// [ \ ] '
/*224*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*240*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/////////////////////////////////////////////////////////////////////////////
// CKeysetGKDlg dialog

BEGIN_MESSAGE_MAP(CKeysetGKDlg, CDialog)
	//{{AFX_MSG_MAP(CKeysetGKDlg)
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*========================================================
Method   : CKeysetGKDlg::CKeysetGKDlg
=========================================================*/
/* #FN#
   Standard constructor */
CKeysetGKDlg::
CKeysetGKDlg(
	WORD *pwKeyset,
	int   nKeyIndex,
	CWnd *pParent /*=NULL*/
)
	: CDialog( CKeysetGKDlg::IDD, pParent ),
	  m_pwKeyset( pwKeyset ),
	  m_nKeyIndex( nKeyIndex )
{
	//{{AFX_DATA_INIT(CKeysetGKDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

} /* #OF# CKeysetGKDlg::CKeysetGKDlg */

/*========================================================
Method   : CKeysetGKDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CKeysetGKDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CKeysetGKDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CKeysetGKDlg::~CKeysetGKDlg */


/////////////////////////////////////////////////////////////////////////////
// CKeysetGKDlg message handlers

/*========================================================
Method   : CKeysetGKDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CKeysetGKDlg::
OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strLabel;
	strLabel.LoadString( IDS_KEYSET_PRESS_KEY );

	SetWindowText( strLabel + s_apszGetkeyTexts[ m_nKeyIndex ] );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CKeysetGKDlg::OnInitDialog */

/*========================================================
Method   : CKeysetGKDlg::OnKeyDown
=========================================================*/
/* #FN#
   The framework calls this member function when a nonsystem key is pressed */
void
/* #AS#
   Nothing */
CKeysetGKDlg::
OnKeyDown(
	UINT nChar,   /* #IN# Specifies the virtual-key code of the given key */
	UINT nRepCnt, /* #IN# Repeat count */
	UINT nFlags   /* #IN# Specifies the scan code, key-transition code, previous key state, and context code */
)
{
	int i;
	/* The condition is true if the ALT key is down while another key is released */
	if( !_IsFlagSet( nFlags, 0x2000 ) )
	{
		BOOL bUsedKey = FALSE;

		/* Translate extended or filtrate regular codes */
		nChar = _IsFlagSet( nFlags, 0x100 ) ? g_Input.Key.anExtKeys[ nChar ] : s_anRegKeys[ nChar ];

		if( VK_SHIFT == nChar && _IsFlagSet( nFlags, 0x10 ) )
			nChar = VK_RSHIFT;

		if( VK_ESCAPE == nChar || VK_SCROLL == nChar )
			nChar = 0;

		for( i = 0; i < NUM_KBJOY_KEYS; i++ )
			if( nChar && m_nKeyIndex != i &&
				nChar == m_pwKeyset[ i ] )
			{
				bUsedKey = TRUE;
				break;
			}

		if( bUsedKey )
		{
			DisplayMessage( GetSafeHwnd(), IDS_WARN_KEY_CONFLICT, 0, MB_ICONWARNING | MB_OK, s_apszGetkeyTexts[ i ] );
		}
		else
			m_pwKeyset[ m_nKeyIndex ] = nChar;

		EndDialog( IDOK );
	}
} /* #OF# CKeysetGKDlg::OnKeyDown */

/*========================================================
Method   : CKeysetGKDlg::OnGetDlgCode
=========================================================*/
/* #FN#
   Normally, Windows handles all arrow-key and TAB-key input to
   a CWnd control. By overriding OnGetDlgCode, a CWnd control
   can choose a particular type of input to process itself */
UINT
/* #AS#
   A value indicating which type of input the application processes */
CKeysetGKDlg::
OnGetDlgCode()
{
	return DLGC_WANTALLKEYS;

} /* #OF# CKeysetGKDlg::OnGetDlgCode */
