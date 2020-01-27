/****************************************************************************
File    : GraphicsAdvDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CGraphicsAdvDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 14.10.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "GraphicsAdvDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_GRAPHICSADV_FIRST		IDC_GRAPHICSADV_BLITMODE
#define IDC_GRAPHICSADV_LAST		IDC_GRAPHICSADV_CANCEL


/////////////////////////////////////////////////////////////////////////////
// CGraphicsAdvDlg dialog

BEGIN_MESSAGE_MAP(CGraphicsAdvDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CGraphicsAdvDlg)
	ON_CBN_SELCHANGE(IDC_GRAPHICSADV_BLITMODE, OnSelchangeBlitMode)
	ON_CBN_SELCHANGE(IDC_GRAPHICSADV_REFRESHRATE, OnSelchangeRefreshRate)
	ON_BN_CLICKED(IDC_GRAPHICSADV_WAITVBL, OnWaitVbl)
	ON_BN_CLICKED(IDC_GRAPHICSADV_SAFEMODE, OnSafeMode)
	ON_CBN_SELCHANGE(IDC_GRAPHICSADV_MEMORYTYPE, OnSelchangeMemoryType)
	ON_CBN_SELCHANGE(IDC_GRAPHICSADV_BLITEFFECTS, OnSelchangeBlitEffects)
	ON_BN_CLICKED(IDC_GRAPHICSADV_OPTIMIZEPRO, OnOptimizePro)
	ON_BN_CLICKED(IDC_GRAPHICSADV_USEMMX, OnUseMMX)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_GRAPHICSADV_OK, OnOK)
	ON_BN_CLICKED(IDC_GRAPHICSADV_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CGraphicsAdvDlg::CGraphicsAdvDlg
=========================================================*/
/* #FN#
   Standard constructor */
CGraphicsAdvDlg::
CGraphicsAdvDlg(
	ULONG *pScreenState,     /* #IN# Pointer to screen mode flags  */
	int   *pMemoryType,      /* #IN# Pointer to memory type switch */
	int   *pRefreshRate,     /* #IN# Requested refresh rate        */
	ULONG  ulScreenMode,     /* #IN# Selected full screen mode     */
	CWnd  *pParent /*=NULL*/ /* #IN# Pointer to the parent window  */
)
	: CCommonDlg( CGraphicsAdvDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CGraphicsAdvDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ASSERT(NULL != pScreenState && NULL != pMemoryType);

	m_pScreenState = pScreenState;
	m_pMemoryType  = pMemoryType;
	m_pRefreshRate = pRefreshRate;
	m_ulScreenMode = ulScreenMode;
	m_nFirstCtrl   = IDC_GRAPHICSADV_FIRST;
	m_nLastCtrl    = IDC_GRAPHICSADV_LAST;

} /* #OF# CGraphicsAdvDlg::CGraphicsAdvDlg */

/*========================================================
Method   : CGraphicsAdvDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
CGraphicsAdvDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CGraphicsAdvDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CGraphicsAdvDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CGraphicsAdvDlg implementation

int compare( const void *arg1, const void *arg2 )
{
	return abs(*(int*)(arg1)) - abs(*(int*)(arg2));
}

/*========================================================
Method   : CGraphicsAdvDlg::PopulateRefreshRateList
=========================================================*/
/* #FN#
   Populates the list (combo box) of refresh rates */
void
/* #AS#
   Nothing */
CGraphicsAdvDlg::
PopulateRefreshRateList()
{
	int i;
	CComboBox *pCombo = (CComboBox *)GetDlgItem( IDC_GRAPHICSADV_REFRESHRATE );
	int nIndex = 0;

	ASSERT(NULL != pCombo);

	int  anRefreshRates[ MAX_REFRESH_RATE_NO + 1 ] = { 0 };
	char szBuffer[ LOADSTRING_SIZE_S + 1 ];

	DD_GetRefreshRates( m_ulScreenMode, anRefreshRates, MAX_REFRESH_RATE_NO );
	/* When no valid refresh frequency values have been received, add an
	   item the user will be allow to select for achieving double-sync */
	if( 0 == anRefreshRates[ 0 ] )
	{
		anRefreshRates[ 0 ] = (Atari800_TV_PAL == Atari800_tv_mode ? DEF_PAL_FREQUENCY : DEF_NTSC_FREQUENCY) * -2;
		anRefreshRates[ 1 ] = 0;
	}
	/* Sort the refresh rate table (not really necessary at the moment) */
	for( i = 0; i < MAX_REFRESH_RATE_NO && anRefreshRates[ i ]; i++ );
	qsort( anRefreshRates, i, sizeof(int), compare );

	pCombo->ResetContent();

	if( CB_ERR != (nIndex = pCombo->AddString( _LoadStringSx( IDS_DEFAULT, szBuffer ) )) )
	{
		CString strFreq;
		pCombo->SetItemData( nIndex, 0 );

		int nFreq = 0;
		for( i = 0; i < MAX_REFRESH_RATE_NO && anRefreshRates[ i ]; i++ )
		{
			if( nFreq != abs( anRefreshRates[ i ] ) ) /* Eliminate the duplicates */
			{
				nFreq = anRefreshRates[ i ];
				if( nFreq < 0 )
				{
					strFreq.Format( "(%d) Hz", -nFreq );
				}
				else
					strFreq.Format( "%d Hz", nFreq );

				if( CB_ERR != (nIndex = pCombo->AddString( strFreq )) )
					/* The same value for PAL and NTSC just for requesting double-sync */
					pCombo->SetItemData( nIndex, (DWORD)max( -1, nFreq ) );
			}
		}
	}
} /* #OF# CGraphicsAdvDlg::PopulateRefreshRateList */

