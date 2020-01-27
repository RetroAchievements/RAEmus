/****************************************************************************
File    : KeysetDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CKeysetDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 18.10.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "KeysetGKDlg.h"
#include "KeysetDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_KEYSET_FIRST	IDC_KEYSET_NORTH
#define IDC_KEYSET_LAST		IDC_KEYSET_CANCEL


/////////////////////////////////////////////////////////////////////////////
// Static objects

/*
   VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
   Used only as parameters to GetAsyncKeyState() and GetKeyState().
   No other API or message will distinguish left and right keys in this way.

   Warning: '!' means this might be not (or isn't) 100% correct
*/

static char *s_apszKBCodeToString[ 256 ] =
{
//000:?,            VK_LBUTTON,    VK_RBUTTON,   VK_CANCEL,     VK_MBUTTON,   ?,             ?,            ?,            VK_BACK,       VK_TAB,       ?,            ?,            VK_CLEAR,      VK_RETURN,     ?,             ?,
     "None",       "",            "",           "",            "",           "",            "",           "",           "Back- space", "Tab",        "",           "",           "",            "Enter",       "",            "",
//016:!VK_SHIFT,    !VK_CONTROL,   VK_MENU,      VK_PAUSE,      VK_CAPITAL,   VK_KANA,       ?,            VK_JUNJA,     VK_FINAL,      VK_KANJI,     ?,            VK_ESCAPE,    VK_CONVERT,    VK_NONCONVERT, VK_ACCEPT,     VK_MODECHANGE,
	 "Left Shift", "Left Ctrl",   "Alt",        "Pause Break", "Caps Lock",  "",            "",           "",           "",            "",           "",           "Esc",        "",            "",            "",            "",
//032:VK_SPACE,     VK_PRIOR,      VK_NEXT,      VK_END,        VK_HOME,      VK_LEFT,       VK_UP,        VK_RIGHT,     VK_DOWN,       VK_SELECT,    VK_PRINT,     VK_EXECUTE,   VK_SNAPSHOT,   VK_INSERT,     VK_DELETE,     VK_HELP,
	 "Space",      "Page Up",     "Page Down",  "End",         "Home",       "Left",        "Up",         "Right",      "Down",        "",           "Print Scrn", "",           "",            "Insert",      "Delete",      "Help",
//048:VK_0,         VK_1,          VK_2,         VK_3,          VK_4,         VK_5,          VK_6,         VK_7,         VK_8,          VK_9,         ?,            ?,            ?,             ?,             ?,             ?,
	 "0",          "1",           "2",          "3",           "4",          "5",           "6",          "7",          "8",           "9",          "",           "",           "",            "",            "",            "",
//064:?,            VK_A,          VK_B,         VK_C,          VK_D,         VK_E,          VK_F,         VK_G,         VK_H,          VK_I,         VK_J,         VK_K,         VK_L,          VK_M,          VK_N,          VK_O,
	 "",           "A",           "B",          "C",           "D",          "E",           "F",          "G",          "H",           "I",          "J",          "K",          "L",           "M",           "N",           "O",
//080:VK_P,         VK_Q,          VK_R,         VK_S,          VK_T,         VK_U,          VK_V,         VK_W,         VK_X,          VK_Y,         VK_Z,         VK_LWIN,      VK_RWIN,       VK_APPS,       ?,             !,
	 "P",          "Q",           "R",          "S",           "T",          "U",           "V",          "W",          "X",           "Y",          "Z",          "Left Win95", "Right Win95", "",            "",            "Num Enter",
//096:VK_NUMPAD0,   VK_NUMPAD1,    VK_NUMPAD2,   VK_NUMPAD3,    VK_NUMPAD4,   VK_NUMPAD5,    VK_NUMPAD6,   VK_NUMPAD7,   VK_NUMPAD8,    VK_NUMPAD9,   VK_MULTIPLY,  VK_ADD,       VK_SEPARATOR,  VK_SUBTRACT,   VK_DECIMAL,    VK_DIVIDE,
	 "Num- pad 0", "Num- pad 1",  "Num- pad 2", "Num- pad 3",  "Num- pad 4", "Num- pad 5",  "Num- pad 6", "Num- pad 7", "Num- pad 8",  "Num- pad 9", "Num- pad *", "Num- pad +", "",            "Num- pad -",  "Num- pad .",  "Num- pad /",
//112:VK_F1,        VK_F2,         VK_F3,        VK_F4,         VK_F5,        VK_F6,         VK_F7,        VK_F8,        VK_F9,         VK_F10,       VK_F11,       VK_F12,       VK_F13,        VK_F14,        VK_F15,        VK_F16,
	 "F1",         "F2",          "F3",         "F4",          "F5",         "F6",          "F7",         "F8",         "F9",          "F10",        "F11",        "F12",        "F13",         "F14",         "F15",         "F16",
//128:VK_F17,       VK_F18,        VK_F19,       VK_F20,        VK_F21,       VK_F22,        VK_F23,       VK_F24,       ?,             ?,            ?,            ?,            ?,             ?,             ?,             ?,
	 "F17",        "F18",         "F19",        "F20",         "F21",        "F22",         "F23",        "F24",        "",            "",           "",           "",           "",            "",            "",            "",
//144:VK_NUMLOCK,   VK_SCROLL,     ?,            ?,             ?,            ?,             ?,            ?,            ?,             ?,            ?,            ?,            ?,             ?,             ?,             ?,
	 "Num Lock",   "Scroll Lock", "",           "",            "",           "",            "",           "",           "",            "",           "",           "",           "",            "",            "",            "",
//160:VK_LSHIFT,    VK_RSHIFT,     VK_LCONTROL,  VK_RCONTROL,   VK_LMENU,     VK_RMENU,      ?,            ?,            ?,             ?,            ?,            ?,            ?,             ?,             ?,             ?,
	 "",           "Right Shift", "",           "Right Ctrl",  "",           "",            "",           "",           "",            "",           "",           "",           "",            "",            "",            "",
//176:?,            ?,             ?,            ?,             ?,            ?,             ?,            ?,            ?,             ?,            VK_OEM_1,     VK_OEM_PLUS,  VK_OEM_COMMA,  VK_OEM_MINUS,  VK_OEM_PERIOD, VK_OEM_2,
	 "",           "",            "",           "",            "",           "",            "",           "",           "",            "",           ";",          "=",          ",",           "-",           ".",           "/",
//192:VK_OEM_3,     ?,             ?,            ?,             ?,            ?,             ?,            ?,            ?,             ?,            ?,            ?,            ?,             ?,             ?,             ?,
	 "`",          "",            "",           "",            "",           "",            "",           "",           "",            "",           "",           "",           "",            "",            "",            "",
//208:?,            ?,             ?,            ?,             ?,            ?,             ?,            ?,            ?,             ?,            ?,            VK_OEM_4,     VK_OEM_5,      VK_OEM_6,      VK_OEM_7,      ?,
	 "",           "",            "",           "",            "",           "",            "",           "",           "",            "",           "",           "[",          "\\",          "]",           "'",           "",
//224:?,            ?,             ?,            ?,             ?,            VK_PROCESSKEY, ?,            ?,            ?,             ?,            ?,            ?,            ?,             ?,             ?,             ?,
	 "",           "",            "",           "",            "",           "",            "",           "",           "",            "",           "",           "",           "",            "",            "",            "",
//240:?,            ?,             ?,            ?,             ?,            ?,             VK_ATTN,      VK_CRSEL,     VK_EXSEL,      VK_EREOF,     VK_PLAY,      VK_ZOOM,      VK_NONAME,     VK_PA1,        VK_OEM_CLEAR,  ?
	 "",           "",            "",           "",            "",           "",            "",           "",           "",            "",           "",           "",           "",            "",            "",            ""
};

