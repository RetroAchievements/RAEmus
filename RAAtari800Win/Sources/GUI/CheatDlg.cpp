/****************************************************************************
File    : CheatDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CCheatDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "MainFrame.h"
#include "Helpers.h"
#include "FileService.h"
#include "CheatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CHEATSNAPSHOT_VERSION	1

#define IDC_CHEAT_FIRST			IDC_CHEAT_DISABLECOLLISIONS
#define IDC_CHEAT_LAST			IDC_CHEAT_CANCEL

#define MIN_MEMO				0
#define MAX_MEMO				255
#define MIN_SEARCH				0
#define MAX_SEARCH				255
#define MIN_LOCK				0
#define MAX_LOCK				255

#define MAX_ITEM_LENGTH			16


/////////////////////////////////////////////////////////////////////////////
// CCheatDlg dialog

BEGIN_MESSAGE_MAP(CCheatDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CCheatDlg)
	ON_BN_CLICKED(IDC_CHEAT_DISABLECOLLISIONS, OnDisableCollisions)
	ON_BN_CLICKED(IDC_CHEAT_PLAYERPLAYER, OnPlayerPlayer)
	ON_BN_CLICKED(IDC_CHEAT_PLAYERPLAYFIELD, OnPlayerPlayfield)
	ON_BN_CLICKED(IDC_CHEAT_MISSILEPLAYER, OnMissilePlayer)
	ON_BN_CLICKED(IDC_CHEAT_MISSILEPLAYFIELD, OnMissilePlayfield)
	ON_BN_CLICKED(IDC_CHEAT_SEARCH_BUTTON, OnSearchButton)
	ON_BN_CLICKED(IDC_CHEAT_MEMO_BUTTON, OnMemoButton)
	ON_NOTIFY(UDN_DELTAPOS, IDC_CHEAT_MEMO_SPIN, OnDeltaposMemoSpin)
	ON_EN_KILLFOCUS(IDC_CHEAT_MEMO, OnKillfocusMemo)
	ON_NOTIFY(UDN_DELTAPOS, IDC_CHEAT_SEARCH_SPIN, OnDeltaposSearchSpin)
	ON_EN_KILLFOCUS(IDC_CHEAT_SEARCH, OnKillfocusSearch)
	ON_NOTIFY(UDN_DELTAPOS, IDC_CHEAT_LOCK_SPIN, OnDeltaposLockSpin)
	ON_EN_KILLFOCUS(IDC_CHEAT_LOCK, OnKillfocusLock)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_CHEAT_POKELIST, OnItemchangedPokeList)
	ON_BN_CLICKED(IDC_CHEAT_LOCK_CHECK, OnLockCheck)
	ON_BN_CLICKED(IDC_CHEAT_LOAD, OnLoadButton)
	ON_BN_CLICKED(IDC_CHEAT_SAVE, OnSaveButton)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHEAT_OK, OnOK)
	ON_BN_CLICKED(IDC_CHEAT_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CCheatDlg::CCheatDlg
=========================================================*/
/* #FN#
   Standard constructor */
CCheatDlg::
CCheatDlg(
	CCheatServer *pCheatServer,
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CCheatDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CCheatDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pCheatServer    = pCheatServer;
	m_bTrainerChanged = FALSE;
	m_bListNotify     = FALSE;
	m_nFirstCtrl      = IDC_CHEAT_FIRST;
	m_nLastCtrl       = IDC_CHEAT_LAST;

	m_bFontCreated = m_fontList.CreateFont(
		g_bLargeFonts ? 16 : 13,
		0, 0, 0,
		FW_NORMAL,
		0, 0, 0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		"Courier New"
	);
} /* #OF# CCheatDlg::CCheatDlg */

/*========================================================
Method   : CCheatDlg::~CCheatDlg
=========================================================*/
/* #FN#
   Destructor */