/*========================================================
Method   : CGraphicsAdvDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CGraphicsAdvDlg::
SetDlgState()
{
	int i;
	CComboBox *pBlitCombo = (CComboBox *)GetDlgItem( IDC_GRAPHICSADV_BLITEFFECTS );
	CComboBox *pRateCombo = (CComboBox *)GetDlgItem( IDC_GRAPHICSADV_REFRESHRATE );

	ULONG ulIndex = (m_ulScreenState >> 28) & 0x0f; 

	ASSERT(NULL != pBlitCombo && NULL != pRateCombo);

	_SetChkBttn( IDC_GRAPHICSADV_WAITVBL,     _IsFlagSet( m_ulScreenState, SM_OPTN_DDVBL_WAIT   ) );
	_SetChkBttn( IDC_GRAPHICSADV_SAFEMODE,    _IsFlagSet( m_ulScreenState, SM_OPTN_SAFE_MODE    ) );
	_SetChkBttn( IDC_GRAPHICSADV_OPTIMIZEPRO, _IsFlagSet( m_ulScreenState, SM_OPTN_OPTIMIZE_PRO ) );
	_SetChkBttn( IDC_GRAPHICSADV_USEMMX,      _IsFlagSet( m_ulScreenState, SM_OPTN_USE_MMX      ) );
	_EnableCtrl( IDC_GRAPHICSADV_USEMMX,      _IsFlagSet( g_Misc.unSystemInfo, SYS_PRC_MMX      ) );

	_EnableCtrl( IDC_GRAPHICSADV_REFRESHRATE,       pRateCombo->GetCount() > 1 );
	_EnableCtrl( IDC_GRAPHICSADV_REFRESHRATE_LABEL, pRateCombo->GetCount() > 1 );

	/* Set the combos */
	_SetSelCbox( IDC_GRAPHICSADV_BLITMODE,   _IsFlagSet( m_ulScreenState, SM_OPTN_FLIP_BUFFERS ) ? 1 : 0 );
	_SetSelCbox( IDC_GRAPHICSADV_MEMORYTYPE, m_nMemoryType   );

	for( i = 0; i < pRateCombo->GetCount(); i++ )
		if( m_nRefreshRate == (int)pRateCombo->GetItemData( i ) )
		{
			pRateCombo->SetCurSel( i );
			break;
		}
	if( i == pRateCombo->GetCount() )
		pRateCombo->SetCurSel( 0 );

	for( i = 0; i < BLIT_EFFECTS_NO; i++ )
		if( ulIndex & (1 << i) )
		{
			pBlitCombo->SetCurSel( i + 1 );
			break;
		}
	if( i == BLIT_EFFECTS_NO )
		pBlitCombo->SetCurSel( 0 );

} /* #OF# CGraphicsAdvDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CGraphicsAdvDlg message handlers

/*========================================================
Method   : CGraphicsAdvDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CGraphicsAdvDlg::
OnInitDialog() 
{
	CCommonDlg::OnInitDialog();

	m_ulScreenState = *m_pScreenState;
	m_nMemoryType   = *m_pMemoryType;
	m_nRefreshRate  = *m_pRefreshRate;

	PopulateRefreshRateList();

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CGraphicsAdvDlg::OnInitDialog */

/*========================================================
Method   : CGraphicsAdvDlg::OnSelchangeBlitMode
=========================================================*/
/* #FN#
   Turns off the flipping buffers method */
void
/* #AS#
   Nothing */
CGraphicsAdvDlg::
OnSelchangeBlitMode()
{
	_ModifyFlag( _GetSelCbox( IDC_GRAPHICSADV_BLITMODE ), m_ulScreenState, SM_OPTN_FLIP_BUFFERS );

} /* #OF# CGraphicsAdvDlg::OnSelchangeBlitMode */