static WORD s_anKeyset[ NUM_KBJOY_KEYS ] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/////////////////////////////////////////////////////////////////////////////
// CKeysetDlg dialog

BEGIN_MESSAGE_MAP(CKeysetDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CKeysetDlg)
	ON_BN_CLICKED(IDC_KEYSET_NW, OnKeysetNw)
	ON_BN_CLICKED(IDC_KEYSET_NORTH, OnKeysetNorth)
	ON_BN_CLICKED(IDC_KEYSET_NE, OnKeysetNe)
	ON_BN_CLICKED(IDC_KEYSET_EAST, OnKeysetEast)
	ON_BN_CLICKED(IDC_KEYSET_SE, OnKeysetSe)
	ON_BN_CLICKED(IDC_KEYSET_SOUTH, OnKeysetSouth)
	ON_BN_CLICKED(IDC_KEYSET_SW, OnKeysetSw)
	ON_BN_CLICKED(IDC_KEYSET_WEST, OnKeysetWest)
	ON_BN_CLICKED(IDC_KEYSET_FIRE, OnKeysetFire)
	ON_BN_CLICKED(IDC_KEYSET_CENTRE, OnKeysetCentre)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_KEYSET_OK, OnOK)
	ON_BN_CLICKED(IDC_KEYSET_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CKeysetDlg::CKeysetDlg
=========================================================*/
/* #FN#
   Standard constructor */
CKeysetDlg::
CKeysetDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CKeysetDlg::IDD, pParent ),
	  m_strTitlePostfix( "" ),
	  m_nCurrentKeyset( 0 ),
	  m_bChanged( FALSE )
{
	//{{AFX_DATA_INIT(CKeysetDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nFirstCtrl = IDC_KEYSET_FIRST;
	m_nLastCtrl  = IDC_KEYSET_LAST;

} /* #OF# CKeysetDlg::CKeysetDlg */

