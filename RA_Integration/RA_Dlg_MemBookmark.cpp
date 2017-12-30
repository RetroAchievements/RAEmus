#include "RA_Dlg_MemBookmark.h"

#include "RA_Resource.h"
#include "RA_GameData.h"
#include "RA_Dlg_Memory.h"
#include "RA_MemManager.h"

#include <strsafe.h>

Dlg_MemBookmark g_MemBookmarkDialog;

WNDPROC EOldProcBM;
HWND g_hIPEEditBM;
int nSelItemBM;
int nSelSubItemBM;

namespace
{
	const char* COLUMN_TITLE[] = { "Description", "Address", "Value", "Prev.", "Changes" };
	const int COLUMN_WIDTH[] = { 112, 64, 64, 64, 54 };
	static_assert( SIZEOF_ARRAY( COLUMN_TITLE ) == SIZEOF_ARRAY( COLUMN_WIDTH ), "Must match!" );
}

enum BookmarkSubItems
{
	CSI_DESC,
	CSI_ADDRESS,
	CSI_VALUE,
	CSI_PREVIOUS,
	CSI_CHANGES,

	NumColumns
};

INT_PTR CALLBACK Dlg_MemBookmark::s_MemBookmarkDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return g_MemBookmarkDialog.MemBookmarkDialogProc( hDlg, uMsg, wParam, lParam );
}

long _stdcall EditProcBM( HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( nMsg )
	{
		case WM_DESTROY:
			g_hIPEEditBM = nullptr;
			break;

		case WM_KILLFOCUS:
		{
			LV_DISPINFO lvDispinfo;
			ZeroMemory( &lvDispinfo, sizeof( LV_DISPINFO ) );
			lvDispinfo.hdr.hwndFrom = hwnd;
			lvDispinfo.hdr.idFrom = GetDlgCtrlID( hwnd );
			lvDispinfo.hdr.code = LVN_ENDLABELEDIT;
			lvDispinfo.item.mask = LVIF_TEXT;
			lvDispinfo.item.iItem = nSelItemBM;
			lvDispinfo.item.iSubItem = nSelSubItemBM;
			lvDispinfo.item.pszText = nullptr;

			wchar_t sEditText[ 256 ];
			GetWindowText( hwnd, sEditText, 256 );
			g_MemBookmarkDialog.Bookmarks()[ nSelItemBM ]->SetDescription( sEditText );

			HWND hList = GetDlgItem( g_MemBookmarkDialog.GetHWND(), IDC_RA_LBX_ADDRESSES );

			//	the LV ID and the LVs Parent window's HWND
			SendMessage( GetParent( hList ), WM_NOTIFY, static_cast<WPARAM>( IDC_RA_LBX_ADDRESSES ), reinterpret_cast<LPARAM>( &lvDispinfo ) );	//	##reinterpret? ##SD

			DestroyWindow( hwnd );
			break;
		}

		case WM_KEYDOWN:
		{
			if ( wParam == VK_RETURN || wParam == VK_ESCAPE )
			{
				DestroyWindow( hwnd );	//	Causing a killfocus :)
			}
			else
			{
				//	Ignore keystroke, or pass it into the edit box!
				break;
			}

			break;
		}
	}

	return CallWindowProc( EOldProcBM, hwnd, nMsg, wParam, lParam );
}