/*========================================================
Method   : CGraphicsAdvDlg::OnSelchangeRefreshRate
=========================================================*/
/* #FN#
   Turns off the flipping buffers method */
void
/* #AS#
   Nothing */
CGraphicsAdvDlg::
OnSelchangeRefreshRate()
{
	CComboBox *pCombo = (CComboBox *)GetDlgItem( IDC_GRAPHICSADV_REFRESHRATE );
	int nIndex = 0;

	ASSERT(NULL != pCombo);

	if( CB_ERR == (nIndex = pCombo->GetCurSel()) )
	{
		pCombo->SetCurSel( 0 );
		nIndex = 0;
	}
	m_nRefreshRate = (int)pCombo->GetItemData( nIndex );

} /* #OF# CGraphicsAdvDlg::OnSelchangeRefreshRate */

/*========================================================
Method   : CGraphicsAdvDlg::OnWaitVbl
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CGraphicsAdvDlg::
OnWaitVbl()
{
	if( _GetChkBttn( IDC_GRAPHICSADV_WAITVBL ) )
	{
		DisplayWarning( IDS_WARN_VBLWAIT, DONT_SHOW_VBLWAIT_WARN, FALSE );
		_SetFlag( m_ulScreenState, SM_OPTN_DDVBL_WAIT );
	}
	else
		_ClrFlag( m_ulScreenState, SM_OPTN_DDVBL_WAIT );

	SetDlgState();

} /* #OF# CGraphicsAdvDlg::OnWaitVbl */

/*========================================================
Method   : CGraphicsAdvDlg::OnSafeMode
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CGraphicsAdvDlg::
OnSafeMode()
{
	_ClickButton( IDC_GRAPHICSADV_SAFEMODE, m_ulScreenState, SM_OPTN_SAFE_MODE );

} /* #OF# CGraphicsAdvDlg::OnSafeMode */

/*========================================================
Method   : CGraphicsAdvDlg::OnOptimizePro
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CGraphicsAdvDlg::
OnOptimizePro()
{
	_ClickButton( IDC_GRAPHICSADV_OPTIMIZEPRO, m_ulScreenState, SM_OPTN_OPTIMIZE_PRO );

} /* #OF# CGraphicsAdvDlg::OnOptimizePro */

/*========================================================
Method   : CGraphicsAdvDlg::OnUseMMX
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CGraphicsAdvDlg::
OnUseMMX()
{
	_ClickButton( IDC_GRAPHICSADV_USEMMX, m_ulScreenState, SM_OPTN_USE_MMX );

} /* #OF# CGraphicsAdvDlg::OnUseMMX */

/*========================================================
Method   : CGraphicsAdvDlg::OnSelchangeMemoryType
=========================================================*/
/* #FN#
   Allows choosing system/AGP/video memory for Direct Draw offscreen surfaces */
void
/* #AS#
   Nothing */
CGraphicsAdvDlg::
OnSelchangeMemoryType()
{
	m_nMemoryType = _GetSelCbox( IDC_GRAPHICSADV_MEMORYTYPE );

} /* #OF# CGraphicsAdvDlg::OnSelchangeMemoryType */

/*========================================================
Method   : CGraphicsAdvDlg::OnSelchangeBlitEffects
=========================================================*/
/* #FN#
   Allows choosing a certain graphic FX effects */
void
/* #AS#
   Nothing */
CGraphicsAdvDlg::
OnSelchangeBlitEffects()
{
	DWORD dwSMFxFlags[ BLIT_EFFECTS_NO ] = {
		SM_DDFX_MIRRORLEFTRIGHT,
		SM_DDFX_MIRRORUPDOWN,
		SM_DDFX_NOTEARING };

	int nSelection = _GetSelCbox( IDC_GRAPHICSADV_BLITEFFECTS );

	if( CB_ERR != nSelection )
	{
		_ClrFlag( m_ulScreenState, SM_DDFX_MASK );
		if( nSelection > 0 )
		{
			_SetFlag( m_ulScreenState, dwSMFxFlags[ nSelection - 1 ] );
		}
	}
} /* #OF# CGraphicsAdvDlg::OnSelchangeBlitEffects */

/*========================================================
Method   : CGraphicsAdvDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CGraphicsAdvDlg::
OnOK()
{
	if( m_ulScreenState != *m_pScreenState )
		*m_pScreenState = m_ulScreenState;

	if( m_nMemoryType != *m_pMemoryType )
		*m_pMemoryType = m_nMemoryType;

	if( m_nRefreshRate != *m_pRefreshRate )
		*m_pRefreshRate = m_nRefreshRate;

	CCommonDlg::OnOK();

} /* #OF# CGraphicsAdvDlg::OnOK */
