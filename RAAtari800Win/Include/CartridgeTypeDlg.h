/****************************************************************************
File    : CartridgeTypeDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CCartridgeTypeDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 23.12.2003
*/

#ifndef __CARTRIDGETYPE_H__
#define __CARTRIDGETYPE_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CCartridgeTypeDlg dialog

class CCartridgeTypeDlg : public CCommonDlg
{
// Type definitions
public:
	struct CartTypeInfo_t
	{
		LPCSTR pszCartName;
		int    nCartType;
		int    nCartSize;
		UINT   nCartDesc;
	};

// Construction
public:
	CCartridgeTypeDlg( int nCartSize, CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:
	int GetCartridgeType() { return m_nCartType; }
	int CountTypes( int nCartSize );

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CCartridgeTypeDlg)
	enum { IDD = IDD_CARTRIDGETYPE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCartridgeTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	int m_nCartSize;
	int m_nCartType;

	CartTypeInfo_t *m_pCartTypeInfo;

// Generated message map functions
protected:
	//{{AFX_MSG(CCartridgeTypeDlg)
	afx_msg void OnSelchangeCartridge();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__CARTRIDGETYPE_H__
