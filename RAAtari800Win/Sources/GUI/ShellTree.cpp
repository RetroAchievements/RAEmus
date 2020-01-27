/****************************************************************************
File    : ShellTree.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# ShellTree implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 03.08.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "ShellTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CShellTree
//
// This source was part of CShellTree - Selom Ofori
// 
// Version: 1.02 (any previously unversioned copies are older/inferior)
// Rewritten for PLus purposes by Tomasz Szymankowski
//
// This code is free for all to use. Mutatilate it as much as you want
// See MFCENUM sample from microsoft

/////////////////////////////////////////////////////////////////////////////
// CShellTree control

BEGIN_MESSAGE_MAP(CShellTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CShellTree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*========================================================
Method   : CShellTree::CShellTree
=========================================================*/
/* #FN#
   Standard constructor */
CShellTree::
CShellTree()
{
	m_bFolderMode = TRUE;

} /* #OF# CShellTree::CShellTree */

/*========================================================
Method   : CShellTree::CShellTree
=========================================================*/
/* #FN#
   Destructor */
CShellTree::
~CShellTree()
{
} /* #OF# CShellTree::~CShellTree */


/////////////////////////////////////////////////////////////////////////////
// CShellTree implementation

/*========================================================
Method   : CShellTree::PopulateTree
=========================================================*/
/* #FN#
   Populates the tree view control with file list */
void
/* #AS#
   Nothing */
CShellTree::
PopulateTree()
{
	LPSHELLFOLDER lpsf = NULL;
	LPITEMIDLIST  lpi  = NULL;
	HRESULT hr;

	/* Get a pointer to the desktop folder */
	hr = SHGetDesktopFolder( &lpsf );

	if( SUCCEEDED(hr) )
	{
		/* Initialize the tree view to be empty */
		DeleteAllItems();

		/* Fill in the tree view from the root */
		FillTreeView( lpsf, NULL, TVI_ROOT );
		/* Release the folder pointer */
		lpsf->Release();
	}
	TV_SORTCB tvscb;
	tvscb.hParent     = TVI_ROOT;
	tvscb.lParam      = 0;
	tvscb.lpfnCompare = TreeViewCompareProc;

	/* Sort the items in the tree view */
	SortChildrenCB( &tvscb /*, FALSE*/ );

	HTREEITEM hItem = GetRootItem();
	Expand( hItem, TVE_EXPAND );
	Select( GetRootItem(), TVGN_CARET );

} /* #OF# CShellTree::PopulateTree */

/*========================================================
Method   : CShellTree::FillTreeView
=========================================================*/
/* #FN#
   Fills a branch of the TreeView control.
   
   Given the shell folder, enumerate the subitems of this folder, and
   add the appropriate items to the tree. This function enumerates the
   items in the folder identifed by lpsf. Note that since we are filling
   the left hand pane, we will only add items that are folders and/or
   have sub-folders. We *could* put all items in here if we wanted, but
   that's not the intent. */
void
/* #AS#
   Nothing */
