/****************************************************************************
File    : DlgBase.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CDlgBase implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 17.03.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "DlgBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CDlgBase class

/*========================================================
Method   : CDlgBase::CDlgBase
=========================================================*/
/* #FN#
   Standard constructor */
CDlgBase::
CDlgBase()
{
} /* #OF# CDlgBase::CDlgBase */

/*========================================================
Method   : CDlgBase::~CDlgBase
=========================================================*/
/* #FN#
   Destructor */
CDlgBase::
~CDlgBase()
{
} /* #OF# CDlgBase::~CDlgBase */

/*========================================================
Method   : CDlgBase::DataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange */
void
/* #AS#
   Nothing */
CDlgBase::
DataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	pDX; // not used

} /* #OF# CDlgBase::DataExchange */


/////////////////////////////////////////////////////////////////////////////
// CDlgBase implementation

/*========================================================
Method   : CDlgBase::InitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CDlgBase::
InitDialog()
{
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CDlgBase::InitDialog */
