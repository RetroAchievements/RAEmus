/****************************************************************************
File    : Atari800WinDoc.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CAtari800WinDoc class
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 10.03.2002
*/

#ifndef __ATARI800WINDOC_H__
#define __ATARI800WINDOC_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinDoc:
// See Atari800WinDoc.cpp for the implementation of this class
//

class CAtari800WinDoc : public CDocument
{
// Construction/destruction
protected:
	// Create from serialization only
	DECLARE_DYNCREATE(CAtari800WinDoc)

	CAtari800WinDoc();
	virtual ~CAtari800WinDoc();

// Interface: operations
public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Interface: attributes
public:

// Implementation: operations
private:

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAtari800WinDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CAtari800WinDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif //__ATARI800WINDOC_H__