CCheatDlg::
~CCheatDlg()
{
	/* Done with the font, so delete the font object */
	if( m_bFontCreated )
		m_fontList.DeleteObject();

} /* #OF# CCheatDlg::~CCheatDlg */

/*========================================================
Method   : CCheatDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange */
void
/* #AS#
   Nothing */
CCheatDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CCheatDlg)
	DDX_Control(pDX, IDC_CHEAT_POKELIST, m_listPoke);
	//}}AFX_DATA_MAP

} /* #OF# CCheatDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CCheatDlg implementation

/*========================================================
Method   : CCheatDlg::ReadTrainerSnapshot
=========================================================*/
/* #FN#
   Static. Reads trainer information from a given file */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
CCheatDlg::
ReadTrainerSnapshot(
	LPCSTR  pszFileName,
	int    *pMemo,
	int    *pSearch,
	int    *pLockAt,
	BOOL   *pLock,
	PDWORD *ppValues,
	int    *pCount,
	HWND    hWnd /*=NULL*/
)
{
	BOOL bResult = FALSE;
	int  iFile;

	iFile = _open( pszFileName, O_RDONLY | O_BINARY, 0777 );
	if( iFile != -1 )
	{
		char szTag[ 4 ] = { 0 };

		_read( iFile, &szTag, 3 );

		if( strcmp( szTag, "A8T" ) != 0 )
			DisplayMessage( hWnd, IDS_ERROR_A8T_TYPE, 0, MB_ICONEXCLAMATION | MB_OK );
		else
		{
			UCHAR ucData;
			if( (_read( iFile, &ucData, 1 ) == -1) || ucData != CHEATSNAPSHOT_VERSION )
				DisplayMessage( hWnd, IDS_ERROR_A8T_VER, 0, MB_ICONEXCLAMATION | MB_OK );
			else
			{
				int anParams[ 4 ];
				if( _read( iFile, &anParams, 4 * sizeof(int) ) != -1 )
				{
					if( _read( iFile, &ucData, 1 ) != -1 )
					{
						*ppValues = (PDWORD)calloc( ucData, sizeof(DWORD) );
						if( NULL != *ppValues )
						{
							if( _read( iFile, *ppValues, ucData * sizeof(DWORD) ) != -1 )
							{
								*pMemo   = anParams[ 0 ];
								*pSearch = anParams[ 1 ];
								*pLockAt = anParams[ 2 ];
								*pLock   = (BOOL)anParams[ 3 ];
								*pCount  = ucData;

								bResult = TRUE;
							}
							else
							{
								free( *ppValues );
								*ppValues = NULL;
							}
						}
					}
				}
			}
		}
		_close( iFile );
	}
	return bResult;

} /* #OF# CCheatDlg::ReadTrainerSnapshot */

/*========================================================
Method   : CCheatDlg::SaveTrainerSnapshot
=========================================================*/
/* #FN#
   Static. Saves trainer information to a given file */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
CCheatDlg::
SaveTrainerSnapshot(
	LPCSTR pszFileName,
	int    nMemo,
	int    nSearch,
	int    nLock,
	BOOL   bLock,
	PDWORD pValues,
	int    nCount
)
{
	BOOL bResult = FALSE;
	int  iFile;

	iFile = _open( pszFileName, O_CREAT | O_RDWR | O_TRUNC | O_BINARY, 0777 );
	if( iFile != -1 )
	{
		if( _write( iFile, "A8T", 3 ) != -1 )
		{
			UCHAR ucData = CHEATSNAPSHOT_VERSION;

			if( _write( iFile, &ucData, 1 ) != -1 )
			{
				int anParams[ 4 ] = { nMemo, nSearch, nLock, (int)bLock };
				if( _write( iFile, &anParams, 4 * sizeof(int) ) != -1 )
				{
					ucData = nCount;
					if( _write( iFile, &ucData, 1 ) != -1 )
					{
						if( _write( iFile, pValues, ucData * sizeof(DWORD) ) != -1 )
						{
							bResult = TRUE;
						}
					}
				}
			}
		}
		_close( iFile );
	}
	return bResult;

} /* #OF# CCheatDlg::SaveTrainerSnapshot */

