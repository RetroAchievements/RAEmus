/****************************************************************************
File    : KeyTemplateDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CKeyTemplateDlg implementation file
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "KeyboardDlg.h"
#include "KeyTemplateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NUM_OF_PC_KEYS			72
#define NUM_OF_A8_KEYS			54

#define KEYTEMPLATE_VERSION		1

#define IDC_KEYTEMPLATE_FIRST	IDC_KEYTEMPLATE_PCKEYS
#define IDC_KEYTEMPLATE_LAST	IDC_KEYTEMPLATE_CANCEL


/////////////////////////////////////////////////////////////////////////////
// Static objects

static const int s_anA8KeyVals[ NUM_OF_A8_KEYS ] =
{
	AKEY_ESCAPE,  AKEY_1,		  AKEY_2,	  AKEY_3,		 AKEY_4,		  AKEY_5,	 // ESC 1 2 3 4 5
	AKEY_6,		  AKEY_7,		  AKEY_8,	  AKEY_9,		 AKEY_0,		  AKEY_LESS, // 6 7 8 9 0 <
	AKEY_GREATER, AKEY_BACKSPACE, AKEY_TAB,	  AKEY_q,		 AKEY_w,		  AKEY_e,	 // > BACKSPC TAB q w e
	AKEY_r,		  AKEY_t,		  AKEY_y,	  AKEY_u,		 AKEY_i,		  AKEY_o,	 // r t y u i o
	AKEY_p,		  AKEY_MINUS,	  AKEY_EQUAL, AKEY_RETURN,	 AKEY_a,		  AKEY_s,	 // p - = RET a s
	AKEY_d,		  AKEY_f,		  AKEY_g,	  AKEY_h,		 AKEY_j,		  AKEY_k,	 // d f g h j k
	AKEY_l,		  AKEY_SEMICOLON, AKEY_PLUS,  AKEY_ASTERISK, AKEY_CAPSTOGGLE, AKEY_z,	 // l ; + * CAPS z
	AKEY_x,		  AKEY_c,		  AKEY_v,	  AKEY_b,		 AKEY_n,		  AKEY_m,	 // x c v b n m
	AKEY_COMMA,	  AKEY_FULLSTOP,  AKEY_SLASH, AKEY_ATARI,	 AKEY_SPACE,	  AKEY_NONE	 // , . / ATARI SPC NONE
};

/* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
/* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A) */

static const int s_anPCKeyVals[ NUM_OF_PC_KEYS ] =
{
	VK_ESCAPE,	0xc0,		0x31,		0x32,		0x33,			0x34,			// ESC ` 1 2 3 4
	0x35,		0x36,		0x37,		0x38,		0x39,			0x30,			// 5 6 7 8 9 0
	0xbd,		0xbb,		VK_BACK,	VK_TAB,		0x51,			0x57,			// - = BACKSPC TAB Q W
	0x45,		0x52,		0x54,		0x59,		0x55,			0x49,			// E R T Y U I
	0x4f,		0x50,		0xdb,		0xdd,		0xdc,			VK_CAPITAL,		// O P [ ] BACKSLASH CAPSLOCK
	0x41,		0x53,		0x44,		0x46,		0x47,			0x48,			// A S D F G H
	0x4a,		0x4b,		0x4c,		0xba,		0xde,			VK_RETURN,		// J K L ; ' ENTER
	0x5a,		0x58,		0x43,		0x56,		0x42,			0x4e,			// Z X C V B N
	0x4d,		0xbc,		0xbe,		0xbf,		VK_SPACE,		VK_UP,			// M , . / SPACE UP
	VK_LEFT,	VK_DOWN,	VK_RIGHT,	VK_DIVIDE,	VK_MULTIPLY,	VK_SUBTRACT,	// LEFT DOWN RIGHT / * -
	VK_NUMPAD7,	VK_NUMPAD8,	VK_NUMPAD9,	VK_NUMPAD4,	VK_NUMPAD5,		VK_NUMPAD6,		// 7 8 9 4 5 6
	VK_NUMPAD1,	VK_NUMPAD2,	VK_NUMPAD3,	VK_ADD,		VK_NUMPAD0,		VK_DECIMAL		// 1 2 3 + 0 .
};

