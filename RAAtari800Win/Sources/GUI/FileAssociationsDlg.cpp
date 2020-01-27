/****************************************************************************
File    : FileAssociationsDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CFileAssociationsDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileAssociationsBase.h"
#include "FileAssociationsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_FILEASSOCIATIONS_FIRST		IDC_FILEASSOCIATIONS_ATR
#define IDC_FILEASSOCIATIONS_LAST		IDC_FILEASSOCIATIONS_CANCEL


/////////////////////////////////////////////////////////////////////////////
// CFileAssociationsDlg Dialog

BEGIN_MESSAGE_MAP(CFileAssociationsDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CFileAssociationsDlg)
	ON_BN_CLICKED(IDC_FILEASSOCIATIONS_SELECTALL, OnSelectAll)
	ON_BN_CLICKED(IDC_FILEASSOCIATIONS_REMOVEALL, OnRemoveAll)
	ON_BN_CLICKED(IDC_FILEASSOCIATIONS_AUTOREG, OnAutoReg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_FILEASSOCIATIONS_OK, OnOK)
	ON_BN_CLICKED(IDC_FILEASSOCIATIONS_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CFileAssociationsDlg::CFileAssociationsDlg
=========================================================*/
/* #FN#
   Standard constructor */
CFileAssociationsDlg::
CFileAssociationsDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CFileAssociationsDlg::IDD, pParent ),
	  CFileAssociationsBase()
{
	//{{AFX_DATA_INIT(CFileAssociationsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFirstCtrl = IDC_FILEASSOCIATIONS_FIRST;
	m_nLastCtrl  = IDC_FILEASSOCIATIONS_LAST;

} /* #OF# CFileAssociationsDlg::CFileAssociationsDlg */

/*========================================================
Method   : CFileAssociationsDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CFileAssociationsDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	CFileAssociationsBase::DataExchange( pDX );
	//{{AFX_DATA_MAP(CFileAssociationsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CFileAssociationsDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CFileAssociationsDlg implementation

/*========================================================
Method   : CFileAssociationsDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CFileAssociationsDlg::
SetDlgState()
{
	_SetChkBttn( IDC_FILEASSOCIATIONS_AUTOREG, _IsFlagSet( m_ulMiscState, MS_AUTO_REG_FILES ) );

	for( int i = 0; i < m_nFileExtInfoNo; i++ )
	{
		_SetChkBttn( m_pFileExtInfo[ i ].nCtrlID, _IsFlagSet( m_ulFileAssociations, m_pFileExtInfo[ i ].dwRegFlag ) );
	}
} /* #OF# CFileAssociationsDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CFileAssociationsDlg message handlers

/*========================================================
Method   : CFileAssociationsDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CFileAssociationsDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();
	CFileAssociationsBase::InitDialog();

	const UINT anCtrls[ /*m_nFileExtInfoNo*/ ] =
	{
		IDC_FILEASSOCIATIONS_ATR,
		IDC_FILEASSOCIATIONS_XFD,
		IDC_FILEASSOCIATIONS_DCM,
		IDC_FILEASSOCIATIONS_ATZ,
		IDC_FILEASSOCIATIONS_XFZ,
		IDC_FILEASSOCIATIONS_XEX,
		IDC_FILEASSOCIATIONS_CAS,
		IDC_FILEASSOCIATIONS_ROM,
		IDC_FILEASSOCIATIONS_BIN,
		IDC_FILEASSOCIATIONS_CAR,
		IDC_FILEASSOCIATIONS_A8S
	};

	/* Set the appropriate controls IDs */
	for( int i = 0; i < m_nFileExtInfoNo; i++ )
		m_pFileExtInfo[ i ].nCtrlID = anCtrls[ i ];

	m_ulMiscState = g_Misc.ulState;

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CFileAssociationsDlg::OnInitDialog */

/*========================================================
Method   : CFileAssociationsDlg::OnSelectAll
=========================================================*/
/* #FN#
   Sets all the check-boxes to 'checked' state */
void
/* #AS#
   Nothing */
CFileAssociationsDlg::
OnSelectAll()
{
	SelectAll();

} /* #OF# CFileAssociationsDlg::OnSelectAll */

/*========================================================
Method   : CFileAssociationsDlg::OnRemoveAll
=========================================================*/
/* #FN#
   Sets all the check-boxes to 'unchecked' state */
void
/* #AS#
   Nothing */
CFileAssociationsDlg::
OnRemoveAll()
{
	RemoveAll();

} /* #OF# CFileAssociationsDlg::OnRemoveAll */

/*========================================================
Method   : CFileAssociationsDlg::OnAutoReg
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate check box */
void
CFileAssociationsDlg::
OnAutoReg()
{
	_ClickButton( IDC_FILEASSOCIATIONS_AUTOREG, m_ulMiscState, MS_AUTO_REG_FILES );

} /* #OF# CFileAssociationsDlg::OnAutoReg */

/*========================================================
Method   : CFileAssociationsDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CFileAssociationsDlg::
OnOK() 
{
	CButton *pButton = NULL;
	for( int i = 0; i < m_nFileExtInfoNo; i++ )
	{
		_ClickButton( m_pFileExtInfo[ i ].nCtrlID, m_ulFileAssociations, m_pFileExtInfo[ i ].dwRegFlag );
	}
	/* Write dialog status only if necessary */
	if( m_ulFileAssociations != g_Misc.ulFileAssociations )
	{
		g_Misc.ulFileAssociations = m_ulFileAssociations;
		WriteRegDWORD( NULL, REG_FILE_ASSOCIATIONS, g_Misc.ulFileAssociations );

		WriteRegFileExt( g_Misc.ulFileAssociations, m_szHomeDir, g_szCmdLine );
	}
	if( m_ulMiscState != g_Misc.ulState )
	{
		g_Misc.ulState = m_ulMiscState;
		WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );
	}	
	CCommonDlg::OnOK();

} /* #OF# CFileAssociationsDlg::OnOK */