/*========================================================
Method   : CCheatDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CCheatDlg::
SetDlgState()
{
	BOOL bChecked = CheckedItemsCount( TRUE ) > 0;

	ASSERT(NULL != m_pCheatServer);

	/* Set up collision group state */
	_SetChkBttn( IDC_CHEAT_DISABLECOLLISIONS, _IsFlagSet( m_ulMiscState, MS_DISABLE_COLLISIONS ) );

	_SetChkBttn( IDC_CHEAT_PLAYERPLAYER,      _IsFlagSet( m_ulCollisions, DC_PLAYER_PLAYER     ) );
	_EnableCtrl( IDC_CHEAT_PLAYERPLAYER,      _IsFlagSet( m_ulMiscState, MS_DISABLE_COLLISIONS ) );
	_SetChkBttn( IDC_CHEAT_PLAYERPLAYFIELD,   _IsFlagSet( m_ulCollisions, DC_PLAYER_PLAYFIELD  ) );
	_EnableCtrl( IDC_CHEAT_PLAYERPLAYFIELD,   _IsFlagSet( m_ulMiscState, MS_DISABLE_COLLISIONS ) );
	_SetChkBttn( IDC_CHEAT_MISSILEPLAYER,     _IsFlagSet( m_ulCollisions, DC_MISSILE_PLAYER    ) );
	_EnableCtrl( IDC_CHEAT_MISSILEPLAYER,     _IsFlagSet( m_ulMiscState, MS_DISABLE_COLLISIONS ) );
	_SetChkBttn( IDC_CHEAT_MISSILEPLAYFIELD,  _IsFlagSet( m_ulCollisions, DC_MISSILE_PLAYFIELD ) );
	_EnableCtrl( IDC_CHEAT_MISSILEPLAYFIELD,  _IsFlagSet( m_ulMiscState, MS_DISABLE_COLLISIONS ) );

	/* Set up trainer group state */
	_SetChkBttn( IDC_CHEAT_LOCK_CHECK,        _IsFlagSet( m_ulMiscState, MS_CHEAT_LOCK ) );
	_EnableCtrl( IDC_CHEAT_LOCK_CHECK,        bChecked );

	_EnableCtrl( IDC_CHEAT_LOCK,              bChecked && _IsFlagSet( m_ulMiscState, MS_CHEAT_LOCK ) );
	_EnableCtrl( IDC_CHEAT_LOCK_SPIN,         bChecked && _IsFlagSet( m_ulMiscState, MS_CHEAT_LOCK ) );
	_EnableCtrl( IDC_CHEAT_SEARCH_BUTTON,     (BOOL)m_pCheatServer->GetMemoCount() );
	_EnableCtrl( IDC_CHEAT_SEARCH,            (BOOL)m_pCheatServer->GetMemoCount() );
	_EnableCtrl( IDC_CHEAT_SEARCH_SPIN,       (BOOL)m_pCheatServer->GetMemoCount() );
	_EnableCtrl( IDC_CHEAT_SAVE,              bChecked );

	m_listPoke.EnableWindow( m_listPoke.GetItemCount() > 0 );

	SetDlgItemInt( IDC_CHEAT_MEMO,   m_nMemo,   FALSE );
	SetDlgItemInt( IDC_CHEAT_SEARCH, m_nSearch, FALSE );
	SetDlgItemInt( IDC_CHEAT_LOCK,   m_nLock,   FALSE );