/* Not used anymore, default.a8k keyboard template has been appended to the emulator package
static const int s_anKBDefault[ NUM_OF_PC_KEYS ] =
{
	AKEY_ESCAPE,	AKEY_NONE,		AKEY_1,			AKEY_2,			AKEY_3,			AKEY_4,			// ESC ` 1 2 3 4
	AKEY_5,			AKEY_6,			AKEY_7,			AKEY_8,			AKEY_9,			AKEY_0,			// 5 6 7 8 9 0
	AKEY_MINUS,		AKEY_EQUAL,		AKEY_BACKSPACE,	AKEY_TAB,		AKEY_q,			AKEY_w,			// - = BACKSPC TAB Q W
	AKEY_e,			AKEY_r,			AKEY_t,			AKEY_y,			AKEY_u,			AKEY_i,			// E R T Y U I
	AKEY_o,			AKEY_p,			AKEY_PLUS,		AKEY_ASTERISK,	AKEY_NONE,		AKEY_CAPSTOGGLE,// O P [ ] BACKSLASH CAPSLOCK
	AKEY_a,			AKEY_s,			AKEY_d,			AKEY_f,			AKEY_g,			AKEY_h,			// A S D F G H
	AKEY_j,			AKEY_k,			AKEY_l,			AKEY_SEMICOLON,	AKEY_NONE,		AKEY_RETURN,	// J K L ; ' ENTER
	AKEY_z,			AKEY_x,			AKEY_c,			AKEY_v,			AKEY_b,			AKEY_n,			// Z X C V B N
	AKEY_m,			AKEY_COMMA,		AKEY_FULLSTOP,	AKEY_SLASH,		AKEY_SPACE,		AKEY_UP,		// M , . / SPACE UP
	AKEY_LEFT,		AKEY_DOWN,		AKEY_RIGHT,		AKEY_SLASH,		AKEY_ASTERISK,	AKEY_MINUS,		// LEFT DOWN RIGHT / * -
	AKEY_7,			AKEY_8,			AKEY_9,			AKEY_4,			AKEY_5,			AKEY_6,			// 7 8 9 4 5 6
	AKEY_1,			AKEY_2,			AKEY_3,			AKEY_PLUS,		AKEY_0,			AKEY_FULLSTOP	// 1 2 3 + 0 .
};
*/

/////////////////////////////////////////////////////////////////////////////
// CKeyTemplateDlg dialog

BEGIN_MESSAGE_MAP(CKeyTemplateDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CKeyTemplateDlg)
	ON_CBN_SELCHANGE(IDC_KEYTEMPLATE_PCKEYS, OnSelchangePcKeys)
	ON_CBN_SELCHANGE(IDC_KEYTEMPLATE_A8KEYS, OnSelchangeAtariKeys)
	ON_BN_CLICKED(IDC_KEYTEMPLATE_CTRL, OnCtrl)
	ON_BN_CLICKED(IDC_KEYTEMPLATE_SHIFT, OnShift)
	ON_BN_CLICKED(IDC_KEYTEMPLATE_SAVE, OnSaveTemplate)
	ON_EN_KILLFOCUS(IDC_KEYTEMPLATE_DESCRIPTION, OnKillfocusDescription)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_KEYTEMPLATE_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_KEYTEMPLATE_OK, OnOK)
END_MESSAGE_MAP()

/*========================================================
Method   : CKeyTemplateDlg::CKeyTemplateDlg
=========================================================*/
/* #FN#
   Standard constructor */
CKeyTemplateDlg::
CKeyTemplateDlg(
	LPSTR pszTemplateFile,
	LPSTR pszTemplateDesc,
	int  *pKBTable,
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CKeyTemplateDlg::IDD, pParent ),
	  m_pKBTable( pKBTable )
{
	//{{AFX_DATA_INIT(CKeyTemplateDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ASSERT(NULL != pszTemplateFile);
	m_pszTemplateFile = pszTemplateFile;
	_strncpy( m_szTemplateFile, m_pszTemplateFile, MAX_PATH );

	ASSERT(NULL != pszTemplateDesc);
	m_pszTemplateDesc = pszTemplateDesc;
	_strncpy( m_szTemplateDesc, m_pszTemplateDesc, TEMPLATE_DESC_LENGTH );

	m_nFirstCtrl = IDC_KEYTEMPLATE_FIRST;
	m_nLastCtrl  = IDC_KEYTEMPLATE_LAST;

} /* #OF# CKeyTemplateDlg::CKeyTemplateDlg */