CShellTree::
FillTreeView(
	LPSHELLFOLDER lpsf,   /* #IN# Pointer to shell folder that we want to enumerate items */
	LPITEMIDLIST  lpifq,  /* #IN# Fully qualified item id list to the item that we are enumerating items for; in other words, this is the PIDL to the item identified by the lpsf parameter */
	HTREEITEM     hParent /* #IN# Parent node */
)
{
	TV_ITEM         tvi;	/* TreeView Item */
	TV_INSERTSTRUCT tvins;	/* TreeView Insert Struct */

	HTREEITEM     hPrev         = NULL;	/* Previous Item Added */
	LPSHELLFOLDER lpsf2         = NULL;
	LPENUMIDLIST  lpe           = NULL;
	LPITEMIDLIST  lpi           = NULL;
	LPITEMIDLIST  lpifqThisItem = NULL;
	LPTVITEMDATA  lptvid        = NULL;
	LPMALLOC      lpMalloc      = NULL;

	ULONG   ulFetched = 0;
	HWND    hwnd      = ::GetParent( m_hWnd );
	char    szBuff[ MAX_PATH + 1 ];
	char    szPath[ MAX_PATH + 1 ];
	HRESULT hr;

	/* Allocate a shell memory object */
	hr = ::SHGetMalloc( &lpMalloc );
	if( FAILED(hr) )
		return;

	if( SUCCEEDED(hr) )
	{
		/* Get the IEnumIDList object for the given folder */
		hr = lpsf->EnumObjects( hwnd, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN, &lpe );

		if( S_OK == hr && lpe )
		{
			/* Enumerate throught the list of folder and non-folder objects */
			while( S_OK == lpe->Next( 1, &lpi, &ulFetched ) )
			{
				/* Create a fully qualified path to the current item. The SH* shell API's
				   take a fully qualified path pidl, (see GetIcon above where I call
				   SHGetFileInfo) whereas the interface methods take a relative path pidl */
				ULONG ulAttrs = SFGAO_FOLDER | SFGAO_HASSUBFOLDER;

				if( !m_bFolderMode )
					ulAttrs |= SFGAO_FILESYSTEM | SFGAO_LINK;

				/* Determine what type of object we have */
				lpsf->GetAttributesOf( 1, (const struct _ITEMIDLIST **)&lpi, &ulAttrs );

				if( m_bFolderMode && ulAttrs & (SFGAO_HASSUBFOLDER | SFGAO_FOLDER) ||
				  (!m_bFolderMode && ulAttrs & (SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_FILESYSTEM) && !(ulAttrs & SFGAO_LINK)) )
				{
					tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
					/* We need this next if statement so that we don't add things like
					   the MSN to our tree. MSN is not a folder, but according to the
					   shell it has subfolders */
					/* OK, let's get some memory for our ITEMDATA struct */
					lptvid = (LPTVITEMDATA)lpMalloc->Alloc( sizeof(TVITEMDATA) );
					if( !lptvid )
						goto Done; /* Error - could not allocate memory */

					/* Now get the friendly name that we'll put in the treeview */
					if( !GetName( lpsf, lpi, SHGDN_NORMAL, szBuff ) )
						goto Done; /* Error - could not get friendly name */

					tvi.pszText    = szBuff;
					tvi.cchTextMax = MAX_PATH;

					/* Allocate/create the fully qualified PIDL, consisting
					   of the parents full PIDL and our relative PIDL */
					lpifqThisItem = ConcatPidls( lpifq, lpi );

					if( ulAttrs & SFGAO_FOLDER && (ulAttrs & SFGAO_HASSUBFOLDER ||
						/* There are not any subfolders but what about files? */
						(!m_bFolderMode && SHGetPathFromIDList( lpifqThisItem, szPath ) && !IsFolderEmpty( szPath ))) )
					{
						/* This item has sub-folders, so let's put the + in the TreeView.
						   The first time the user clicks on the item, we'll populate the
						   sub-folders */
						tvi.cChildren = 1;
						tvi.mask |= TVIF_CHILDREN;
					}
					/* Now, make a copy of the ITEMIDLIST (non-qualified) */
					lptvid->lpi = CopyITEMID( lpMalloc, lpi );

					tvi.iImage =
						GetItemIcon( lpifqThisItem,
									 SHGFI_PIDL |
									 SHGFI_SYSICONINDEX |
									 SHGFI_SMALLICON );

					tvi.iSelectedImage =
						GetItemIcon( lpifqThisItem,
									 SHGFI_PIDL |
									 SHGFI_SYSICONINDEX |
									 SHGFI_SMALLICON |
									 SHGFI_OPENICON );

					lptvid->lpsfParent = lpsf; /* Store the parent folders SF */
					lpsf->AddRef();            /* Increment our saved reference */

					/* Now create another PIDL from our Full parents PIDL and
					   the releative one that we'll save */
					lptvid->lpifq = ConcatPidls( lpifq, lpi );

					/* Populate the TreeView Insert Struct. The item is the one
					   filled above. Insert it after the last item inserted at
					   this level. And indicate this is a root entry */
					tvi.lParam         = (LPARAM)lptvid;
					tvins.item         = tvi;
					tvins.hInsertAfter = hPrev;
					tvins.hParent      = hParent;

					/* Add the item to the tree */
					hPrev = InsertItem( &tvins );

					/* Free this item with task allocator */
					lpMalloc->Free( lpifqThisItem );
					lpifqThisItem = 0;
				}
				lpMalloc->Free( lpi );  /* Free the pidl that the shell gave us */
				lpi = 0;
			}
		}
	}
	else
		return;

Done:
	if( lpe )
		lpe->Release();

	/* The following 2 if statements will only be TRUE if we got here
	   on an error condition from the "goto" statement. Otherwise, we
	   free this memory at the end of the while loop above */
	if( lpi && lpMalloc )
		lpMalloc->Free( lpi );
	if( lpifqThisItem && lpMalloc )
		lpMalloc->Free( lpifqThisItem );

	if( lpMalloc )
		lpMalloc->Release();

} /* #OF# CShellTree::FillTreeView */

