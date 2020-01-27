/****************************************************************************
File    : ShellTree.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CShellTree class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 16.03.2002
*/

#ifndef __SHELLTREE_H__
#define __SHELLTREE_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CShellTree control
//
// This source was part of CShellTree - Selom Ofori
// 
// Version: 1.02 (any previously unversioned copies are older/inferior)
// Rewritten for PLus purposes by Tomasz Szymankowski
//
// This code is free for all to use. Mutatilate it as much as you want
// See MFCENUM sample from microsoft

class CShellTree : public CTreeCtrl, public CShellPidl
{
// Construction/destruction
public:
	CShellTree();   // standard constructor
	virtual ~CShellTree();

// Interface: operations
public:
	void SetFolderMode( BOOL bFolderMode ) { m_bFolderMode = bFolderMode; }
	BOOL GetFolderMode() { return m_bFolderMode; }
	
	void PopulateTree();
	void TunnelTree  ( LPCSTR lpszPath );

	void EnableImages();

	void OnFolderExpanding( NMHDR *pNMHDR, LRESULT *pResult );
	void OnDeleteShellItem( NMHDR *pNMHDR, LRESULT *pResult );
	BOOL OnFolderSelected ( NMHDR *pNMHDR, LRESULT *pResult, CString &szPath );

// Interface: attributes
public:

// Implementation: operations
private:
	void FillTreeView ( LPSHELLFOLDER lpsf, LPITEMIDLIST lpifq, HTREEITEM hParent );
	BOOL SearchTree   ( HTREEITEM hItem, LPCSTR lpszSearchPath );
	BOOL IsFolderEmpty( LPCSTR lpszPath );

	static int CALLBACK TreeViewCompareProc( LPARAM, LPARAM, LPARAM );

// Implementation: overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShellTree)
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	BOOL m_bFolderMode;

// Generated message map functions
protected:
	//{{AFX_MSG(CShellTree)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__SHELLTREE_H__