/*========================================================
Method   : CKeyTemplateDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CKeyTemplateDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CKeyTemplateDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CKeyTemplateDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CKeyTemplateDlg implementation

/*========================================================
Method   : CKeyTemplateDlg::ReadKeyTemplate
=========================================================*/
/* #FN#
   Static. Reads keyboard template from a given file */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
CKeyTemplateDlg::
ReadKeyTemplate(
	LPCSTR pszFileName,
	LPSTR  pszFileDesc,
	int   *pKBTable,
	HWND   hWnd /*=NULL*/
)
{
	BOOL bResult = FALSE;
	int  iFile;

	iFile = _open( pszFileName, O_RDONLY | O_BINARY, 0777 );
	if( iFile != -1 )
	{
		char szTag[ 4 ] = { 0 };

		_read( iFile, &szTag, 3 );

		if( strcmp( szTag, "A8K" ) != 0 )
			DisplayMessage( hWnd, IDS_ERROR_A8K_TYPE, 0, MB_ICONEXCLAMATION | MB_OK );
		else
		{
			UCHAR ucData;
			if( (_read( iFile, &ucData, 1 ) == -1) || ucData != KEYTEMPLATE_VERSION )
				DisplayMessage( hWnd, IDS_ERROR_A8K_VER, 0, MB_ICONEXCLAMATION | MB_OK );
			else
			{
				if( _read( iFile, &ucData, 1 ) != -1 )
				{
					if( _read( iFile, pszFileDesc, ucData ) != -1 )
					{
						pszFileDesc[ ucData ] = 0;
						if( _read( iFile, pKBTable, KEYBOARD_TABLE_SIZE * sizeof(int) ) != -1 )
							bResult = TRUE;
					}
				}
			}
		}
		_close( iFile );
	}
	return bResult;

} /* #OF# CKeyTemplateDlg::PrepareTemplate */

/*========================================================
Method   : CKeyTemplateDlg::SaveKeyTemplate
=========================================================*/
/* #FN#
   Static. Saves keyboard template to a given file */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
CKeyTemplateDlg::
SaveKeyTemplate(
	LPCSTR pszFileName,
	LPCSTR pszFileDesc,
	int   *pKBTable
)
{
	BOOL bResult = FALSE;
	int  iFile;

	iFile = _open( pszFileName, O_CREAT | O_RDWR | O_TRUNC | O_BINARY, 0777 );
	if( iFile != -1 )
	{
		if( _write( iFile, "A8K", 3 ) != -1 )
		{
			UCHAR ucData = KEYTEMPLATE_VERSION;

			if( _write( iFile, &ucData, 1 ) != -1 )
			{
				ucData = strlen( pszFileDesc );
				if( _write( iFile, &ucData, 1 ) != -1 )
				{
					if( _write( iFile, pszFileDesc, ucData ) != -1 )
					{
						if( _write( iFile, pKBTable, KEYBOARD_TABLE_SIZE * sizeof(int) ) != -1 )
							bResult = TRUE;
					}
				}
			}
		}
		_close( iFile );
	}
	return bResult;

} /* #OF# CKeyTemplateDlg::SaveKeyTemplate */

/*========================================================
Method   : CKeyTemplateDlg::SetDefaultTemplate
=========================================================*/
/* #FN#
   Fills template table with default values */
//void
/* #AS#
   Nothing */
//CKeyTemplateDlg::
//SetDefaultTemplate(
//	int *pKBTable
//)
//{
//	int i;
//	for( i = 0; i < KEYBOARD_TABLE_SIZE; i++ )
//		pKBTable[ i ] = AKEY_NONE;

//	for( i = 0; i < NUM_OF_PC_KEYS; i++ )
//		pKBTable[ s_anPCKeyVals[ i ] ] = s_anKBDefault[ i ];

//} /* #OF# CKeyTemplateDlg::SetDefaultTemplate */

/*========================================================
Method   : CKeyTemplateDlg::SetAtariSelect
=========================================================*/
/* #FN#
   Sets Atari's key regarding to PC key */
void
/* #AS#
   Nothing */
CKeyTemplateDlg::
SetAtariSelect()
{
	int nAKey = m_pKBTable[ s_anPCKeyVals[ m_nPCSelect ] ];
	if( AKEY_NONE != nAKey )
	{
		nAKey &= ~(AKEY_SHFT | AKEY_CTRL);

		for( int i = 0; i < NUM_OF_A8_KEYS; i++ )
			if( s_anA8KeyVals[ i ] == nAKey )
			{
				m_nA8Select = i;
				break;
			}
	}
	else
		m_nA8Select = NUM_OF_A8_KEYS - 1; /* AKEY_NONE */

} /* #OF# CKeyTemplateDlg::SetAtariSelect */