/*========================================================
Method   : CKeysetDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CKeysetDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CKeysetDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CKeysetDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CKeysetDlg implementation

/*========================================================
Method   : CKeysetDlg::SetTitlePostfix
=========================================================*/
/* #FN#
   Sets a postfix added to the window title (A / B) */
void
/* #AS#
   Nothing */
CKeysetDlg::
SetTitlePostfix(
	LPCSTR pszPostfix /* #IN# Postfix text */
)
{
	m_strTitlePostfix = pszPostfix;

} /* #OF# CKeysetDlg::SetTitlePostfix */

/*========================================================
Method   : CKeysetDlg::SetCurrentKeyset
=========================================================*/
/* #FN#
   Sets a given keyset (A / B) */
void
/* #AS#
   Nothing */
CKeysetDlg::
SetCurrentKeyset(
	int nKeyset /* #IN# Code of the keyset */
)
{
	m_nCurrentKeyset = nKeyset;

} /* #OF# CKeysetDlg::SetCurrentKeyset */

/*========================================================
Method   : CKeysetDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CKeysetDlg::
SetDlgState()
{
	SetDlgItemText( IDC_KEYSET_NW,     s_apszKBCodeToString[ s_anKeyset[ KEYSET_NW     ] ] );
	SetDlgItemText( IDC_KEYSET_NORTH,  s_apszKBCodeToString[ s_anKeyset[ KEYSET_NORTH  ] ] );
	SetDlgItemText( IDC_KEYSET_NE,     s_apszKBCodeToString[ s_anKeyset[ KEYSET_NE     ] ] );
	SetDlgItemText( IDC_KEYSET_EAST,   s_apszKBCodeToString[ s_anKeyset[ KEYSET_EAST   ] ] );
	SetDlgItemText( IDC_KEYSET_SE,     s_apszKBCodeToString[ s_anKeyset[ KEYSET_SE     ] ] );
	SetDlgItemText( IDC_KEYSET_SOUTH,  s_apszKBCodeToString[ s_anKeyset[ KEYSET_SOUTH  ] ] );
	SetDlgItemText( IDC_KEYSET_SW,     s_apszKBCodeToString[ s_anKeyset[ KEYSET_SW     ] ] );
	SetDlgItemText( IDC_KEYSET_WEST,   s_apszKBCodeToString[ s_anKeyset[ KEYSET_WEST   ] ] );
	SetDlgItemText( IDC_KEYSET_CENTRE, s_apszKBCodeToString[ s_anKeyset[ KEYSET_CENTRE ] ] );
	SetDlgItemText( IDC_KEYSET_FIRE,   s_apszKBCodeToString[ s_anKeyset[ KEYSET_FIRE   ] ] );

} /* #OF# CKeysetDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CKeysetDlg message handlers

/*========================================================
Method   : CKeysetDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CKeysetDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	CString strTitle;

	GetWindowText( strTitle );
	SetWindowText( strTitle + m_strTitlePostfix );

	for( int i = 0; i < NUM_KBJOY_KEYS; i++ )
		s_anKeyset[ i ] = g_Input.Joy.anKeysets[ m_nCurrentKeyset ][ i ];

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CKeysetDlg::OnInitDialog */