/*========================================================
Method   : CShellTree::EnableImages
=========================================================*/
/* #FN#
   Obtains a handle to the system image list and attaches it to the
   tree control. DO NOT DELETE the imagelist */
void
/* #AS#
   Nothing */
CShellTree::
EnableImages()
{
	/* Get the handle to the system image list, for our icons */
	SHFILEINFO sfi;

	HIMAGELIST hImageList =
		(HIMAGELIST)SHGetFileInfo(
			(LPCSTR)"C:\\",
			0,
			&sfi,
			sizeof(SHFILEINFO),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON );

	/* Attach ImageList to TreeView */
	if( hImageList )
		::SendMessage( m_hWnd, TVM_SETIMAGELIST, (WPARAM)TVSIL_NORMAL, (LPARAM)hImageList );

} /* #OF# CShellTree::EnableImages */

/*========================================================
Method   : CShellTree::SearchTree
=========================================================*/
/* #FN#
   Only works if you use the default PopulateTree(). Not guaranteed to work
   on any future or existing version of windows. Use with caution. Pretty
   much ok if you're using on local drives */
BOOL
/* #AS#
   TRUE if the node was found, otherwise FALSE */
CShellTree::
SearchTree(
	HTREEITEM hItem,         /* #IN# */
	LPCSTR    lpszSearchPath /* #IN# */
)
{
	LPTVITEMDATA lptvid = NULL;  /* Long pointer to TreeView item data */

	char szCompare[ MAX_PATH + 1 ];
	BOOL bResult = FALSE;

	while( hItem && !bResult )
	{
		lptvid = (LPTVITEMDATA)GetItemData( hItem );
		if( lptvid && lptvid->lpsfParent && lptvid->lpi )
		{
			ULONG ulAttrs = SFGAO_FILESYSTEM;
			lptvid->lpsfParent->GetAttributesOf( 1, (const struct _ITEMIDLIST **)&lptvid->lpi, &ulAttrs );

			if( ulAttrs & SFGAO_FILESYSTEM )
			{
				if( SHGetPathFromIDList( lptvid->lpifq, szCompare ) )
				{
					if( _stricmp( szCompare, lpszSearchPath ) == 0 )
					{
						EnsureVisible( hItem );
						SelectItem( hItem );
						bResult = TRUE;
					}
				}
			}
		}
		hItem = GetNextSiblingItem( hItem );
	}
	return bResult;

} /* #OF# CShellTree::SearchTree */

/*========================================================
Method   : CShellTree::TunnelTree
=========================================================*/
/* #FN#
   Pushes down and selects an item one level at a time until either we
   can't find anymore parts of the path, or we've finished searching

   Doesn't work on network (UNC) paths (Could search NETHOOD when
   prefix is \\) */
void
/* #AS#
   Nothing */