/*========================================================
Method   : CKeyTemplateDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CKeyTemplateDlg::
SetDlgState()
{
	CButton *pButton = NULL;
	char     szBuffer[ 6 ];

	_SetSelCbox( IDC_KEYTEMPLATE_PCKEYS, m_nPCSelect );
	_SetSelCbox( IDC_KEYTEMPLATE_A8KEYS, m_nA8Select );

	pButton = (CButton *)GetDlgItem( IDC_KEYTEMPLATE_CTRL );
	ASSERT(NULL != pButton);
	pButton->SetCheck( 0 );

	if( m_pKBTable[ s_anPCKeyVals[ m_nPCSelect ] ] != AKEY_NONE )
	{
		if( _IsFlagSet( m_pKBTable[ s_anPCKeyVals[ m_nPCSelect ] ], AKEY_CTRL ) )
			pButton->SetCheck( 1 );

		pButton->EnableWindow( TRUE );
	}
	else
		pButton->EnableWindow( FALSE );

	pButton = (CButton *)GetDlgItem( IDC_KEYTEMPLATE_SHIFT );
	ASSERT(NULL != pButton);
	pButton->SetCheck( 0 );

	if( m_pKBTable[ s_anPCKeyVals[ m_nPCSelect ] ] != AKEY_NONE )
	{
		if( _IsFlagSet( m_pKBTable[ s_anPCKeyVals[ m_nPCSelect ] ], AKEY_SHFT ) )
			pButton->SetCheck( 1 );

		pButton->EnableWindow( TRUE );
	}
	else
		pButton->EnableWindow( FALSE );

	sprintf( szBuffer, "[%d]", s_anPCKeyVals[ m_nPCSelect ] );
	SetDlgItemText( IDC_KEYTEMPLATE_VIRTKEY, szBuffer );

	SetDlgItemText( IDC_KEYTEMPLATE_DESCRIPTION, m_szTemplateDesc );

} /* #OF# CKeyTemplateDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CKeyTemplateDlg message handlers

/*========================================================
Method   : CKeyTemplateDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CKeyTemplateDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	m_nPCSelect = 0;
	m_nA8Select = 0;
	m_bChanged  = FALSE;

	SetAtariSelect();
	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CKeyTemplateDlg::OnInitDialog */

/*========================================================
Method   : CKeyTemplateDlg::OnCtlColor
=========================================================*/
/* #FN#
   The framework calls this member function when a child control
   is about to be drawn */
HBRUSH
/* #AS#
   Handle to the brush used for painting the control background */
CKeyTemplateDlg::
OnCtlColor(
	CDC  *pDC,
	CWnd *pWnd,
	UINT  nCtlColor
)
{
	HBRUSH hbr = CCommonDlg::OnCtlColor( pDC, pWnd, nCtlColor );

	if( CTLCOLOR_STATIC == nCtlColor )
	{
		if( pWnd->GetDlgCtrlID() == IDC_KEYTEMPLATE_VIRTKEY ||
			pWnd->GetDlgCtrlID() == IDC_KEYTEMPLATE_VIRTKEY_LABEL )
		{
			/* Set a text color of information for advanced users */
			pDC->SetTextColor( GetSysColor( COLOR_BTNSHADOW ) );
			/* Set the background mode for text to transparent
			   so background will show thru. */
//			pDC->SetBkMode( TRANSPARENT );
		}
	}
	/* TODO: Return a different brush if the default is not desired */
	return hbr;

} /* #OF# CKeyTemplateDlg::OnCtlColor */

/*========================================================
Method   : CKeyTemplateDlg::OnSelchangePcKeys
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CKeyTemplateDlg::
OnSelchangePcKeys()
{
	m_nPCSelect = _GetSelCbox( IDC_KEYTEMPLATE_PCKEYS );
	if( CB_ERR == m_nPCSelect || m_nPCSelect < 0 || m_nPCSelect > NUM_OF_PC_KEYS - 1 )
		m_nPCSelect = 0;

	SetAtariSelect();
	SetDlgState();

} /* #OF# CKeyTemplateDlg::OnSelchangePcKeys */