/*========================================================
Method   : CKeysetDlg::OnKeysetNw
=========================================================*/
/* #FN#
   Receives a key code for north-west keyjoy direction */
void
/* #AS#
   Nothing */
CKeysetDlg::
OnKeysetNw()
{
	CKeysetGKDlg dlgKeysetGK( s_anKeyset, KEYSET_NW );
	if( IDOK == dlgKeysetGK.DoModal() )
	{
		SetDlgItemText( IDC_KEYSET_NW, s_apszKBCodeToString[ s_anKeyset[ KEYSET_NW ] ] );
		m_bChanged = TRUE;
	}
} /* #OF# CKeysetDlg::OnKeysetNw */

/*========================================================
Method   : CKeysetDlg::OnKeysetNorth
=========================================================*/
/* #FN#
   Receives a key code for north keyjoy direction */
void
/* #AS#
   Nothing */
CKeysetDlg::
OnKeysetNorth()
{
	CKeysetGKDlg dlgKeysetGK( s_anKeyset, KEYSET_NORTH );
	if( IDOK == dlgKeysetGK.DoModal() )
	{
		SetDlgItemText( IDC_KEYSET_NORTH, s_apszKBCodeToString[ s_anKeyset[ KEYSET_NORTH ] ] );
		m_bChanged = TRUE;
	}
} /* #OF# CKeysetDlg::OnKeysetNorth */

/*========================================================
Method   : CKeysetDlg::OnKeysetNe
=========================================================*/
/* #FN#
   Receives a key code for north-east keyjoy direction */
void
/* #AS#
   Nothing */
CKeysetDlg::
OnKeysetNe()
{
	CKeysetGKDlg dlgKeysetGK( s_anKeyset, KEYSET_NE );
	if( IDOK == dlgKeysetGK.DoModal() )
	{
		SetDlgItemText( IDC_KEYSET_NE, s_apszKBCodeToString[ s_anKeyset[ KEYSET_NE ] ] );
		m_bChanged = TRUE;
	}
} /* #OF# CKeysetDlg::OnKeysetNe */

/*========================================================
Method   : CKeysetDlg::OnKeysetEast
=========================================================*/
/* #FN#
   Receives a key code for east keyjoy direction */
void
/* #AS#
   Nothing */
CKeysetDlg::
OnKeysetEast()
{
	CKeysetGKDlg dlgKeysetGK( s_anKeyset, KEYSET_EAST );
	if( IDOK == dlgKeysetGK.DoModal() )
	{
		SetDlgItemText( IDC_KEYSET_EAST, s_apszKBCodeToString[ s_anKeyset[ KEYSET_EAST ] ] );
		m_bChanged = TRUE;
	}
} /* #OF# CKeysetDlg::OnKeysetEast */

/*========================================================
Method   : CKeysetDlg::OnKeysetSe
=========================================================*/
/* #FN#
   Receives a key code for south-east keyjoy direction */
void
/* #AS#
   Nothing */
CKeysetDlg::
OnKeysetSe()
{
	CKeysetGKDlg dlgKeysetGK( s_anKeyset, KEYSET_SE );
	if( IDOK == dlgKeysetGK.DoModal() )
	{
		SetDlgItemText( IDC_KEYSET_SE, s_apszKBCodeToString[ s_anKeyset[ KEYSET_SE ] ] );
		m_bChanged = TRUE;
	}
} /* #OF# CKeysetDlg::OnKeysetSe */

