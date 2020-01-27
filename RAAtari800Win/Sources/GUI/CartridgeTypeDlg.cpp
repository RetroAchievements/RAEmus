/****************************************************************************
File    : CartridgeTypeDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CCartridgeTypeDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 23.12.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "CartridgeTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_CARTRIDGETYPE_FIRST		IDC_CARTRIDGETYPE_CARTRIDGE
#define IDC_CARTRIDGETYPE_LAST		IDC_CARTRIDGETYPE_CANCEL


/////////////////////////////////////////////////////////////////////////////
// Static objects

static CCartridgeTypeDlg::CartTypeInfo_t s_aCartTypeInfo[] =
{
	{ "Standard 8 KB cartridge",          CARTRIDGE_STD_8,       8,    IDS_CART_STD_8       },
	{ "Standard 16 KB cartridge",         CARTRIDGE_STD_16,      16,   IDS_CART_STD_16      },
	{ "One chip 16 KB 5200 cartridge",    CARTRIDGE_5200_NS_16,  16,   IDS_CART_5200_NS_16  },
	{ "Two chip 16 KB 5200 cartridge",    CARTRIDGE_5200_EE_16,  16,   IDS_CART_5200_EE_16  },
	{ "Standard 32 KB 5200 cartridge",    CARTRIDGE_5200_32,     32,   IDS_CART_5200_32     },
	{ "Bounty Bob 40 KB 5200 cartridge",  CARTRIDGE_5200_40,     40,   IDS_CART_5200_40     },
	{ "DB 32 KB cartridge",               CARTRIDGE_DB_32,       32,   IDS_CART_DB_32       },
	{ "Williams 64 KB cartridge",         CARTRIDGE_WILL_64,     64,   IDS_CART_WILL_64     },
	{ "Express 64 KB cartridge",          CARTRIDGE_EXP_64,      64,   IDS_CART_EXP_64      },
	{ "Diamond 64 KB cartridge",          CARTRIDGE_DIAMOND_64,  64,   IDS_CART_DIAMOND_64  },
	{ "SpartaDOS X 64 KB cartridge",      CARTRIDGE_SDX_64,      64,   IDS_CART_SDX_64      },
	{ "SpartaDOS X 128 KB cartridge",     CARTRIDGE_SDX_128,     128,  IDS_CART_SDX_128     },
	{ "XEGS 32 KB cartridge",             CARTRIDGE_XEGS_32,     32,   IDS_CART_XEGS_32     },
	{ "XEGS 64 KB cartridge",             CARTRIDGE_XEGS_64,     64,   IDS_CART_XEGS_64     },
	{ "XEGS 128 KB cartridge",            CARTRIDGE_XEGS_128,    128,  IDS_CART_XEGS_128    },
	{ "OSS '034M' 16 KB cartridge",       CARTRIDGE_OSS_16,      16,   IDS_CART_OSS_16      },
	{ "OSS 'M091' 16 KB cartridge",       CARTRIDGE_OSS2_16,     16,   IDS_CART_OSS2_16     },
	{ "Atrax 128 KB cartridge",           CARTRIDGE_ATRAX_128,   128,  IDS_CART_ATRAX_128   },
	{ "Bounty Bob 40 KB cartridge",       CARTRIDGE_BBSB_40,     40,   IDS_CART_BBSB_40     },
	{ "Standard 8 KB 5200 cartridge",     CARTRIDGE_5200_8,      8,    IDS_CART_5200_8      },
	{ "Standard 4 KB 5200 cartridge",     CARTRIDGE_5200_4,      4,    IDS_CART_5200_4      },
	{ "Right slot 8 KB cartridge",        CARTRIDGE_RIGHT_8,     8,    IDS_CART_RIGHT_8     },
	{ "Williams 32 KB cartridge",         CARTRIDGE_WILL_32,     32,   IDS_CART_WILL_32     },
	{ "XEGS 256 KB cartridge",            CARTRIDGE_XEGS_256,    256,  IDS_CART_XEGS_256    },
	{ "XEGS 512 KB cartridge",            CARTRIDGE_XEGS_512,    512,  IDS_CART_XEGS_512    },
	{ "XEGS 1 MB cartridge",              CARTRIDGE_XEGS_1024,   1024, IDS_CART_XEGS_1024   },
	{ "MegaCart 16 KB cartridge",         CARTRIDGE_MEGA_16,     16,   IDS_CART_MEGA_16     },
	{ "MegaCart 32 KB cartridge",         CARTRIDGE_MEGA_32,     32,   IDS_CART_MEGA_32     },
	{ "MegaCart 64 KB cartridge",         CARTRIDGE_MEGA_64,     64,   IDS_CART_MEGA_64     },
	{ "MegaCart 128 KB cartridge",        CARTRIDGE_MEGA_128,    128,  IDS_CART_MEGA_128    },
	{ "MegaCart 256 KB cartridge",        CARTRIDGE_MEGA_256,    256,  IDS_CART_MEGA_256    },
	{ "MegaCart 512 KB cartridge",        CARTRIDGE_MEGA_512,    512,  IDS_CART_MEGA_512    },
	{ "MegaCart 1 MB cartridge",          CARTRIDGE_MEGA_1024,   1024, IDS_CART_MEGA_1024   },
	{ "Switchable XEGS 32 KB cartridge",  CARTRIDGE_SWXEGS_32,   32,   IDS_CART_SWXEGS_32   },
	{ "Switchable XEGS 64 KB cartridge",  CARTRIDGE_SWXEGS_64,   64,   IDS_CART_SWXEGS_64   },
	{ "Switchable XEGS 128 KB cartridge", CARTRIDGE_SWXEGS_128,  128,  IDS_CART_SWXEGS_128  },
	{ "Switchable XEGS 256 KB cartridge", CARTRIDGE_SWXEGS_256,  256,  IDS_CART_SWXEGS_256  },
	{ "Switchable XEGS 512 KB cartridge", CARTRIDGE_SWXEGS_512,  512,  IDS_CART_SWXEGS_512  },
	{ "Switchable XEGS 1 MB cartridge",   CARTRIDGE_SWXEGS_1024, 1024, IDS_CART_SWXEGS_1024 },
	{ "Phoenix 8 KB cartridge",           CARTRIDGE_PHOENIX_8,   8,    IDS_CART_PHOENIX_8   },
	{ "Blizzard 16 KB cartridge",         CARTRIDGE_BLIZZARD_16, 16,   IDS_CART_BLIZZARD_16 },
	{ "Atarimax 1 Mb Flash cartridge",    CARTRIDGE_ATMAX_128,   128,  IDS_CART_ATMAX_128   },
	{ "Atarimax 8 Mb Flash cartridge",    CARTRIDGE_ATMAX_1024,  1024, IDS_CART_ATMAX_1024  }
};

static const int s_nCartTypeInfoNo = sizeof(s_aCartTypeInfo)/sizeof(s_aCartTypeInfo[0]);


/////////////////////////////////////////////////////////////////////////////
// CCartridgeTypeDlg dialog

BEGIN_MESSAGE_MAP(CCartridgeTypeDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CCartridgeTypeDlg)
	ON_CBN_SELCHANGE(IDC_CARTRIDGETYPE_CARTRIDGE, OnSelchangeCartridge)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CARTRIDGETYPE_OK, CCommonDlg::OnOK)
	ON_BN_CLICKED(IDC_CARTRIDGETYPE_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CCartridgeTypeDlg::CCartridgeTypeDlg
=========================================================*/
/* #FN#
   Standard constructor */
