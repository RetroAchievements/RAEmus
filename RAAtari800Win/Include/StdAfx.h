// StdAfx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef __STDAFX_H__
#define __STDAFX_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN         // Exclude rarely-used stuff from Windows headers

#include <afxwin.h>          // MFC core and standard components
#include <afxext.h>          // MFC extensions
#include <afxtempl.h>        // MFC collections
#include <afxmt.h>			 // MFC Multithreaded Extensions (Syncronization Objects)
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>          // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <shlobj.h>
#include "ShellPidl.h"
#include "DlgBase.h"
#include "CommonDlg.h"

#include <io.h>
#include <fcntl.h>
#include <mmsystem.h>
#include <vfw.h>

#include "WinConfig.h"       // AtariWin configuration file
#include "atari800.h"        // Atari800 kernel that AtariWin is based on
#include "core.h"            // AtariWin core

#define HID_BASE_COMMAND    0x00010000UL    // ID and IDM
#define HID_BASE_RESOURCE   0x00020000UL    // IDR and IDD
#define HID_BASE_PROMPT     0x00030000UL    // IDP
#define HID_BASE_NCAREAS    0x00040000UL
#define HID_BASE_CONTROL    0x00050000UL    // IDC
#define HID_BASE_DISPATCH   0x00060000UL    // IDispatch help codes

#endif //__STDAFX_H__