#ifdef WIN_NETWORK_GAMES
	/* The cheat server is reset when starting a network game */
	if( ST_KAILLERA_ACTIVE )
	{
		_EnableCtrl( IDC_CHEAT_DISABLECOLLISIONS, FALSE );
		_EnableCtrl( IDC_CHEAT_MEMO_BUTTON,       FALSE );
		_EnableCtrl( IDC_CHEAT_MEMO,              FALSE );
		_EnableCtrl( IDC_CHEAT_MEMO_SPIN,         FALSE );
		_EnableCtrl( IDC_CHEAT_LOAD,              FALSE );
	}
#endif
} /* #OF# CCheatDlg::SetDlgState */

/*========================================================
Method   : CCheatDlg::CheckedItemsCount
=========================================================*/
/* #FN#
   Counts the number of checked items of the list */
int
/* #AS#
   Number of checked items */
CCheatDlg::
CheckedItemsCount(
	BOOL bCheckIfAny /*=FALSE*/
)
{
	int nCount = 0;

	for( int i = 0; i < m_listPoke.GetItemCount(); i++ )
	{
		if( m_listPoke.GetCheck( i ) )
		{
			nCount++;
			if( bCheckIfAny )
				break;
		}
	}
	return nCount;

} /* #OF# CCheatDlg::CheckedItemsCount */

/*========================================================
Method   : CCheatDlg::EnableListUpdates
=========================================================*/
/* #FN#
   Enables or disables list control redrawing */
void
/* #AS#
   Nothing */
CCheatDlg::
EnableListUpdates(
	BOOL bEnable /*=TRUE*/
)
{
	m_listPoke.SetRedraw( bEnable );
	/* Show the changes */
	if( bEnable )
		m_listPoke.Invalidate();

	m_bListNotify = bEnable;

} /* #OF# CCheatDlg::EnableListUpdates */

/*========================================================
Method   : CCheatDlg::SetTrainerFileName
=========================================================*/
/* #FN#
   Establishes name of the network game */
BOOL
/* #AS#
   TRUE if the game name has been determined, otherwise FALSE */
CCheatDlg::
SetTrainerFileName()
{
	const int nBufferLen = max(MAX_PATH, FILENAME_MAX);
	char szFilePath[ nBufferLen + 1 ];
	UINT unFileType = IAF_ATARI_FILE;

	BOOL bImageAttached = FALSE;

	if( GetBootFileInfo( szFilePath, nBufferLen, &unFileType ) )
	{
		char szTemp[ _MAX_PATH  + 1 ];
		char szFile[ _MAX_FNAME + 1 ];

		/* Split disk full path */
		_tsplitpath( szFilePath, szTemp, szTemp, szFile, szTemp );
		_strncpy( m_szTrainerFile, szFile, MAX_PATH );
		/* Add an extension to the file name */
		strcat( m_szTrainerFile, ".a8t" );

		bImageAttached = TRUE;
	}
	else
		strcpy( m_szTrainerFile, DEFAULT_A8T );

	return bImageAttached;

} /* #OF# CCheatDlg::SetTrainerFileName */


/////////////////////////////////////////////////////////////////////////////
// CCheatDlg message handlers

/*========================================================
Method   : CCheatDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CCheatDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	m_ulMiscState  = g_Misc.ulState;
	m_ulCollisions = g_Misc.Cheat.ulCollisions;
	m_nMemo        = g_Misc.Cheat.nMemo;
	m_nSearch      = g_Misc.Cheat.nSearch;
	m_nLock        = g_Misc.Cheat.nLock;

	SetTrainerFileName();

	/* We need a list control with checkboxes */
	ListView_SetExtendedListViewStyle( m_listPoke.m_hWnd, LVS_EX_CHECKBOXES );

	/* Fixed fonts would be appreciated */
	if( m_bFontCreated )
		m_listPoke.SetFont( &m_fontList );

	/* Populate the address list control */
	_CursorBusy();

	EnableListUpdates( FALSE );
	m_pCheatServer->ListToCtrl( &m_listPoke );
	EnableListUpdates();

	_CursorFree();

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CCheatDlg::OnInitDialog */

