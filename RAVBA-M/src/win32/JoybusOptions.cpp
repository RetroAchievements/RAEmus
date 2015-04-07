#include "stdafx.h"

#ifndef NO_LINK

#include "vba.h"
#include "JoybusOptions.h"
#include "../gba/GBALink.h"
#include "RA_Interface.h"

// JoybusOptions dialog

IMPLEMENT_DYNAMIC(JoybusOptions, CDialog)

JoybusOptions::JoybusOptions(CWnd* pParent /*=NULL*/)
	: CDialog(JoybusOptions::IDD, pParent)
{
}

JoybusOptions::~JoybusOptions()
{
}

void JoybusOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_JOYBUS_ENABLE, enable_check);
	DDX_Control(pDX, IDC_JOYBUS_HOSTNAME, hostname);
}

BEGIN_MESSAGE_MAP(JoybusOptions, CDialog)
	ON_BN_CLICKED(IDC_JOYBUS_ENABLE, &JoybusOptions::OnBnClickedJoybusEnable)
	ON_BN_CLICKED(IDOK, &JoybusOptions::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL JoybusOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

	enable_check.SetCheck(gba_joybus_enabled ? BST_CHECKED : BST_UNCHECKED);

	hostname.EnableWindow(enable_check.GetCheck() == BST_CHECKED);
	hostname.SetWindowText( StrToWideStr(joybusHostAddr.ToString() ).c_str() );

	return TRUE;
}

void JoybusOptions::OnBnClickedJoybusEnable()
{
	hostname.EnableWindow(enable_check.GetCheck() == BST_CHECKED);
}

void JoybusOptions::OnBnClickedOk()
{
	if ( (hostname.GetWindowTextLength() == 0)
		&& (enable_check.GetCheck() == BST_CHECKED) )
	{
		hostname.SetWindowText( _T( "Enter IP or Hostname" ) );
		return;
	}

	gba_joybus_enabled = enable_check.GetCheck() == BST_CHECKED;

	CString address;
	hostname.GetWindowText(address);

	sf::IPAddress new_server;
	new_server = std::wstring(address);

	if (!new_server.IsValid())
	{
		hostname.SetWindowText( L"Enter IP or Hostname" );
		return;
	}

	joybusHostAddr = new_server;
	JoyBusConnect();

	OnOK();
}

#endif // NO_LINK
