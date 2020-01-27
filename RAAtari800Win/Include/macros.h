/****************************************************************************
File    : macros.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Usefull macros
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 10.10.2003
*/

#ifndef __MACROS_H__
#define __MACROS_H__

#include <process.h>

#ifdef __cplusplus
extern "C" {
#endif

/* State of the emulator */

#define ST_DOUBLEWND_REFRESH \
	(g_Misc.ulState & MS_USE_DOUBLEWND_REFRESH && \
	 g_Screen.ulState & SM_MODE_WIND && \
	 g_Screen.ulState & SM_WRES_DOUBLE)

#define ST_FULLSPEED_REFRESH \
	(g_Misc.ulState & MS_USE_FULLSPEED_REFRESH && \
	 g_Misc.ulState & MS_FULL_SPEED)

#define ST_MENUBAR_HIDDEN \
	(g_Screen.ulState & SM_MODE_FULL && \
	(g_Screen.ulState & (SM_OPTN_FLIP_BUFFERS | SM_ATTR_NO_MENU)))

#define ST_FLIPPED_BUFFERS \
	(g_Screen.ulState & SM_MODE_FULL && \
	 g_Screen.ulState & SM_OPTN_FLIP_BUFFERS)

#define ST_ATARI_STOPPED \
	(g_ulAtariState & (ATARI_UNINITIALIZED | ATARI_CRASHED | ATARI_PAUSED))

#define ST_ATARI_FAILED \
	(g_ulAtariState & (ATARI_UNINITIALIZED | ATARI_CRASHED))

#define ST_MENU_VISIBLE \
	(g_Screen.bLocked || !ST_MENUBAR_HIDDEN || ST_ATARI_STOPPED)

#define ST_MOUSE_HIDDEN \
	(g_Screen.ulState & SM_MODE_FULL && \
	 g_Screen.ulState & SM_OPTN_HIDE_CURSOR || \
	 g_Input.ulState & IS_CAPTURE_MOUSE)

#define ST_CTRLESC_CAPTURED \
	(g_Input.ulState & IS_CAPTURE_CTRLESC || \
	(g_Screen.ulState & SM_MODE_FULL && g_Misc.unSystemInfo & SYS_WIN_9x))

/* Handling of dialog controls */

#define _DeltaposSpin(phdr, ctrl, var, min, max)	\
{													\
	NM_UPDOWN *pNMUpDown = (NM_UPDOWN*)phdr;		\
	var -= pNMUpDown->iDelta;						\
	if(var < min) var = min;						\
	if(var > max) var = max;						\
	SetDlgItemInt(ctrl, var, FALSE);				\
}

#define _KillfocusSpin(ctrl, var, min, max)			\
{													\
	BOOL bTrans;									\
	int nVar = GetDlgItemInt(ctrl, &bTrans, FALSE);	\
	if(bTrans) {									\
		var = nVar;									\
		if(var < min) var = min;					\
		if(var > max) var = max;					\
	}												\
	if(!bTrans || nVar < min || nVar > max)			\
		SetDlgItemInt(ctrl, var, FALSE);			\
}

#define _ClickButton(ctrl, var, opt)				\
{													\
	CButton *pButton = (CButton*)GetDlgItem(ctrl);	\
	ASSERT(ctrl);									\
	if(pButton->GetCheck())							\
		var |= (opt);								\
	else											\
		var &= ~(opt);								\
}

#define _EnableCtrl(ctrl, en)                       \
	(GetDlgItem(ctrl)->EnableWindow(en))

#define _SetSelCbox(cbox, cs)                       \
	(((CComboBox*)GetDlgItem(cbox))->SetCurSel(cs))

#define _GetSelCbox(cbox)                           \
	(((CComboBox*)GetDlgItem(cbox))->GetCurSel())

#define _SetDatCbox(cbox, i, id)                    \
	(((CComboBox*)GetDlgItem(cbox))->SetItemData(i, id))

#define _GetDatCbox(cbox, i)                        \
	(((CComboBox*)GetDlgItem(cbox))->GetItemData(i))

#define _SetChkBttn(bttn, sc)                       \
	(((CButton*)GetDlgItem(bttn))->SetCheck(sc))

#define _GetChkBttn(bttn)                           \
	(((CButton*)GetDlgItem(bttn))->GetCheck())

#define _RdOnlyEdit(edit, ro)                       \
	(((CEdit*)GetDlgItem(edit))->SetReadOnly(ro))

/* Create thread wrapper */

typedef unsigned (__stdcall *PTHREAD_START)(void *);

#define _CreateThreadEx(psa, cbStack, pfnStartAddr,	\
	pvParam, fdwCreate, pdwThreadID)				\
		((HANDLE) _beginthreadex(					\
			(void *)(psa),							\
			(unsigned)(cbStack),					\
			(PTHREAD_START)(pfnStartAddr),			\
			(void *)(pvParam),						\
			(unsigned)(fdwCreate),					\
			(unsigned *)(pdwThreadID)))

/* Misc helpers */

#define _ModifyFlag(cond, reg, flag) \
	((cond) ? (reg |= (flag)) : (reg &= ~(flag)))

#define _ToggleFlag(reg, flag) \
	(0 == (reg & (flag)) ? (reg |= (flag)) : (reg &= ~(flag)))

#define _SetFlag(reg, flag) \
	(reg |= (flag))

#define _ClrFlag(reg, flag) \
	(reg &= ~(flag))

#define _IsFlagSet(reg, flag) \
	(0 != ((reg) & (flag)))

#define _IsMaskSet(reg, mask) \
	((mask) == ((reg) & (mask)))

#define _GetRefreshRate() \
	(ST_FULLSPEED_REFRESH ? g_Misc.Refresh.nFullSpeed : \
	(ST_DOUBLEWND_REFRESH ? g_Misc.Refresh.nDoubleWnd : Atari800_refresh_rate))

#define _IsPathAvailable(sz) \
	(BOOL)(*(sz) != '\0' && _stricmp(sz, FILE_NONE) != 0)

#define _NullPathToNone(sz) \
	if(*(sz) == '\0') strcpy(sz, FILE_NONE)

#define _IsKeyPressed(key) \
	(BOOL)(GetAsyncKeyState(key) >> ((sizeof(SHORT) * 8) - 1))

#define _LoadStringSx(nID, sz) \
	LoadStringEx(nID, sz, LOADSTRING_SIZE_S)

#define _LoadStringMx(nID, sz) \
	LoadStringEx(nID, sz, LOADSTRING_SIZE_M)

#define _LoadStringLx(nID, sz) \
	LoadStringEx(nID, sz, LOADSTRING_SIZE_L)

#define _CursorBusy() \
	AfxGetApp()->DoWaitCursor(1)

#define _CursorFree() \
	AfxGetApp()->DoWaitCursor(-1)

#define _strncpy(dest, src, len) \
	(strncpy(dest, src, len)[ len ] = '\0')

#ifdef __cplusplus
}
#endif

#endif /*__MACROS_H__*/
