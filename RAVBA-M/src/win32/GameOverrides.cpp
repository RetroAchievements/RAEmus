#include "stdafx.h"
#include "vba.h"
#include "GameOverrides.h"
#include "../gba/GBA.h"
#include "RA_Interface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// GameOverrides dialog


GameOverrides::GameOverrides(CWnd* pParent /*=NULL*/)
  : CDialog(GameOverrides::IDD, pParent)
{
  //{{AFX_DATA_INIT(GameOverrides)
  // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}


void GameOverrides::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GameOverrides)
	DDX_Control(pDX, IDC_NAME, m_name);
	DDX_Control(pDX, IDC_MIRRORING, m_mirroring);
	DDX_Control(pDX, IDC_FLASH_SIZE, m_flashSize);
	DDX_Control(pDX, IDC_SAVE_TYPE, m_saveType);
	DDX_Control(pDX, IDC_RTC, m_rtc);
	DDX_Control(pDX, IDC_COMMENT, m_comment);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(GameOverrides, CDialog)
  //{{AFX_MSG_MAP(GameOverrides)
  ON_BN_CLICKED(IDC_DEFAULTS, OnDefaults)
  //}}AFX_MSG_MAP
  END_MESSAGE_MAP()

  /////////////////////////////////////////////////////////////////////////////
// GameOverrides message handlers

void GameOverrides::OnOK()
{
  TCHAR tempName[2048];
  GetModuleFileName(NULL, tempName, 2048);

  //	Find first backslash, replace with '\0'
  TCHAR* p = _tcsrchr( tempName, '\\');
  if( p )
    *p = '\0';

  char buffer[5];
  strncpy(buffer, (const char *)&rom[0xac], 4);
  buffer[4] = 0;

  _tcscat_s(tempName, 2048, _T( "\\vba-over.ini" ) );

  TCHAR comment[0xFF];
  m_comment.GetWindowText(comment, 0xFF);
  WritePrivateProfileString( StrToWideStr( buffer ).c_str(), _T( "comment" ), !_tcsnccmp( comment, _T( "" ), 0xFF) ? NULL : comment, tempName);

  int rtc = m_rtc.GetCurSel();
  int flash = m_flashSize.GetCurSel();
  int save = m_saveType.GetCurSel();
  int mirroring = m_mirroring.GetCurSel();
  if(rtc == 0 && flash == 0 && save == 0 && mirroring == 0)
    WritePrivateProfileString( StrToWideStr( buffer ).c_str(), NULL, NULL, tempName );
  else {
    TCHAR* value = NULL;
    switch(rtc) {
    case 1:
      value = _T("0");
      break;
    case 2:
      value = _T("1");
      break;
    }
    WritePrivateProfileString(StrToWideStr( buffer ).c_str(), _T("rtcEnabled"), value, tempName);
    value = NULL;
    switch(flash) {
    case 1:
      value = _T("0x10000");
      break;
    case 2:
      value = _T("0x20000");
      break;
    }
    WritePrivateProfileString(StrToWideStr( buffer ).c_str(), _T("flashSize"), value, tempName);
    value = NULL;
    switch(save) {
    case 1:
      value = _T("0");
      break;
    case 2:
      value = _T("1");
      break;
    case 3:
      value = _T("2");
      break;
    case 4:
      value = _T("3");
      break;
    case 5:
      value = _T("4");
      break;
    case 6:
      value = _T("5");
      break;
    }
    WritePrivateProfileString(StrToWideStr( buffer ).c_str(), _T( "saveType" ), value, tempName);
    value = NULL;
    switch(mirroring) {
    case 1:
      value = _T("0");
      break;
    case 2:
      value = _T("1");
      break;
    }
    WritePrivateProfileString(StrToWideStr( buffer ).c_str(), _T( "mirroringEnabled" ), value, tempName);
  }
  CDialog::OnOK();
}

void GameOverrides::OnDefaults()
{
  m_rtc.SetCurSel(0);
  m_flashSize.SetCurSel(0);
  m_saveType.SetCurSel(0);
  m_mirroring.SetCurSel(0);
}

void GameOverrides::OnCancel()
{
  CDialog::OnCancel();
}

BOOL GameOverrides::OnInitDialog()
{
  CDialog::OnInitDialog();

  const TCHAR* rtcValues[] = {
    _T("Default"),
    _T("Disabled"),
    _T("Enabled")
  };
  const TCHAR* flashValues[] = {
    _T("Default"),
    _T("64K"),
    _T("128K")
  };
  const TCHAR* saveValues[] = {
    _T("Default"),
    _T("Automatic"),
    _T("EEPROM"),
    _T("SRAM"),
    _T("Flash"),
    _T("EEPROM+Sensor"),
    _T("None")
  };
  const TCHAR* mirroringValues[] = {
    _T("Default"),
    _T("Disabled"),
    _T("Enabled")
  };

  int i;

  for(i = 0; i < 3; i++) {
    m_rtc.AddString(rtcValues[i]);
  }
  for(i = 0; i < 3; i++) {
    m_flashSize.AddString(flashValues[i]);
  }
  for(i = 0; i < 7; i++) {
    m_saveType.AddString(saveValues[i]);
  }
  for(i = 0; i < 3; i++) {
    m_mirroring.AddString(mirroringValues[i]);
  }
  
  TCHAR tempName[2048];
  GetModuleFileName(NULL, tempName, 2048);
  
  TCHAR* p = _tcsrchr(tempName, '\\');
  if(p)
    *p = 0;

  char buffer[5];
  strncpy(buffer, (const char *)&rom[0xac], 4);
  buffer[4] = 0;

  _tcscat_s(tempName, 2048, _T("\\vba-over.ini" ));

  m_name.SetWindowText( StrToWideStr( buffer ).c_str() );

  TCHAR comment[0xFF];
  GetPrivateProfileString(StrToWideStr( buffer ).c_str(),
	  _T("comment"),
	  _T(""),
	  comment,
	  0xFF,
	  tempName);
  m_comment.SetWindowText(comment);

  UINT v = GetPrivateProfileInt(StrToWideStr( buffer ).c_str(),
                                _T("rtcEnabled"),
                                -1,
                                tempName);
  switch(v) {
  case 0:
    m_rtc.SetCurSel(1);
    break;
  case 1:
    m_rtc.SetCurSel(2);
    break;
  default:
    m_rtc.SetCurSel(0);
  }
  v = GetPrivateProfileInt(StrToWideStr( buffer ).c_str(),
                           _T("flashSize"),
                           -1,
                           tempName);
  switch(v) {
  case 0x10000:
    m_flashSize.SetCurSel(1);
    break;
  case 0x20000:
    m_flashSize.SetCurSel(2);
    break;
  default:
    m_flashSize.SetCurSel(0);
  }

  v = GetPrivateProfileInt(StrToWideStr( buffer ).c_str(),
                           _T("saveType"),
                           -1,
                           tempName);
  if(v != (UINT)-1 && (v > 5))
    v = (UINT)-1;
  if(v != (UINT)-1)
    m_saveType.SetCurSel(v+1);
  else
    m_saveType.SetCurSel(0);

  v = GetPrivateProfileInt(StrToWideStr( buffer ).c_str(),
                           _T("mirroringEnabled"),
                            -1,
                            tempName);
  switch(v) {
  case 0:
    m_mirroring.SetCurSel(1);
    break;
  case 1:
    m_mirroring.SetCurSel(2);
    break;
  default:
    m_mirroring.SetCurSel(0);
  }

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}
