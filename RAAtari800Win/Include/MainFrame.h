/****************************************************************************
File    : MainFrame.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CMainFrame class
@(#) #BY# Tomasz Szymankowski, Richard Lawrence
@(#) #LM# 05.11.2002
*/

#ifndef __MAINFRAME_H__
#define __MAINFRAME_H__

#if _MSC_VER >= 1000
#pragma once
#endif //_MSC_VER >= 1000

#include "StatusTray.h"

class CAtari800WinApp;
class CAtari800WinView;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame window

class CMainFrame : public CFrameWnd
{
// Construction/destruction
protected:
	// Create from serialization only
	DECLARE_DYNCREATE(CMainFrame)

	CMainFrame();
	virtual ~CMainFrame();

// Interface: operations
public:
	void SetMainView    ( CAtari800WinView *pMainView ) { m_pMainView = pMainView; };
	void UpdateStatus   ( BOOL bForceShow = FALSE, int nSpeed = -1, int nPane = ID_INDICATOR_RUN,
						  BOOL bWinMode = FALSE, BOOL bUpdateIndicator = TRUE );
	void UpdateSpeed    ( BOOL bForceShow = FALSE, int nSpeed = -1, BOOL bWinMode = FALSE );
	void UpdateIndicator( int nPane );
	void CleanScreen    ( BOOL bDeepClear = FALSE );

	static BOOL LaunchWizard( CWnd *pWnd, BOOL &bReboot );

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Interface: attributes
public:

// Implementation: operations
private:
	BOOL AutobootAtariImage( LPSTR pszFileName, BOOL bReinit = TRUE, BOOL bReboot = TRUE );

	void DriveInsert ( int nDriveNum );
	void DriveRemove ( int nDriveNum );
	void SaveState   ( int nSaveRom );
	void SaveSnapshot( int nInterlace );
	void SaveVideo   ( BOOL bWithSound );

	void MachineTypeChanged( int nDefaultSystem );
	void AutofireChanged   ( int nAutofireMode );
	void MouseDeviceChanged( int nMouseMode );
	void ArrowKeysChanged  ( int nArrowsMode );
	void StretchModeChanged( int nStretchMode );
	void MemorySizeChanged ( int nRamSize );

	void ConvertAtasciiToAscii( BOOL bA2PC );
	void ConvertRomToCart();
	void ConvertXfdToAtr ();
	void ConvertDcmToAtr ();

	void ChangeMouseDevice( BOOL bForward = TRUE );
	void ChangeAutofire   ( BOOL bForward = TRUE );
	void MouseButtonDblClk(	UINT nFlags, CPoint point, BOOL bForward = TRUE );

	void SetIndicatorDesc ( CPoint &point );

	void StartThread();
	void StopThread();
	void ResumeThread();
	void SuspendThread();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void ActivateFrame(int nCmdShow = -1);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation: attributes
protected:
	static CAtari800WinApp *m_pMainApp;
	CAtari800WinView *m_pMainView;

	// Control bar embedded members
	CStatusTray m_wndStatusBar;

private:
	CString m_strWindowTitle;
	CString m_strSoundName;
	CString m_strVideoName;
	CString m_strScreenshotName;
	BOOL    m_bPosChanging;

	CStatusBarCtrl *m_pSBCtrl;