/*========================================================
Method   : CKeyTemplateDlg::OnSelchangeAtariKeys
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CKeyTemplateDlg::
OnSelchangeAtariKeys()
{
	m_nA8Select = _GetSelCbox( IDC_KEYTEMPLATE_A8KEYS );
	if( CB_ERR == m_nA8Select || m_nA8Select < 0 || m_nA8Select > NUM_OF_A8_KEYS - 1 )
		m_nA8Select = 0;

	m_pKBTable[ s_anPCKeyVals[ m_nPCSelect ] ] = s_anA8KeyVals[ m_nA8Select ];

	m_bChanged = TRUE;
	SetDlgState();

} /* #OF# CKeyTemplateDlg::OnSelchangeAtariKeys */

/*========================================================
Method   : CSettingsAdvDlg::OnCtrl
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CKeyTemplateDlg::
OnCtrl()
{
	_ClickButton( IDC_KEYTEMPLATE_CTRL, m_pKBTable[ s_anPCKeyVals[ m_nPCSelect ] ], AKEY_CTRL );
	m_bChanged = TRUE;

} /* #OF# CKeyTemplateDlg::OnCtrl */

/*========================================================
Method   : CSettingsAdvDlg::OnShift
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CKeyTemplateDlg::
OnShift()
{
	_ClickButton( IDC_KEYTEMPLATE_SHIFT, m_pKBTable[ s_anPCKeyVals[ m_nPCSelect ] ], AKEY_SHFT );
	m_bChanged = TRUE;

} /* #OF# CKeyTemplateDlg::OnShift */

/*========================================================
Method   : CKeyTemplateDlg::OnKillfocusDescription
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CKeyTemplateDlg::
OnKillfocusDescription()
{
	char szTemplateOld[ MAX_PATH + 1 ];

	strcpy( szTemplateOld, m_szTemplateDesc );
	GetDlgItemText( IDC_KEYTEMPLATE_DESCRIPTION, m_szTemplateDesc, TEMPLATE_DESC_LENGTH );

	if( _stricmp( szTemplateOld, m_szTemplateDesc ) != 0 )
		m_bChanged = TRUE;

} /* #OF# CKeyTemplateDlg::OnKillfocusDescription */

/*========================================================
Method   : CKeyTemplateDlg::OnSaveTemplate
=========================================================*/
/* #FN#
   Saves the keyboard template to a file */
void
/* #AS#
   Nothing */
CKeyTemplateDlg::
OnSaveTemplate()
{
	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	if( PickFileName( FALSE, m_szTemplateFile, IDS_SELECT_A8K_SAVE, IDS_FILTER_A8K,
					  "a8k", PF_SAVE_FLAGS, FALSE, DEFAULT_A8K, this ) &&
		*m_szTemplateFile != '\0' )
	{
		if( !SaveKeyTemplate( m_szTemplateFile, m_szTemplateDesc, m_pKBTable ) )
		{
			DisplayMessage( GetSafeHwnd(), IDS_ERROR_A8K_SAVE, 0, MB_ICONEXCLAMATION | MB_OK );
		}
		else
			m_bChanged = FALSE;
	}
} /* #OF# CKeyTemplateDlg::OnSaveTemplate */

/*========================================================
Method   : CKeyTemplateDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CKeyTemplateDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	if( IDC_KEYTEMPLATE_DESCRIPTION == nCtrlID )
	{
		OnKillfocusDescription();
	}
} /* #OF# CKeyTemplateDlg::ReceiveFocused */

/*========================================================
Method   : CKeyTemplateDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CKeyTemplateDlg::
OnOK()
{
	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	if( m_bChanged )
	{
		if( IDNO == DisplayMessage( GetSafeHwnd(), IDS_WARN_KEY_TEMPLATE, 0, MB_ICONQUESTION | MB_YESNO ) )
			return;
	}
	if( !m_bChanged )
	{
		if( *m_szTemplateFile == '\0' )
			strcpy( m_szTemplateFile, DEFAULT_A8K );

		if( _stricmp( m_pszTemplateFile, m_szTemplateFile ) != 0 )
			_strncpy( m_pszTemplateFile, m_szTemplateFile, MAX_PATH );

		if( _stricmp( m_pszTemplateDesc, m_szTemplateDesc ) != 0 )
			_strncpy( m_pszTemplateDesc, m_szTemplateDesc, TEMPLATE_DESC_LENGTH );
	}
	CCommonDlg::OnOK();

} /* #OF# CKeyTemplateDlg::OnOK */