/*========================================================
Method   : CKeysetDlg::OnKeysetSouth
=========================================================*/
/* #FN#
   Receives a key code for south keyjoy direction */
void
/* #AS#
   Nothing */
CKeysetDlg::
OnKeysetSouth()
{
	CKeysetGKDlg dlgKeysetGK( s_anKeyset, KEYSET_SOUTH );
	if( IDOK == dlgKeysetGK.DoModal() )
	{
		SetDlgItemText( IDC_KEYSET_SOUTH, s_apszKBCodeToString[ s_anKeyset[ KEYSET_SOUTH ] ] );
		m_bChanged = TRUE;
	}
} /* #OF# CKeysetDlg::OnKeysetSouth */

/*========================================================
Method   : CKeysetDlg::OnKeysetSw
=========================================================*/
/* #FN#
   Receives a key code for south-west keyjoy direction */
void
/* #AS#
   Nothing */
CKeysetDlg::
OnKeysetSw()
{
	CKeysetGKDlg dlgKeysetGK( s_anKeyset, KEYSET_SW );
	if( IDOK == dlgKeysetGK.DoModal() )
	{
		SetDlgItemText( IDC_KEYSET_SW, s_apszKBCodeToString[ s_anKeyset[ KEYSET_SW ] ] );
		m_bChanged = TRUE;
	}
} /* #OF# CKeysetDlg::OnKeysetSw */

/*========================================================
Method   : CKeysetDlg::OnKeysetWest
=========================================================*/
/* #FN#
   Receives a key code for west keyjoy direction */
void
/* #AS#
   Nothing */
CKeysetDlg::
OnKeysetWest()
{
	CKeysetGKDlg dlgKeysetGK( s_anKeyset, KEYSET_WEST );
	if( IDOK == dlgKeysetGK.DoModal() )
	{
		SetDlgItemText( IDC_KEYSET_WEST, s_apszKBCodeToString[ s_anKeyset[ KEYSET_WEST ] ] );
		m_bChanged = TRUE;
	}
} /* #OF# CKeysetDlg::OnKeysetWest */

/*========================================================
Method   : CKeysetDlg::OnKeysetCentre
=========================================================*/
/* #FN#
   Receives a key code for the centre */
void
/* #AS#
   Nothing */
CKeysetDlg::
OnKeysetCentre()
{
	CKeysetGKDlg dlgKeysetGK( s_anKeyset, KEYSET_CENTRE );
	if( IDOK == dlgKeysetGK.DoModal() )
	{
		SetDlgItemText( IDC_KEYSET_CENTRE, s_apszKBCodeToString[ s_anKeyset[ KEYSET_CENTRE ] ] );
		m_bChanged = TRUE;
	}
} /* #OF# CKeysetDlg::OnKeysetCentre */

/*========================================================
Method   : CKeysetDlg::OnKeysetFire
=========================================================*/
/* #FN#
   Receives a key code for the fire */
void
/* #AS#
   Nothing */
CKeysetDlg::
OnKeysetFire()
{
	CKeysetGKDlg dlgKeysetGK( s_anKeyset, KEYSET_FIRE );
	if( IDOK == dlgKeysetGK.DoModal() )
	{
		SetDlgItemText( IDC_KEYSET_FIRE, s_apszKBCodeToString[ s_anKeyset[ KEYSET_FIRE ] ] );
		m_bChanged = TRUE;
	}
} /* #OF# CKeysetDlg::OnKeysetFire */

/*========================================================
Method   : CKeysetDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CKeysetDlg::
OnOK()
{
	if( m_bChanged )
	{
		for( int i = 0; i < NUM_KBJOY_KEYS; i++ )
			g_Input.Joy.anKeysets[ m_nCurrentKeyset ][ i ] = s_anKeyset[ i ];

		/* Write changed keyset to registry */
		WriteRegKeyset( NULL, m_nCurrentKeyset - NUM_KBJOY_DEVICES );
	}
	CCommonDlg::OnOK();

} /* #OF# CKeysetDlg::OnOK */