/*========================================================
Method   : CCheatDlg::OnItemchangedPokeList
=========================================================*/
/* #FN#
   Called by framework when a item of the list control changes */
void
/* #AS#
   Nothing */
CCheatDlg::
OnItemchangedPokeList(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	if( m_bListNotify )
	{
		NM_LISTVIEW *pNMListView = (NM_LISTVIEW*)pNMHDR;

		/* We are looking only for changes of state of a check box */
		if( (pNMListView->uNewState & LVIS_STATEIMAGEMASK) !=
			(pNMListView->uOldState & LVIS_STATEIMAGEMASK) )
		{
			m_bTrainerChanged = TRUE;
			SetDlgState();
		}
	}
	*pResult = 0;

} /* #OF# CCheatDlg::OnItemchangedPokeList */

/*========================================================
Method   : CCheatDlg::OnDisableCollisions
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CCheatDlg::
OnDisableCollisions()
{
	_ClickButton( IDC_CHEAT_DISABLECOLLISIONS, m_ulMiscState, MS_DISABLE_COLLISIONS );
	/* Set the activity of other controls of the collisions group */
	SetDlgState();

} /* #OF# CCheatDlg::OnDisableCollisions */

/*========================================================
Method   : CCheatDlg::OnPlayerPlayer
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CCheatDlg::
OnPlayerPlayer()
{
	_ClickButton( IDC_CHEAT_PLAYERPLAYER, m_ulCollisions, DC_PLAYER_PLAYER );

} /* #OF# CCheatDlg::OnPlayerPlayer */

/*========================================================
Method   : CCheatDlg::OnPlayerPlayfield
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CCheatDlg::
OnPlayerPlayfield()
{
	_ClickButton( IDC_CHEAT_PLAYERPLAYFIELD, m_ulCollisions, DC_PLAYER_PLAYFIELD );

} /* #OF# CCheatDlg::OnPlayerPlayfield */

/*========================================================
Method   : CCheatDlg::OnMissilePlayer
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CCheatDlg::
OnMissilePlayer()
{
	_ClickButton( IDC_CHEAT_MISSILEPLAYER, m_ulCollisions, DC_MISSILE_PLAYER );

} /* #OF# CCheatDlg::OnMissilePlayer */

/*========================================================
Method   : CCheatDlg::OnMissilePlayfield
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CCheatDlg::
OnMissilePlayfield()
{
	_ClickButton( IDC_CHEAT_MISSILEPLAYFIELD, m_ulCollisions, DC_MISSILE_PLAYFIELD );

} /* #OF# CCheatDlg::OnMissilePlayfield */

/*========================================================
Method   : CCheatDlg::OnMemoButton
=========================================================*/
/* #FN#
   Builds a list of addresses containing a given value */
void
/* #AS#
   Nothing */
CCheatDlg::
OnMemoButton()
{
	_CursorBusy();

	m_pCheatServer->ShotMemo( m_nMemo, MEMORY_mem, 65536 );
	m_bTrainerChanged = TRUE;

	EnableListUpdates( FALSE );
	m_listPoke.DeleteAllItems();
	EnableListUpdates();

	_CursorFree();

	SetDlgState();

} /* #OF# CCheatDlg::OnMemoButton */

/*========================================================
Method   : CCheatDlg::OnSearchButton
=========================================================*/
/* #FN#
   Builds a list of addresses with changed values */
void
/* #AS#
   Nothing */
CCheatDlg::
OnSearchButton()
{
	_CursorBusy();

	m_pCheatServer->ShotAddr( m_nSearch );
	m_bTrainerChanged = TRUE;

	EnableListUpdates( FALSE );
	m_pCheatServer->ListToCtrl( &m_listPoke );
	EnableListUpdates();

	_CursorFree();

	SetDlgState();

} /* #OF# CCheatDlg::OnSearchButton */