INT_PTR Dlg_MemBookmark::MemBookmarkDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	PMEASUREITEMSTRUCT pmis;
	PDRAWITEMSTRUCT pdis;
	int nSelect;
	HWND hList;
	int offset = 2;

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

					hList = GetDlgItem( hDlg, IDC_RA_LBX_ADDRESSES );

					ListView_GetItemRect( hList, pdis->itemID, &rcBounds, LVIR_BOUNDS );
					ListView_GetItemRect( hList, pdis->itemID, &rcLabel, LVIR_LABEL );
					RECT rcCol ( rcBounds );
					rcCol.right = rcCol.left + ListView_GetColumnWidth( hList, 0 );

					// Draw Item Label - Column 0
					wchar_t buffer[ 256 ];
					swprintf ( buffer, sizeof( buffer ), L"%s", m_vBookmarks[ pdis->itemID ]->Description().c_str() );

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

						switch ( i )
						{
							case CSI_ADDRESS:
								swprintf ( buffer, sizeof( buffer ), L"%06x", m_vBookmarks[ pdis->itemID ]->Address() );
								break;
							case CSI_VALUE:
								swprintf ( buffer, sizeof( buffer ), L"%s", m_vBookmarks[ pdis->itemID ]->Value().c_str() );
								break;
							case CSI_PREVIOUS:
								swprintf ( buffer, sizeof( buffer ), L"%s", m_vBookmarks[ pdis->itemID ]->Previous().c_str() );
								break;
							case CSI_CHANGES:
								swprintf ( buffer, sizeof( buffer ), L"%d", m_vBookmarks[ pdis->itemID ]->Count() );
								break;
							default:
								swprintf ( buffer, sizeof( buffer ), L"" );
								break;
						}

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
						hList = GetDlgItem( hDlg, IDC_RA_LBX_ADDRESSES );

						nSelect = ListView_GetNextItem( hList, -1, LVNI_FOCUSED );

						if ( nSelect == -1 )
							break;
					}
					else if ( ( (LPNMHDR)lParam )->code == NM_DBLCLK )
					{
						hList = GetDlgItem( hDlg, IDC_RA_LBX_ADDRESSES );

						LPNMITEMACTIVATE pOnClick = (LPNMITEMACTIVATE)lParam;

						if ( pOnClick->iItem != -1 && pOnClick->iSubItem == CSI_DESC )
						{
							nSelItemBM = pOnClick->iItem;
							nSelSubItemBM = pOnClick->iSubItem;

							EditLabel ( pOnClick->iItem, pOnClick->iSubItem );
						}
						else if ( pOnClick->iItem != -1 && pOnClick->iSubItem == CSI_ADDRESS )
						{
							g_MemoryDialog.SetWatchingAddress( m_vBookmarks[ pOnClick->iItem ]->Address() );
							MemoryViewerControl::setAddress( ( m_vBookmarks[ pOnClick->iItem ]->Address() & 
								~( 0xf ) ) - ( (int)( MemoryViewerControl::m_nDisplayedLines / 2 ) << 4 ) + ( 0x50 ) );
						}
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
					HWND hList = GetDlgItem( hDlg, IDC_RA_LBX_ADDRESSES );
					int nSel = ListView_GetNextItem( hList, -1, LVNI_SELECTED );

					if ( nSel != -1 )
					{
						while ( nSel >= 0 )
						{
							MemBookmark* pBookmark = m_vBookmarks[ nSel ];

							// Remove from vector
							m_vBookmarks.erase( m_vBookmarks.begin() + nSel );

							// Remove from map
							std::vector<const MemBookmark*> *pVector;
							pVector = &m_BookmarkMap.find( pBookmark->Address() )->second;
							pVector->erase( std::find( pVector->begin(), pVector->end(), pBookmark ) );
							if ( pVector->size() == 0 )
								m_BookmarkMap.erase( pBookmark->Address() );

							delete pBookmark;

							ListView_DeleteItem( hList, nSel );

							nSel = ListView_GetNextItem( hList, -1, LVNI_SELECTED );
						}

						InvalidateRect( hList, NULL, TRUE );
					}

					return TRUE;
				}
				case IDC_RA_FREEZE:
				{
					if ( m_vBookmarks.size() > 0 )
					{
						hList = GetDlgItem( hDlg, IDC_RA_LBX_ADDRESSES );
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
						hList = GetDlgItem( hDlg, IDC_RA_LBX_ADDRESSES );
						int idx = -1;
						for ( MemBookmark* bookmark : m_vBookmarks )
						{
							idx++;

							bookmark->ResetCount();
						}
						
						InvalidateRect( hList, NULL, TRUE );
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

	for ( MemBookmark* bookmark : m_vBookmarks )
	{
		if ( bookmark->Frozen() && !bForceWrite )
		{
			WriteFrozenValue( *bookmark );
			continue;
		}

		std::wstring mem_string = GetMemory( bookmark->Address(), bookmark->Type() );

		if ( bookmark->Value() != mem_string )
		{
			bookmark->SetPrevious( bookmark->Value() );
			bookmark->SetValue( mem_string );
			bookmark->IncreaseCount();
		}
	}

	InvalidateRect( hList, NULL, TRUE );
}

void Dlg_MemBookmark::PopulateList()
{
	if ( m_vBookmarks.size() == 0 )
		return;

	HWND hList = GetDlgItem( m_hMemBookmarkDialog, IDC_RA_LBX_ADDRESSES );
	if ( hList == NULL )
		return;

	int topIndex = ListView_GetTopIndex( hList );
	ListView_DeleteAllItems( hList );
	m_nNumOccupiedRows = 0;

	for ( MemBookmark* bookmark : m_vBookmarks )
	{
		LV_ITEM item;
		ZeroMemory( &item, sizeof( item ) );
		item.mask = LVIF_TEXT;
		item.cchTextMax = 256;
		item.iItem = m_nNumOccupiedRows;
		item.iSubItem = 0;
		item.iItem = ListView_InsertItem( hList, &item );

		ASSERT( item.iItem == m_nNumOccupiedRows );

		m_nNumOccupiedRows++;
	}

	ListView_EnsureVisible( hList, m_vBookmarks.size() - 1, FALSE ); // Scroll to bottom.
	//ListView_EnsureVisible( hList, topIndex, TRUE ); // return to last position.
}

void Dlg_MemBookmark::SetupColumns( HWND hList )
{
	//	Remove all columns,
	while ( ListView_DeleteColumn( hList, 0 ) ) {}

	//	Remove all data.
	ListView_DeleteAllItems( hList );

	LV_COLUMN col;
	ZeroMemory( &col, sizeof( col ) );

	for ( size_t i = 0; i < NumColumns; ++i )
	{
		col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
		col.cx = COLUMN_WIDTH[ i ];
		std::wstring colTitle = Widen( COLUMN_TITLE[ i ] );
		col.pszText = const_cast<LPWSTR>( colTitle.c_str() );
		col.cchTextMax = 255;
		col.iSubItem = i;

		col.fmt = LVCFMT_CENTER | LVCFMT_FIXED_WIDTH;
		if ( i == NumColumns - 1 )
			col.fmt |= LVCFMT_FILL;

		ListView_InsertColumn( hList, i, (LPARAM)&col );
	}

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
		NewBookmark->SetDescription( Widen( pSavedNote->Note().c_str() ) );

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

std::wstring Dlg_MemBookmark::GetMemory( unsigned int nAddr, int type )
{
	wchar_t memory_buffer[ 128 ];

	switch ( type )
	{
		case 1:
			swprintf_s( memory_buffer, L"%02x", g_MemManager.ActiveBankRAMByteRead( nAddr ) );
			break;
		case 2:
			swprintf_s( memory_buffer, L"%04x", g_MemManager.ActiveBankRAMByteRead( nAddr ) | ( g_MemManager.ActiveBankRAMByteRead( nAddr + 1 ) << 8 ) );
			break;
		case 3:
			swprintf_s( memory_buffer, L"%08x", g_MemManager.ActiveBankRAMByteRead( nAddr ) | ( g_MemManager.ActiveBankRAMByteRead( nAddr + 1 ) << 8 ) |
				( g_MemManager.ActiveBankRAMByteRead( nAddr + 2 ) << 16 ) | ( g_MemManager.ActiveBankRAMByteRead( nAddr + 3 ) << 24 ) );
			break;
	}

	//for (int i = 0; i < strlen(memory_buffer); i++)
	//	memory_buffer[i] = toupper( memory_buffer[i] );

	return memory_buffer;
}

BOOL Dlg_MemBookmark::EditLabel ( int nItem, int nSubItem )
{
	HWND hList = GetDlgItem( g_MemBookmarkDialog.GetHWND(), IDC_RA_LBX_ADDRESSES );
	RECT rcSubItem;
	ListView_GetSubItemRect( hList, nItem, nSubItem, LVIR_BOUNDS, &rcSubItem );

	RECT rcOffset;
	GetWindowRect ( hList, &rcOffset );
	rcSubItem.left += rcOffset.left;
	rcSubItem.right += rcOffset.left;
	rcSubItem.top += rcOffset.top;
	rcSubItem.bottom += rcOffset.top;

	int nHeight = rcSubItem.bottom - rcSubItem.top;
	int nWidth = rcSubItem.right - rcSubItem.left;

	ASSERT ( g_hIPEEditBM == nullptr );
	if ( g_hIPEEditBM ) return FALSE;

	g_hIPEEditBM = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",
		L"",
		WS_CHILD | WS_VISIBLE | WS_POPUPWINDOW | WS_BORDER | ES_WANTRETURN,
		rcSubItem.left, rcSubItem.top, nWidth, (int)( 1.5f*nHeight ),
		g_MemBookmarkDialog.GetHWND(),
		0,
		GetModuleHandle( NULL ),
		NULL );

	if ( g_hIPEEditBM == NULL )
	{
		ASSERT( !"Could not create edit box!" );
		MessageBox( nullptr, L"Could not create edit box.", L"Error", MB_OK | MB_ICONERROR );
		return FALSE;
	};

	SendMessage( g_hIPEEditBM, WM_SETFONT, (WPARAM)GetStockObject( DEFAULT_GUI_FONT ), TRUE );
	SetWindowText( g_hIPEEditBM, m_vBookmarks[ nItem ]->Description().c_str() );

	SendMessage( g_hIPEEditBM, EM_SETSEL, 0, -1 );
	SetFocus( g_hIPEEditBM );
	EOldProcBM = (WNDPROC)SetWindowLong( g_hIPEEditBM, GWL_WNDPROC, (LONG)EditProcBM );

	return TRUE;
}