CShellTree::
TunnelTree(
	LPCSTR lpszPath /* #IN# */
)
{
	char szSearchPath[ MAX_PATH + 1 ];
	BOOL bFound = FALSE;
	int  i = 0;

	HTREEITEM hTopLevelItem;
	HTREEITEM hItem;

	if( (hTopLevelItem = GetRootItem()) )
	{
		do /* Enumerate the top level items */
		{
			if( Expand( hTopLevelItem, TVE_EXPAND ) )
			{
				hItem = GetChildItem( hTopLevelItem );

				while( hItem )
				{
					for( ; i < (int)strlen( lpszPath ) && lpszPath[ i ] != '\\'; i++ )
						szSearchPath[ i ] = lpszPath[ i ];

					szSearchPath[ i++ ] = '\0';
					/* Add ending backslash to drive name */
					if( strlen( szSearchPath ) == 2 && szSearchPath[ 1 ] == ':' )
						strcat( szSearchPath, "\\" );

					if( SearchTree( hItem, szSearchPath ) )
					{
						hItem = GetSelectedItem();
						if( Expand( hItem, TVE_EXPAND ) )
						{
							/* Get first leaf of the new bunch */
							hItem = GetChildItem( hItem );
						}
						bFound = TRUE;
					}
					else
						break;
					/* Append a folder delimiter */
					szSearchPath[ i - 1 ] = '\\';
				}
				/* The path has not been found, reset the searching "engine" */
				if( !bFound )
				{
					Expand( hTopLevelItem, TVE_COLLAPSE );
					i = 0;
				}
			}
		}
		while( !bFound && (hTopLevelItem = GetNextSiblingItem( hTopLevelItem )) );
	}
} /* #OF# CShellTree::TunnelTree */

/*========================================================
Method   : CShellTree::IsFolderEmpty
=========================================================*/
/* #FN#
   Check if the folder contents minimum one file/folder */
BOOL
/* #AS#
   FALSE if a file/folder has been found, otherwise TRUE */
CShellTree::
IsFolderEmpty(
	LPCSTR lpszPath /* #IN# */
)
{
	CString strPattern = lpszPath;

	WIN32_FIND_DATA fileData;
	HANDLE hFile;

	if( strPattern.GetAt( strPattern.GetLength() - 1 ) != '\\' )
		strPattern += "\\";
	strPattern += "*.*";

	hFile = ::FindFirstFile( (LPCSTR)strPattern, &fileData );

	while( INVALID_HANDLE_VALUE != hFile )
	{
		if( /*!(fileData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) &&*/
			*fileData.cFileName != '.' )
		{
			::FindClose( hFile );
			return FALSE;
		}
		/* Try to find a next file */
		if( !::FindNextFile( hFile, &fileData ) )//&& (GetLastError() == ERROR_NO_MORE_FILES) )
		{
			::FindClose( hFile );
			hFile = INVALID_HANDLE_VALUE; /* Ending the loop */
		}
	}
	return TRUE;

} /* #OF# CShellTree::IsFolderEmpty */

/*========================================================
Method   : CShellTree::TreeViewCompareProc
=========================================================*/
/* #FN#
   Callback routine for sorting the tree */
int CALLBACK
/* #AS#
   A negative value if the first item should precede the second,
   a positive value if the first item should follow the second,
   or zero if the two items are equivalent */
CShellTree::
TreeViewCompareProc(
	LPARAM lparam1,   /* #IN# */
	LPARAM lparam2,   /* #IN# */
	LPARAM lparamSort /* #IN# */
)
{
	LPTVITEMDATA lptvid1 = (LPTVITEMDATA)lparam1;
	LPTVITEMDATA lptvid2 = (LPTVITEMDATA)lparam2;

	HRESULT hr = lptvid1->lpsfParent->CompareIDs( 0, lptvid1->lpi, lptvid2->lpi );

	if( FAILED(hr) )
		return 0;

	return (short)SCODE_CODE(GetScode( hr ));

} /* #OF# CShellTree::TreeViewCompareProc */


/////////////////////////////////////////////////////////////////////////////
// CShellTree message handlers

/*========================================================
Method   : CShellTree::OnFolderExpanding
=========================================================*/
/* #FN#
   Reponds to TVN_ITEMEXPANDING message in order to fill up subdirectories.
   Pass the parameters from OnItemExpanding() to this function. You need to
   do that or your folders won't expand */
void
/* #AS#
   Nothing */