/*========================================================
Method   : CCheatDlg::OnDeltaposMemoSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CCheatDlg::
OnDeltaposMemoSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_CHEAT_MEMO, m_nMemo, MIN_MEMO, MAX_MEMO );
	*pResult = 0;

} /* #OF# CCheatDlg::OnDeltaposMemoSpin */

/*========================================================
Method   : CCheatDlg::OnKillfocusMemo
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CCheatDlg::
OnKillfocusMemo()
{
	_KillfocusSpin( IDC_CHEAT_MEMO, m_nMemo, MIN_MEMO, MAX_MEMO );

} /* #OF# CCheatDlg::OnKillfocusMemo */

/*========================================================
Method   : CCheatDlg::OnDeltaposSearchSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CCheatDlg::
OnDeltaposSearchSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_CHEAT_SEARCH, m_nSearch, MIN_SEARCH, MAX_SEARCH );
	*pResult = 0;

} /* #OF# CCheatDlg::OnDeltaposSearchSpin */

/*========================================================
Method   : CCheatDlg::OnKillfocusSearch
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CCheatDlg::
OnKillfocusSearch()
{
	_KillfocusSpin( IDC_CHEAT_SEARCH, m_nSearch, MIN_SEARCH, MAX_SEARCH );

} /* #OF# CCheatDlg::OnKillfocusSearch */

/*========================================================
Method   : CCheatDlg::OnLockCheck
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CCheatDlg::
OnLockCheck()
{
	_ClickButton( IDC_CHEAT_LOCK_CHECK, m_ulMiscState, MS_CHEAT_LOCK );
	/* Set the activity of the edit and spin lock controls */
	SetDlgState();

} /* #OF# CCheatDlg::OnLockCheck */

/*========================================================
Method   : CCheatDlg::OnDeltaposLockSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CCheatDlg::
OnDeltaposLockSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_CHEAT_LOCK, m_nLock, MIN_LOCK, MAX_LOCK );
	*pResult = 0;

} /* #OF# CCheatDlg::OnDeltaposLockSpin */

/*========================================================
Method   : CCheatDlg::OnKillfocusLock
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CCheatDlg::
OnKillfocusLock()
{
	_KillfocusSpin( IDC_CHEAT_LOCK, m_nLock, MIN_LOCK, MAX_LOCK );

} /* #OF# CCheatDlg::OnKillfocusLock */

/*========================================================
Method   : CCheatDlg::OnLoadButton
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
CCheatDlg::
OnLoadButton()
{
	if( PickFileName( TRUE, m_szTrainerFile, IDS_SELECT_A8T_LOAD, IDS_FILTER_A8T,
					  "a8t", PF_LOAD_FLAGS, FALSE, DEFAULT_A8T, this ) &&
		*m_szTrainerFile != '\0' )
	{
		PDWORD pValues = NULL;

		int  nMemo = 0, nSearch = 0, nLock = 0, nCount = 0;
		BOOL bLock = FALSE;

		_CursorBusy();

		/* Read memory addresses from a trainer information file */
		if( !ReadTrainerSnapshot( m_szTrainerFile, &nMemo, &nSearch, &nLock, &bLock, &pValues, &nCount ) )
		{
			DisplayMessage( GetSafeHwnd(), IDS_ERROR_A8T_LOAD, 0, MB_ICONEXCLAMATION | MB_OK );
		}
		else
		{
			/* Send the addresses to the cheat server */
			m_pCheatServer->ImportAddr( pValues, nCount, MEMORY_mem );
			/* Release the buffer allocated by ReadTrainerSnapshot */
			free( pValues );

			_ModifyFlag( bLock, m_ulMiscState, MS_CHEAT_LOCK );
			m_nMemo   = nMemo;
			m_nSearch = nSearch;
			m_nLock   = nLock;
			
			/* Populate the list control basing on the server state */
			EnableListUpdates( FALSE );
			m_pCheatServer->ListToCtrl( &m_listPoke );
			EnableListUpdates();

			m_bTrainerChanged = TRUE;
			SetDlgState();
		}
		_CursorFree();
	}
} /* #OF# CCheatDlg::OnLoadButton */

