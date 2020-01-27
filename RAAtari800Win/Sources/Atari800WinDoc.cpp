/****************************************************************************
File    : Atari800WinDoc.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CAtari800WinDoc implementation file
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 17.03.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Atari800WinDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinDoc

IMPLEMENT_DYNCREATE(CAtari800WinDoc, CDocument)

BEGIN_MESSAGE_MAP(CAtari800WinDoc, CDocument)
	//{{AFX_MSG_MAP(CAtari800WinDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*========================================================
Method   : CAtari800WinDoc::CAtari800WinDoc
=========================================================*/
/* #FN#
   Standard constructor */
CAtari800WinDoc::
CAtari800WinDoc()
{
	// TODO: add one-time construction code here

} /* #OF# CAtari800WinDoc::CAtari800WinDoc */

/*========================================================
Method   : CAtari800WinDoc::~CAtari800WinDoc
=========================================================*/
/* #FN#
   Destructor */
CAtari800WinDoc::
~CAtari800WinDoc()
{
} /* #OF# CAtari800WinDoc::~CAtari800WinDoc */


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinDoc implementation

/*========================================================
Method   : CAtari800WinDoc::OnNewDocument
=========================================================*/
/* #FN#
   Called by the framework as part of the File New command */
BOOL
/* #AS#
   Nonzero if the document was successfully initialized; otherwise 0 */
CAtari800WinDoc::
OnNewDocument()
{
	if( !CDocument::OnNewDocument() )
		return FALSE;

	return TRUE;

} /* #OF# CAtari800WinDoc::OnNewDocument */


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinDoc serialization

/*========================================================
Method   : CAtari800WinDoc::Serialize
=========================================================*/
/* #FN#
   Reads or writes this object from or to an archive */
void
/* #AS#
   Nothing */
CAtari800WinDoc::
Serialize(
	CArchive& ar
)
{
	if( ar.IsStoring() )
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
} /* #OF# CAtari800WinDoc::Serialize */


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinDoc diagnostics

#ifdef _DEBUG
void CAtari800WinDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAtari800WinDoc::Dump( CDumpContext& dc ) const
{
	CDocument::Dump( dc );
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinDoc commands
