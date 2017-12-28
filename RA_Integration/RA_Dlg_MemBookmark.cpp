#include "RA_Dlg_MemBookmark.h"

#include "RA_Resource.h"
#include "RA_GameData.h"
#include "RA_Dlg_Memory.h"
#include "RA_MemManager.h"

#include <strsafe.h>

Dlg_MemBookmark g_MemBookmarkDialog;

namespace
{
	const char* COLUMN_TITLE[] = { "Description", "Address", "Type", "Value", "Prev.", "Changes" };
	const int COLUMN_WIDTH[] = { 72, 64, 40, 64, 64, 54 };
	static_assert( SIZEOF_ARRAY( COLUMN_TITLE ) == SIZEOF_ARRAY( COLUMN_WIDTH ), "Must match!" );
}

enum BookmarkSubItems
{
	CSI_DESC,
	CSI_ADDRESS,
	CSI_TYPE,
	CSI_VALUE,
	CSI_PREVIOUS,
	CSI_CHANGES,

	NumColumns
};

INT_PTR CALLBACK Dlg_MemBookmark::s_MemBookmarkDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return g_MemBookmarkDialog.MemBookmarkDialogProc( hDlg, uMsg, wParam, lParam );
}

INT_PTR Dlg_MemBookmark::MemBookmarkDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC hdcMem;
	HBITMAP hbmp;
	TCHAR achBuffer[ MAX_PATH ];
	PMEASUREITEMSTRUCT pmis;
	PDRAWITEMSTRUCT pdis;
	size_t cch;
	int yPos;
	int nSelect;
	TEXTMETRIC tm;
	RECT rcBitmap;
	HRESULT hr;
	HWND hList;
	int offset = 2;
	BOOL bHighlight;

	RECT rcBounds, rcLabel;

	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			RECT rc;
			GetWindowRect( g_MemoryDialog.GetHWND(), &rc );
			SetWindowPos( hDlg, NULL, rc.left - 64, rc.top + 64, NULL, NULL, SWP_NOSIZE | SWP_NOZORDER );

			m_hMemBookmarkDialog = hDlg;
			hList = GetDlgItem( m_hMemBookmarkDialog, IDC_RA_LBX_ADDRESSES );

			SetupColumns( hList );

			return TRUE;
		}

		case WM_MEASUREITEM:
			pmis = (PMEASUREITEMSTRUCT)lParam;
			pmis->itemHeight = 16;
			return TRUE;
		case WM_DRAWITEM:
		{
			pdis = (PDRAWITEMSTRUCT)lParam;

			// If there are no list items, skip this message.
			if ( pdis->itemID == -1 )
				break;

			switch ( pdis->itemAction )
			{
				case ODA_SELECT:
				case ODA_DRAWENTIRE:

					hList = GetDlgItem( m_hMemBookmarkDialog, IDC_RA_LBX_ADDRESSES );

					ListView_GetItemRect( hList, pdis->itemID, &rcBounds, LVIR_BOUNDS );
					ListView_GetItemRect( hList, pdis->itemID, &rcLabel, LVIR_LABEL );
					RECT rcCol ( rcBounds );
					rcCol.right = rcCol.left + ListView_GetColumnWidth( hList, 0 );

					// Draw Item Label - Column 0
					wchar_t buffer[ 128 ];
					ListView_GetItemText( hList, pdis->itemID, 0, buffer, 128 );

					if ( pdis->itemState & ODS_SELECTED )
					{
						SetTextColor( pdis->hDC, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
						SetBkColor( pdis->hDC, GetSysColor( COLOR_HIGHLIGHT ) );
						FillRect( pdis->hDC, &rcBounds, GetSysColorBrush( COLOR_HIGHLIGHT ) );
					}
					else
					{
						SetTextColor( pdis->hDC, GetSysColor( COLOR_WINDOWTEXT ) );

						COLORREF color;

						if ( m_vBookmarks[ pdis->itemID ]->Frozen() )
							color = RGB( 255, 255, 160 );
						else
							color = GetSysColor( COLOR_WINDOW );

						HBRUSH hBrush = CreateSolidBrush( color );
						SetBkColor( pdis->hDC, color );
						FillRect( pdis->hDC, &rcBounds, hBrush );
						DeleteObject( hBrush );
					}

					if ( wcslen( buffer ) > 0 )
					{
						rcLabel.left += ( offset / 2 );
						rcLabel.right -= offset;

						DrawText( pdis->hDC, buffer, wcslen( buffer ), &rcLabel, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER | DT_END_ELLIPSIS );
					}

					// Draw Item Label for remaining columns
					LV_COLUMN lvc;
					lvc.mask = LVCF_FMT | LVCF_WIDTH;

					for ( size_t i = 1; ListView_GetColumn( hList, i, &lvc ); ++i )
					{
						rcCol.left = rcCol.right;
						rcCol.right += lvc.cx;

						ListView_GetItemText( hList, pdis->itemID, i, buffer, 256 );
						if ( wcslen( buffer ) == 0 )
							continue;

						UINT nJustify = DT_LEFT;
						switch ( lvc.fmt & LVCFMT_JUSTIFYMASK )
						{
							case LVCFMT_RIGHT:
								nJustify = DT_RIGHT;
								break;
							case LVCFMT_CENTER:
								nJustify = DT_CENTER;
								break;
							default:
								break;
						}

						rcLabel = rcCol;
						rcLabel.left += offset;
						rcLabel.right -= offset;

						DrawText( pdis->hDC, buffer, wcslen( buffer ), &rcLabel, nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS );
					}

					//if (pdis->itemState & ODS_SELECTED) //&& (GetFocus() == this)
					//	DrawFocusRect(pdis->hDC, &rcBounds);

					break;

				case ODA_FOCUS:
					break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
		{
			switch ( LOWORD( wParam ) )
			{
				case IDC_RA_LBX_ADDRESSES:
					if ( ( (LPNMHDR)lParam )->code == NM_CLICK )
					{
						hList = GetDlgItem( m_hMemBookmarkDialog, IDC_RA_LBX_ADDRESSES );

						nSelect = ListView_GetNextItem( hList, -1, LVNI_FOCUSED );

						if ( nSelect == -1 )
							break;
					}
			}
			return TRUE;
		}
		case WM_COMMAND:
		{
			switch ( LOWORD( wParam ) )
			{
				case IDOK:
				case IDCLOSE:
				case IDCANCEL:
					EndDialog( hDlg, true );
					return TRUE;

				case IDC_RA_ADD_BOOKMARK:
				{
					if ( g_MemoryDialog.GetHWND() != nullptr )
						AddAddress();

					return TRUE;
				}
				case IDC_RA_DEL_BOOKMARK:
				{
					return TRUE;
				}
				case IDC_RA_FREEZE:
				{
					if ( m_vBookmarks.size() > 0 )
					{
						hList = GetDlgItem( m_hMemBookmarkDialog, IDC_RA_LBX_ADDRESSES );
						unsigned int uSelectedCount = ListView_GetSelectedCount( hList );

						if ( uSelectedCount > 0 )
						{
							for ( int i = ListView_GetNextItem( hList, -1, LVNI_SELECTED ); i >= 0; i = ListView_GetNextItem( hList, i, LVNI_SELECTED ) )
								m_vBookmarks[ i ]->SetFrozen( !m_vBookmarks[ i ]->Frozen() );
						}
						ListView_SetItemState( hList, -1, LVIF_STATE, LVIS_SELECTED );
					}
					return TRUE;
				}
				case IDC_RA_CLEAR_CHANGE:
				{
					if ( m_vBookmarks.size() > 0 )
					{
						hList = GetDlgItem( m_hMemBookmarkDialog, IDC_RA_LBX_ADDRESSES );
						int idx = -1;
						for ( MemBookmark* bookmark : m_vBookmarks )
						{
							idx++;

							bookmark->ResetCount();

							LV_ITEM item;
							ZeroMemory( &item, sizeof( item ) );
							item.mask = LVIF_TEXT;
							item.iItem = idx;
							item.cchTextMax = 256;

							UpdateListItem( hList, item, *bookmark );
						}
					}

					return TRUE;
				}
				default:
					return FALSE;
			}
		}
		default:
			break;
	}

	return FALSE;
}

void Dlg_MemBookmark::UpdateBookmarks( bool bForceWrite )
{
	if ( !IsWindowVisible( m_hMemBookmarkDialog ) )
		return;

	if ( m_vBookmarks.size() == 0 )
		return;

	HWND hList = GetDlgItem( m_hMemBookmarkDialog, IDC_RA_LBX_ADDRESSES );

	int idx = -1;
	for ( MemBookmark* bookmark : m_vBookmarks )
	{
		idx++;

		if ( bookmark->Frozen() && !bForceWrite )
		{
			WriteFrozenValue( *bookmark );
			continue;
		}

		std::string mem_string = GetMemory( bookmark->Address(), bookmark->Type() );

		if ( bookmark->Value() != mem_string )
		{
			bookmark->SetPrevious( bookmark->Value() );
			bookmark->SetValue( mem_string );
			bookmark->IncreaseCount();

			LV_ITEM item;
			ZeroMemory( &item, sizeof( item ) );
			item.mask = LVIF_TEXT;
			item.iItem = idx;
			item.cchTextMax = 256;

			UpdateListItem( hList, item, *bookmark );
		}
	}
}

void Dlg_MemBookmark::PopulateList()
{
	if ( m_vBookmarks.size() == 0 )
		return;

	HWND hAddrList = GetDlgItem( m_hMemBookmarkDialog, IDC_RA_LBX_ADDRESSES );
	if ( hAddrList == NULL )
		return;

	ListView_DeleteAllItems( hAddrList );
	m_nNumOccupiedRows = 0;

	for ( MemBookmark* bookmark : m_vBookmarks )
	{
		LV_ITEM item;
		ZeroMemory( &item, sizeof( item ) );
		item.mask = LVIF_TEXT;
		item.cchTextMax = 256;
		item.iItem = m_nNumOccupiedRows;
		item.iSubItem = 0;
		item.iItem = ListView_InsertItem( hAddrList, &item );

		UpdateListItem( hAddrList, item, *bookmark );

		ASSERT( item.iItem == m_nNumOccupiedRows );

		m_nNumOccupiedRows++;
	}
}

void Dlg_MemBookmark::UpdateListItem( HWND hList, LV_ITEM& item, const MemBookmark& Bookmark )
{
	int nRow = item.iItem;

	snprintf( m_lbxData[ nRow ][ CSI_DESC ], MAX_STRING_TEXT_LEN, "%s", Bookmark.Description().c_str() );
	snprintf( m_lbxData[ nRow ][ CSI_ADDRESS ], MAX_STRING_TEXT_LEN, "%06x", Bookmark.Address() );
	snprintf( m_lbxData[ nRow ][ CSI_VALUE ], MAX_STRING_TEXT_LEN, "%s", Bookmark.Value().c_str() );
	snprintf( m_lbxData[ nRow ][ CSI_PREVIOUS ], MAX_STRING_TEXT_LEN, "%s", Bookmark.Previous().c_str() );
	snprintf( m_lbxData[ nRow ][ CSI_CHANGES ], MAX_STRING_TEXT_LEN, "%d", Bookmark.Count() );

	switch ( Bookmark.Type() )
	{
		case 1:
			snprintf( m_lbxData[ nRow ][ CSI_TYPE ], MAX_STRING_TEXT_LEN, "%s", "8bit" );
			break;
		case 2:
			snprintf( m_lbxData[ nRow ][ CSI_TYPE ], MAX_STRING_TEXT_LEN, "%s", "16bit" );
			break;
		case 3:
			snprintf( m_lbxData[ nRow ][ CSI_TYPE ], MAX_STRING_TEXT_LEN, "%s", "32bit" );
			break;
		default:
			snprintf( m_lbxData[ nRow ][ CSI_TYPE ], MAX_STRING_TEXT_LEN, "%s", "???" );
			break;
	}

	/*if (g_bPreferDecimalVal)
	{
		if (Cond.CompTarget().Type() == ValueComparison)
			sprintf_s(m_lbxData[nRow][CSI_VALUE_TGT], MAX_STRING_TEXT_LEN, "%d", Cond.CompTarget().RawValue());
	}*/

	for ( size_t i = 0; i < NumColumns; ++i )
	{
		item.iSubItem = i;
		std::wstring sData = Widen( m_lbxData[ nRow ][ i ] );
		item.pszText = const_cast<LPWSTR>( sData.c_str() );
		ListView_SetItem( hList, &item );
	}
}

void Dlg_MemBookmark::SetupColumns( HWND hList )
{
	//	Remove all columns,
	while ( ListView_DeleteColumn( hList, 0 ) ) {}

	//	Remove all data.
	ListView_DeleteAllItems( hList );

	LV_COLUMN col;
	ZeroMemory( &col, sizeof( col ) );

	for ( size_t i = 0; i < m_nNumCols; ++i )
	{
		col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
		col.cx = COLUMN_WIDTH[ i ];
		std::wstring colTitle = Widen( COLUMN_TITLE[ i ] );
		col.pszText = const_cast<LPWSTR>( colTitle.c_str() );
		col.cchTextMax = 255;
		col.iSubItem = i;

		col.fmt = LVCFMT_CENTER | LVCFMT_FIXED_WIDTH;
		if ( i == m_nNumCols - 1 )
			col.fmt |= LVCFMT_FILL;

		ListView_InsertColumn( hList, i, (LPARAM)&col );
	}

	ZeroMemory( &m_lbxData, sizeof( m_lbxData ) );

	m_nNumOccupiedRows = 0;

	BOOL bSuccess = ListView_SetExtendedListViewStyle( hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER );
	bSuccess = ListView_EnableGroupView( hList, FALSE );

}

void Dlg_MemBookmark::AddAddress()
{
	if ( g_pCurrentGameData->GetGameID() == 0 )
		return;

	MemBookmark* NewBookmark = new MemBookmark();

	// Fetch Memory Address from Memory Inspector
	wchar_t buffer[ 256 ];
	GetDlgItemText( g_MemoryDialog.GetHWND(), IDC_RA_WATCHING, buffer, 256 );
	unsigned int nAddr = strtol( Narrow( buffer ).c_str(), nullptr, 16 );
	NewBookmark->SetAddress( nAddr );

	// Check Data Type
	if ( SendDlgItemMessage( g_MemoryDialog.GetHWND(), IDC_RA_MEMVIEW8BIT, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
		NewBookmark->SetType( 1 );
	else if ( SendDlgItemMessage( g_MemoryDialog.GetHWND(), IDC_RA_MEMVIEW16BIT, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
		NewBookmark->SetType( 2 );
	else
		NewBookmark->SetType( 3 );

	// Get Memory Value
	NewBookmark->SetValue( GetMemory( nAddr, NewBookmark->Type() ) );
	NewBookmark->SetPrevious( NewBookmark->Value() );

	// Get Code Note and add as description
	const CodeNotes::CodeNoteObj* pSavedNote = g_MemoryDialog.Notes().FindCodeNote( nAddr );
	if ( ( pSavedNote != nullptr ) && ( pSavedNote->Note().length() > 0 ) )
		NewBookmark->SetDescription( pSavedNote->Note() );

	// Add Bookmark to vector and map
	AddBookmark( NewBookmark );
	AddBookmarkMap( NewBookmark );

	PopulateList();
}

void Dlg_MemBookmark::WriteFrozenValue( const MemBookmark & Bookmark )
{
	//if ( !Bookmark.Frozen() )
	//	return;

	unsigned int addr;
	int n;
	char c;

	switch ( Bookmark.Type() )
	{
		case 1:
			addr = Bookmark.Address();

			for ( unsigned int i = 0; i < Bookmark.Value().length(); i++ )
			{
				c = Bookmark.Value()[ i ];
				n = ( c >= 'a' ) ? ( c - 'a' + 10 ) : ( c - '0' );
				MemoryViewerControl::editData( addr, ( i % 2 != 0 ), n );
			}
			break;
		default:
			break;
	}
}

std::string Dlg_MemBookmark::GetMemory( unsigned int nAddr, int type )
{
	char memory_buffer[ 128 ];

	switch ( type )
	{
		case 1:
			sprintf_s( memory_buffer, 128, "%02x", g_MemManager.ActiveBankRAMByteRead( nAddr ) );
			break;
		case 2:
			sprintf_s( memory_buffer, 128, "%04x", g_MemManager.ActiveBankRAMByteRead( nAddr ) | ( g_MemManager.ActiveBankRAMByteRead( nAddr + 1 ) << 8 ) );
			break;
		case 3:
			sprintf_s( memory_buffer, 128, "%08x", g_MemManager.ActiveBankRAMByteRead( nAddr ) | ( g_MemManager.ActiveBankRAMByteRead( nAddr + 1 ) << 8 ) |
				( g_MemManager.ActiveBankRAMByteRead( nAddr + 2 ) << 16 ) | ( g_MemManager.ActiveBankRAMByteRead( nAddr + 3 ) << 24 ) );
			break;
	}

	//for (int i = 0; i < strlen(memory_buffer); i++)
	//	memory_buffer[i] = toupper( memory_buffer[i] );

	return memory_buffer;
}