/*========================================================
Method   : CCheatDlg::OnSaveButton
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
CCheatDlg::
OnSaveButton()
{
	int nCheckedCount = CheckedItemsCount( FALSE );

	if( nCheckedCount > 0 )
	{
		if( PickFileName( FALSE, m_szTrainerFile, IDS_SELECT_A8T_SAVE, IDS_FILTER_A8T,
						  "a8t", PF_SAVE_FLAGS, FALSE, DEFAULT_A8T, this ) &&
			*m_szTrainerFile != '\0' )
		{
			PDWORD pValues = NULL;
			int    nCount  = 0;

			_CursorBusy();

			/* Update the list of locked registers */
			if( m_bTrainerChanged )
				m_pCheatServer->CtrlToList( &m_listPoke );

			/* Receive locked memory addresses from the cheat server */
			m_pCheatServer->ExportAddr( &pValues, &nCount );
			ASSERT(nCount == nCheckedCount);

			/* Save the addresses into a trainer information file */
			if( !SaveTrainerSnapshot( m_szTrainerFile, m_nMemo, m_nSearch, m_nLock, 0 != (m_ulMiscState & MS_CHEAT_LOCK), pValues, nCount ) )
			{
				DisplayMessage( GetSafeHwnd(), IDS_ERROR_A8T_SAVE, 0, MB_ICONEXCLAMATION | MB_OK );
			}
			/* Release the buffer allocated by CCheatServer::ExportAddr */
			free( pValues );

			_CursorFree();
		}
	}
} /* #OF# CCheatDlg::OnSaveButton */

/*========================================================
Method   : CCheatDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CCheatDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	switch( nCtrlID )
	{
		case IDC_CHEAT_MEMO:
			OnKillfocusMemo();
			break;
		case IDC_CHEAT_SEARCH:
			OnKillfocusSearch();
			break;
		case IDC_CHEAT_LOCK:
			OnKillfocusLock();
			break;
	}
} /* #OF# CCheatDlg::ReceiveFocused */

/*========================================================
Method   : CCheatDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CCheatDlg::
OnOK()
{
	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	/* Update the list of locked registers */
	if( m_bTrainerChanged )
	{
		_CursorBusy();
		m_pCheatServer->CtrlToList( &m_listPoke );
		_CursorFree();
	}
	if( m_ulCollisions != g_Misc.Cheat.ulCollisions )
	{
		g_Misc.Cheat.ulCollisions = m_ulCollisions;
		WriteRegDWORD( NULL, REG_CHEAT_COLLISIONS, g_Misc.Cheat.ulCollisions );
	}
	if( m_nMemo != g_Misc.Cheat.nMemo )
	{
		g_Misc.Cheat.nMemo = m_nMemo;
		WriteRegDWORD( NULL, REG_CHEAT_MEMO, g_Misc.Cheat.nMemo );
	}
	if( m_nSearch != g_Misc.Cheat.nSearch )
	{
		g_Misc.Cheat.nSearch = m_nSearch;
		WriteRegDWORD( NULL, REG_CHEAT_SEARCH, g_Misc.Cheat.nSearch );
	}
	if( m_nLock != g_Misc.Cheat.nLock )
	{
		g_Misc.Cheat.nLock = m_nLock;
		WriteRegDWORD( NULL, REG_CHEAT_LOCK, g_Misc.Cheat.nLock );
	}
	if( m_ulMiscState != g_Misc.ulState )
	{
		g_Misc.ulState = m_ulMiscState;
		WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );
	}
	Misc_UpdateCollisions();

	CCommonDlg::OnOK();
} /* #OF# CCheatDlg::OnOK */