CShellTree::
OnFolderExpanding(
	NMHDR   *pNMHDR, /* #IN# */
	LRESULT *pResult /* #OUT# */
)
{
	LPTVITEMDATA  lptvid; /* Long pointer to TreeView item data */
	LPSHELLFOLDER lpsf2 = NULL;
	TV_SORTCB     tvscb;
	static char   szBuff[ MAX_PATH + 1 ];
	HRESULT       hr;

	NM_TREEVIEW *pnmtv = (NM_TREEVIEW *)pNMHDR;

	if( pnmtv->itemNew.state & TVIS_EXPANDEDONCE )
		return;

	lptvid = (LPTVITEMDATA)pnmtv->itemNew.lParam;
	if( lptvid )
	{
		hr = lptvid->lpsfParent->BindToObject( lptvid->lpi, 0, IID_IShellFolder, (LPVOID *)&lpsf2 );
		
		if( SUCCEEDED(hr) )
			FillTreeView( lpsf2, lptvid->lpifq, pnmtv->itemNew.hItem );

		tvscb.hParent     = pnmtv->itemNew.hItem;
		tvscb.lParam      = 0;
		tvscb.lpfnCompare = TreeViewCompareProc;
		
		SortChildrenCB( &tvscb /*, FALSE*/ );
	}
	*pResult = 0;

} /* #OF# CShellTree::OnFolderExpanding */

/*========================================================
Method   : CShellTree::OnDeleteShellItem
=========================================================*/
/* #FN#
   Responds to TVN_DELETEITEM message in order to release the memory
   allocated by the shell folders */
void
/* #AS#
   Nothing */
CShellTree::
OnDeleteShellItem(
	NMHDR   *pNMHDR, /* #IN# */
	LRESULT *pResult /* #OUT# */
)
{
	LPTVITEMDATA lptvid   = NULL;
	LPMALLOC     lpMalloc = NULL;
	HRESULT      hr;

	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *)pNMHDR;

	/* Let's free the memory for the TreeView item data */
	hr = SHGetMalloc( &lpMalloc );
	if( FAILED(hr) )
		return;

	lptvid = (LPTVITEMDATA)pNMTreeView->itemOld.lParam;
	lptvid->lpsfParent->Release();
	lpMalloc->Free( lptvid->lpi );
	lpMalloc->Free( lptvid->lpifq );
	lpMalloc->Free( lptvid );
	lpMalloc->Release();

} /* #OF# CShellTree::OnDeleteShellItem */

/*========================================================
Method   : CShellTree::OnFolderSelected
=========================================================*/
/* #FN#
   Responds to TVN_SELCHANGED message in order to retrieve the path of
   the currently selected string */
BOOL
/* #AS#
   FALSE if the path is not in the filesystem (e.g. MyComputer) or none
   is selected, otherwise TRUE */
CShellTree::
OnFolderSelected(
	NMHDR   *pNMHDR,  /* #IN# */
	LRESULT *pResult, /* #OUT# */
	CString &szPath   /* #OUT# CString object that will hold the folder path */
)
{
	LPTVITEMDATA lptvid; /* Long pointer to TreeView item data */
	HTREEITEM    hItem   = NULL;
	BOOL         bResult = FALSE;

	if( NULL != (hItem = GetSelectedItem()) )
	{
		lptvid = (LPTVITEMDATA)GetItemData( hItem );

		if( lptvid && lptvid->lpsfParent && lptvid->lpi )
		{
			ULONG ulAttrs = SFGAO_FILESYSTEM;

			/* Determine what type of object we have */
			lptvid->lpsfParent->GetAttributesOf( 1, (const struct _ITEMIDLIST **)&lptvid->lpi, &ulAttrs );

			if( ulAttrs & SFGAO_FILESYSTEM )
			{
				char szBuff[ MAX_PATH + 1 ];
				if( SHGetPathFromIDList( lptvid->lpifq, szBuff ) )
				{
					szPath = szBuff;
					bResult = TRUE;
				}
			}
		}
	}
	return bResult;

} /* #OF# CShellTree::OnFolderSelected */
