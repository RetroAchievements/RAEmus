/****************************************************************************
File    : RomImagesBase.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CRomImagesBase class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 03.08.2002
*/

#ifndef __ROMIMAGESBASE_H__
#define __ROMIMAGESBASE_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CRomImagesBase class

class CRomImagesBase : public CDlgBase
{
// Type definitions
public:
	struct RomData_t
	{
		LPSTR  pszName;
		char   szNewName[ MAX_PATH + 1 ];
		UINT   nCtrlID;
		LPCSTR pszOpenPrompt;
		LPSTR  pszRegName;
		UINT   nWarnBit;
	};

// Construction
public:
	CRomImagesBase();
	virtual ~CRomImagesBase();

// Interface: operations
public:

// Implementation: Operations
protected:
	void KillfocusEditRom( RomType rtType );
	void BrowseRomFile   ( RomType rtType );
	void DisplayRomInfo  ();
	BOOL SearchRomImages ();

private:
	LPCSTR GetRomInfo( LPSTR pszFileName, enum RomType rtType );

// Implementation: overrides
protected:

// Implementation: attributes
protected:
	RomData_t *m_pRomData;
};

/////////////////////////////////////////////////////////////////////////////

#endif //__ROMIMAGESBASE_H__
