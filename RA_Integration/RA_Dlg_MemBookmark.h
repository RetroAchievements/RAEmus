#pragma once

#include <wtypes.h>
#include <vector>

#include "RA_Defs.h"

namespace
{
	const size_t MAX_ADDRESSES = 200;
	const size_t MEM_STRING_TEXT_LEN = 80;
}

class MemBookmark
{
public:
	void SetDescription( const std::string& string )		{ m_sDescription = string; }
	void SetAddress( unsigned int nVal )					{ m_nAddress = nVal; }
	void SetType( unsigned int nVal )						{ m_nType = nVal; }
	void SetValue( const std::string& string )				{ m_sValue = string; }
	void SetPrevious( const std::string& string )			{ m_sPrevious = string; }
	void IncreaseCount()									{ m_nCount++; }
	void ResetCount()										{ m_nCount = 0; }

	void SetFrozen( bool b )								{ m_bFrozen = b; }

	inline const std::string& Description() const			{ return m_sDescription; }
	unsigned int Address() const							{ return m_nAddress; }
	unsigned int Type() const								{ return m_nType; }
	inline const std::string& Value() const					{ return m_sValue; }
	inline const std::string& Previous() const				{ return m_sPrevious; }
	unsigned int Count() const								{ return m_nCount; }

	bool Frozen() const										{ return m_bFrozen; }

private:
	std::string m_sDescription;
	unsigned int m_nAddress;
	unsigned int m_nType;
	std::string m_sValue;
	std::string m_sPrevious;
	unsigned int m_nCount = 0;
	bool m_bFrozen = FALSE;
};

class Dlg_MemBookmark
{
public:
	//Dlg_MemBookmark();
	//~Dlg_MemBookmark();

	static INT_PTR CALLBACK s_MemBookmarkDialogProc( HWND, UINT, WPARAM, LPARAM );
	INT_PTR MemBookmarkDialogProc( HWND, UINT, WPARAM, LPARAM );

	void InstallHWND( HWND hWnd )						{ m_hMemBookmarkDialog = hWnd; }
	HWND GetHWND() const								{ return m_hMemBookmarkDialog; }

	void UpdateBookmarks( bool bForceWrite );

	void AddBookmark( const MemBookmark& newBookmark )	{ m_vBookmarks.push_back(newBookmark); }
	void ClearAllBookmarks()							{ m_vBookmarks.clear(); }

	void SetDirty()										{ bIsDirty = TRUE; }
	void ClearDirty()									{ bIsDirty = FALSE; }

private:
	static const int m_nNumCols = 6;
	char m_lbxData[ MAX_ADDRESSES ][ m_nNumCols ][ MEM_STRING_TEXT_LEN ];
	wchar_t m_lbxGroupNames[ MAX_ADDRESSES ][ MEM_STRING_TEXT_LEN ];
	int m_nNumOccupiedRows;
	bool bIsDirty;

	void PopulateList();
	void SetupColumns( HWND hList );
	void AddAddress();
	void UpdateListItem( HWND hList, LV_ITEM& item, const MemBookmark& Bookmark );
	void WriteFrozenValue( const MemBookmark& Bookmark );
	std::string GetMemory( unsigned int nAddr, int type );

private:
	HWND m_hMemBookmarkDialog;
	std::vector<MemBookmark> m_vBookmarks;
};

extern Dlg_MemBookmark g_MemBookmarkDialog;