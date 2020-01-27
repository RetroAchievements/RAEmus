/****************************************************************************
File    : ConvertTypeDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CConvertTypeDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "ConvertTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_CONVERTTYPE_FIRST		IDC_CONVERTTYPE_NONE
#define IDC_CONVERTTYPE_LAST		IDC_CONVERTTYPE_CANCEL

#define POLISH_BUSHES_NO	18
#define CZECH_BUSHES_NO		46

#define EOL	155
#define CR	13
#define LF	10


/////////////////////////////////////////////////////////////////////////////
// Static objects

static UBYTE s_anPanther_1250pl[ POLISH_BUSHES_NO ][ 2 ] =
{
	{0x11, 0xa5}, // •
	{0x01, 0xb9}, // π
	{0x16, 0xc6}, // ∆
	{0x03, 0xe6}, // Ê
	{0x12, 0xca}, //  
	{0x05, 0xea}, // Í
	{0x0c, 0xb3}, // ≥
	{0x7b, 0xa3}, // £
	{0x0d, 0xd1}, // —
	{0x0e, 0xf1}, // Ò
	{0x10, 0xd3}, // ”
	{0x0f, 0xf3}, // Û
	{0x04, 0x8c}, // å
	{0x13, 0x9c}, // ú
	{0x18, 0xaf}, // Ø
	{0x1a, 0xbf}, // ø
	{0x0b, 0x9f}, // ü
	{0x60, 0x8f}  // è
};

static UBYTE s_anCapek_1250cz[ CZECH_BUSHES_NO ][ 2 ] =
{
	{0x01, 0xc1}, // 
	{0x02, 0x2b}, // 
	{0x03, 0xc8}, // 
	{0x04, 0xef}, // 
	{0x05, 0xc9}, // 
	{0x06, 0x28}, // 
	{0x07, 0x24}, // 
	{0x08, 0x2a}, // 
	{0x09, 0xcd}, // 
	{0x0a, 0xd2}, // 
	{0x0b, 0xd3}, // 
	{0x0c, 0xbe}, // 
	{0x0d, 0xcc}, // 
	{0x0e, 0xf2}, // 
	{0x0f, 0xf3}, // 
	{0x10, 0xfc}, // 
	{0x11, 0xe0}, // 
	{0x12, 0xd8}, // 
	{0x13, 0x8a}, // 
	{0x14, 0x9d}, // 
	{0x15, 0xda}, // 
	{0x16, 0xa7}, // 
	{0x17, 0xa9}, // 
	{0x18, 0xf4}, // 
	{0x19, 0xdd}, // 
	{0x1a, 0x8e}, // 
	{0x1c, 0xd9}, // 
	{0x1d, 0x8d}, // 
	{0x1f, 0xcf}, // 
	{0x24, 0x29}, // 
	{0x25, 0xec}, // 
	{0x26, 0x9a}, // 
	{0x27, 0xe8}, // 
	{0x28, 0x9e}, // 
	{0x29, 0xed}, // 
	{0x2a, 0xe1}, // 
	{0x2b, 0xf9}, // 
	{0x40, 0xf8}, // 
	{0x5b, 0xe9}, // 
	{0x5c, 0xbc}, // 
	{0x5d, 0xfa}, // 
	{0x5e, 0xc4}, // 
	{0x60, 0xfd}, // 
	{0x7b, 0xdc}, // 
	{0x7e, 0xf6}, // 
	{0x7f, 0xe4}  // 
};


/////////////////////////////////////////////////////////////////////////////
// CConvertTypeDlg dialog

BEGIN_MESSAGE_MAP(CConvertTypeDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CConvertTypeDlg)
	ON_BN_CLICKED(IDC_CONVERTTYPE_NONE, OnNone)
	ON_BN_CLICKED(IDC_CONVERTTYPE_PANTHER, OnPanther)
	ON_BN_CLICKED(IDC_CONVERTTYPE_CAPEK, OnCapek)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CONVERTTYPE_OK, OnOK)
	ON_BN_CLICKED(IDC_CONVERTTYPE_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CConvertTypeDlg::CConvertTypeDlg
=========================================================*/
/* #FN#
   Standard constructor */
