/****************************************************************************
File    : FileAssociationsBase.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CFileAssociationsBase class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 03.08.2002
*/

#ifndef __FILEASSOCIATIONSBASE_H__
#define __FILEASSOCIATIONSBASE_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CFileAssociationsBase class

class CFileAssociationsBase : public CDlgBase
{
// Type definitions
public:
	enum FileType { FEI_DSK = 0, FEI_BIN, FEI_TAP, FEI_CRT, FEI_A8S };

	struct FileExtInfo_t
	{
		enum   FileType ftType;
		DWORD  dwRegFlag;
		LPCSTR pszFileExt;	/* The pointer to a static text */
		UINT   nCtrlID;
	};

// Construction
public:
	CFileAssociationsBase();
	virtual ~CFileAssociationsBase();

// Interface: operations
public:
	static ULONG ReadRegFileExt ( LPSTR pszAppDir, LPCSTR pszAppName );
	static void  WriteRegFileExt( ULONG ulFileAssociations, LPSTR pszAppDir, LPCSTR pszAppName );

// Implementation: operations
protected:
	void SelectAll();
	void RemoveAll();

// Implementation: overrides
protected:
	virtual BOOL InitDialog();

// Implementation: attributes
protected:
	static FileExtInfo_t *m_pFileExtInfo;
	static const int      m_nFileExtInfoNo;

	char  m_szHomeDir[ MAX_PATH + 1 ];
	ULONG m_ulFileAssociations;
};

/////////////////////////////////////////////////////////////////////////////

#endif //__FILEASSOCIATIONSBASE_H__
