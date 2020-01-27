/****************************************************************************
File    : ConvertTypeDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CConvertTypeDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 25.03.2002
*/

#ifndef __CONVERTTYPE_H__
#define __CONVERTTYPE_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CConvertTypeDlg dialog

class CConvertTypeDlg : public CCommonDlg
{
// Type definitions
public:
	enum ConvertType{ None = 0, Panther, Capek };

// Construction
public:
	CConvertTypeDlg( BOOL bA2PC = TRUE, CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:
	ConvertType GetConvertType() { return m_ctSelOpt; }

	static int  Atascii2Ascii( UBYTE *pubSrc, UBYTE *pubDst, DWORD dwSrcLen, BOOL bA2PC = TRUE );
	static void A8Std2Win1250( UBYTE *pubSrc, UBYTE *pubDst, DWORD dwSrcLen, ConvertType ctConType, BOOL bA2PC = TRUE );

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CConvertTypeDlg)
	enum { IDD = IDD_CONVERTTYPE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConvertTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	BOOL        m_bA2PC;
	ConvertType m_ctSelOpt;

// Generated message map functions
protected:
	//{{AFX_MSG(CConvertTypeDlg)
	afx_msg void OnNone();
	afx_msg void OnPanther();
	afx_msg void OnCapek();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__CONVERTTYPE_H__