CCartridgeTypeDlg::
CCartridgeTypeDlg(
	int   nCartSize,
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: m_nCartSize( nCartSize ), CCommonDlg( CCartridgeTypeDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CCartridgeTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pCartTypeInfo = s_aCartTypeInfo;
	m_nCartType     = CARTRIDGE_NONE;
	m_nFirstCtrl    = IDC_CARTRIDGETYPE_FIRST;
	m_nLastCtrl     = IDC_CARTRIDGETYPE_LAST;

} /* #OF# CCartridgeTypeDlg::CCartridgeTypeDlg */

/*========================================================
Method   : CCartridgeTypeDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CCartridgeTypeDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CCartridgeTypeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CCartridgeTypeDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CCartridgeTypeDlg implementation

/*========================================================
Method   : CCartridgeTypeDlg::CountTypes
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
int
/* #AS#
   Number of known cartridge types of a given size */
CCartridgeTypeDlg::
CountTypes(
	int nCartSize
)
{
	int nItems = 0;
	int nIndex = 0;

	ASSERT(CARTRIDGE_LAST_SUPPORTED == s_nCartTypeInfoNo);

	for( int i = 0; i < s_nCartTypeInfoNo; i++ )
	{
		if( m_pCartTypeInfo[ i ].nCartSize == nCartSize &&
			(CARTRIDGE_IsFor5200( m_pCartTypeInfo[ i ].nCartType ) && Atari800_MACHINE_5200 == Atari800_machine_type ||
			!CARTRIDGE_IsFor5200( m_pCartTypeInfo[ i ].nCartType ) && Atari800_MACHINE_5200 != Atari800_machine_type) )
		{
			nIndex = i;
			nItems++;
		}
	}
	if( 1 == nItems )
		m_nCartType = m_pCartTypeInfo[ nIndex ].nCartType;

	return nItems;

} /* #OF# CCartridgeTypeDlg::CountTypes */

/*========================================================
Method   : CCartridgeTypeDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CCartridgeTypeDlg::
SetDlgState()
{
	CComboBox *pCombo = (CComboBox *)GetDlgItem( IDC_CARTRIDGETYPE_CARTRIDGE );
	ASSERT(NULL != pCombo);

	if( !m_bStateInit )
	{
		int nIndex = 0;
		ASSERT(CARTRIDGE_LAST_SUPPORTED == s_nCartTypeInfoNo);

		pCombo->ResetContent();
		for( int i = 0; i < s_nCartTypeInfoNo; i++ )
		{
			if( m_pCartTypeInfo[ i ].nCartSize == m_nCartSize &&
				(CARTRIDGE_IsFor5200( m_pCartTypeInfo[ i ].nCartType ) && Atari800_MACHINE_5200 == Atari800_machine_type ||
				!CARTRIDGE_IsFor5200( m_pCartTypeInfo[ i ].nCartType ) && Atari800_MACHINE_5200 != Atari800_machine_type) )
			{
				nIndex = pCombo->AddString( m_pCartTypeInfo[ i ].pszCartName );
				pCombo->SetItemData( nIndex, m_pCartTypeInfo[ i ].nCartType );
			}
		}
		pCombo->SetCurSel( 0 );
	}
	/* Set cartridge description */
	for( int i = 0; i < s_nCartTypeInfoNo; i++ )
		if( m_pCartTypeInfo[ i ].nCartType == m_nCartType )
		{
			CWnd *pStatic = GetDlgItem( IDC_CARTRIDGETYPE_DESCRIPTION );
			ASSERT(NULL != pStatic);
			char szCartDesc[ 1024 ];

			pStatic->SetWindowText( LoadStringEx( m_pCartTypeInfo[ i ].nCartDesc, szCartDesc, 1024 ) );
			break;
		}
	m_bStateInit = TRUE;

} /* #OF# CCartridgeTypeDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CCartridgeTypeDlg message handlers

/*========================================================
Method   : CCartridgeTypeDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CCartridgeTypeDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	SetDlgState();
	OnSelchangeCartridge();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CCartridgeTypeDlg::OnInitDialog */

/*========================================================
Method   : CCartridgeTypeDlg::OnSelchangeCartridge
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CCartridgeTypeDlg::
OnSelchangeCartridge()
{
	CComboBox *pCombo = (CComboBox *)GetDlgItem( IDC_CARTRIDGETYPE_CARTRIDGE );
	int nIndex = 0;

	ASSERT(NULL != pCombo);

	if( CB_ERR == (nIndex = pCombo->GetCurSel()) )
	{
		pCombo->SetCurSel( 0 );
		nIndex = 0;
	}
	m_nCartType = pCombo->GetItemData( nIndex );

	SetDlgState();

} /* #OF# CCartridgeTypeDlg::OnSelchangeCartridge */