CConvertTypeDlg::
CConvertTypeDlg(
	BOOL  bA2PC   /*=TRUE*/,
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: m_bA2PC( bA2PC ), CCommonDlg( CConvertTypeDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CConvertTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFirstCtrl = IDC_CONVERTTYPE_FIRST;
	m_nLastCtrl  = IDC_CONVERTTYPE_LAST;

} /* #OF# CConvertTypeDlg::CConvertTypeDlg */

/*========================================================
Method   : CConvertTypeDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CConvertTypeDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CConvertTypeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CConvertTypeDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CConvertTypeDlg implementation

/*========================================================
Method   : CConvertTypeDlg::Atascii2Ascii
=========================================================*/
/* #FN#
   Static. Does converting from/to ATASCII to/from ASCII (EOL) */
int
/* #AS#
   Destination length */
CConvertTypeDlg::
Atascii2Ascii(
	UBYTE *pubSrc,         /* #IN#  */
	UBYTE *pubDst,         /* #IN/OUT# */
	DWORD  dwSrcLen,       /* #IN#  */
	BOOL   bA2PC /*=TRUE*/ /* #IN#  */
)
{
	DWORD s = 0, d = 0;
	UBYTE c;

	for( s = 0; s < dwSrcLen; s++ )
	{
		c = pubSrc[ s ];
		if( c == (bA2PC ? EOL : CR) )
		{
			if( bA2PC )
			{
				pubDst[ d++ ] = CR;
				c = LF;
			}
			else
			/* We have to be sure there is the CR-LF pair */
			if( dwSrcLen > s + 1 && pubSrc[ s + 1 ] == LF )
			{
				c = EOL;
				s++;
			}
		}
		/* Save the char */
		pubDst[ d++ ] = c;
	}
	return d;

} /* #OF# CConvertTypeDlg::Atascii2Ascii */

/*========================================================
Method   : CConvertTypeDlg::A8Std2Win1250
=========================================================*/
/* #FN#
   Static. Does converting from/to A8 national letter standards to/from Win-1250 */
void
/* #AS#
   Nothing */
CConvertTypeDlg::
A8Std2Win1250(
	UBYTE      *pubSrc,         /* #IN#  */
	UBYTE      *pubDst,         /* #IN/OUT# */
	DWORD       dwSrcLen,       /* #IN#  */
	ConvertType ctConType,      /* #IN#  */
	BOOL        bA2PC /*=TRUE*/ /* #IN#  */
)
{
	if( None != ctConType )
	{
		UBYTE (*pConTable)[ 2 ] = NULL;
		DWORD dwBushesNo = 0;

		DWORD s = 0, d = 0, i = 0;
		UBYTE c;

		switch( ctConType )
		{
			/* Convert from/to Panther to/from Polish Win-1250 */
			case Panther:
				pConTable = s_anPanther_1250pl;
				dwBushesNo = POLISH_BUSHES_NO;
				break;

			/* Convert from/to Capek to/from Czech Win-1250 */
			case Capek:
				pConTable = s_anCapek_1250cz;
				dwBushesNo = CZECH_BUSHES_NO;
				break;
		}

		/* Do the converting work */
		for( s = 0; s < dwSrcLen; s++ )
		{
			c = pubSrc[ s ];

			/* Leave the CR-LF pairs alone */
			if( !(c == CR && dwSrcLen > s + 1 && pubSrc[ s + 1 ] == LF) && !(c == LF && 0 < s && pubSrc[ s - 1 ] == CR) )
			{
				for( i = 0; i < dwBushesNo; i++ )
					if( c == pConTable[ i ][ bA2PC ? 0 : 1 ] )
					{
						c = pConTable[ i ][ bA2PC ? 1 : 0 ];
						break;
					}
			}
			/* Save the char */
			pubDst[ d++ ] = c;
		}
	}
} /* #OF# CConvertTypeDlg::A8Std2Win1250 */

/*========================================================
Method   : CConvertTypeDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CConvertTypeDlg::
SetDlgState()
{
	char  szBuffer[ LOADSTRING_SIZE_L + 1 ];
	int   nCtrlID = 0;
	CWnd *pWnd    = NULL;

	switch( m_ctSelOpt )
	{
		case None:
			_LoadStringLx( IDS_CONV_DESC_NONE, szBuffer );
			nCtrlID = IDC_CONVERTTYPE_NONE;
			break;

		case Panther:
			_LoadStringLx( IDS_CONV_DESC_PANTHER, szBuffer );
			nCtrlID = IDC_CONVERTTYPE_PANTHER;
			break;

		case Capek:
			_LoadStringLx( IDS_CONV_DESC_CAPEK, szBuffer );
			nCtrlID = IDC_CONVERTTYPE_CAPEK;
			break;
	}
	/* Set appropriate radio button */
	CheckRadioButton( IDC_CONVERTTYPE_NONE, IDC_CONVERTTYPE_CAPEK, nCtrlID );

	/* Set option description */
	pWnd = GetDlgItem( IDC_CONVERTTYPE_DESCRIPTION );
	ASSERT(NULL != pWnd);
	pWnd->SetWindowText( szBuffer );

} /* #OF# CConvertTypeDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CConvertTypeDlg message handlers

/*========================================================
Method   : CConvertTypeDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CConvertTypeDlg::
OnInitDialog() 
{
	CCommonDlg::OnInitDialog();

	char szLabel[ LOADSTRING_SIZE_S + 1 ];
	CWnd *pWnd = NULL;

	m_ctSelOpt = None;

	/* Set an appropriate window name */
	SetWindowText( _LoadStringSx( m_bA2PC ? IDS_CONVDLG_NAME_A2PC : IDS_CONVDLG_NAME_PC2A, szLabel ) );
	/* Set appropriate option descriptions */
	/* Panther */
	pWnd = GetDlgItem( IDC_CONVERTTYPE_PANTHER );
	ASSERT(NULL != pWnd);
	pWnd->SetWindowText( _LoadStringSx( m_bA2PC ? IDS_CONV_PANTHER2WIN : IDS_CONV_WIN2PANTHER, szLabel ) );
	/* Capek */
	pWnd = GetDlgItem( IDC_CONVERTTYPE_CAPEK );
	ASSERT(NULL != pWnd);
	pWnd->SetWindowText( _LoadStringSx( m_bA2PC ? IDS_CONV_CAPEK2WIN : IDS_CONV_WIN2CAPEK, szLabel ) );

	SetDlgState();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CConvertTypeDlg::OnInitDialog */

/*========================================================
Method   : CConvertTypeDlg::OnNone
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate radio button */
void
/* #AS#
   Nothing */
CConvertTypeDlg::
OnNone()
{
	m_ctSelOpt = None;
	SetDlgState();

} /* #OF# CConvertTypeDlg::OnNone */

/*========================================================
Method   : CConvertTypeDlg::OnPanther
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate radio button */
void
/* #AS#
   Nothing */
CConvertTypeDlg::
OnPanther()
{
	m_ctSelOpt = Panther;
	SetDlgState();

} /* #OF# CConvertTypeDlg::OnPanther */

/*========================================================
Method   : CConvertTypeDlg::OnCapek
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate radio button */
void
/* #AS#
   Nothing */
CConvertTypeDlg::
OnCapek()
{
	m_ctSelOpt = Capek;
	SetDlgState();

} /* #OF# CConvertTypeDlg::OnCapek */

/*========================================================
Method   : CConvertTypeDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CConvertTypeDlg::
OnOK()
{
	CCommonDlg::OnOK();

} /* #OF# CConvertTypeDlg::OnOK */