	char m_szLabels[ 7 ][ LOADSTRING_SIZE_S + 1 ];

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFileBootAtariImage();
	afx_msg void OnFileLoadExecutable();
	afx_msg void OnFileAttachDiskImageDrive1();
	afx_msg void OnFileAttachDiskImageDrive2();
	afx_msg void OnFileAttachDiskImageDrive3();
	afx_msg void OnFileAttachDiskImageDrive4();
	afx_msg void OnFileAttachDiskImageDrive5();
	afx_msg void OnFileAttachDiskImageDrive6();
	afx_msg void OnFileAttachDiskImageDrive7();
	afx_msg void OnFileAttachDiskImageDrive8();
	afx_msg void OnFileDetachDiskImageAll();
	afx_msg void OnFileDetachDiskImageDrive1();
	afx_msg void OnFileDetachDiskImageDrive2();
	afx_msg void OnFileDetachDiskImageDrive3();
	afx_msg void OnFileDetachDiskImageDrive4();
	afx_msg void OnFileDetachDiskImageDrive5();
	afx_msg void OnFileDetachDiskImageDrive6();
	afx_msg void OnFileDetachDiskImageDrive7();
	afx_msg void OnFileDetachDiskImageDrive8();
	afx_msg void OnFileTurnDisk();
	afx_msg void OnFileAttachTapeImage();
	afx_msg void OnFileDetachTapeImage();
	afx_msg void OnFileAttachCartridgeImage();
	afx_msg void OnFileDetachCartridgeImage();
	afx_msg void OnFileReadState();
	afx_msg void OnFileSaveStateNormal();
	afx_msg void OnFileSaveStateVerbose();
	afx_msg void OnAtariMachineType();
	afx_msg void OnAtariMachineTypeOsa();
	afx_msg void OnAtariMachineTypeOsb();
	afx_msg void OnAtariMachineTypeXlXe();
	afx_msg void OnAtariMachineType5200();
	afx_msg void OnAtariMemorySize16Kb();
	afx_msg void OnAtariMemorySize48Kb();
	afx_msg void OnAtariMemorySize52Kb();
	afx_msg void OnAtariMemorySize128Kb();
	afx_msg void OnAtariMemorySize64Kb();
	afx_msg void OnAtariMemorySize320KbCompy();
	afx_msg void OnAtariMemorySize320KbRambo();
	afx_msg void OnAtariMemorySize576Kb();
	afx_msg void OnAtariMemorySize1088Kb();
	afx_msg void OnAtariVideoSystem();
	afx_msg void OnAtariVideoSystemNtsc();
	afx_msg void OnAtariVideoSystemPal();
	afx_msg void OnAtariSettings();
	afx_msg void OnAtariPerformance();
	afx_msg void OnAtariFullSpeed();
	afx_msg void OnAtariPause();
	afx_msg void OnAtariSioPatch();
	afx_msg void OnAtariHPatch();
	afx_msg void OnAtariPPatch();
	afx_msg void OnAtariRPatch();
	afx_msg void OnAtariDisableBasic();
	afx_msg void OnAtariRomImages();
	afx_msg void OnAtariDiskDrives();
	afx_msg void OnAtariTapeControl();
	afx_msg void OnAtariHardDisks();
	afx_msg void OnAtariWarmstart();
	afx_msg void OnAtariColdstart();
	afx_msg void OnViewOptions();
	afx_msg void OnViewToggleModes();
	afx_msg void OnViewGdiForWindows();
	afx_msg void OnViewStretchModePixelDoubling();
	afx_msg void OnViewStretchModeInterpolation();
	afx_msg void OnViewStretchModeScanlines();
	afx_msg void OnViewStretchModeHiEnd();
	afx_msg void OnViewShowDiskActivity();
	afx_msg void OnViewShowSectorCounter();
	afx_msg void OnViewShowStateIndicators();
	afx_msg void OnViewArtifacting();
	afx_msg void OnViewArtifactingNone();
	afx_msg void OnViewArtifactingBluebrown1();
	afx_msg void OnViewArtifactingBluebrown2();
	afx_msg void OnViewArtifactingGtia();
	afx_msg void OnViewArtifactingCtia();
	afx_msg void OnViewPalette();
	afx_msg void OnViewSaveScreenshotNormal();
	afx_msg void OnViewSaveScreenshotInterlaced();
	afx_msg void OnViewSaveVideo();
	afx_msg void OnViewSaveVideoAndSound();
	afx_msg void OnViewPerformanceTest();
	afx_msg void OnSoundOptions();
	afx_msg void OnSoundStereo();
	afx_msg void OnSoundMute();
	afx_msg void OnSoundSaveSound();
	afx_msg void OnSoundPerformanceTest();
	afx_msg void OnInputCaptureMouse();
	afx_msg void OnInputJoysticks();
	afx_msg void OnInputMouse();
	afx_msg void OnInputKeyboard();
	afx_msg void OnInputAutofire();
	afx_msg void OnInputAutofireOff();
	afx_msg void OnInputAutofireFireDependent();
	afx_msg void OnInputAutofireAllTime();
	afx_msg void OnInputMouseDeviceNone();
	afx_msg void OnInputMouseDevicePaddles();
	afx_msg void OnInputMouseDeviceTouchTablet();
	afx_msg void OnInputMouseDeviceKoalaPad();
	afx_msg void OnInputMouseDeviceLightPen();
	afx_msg void OnInputMouseDeviceLightGun();
	afx_msg void OnInputMouseDeviceAmigaMouse();
	afx_msg void OnInputMouseDeviceStMouse();
	afx_msg void OnInputMouseDeviceTrakBall();
	afx_msg void OnInputMouseDeviceJoystick();
	afx_msg void OnInputArrowKeysArrows();
	afx_msg void OnInputArrowKeysControlArrows();
	afx_msg void OnInputArrowKeysF1F4();
	afx_msg void OnInputNetworkGame();
	afx_msg void OnMiscFileAssociations();
	afx_msg void OnMiscConvertRomToCart();
	afx_msg void OnMiscConvertDcmToAtr();
	afx_msg void OnMiscConvertXfdToAtr();
	afx_msg void OnMiscConvertAtasciiToAscii();
	afx_msg void OnMiscConvertAsciiToAtascii();
	afx_msg void OnMiscPauseInactive();
	afx_msg void OnMiscReuseWindow();
	afx_msg void OnMiscHighPriority();
	afx_msg void OnMiscConfirmOnExit();
	afx_msg void OnMiscThreadedEmulation();
	afx_msg void OnMiscCheatOptions();
	afx_msg void OnMiscDisableCollisions();
	afx_msg void OnMiscClearAllSettings();
	afx_msg void OnMiscRestartEmulation();
	afx_msg void OnMiscViewLogFile();
	afx_msg void OnMiscMonitor();
	afx_msg void OnMiscMonitorAsk();
	afx_msg void OnMiscMonitorOpen();
	afx_msg void OnMiscMonitorStop();
	afx_msg void OnHelpKeyboardLayout();
	afx_msg void OnHelpKeyboardShortcuts();
	afx_msg void OnHelpSetupWizard();
	afx_msg void OnHelpWebPage();
	afx_msg void OnHelpAbout();
	afx_msg void OnUpdateFileBootAtariImage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileLoadExecutable(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileAttachDiskImageDrive(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileDetachDiskImageAll(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileDetachDiskImageDrive(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileTurnDisk(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileAttachTapeImage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileDetachTapeImage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileAttachCartridgeImage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileDetachCartridgeImage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileReadState(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAtariMachineType(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAtariMemorySize(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAtariVideoSystem(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAtariFullSpeed(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAtariPause(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAtariSioPatch(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAtariHPatch(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAtariPPatch(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAtariRPatch(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAtariDisableBasic(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewToggleModes(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewGdiForWindows(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewStretchMode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewShowDiskActivity(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewShowSectorCounter(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewShowStateIndicators(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewArtifacting(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSaveVideo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSaveVideoAndSound(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSoundStereo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSoundMute(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSoundSaveSound(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInputCaptureMouse(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInputAutofire(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInputMouseDevice(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInputArrowKeys(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInputNetworkGame(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMiscPauseInactive(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMiscReuseWindow(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMiscHighPriority(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMiscConfirmOnExit(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMiscThreadedEmulation(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMiscClearAllSettings(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMiscDisableCollisions(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMiscMonitor(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMiscMonitorAsk(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMiscMonitorOpen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMiscMonitorStop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHelpSetupWizard(CCmdUI* pCmdUI);
	afx_msg void OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu);
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnClose();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	//}}AFX_MSG
	afx_msg void OnExitMenuLoop( BOOL bIsTrackPopupMenu );
	afx_msg void OnContextHelp();
	afx_msg void OnHelpFinder();
	afx_msg void OnHelp();
	afx_msg LRESULT OnMessageCommandLine(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageKailleraReset(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageKailleraStart(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageKailleraStop(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageKailleraGameStart(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageKailleraGameStop(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__MAINFRAME_H__
