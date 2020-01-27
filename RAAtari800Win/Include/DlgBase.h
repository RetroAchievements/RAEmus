/****************************************************************************
File    : DlgBase.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CDlgBase class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 09.03.2001
*/

#ifndef __DLGBASE_H__
#define __DLGBASE_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CDlgBase class

class CDlgBase
{
// Construction
public:
	CDlgBase();
	virtual ~CDlgBase();

// Interface: operations
public:
	// Returns the pointer to the window inherited from C<xxx>Base
	virtual CWnd* GetWnd() = 0;

// Implementation: overrides
protected:
	virtual BOOL InitDialog();
	virtual void DataExchange( CDataExchange *pDX );
};

/////////////////////////////////////////////////////////////////////////////

#endif //__DLGBASE_H__
