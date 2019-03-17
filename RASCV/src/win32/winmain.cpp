/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 main ]
*/

#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <commdlg.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <stdio.h>
#include <dwmapi.h>
#include "../res/resource.h"
#include "../emu.h"
#include "../fifo.h"
#include "../fileio.h"

#if USE_RETROACHIEVEMENTS
#include "../retroachievements.h"
#endif

// emulation core
EMU* emu;

// menu
HMENU hMenu = NULL;
bool now_menuloop = false;

void update_menu(HWND hWnd, HMENU hMenu);
void show_menu_bar(HWND hWnd);
void hide_menu_bar(HWND hWnd);

// status bar
HWND hStatus = NULL;
bool status_bar_visible = false;

#ifdef USE_FLOPPY_DISK
uint32_t fd_status = 0x80000000;
#endif
#ifdef USE_QUICK_DISK
uint32_t qd_status = 0x80000000;
#endif
#ifdef USE_HARD_DISK
uint32_t hd_status = 0x80000000;
#endif
#ifdef USE_COMPACT_DISC
uint32_t cd_status = 0x80000000;
#endif
#ifdef USE_LASER_DISC
uint32_t ld_status = 0x80000000;
#endif
#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
_TCHAR tape_status[1024] = _T("uninitialized");
#endif

void show_status_bar(HWND hWnd);
void hide_status_bar(HWND hWnd);
int get_status_bar_height();
bool get_status_bar_updated();
void update_status_bar(HINSTANCE hInstance, LPDRAWITEMSTRUCT lpDrawItem);

// file
#ifdef USE_CART
void open_cart_dialog(HWND hWnd, int drv);
void open_recent_cart(int drv, int index);
#endif
#ifdef USE_FLOPPY_DISK
void open_floppy_disk_dialog(HWND hWnd, int drv);
void open_blank_floppy_disk_dialog(HWND hWnd, int drv, uint8_t type);
void open_floppy_disk(int drv, const _TCHAR* path, int bank);
void open_recent_floppy_disk(int drv, int index);
void select_d88_bank(int drv, int index);
void close_floppy_disk(int drv);
#endif
#ifdef USE_QUICK_DISK
void open_quick_disk_dialog(HWND hWnd, int drv);
void open_recent_quick_disk(int drv, int index);
#endif
#ifdef USE_HARD_DISK
void open_hard_disk_dialog(HWND hWnd, int drv);
void open_recent_hard_disk(int drv, int index);
#endif
#ifdef USE_TAPE
void open_tape_dialog(HWND hWnd, int drv, bool play);
void open_recent_tape(int drv, int index);
#endif
#ifdef USE_COMPACT_DISC
void open_compact_disc_dialog(HWND hWnd, int drv);
void open_recent_compact_disc(int drv, int index);
#endif
#ifdef USE_LASER_DISC
void open_laser_disc_dialog(HWND hWnd, int drv);
void open_recent_laser_disc(int drv, int index);
#endif
#ifdef USE_BINARY_FILE
void open_binary_dialog(HWND hWnd, int drv, bool load);
void open_recent_binary(int drv, int index);
#endif
#ifdef USE_BUBBLE
void open_bubble_casette_dialog(HWND hWnd, int drv);
void open_recent_bubble_casette(int drv, int index);
#endif
#if defined(USE_CART) || defined(USE_FLOPPY_DISK) || defined(USE_HARD_DISK) || defined(USE_TAPE) || defined(USE_COMPACT_DISC) || defined(USE_LASER_DISC) || defined(USE_BINARY_FILE) || defined(USE_BUBBLE)
#define SUPPORT_DRAG_DROP
#endif
#ifdef SUPPORT_DRAG_DROP
void open_dropped_file(HDROP hDrop);
void open_any_file(const _TCHAR* path);
#endif

_TCHAR* get_open_file_name(HWND hWnd, const _TCHAR* filter, const _TCHAR* title, const _TCHAR* new_file, _TCHAR* dir, size_t dir_len);

// screen
int desktop_width;
int desktop_height;
int desktop_bpp;
int prev_window_mode = 0;
bool now_fullscreen = false;

#define MAX_WINDOW	10
#define MAX_FULLSCREEN	50

int screen_mode_count;
int screen_mode_width[MAX_FULLSCREEN];
int screen_mode_height[MAX_FULLSCREEN];

void enum_screen_mode();
void set_window(HWND hWnd, int mode);

// input
#ifdef USE_AUTO_KEY
void start_auto_key();
#endif

// dialog
#ifdef USE_SOUND_VOLUME
BOOL CALLBACK VolumeWndProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
#endif
#ifdef USE_JOYSTICK
BOOL CALLBACK JoyWndProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK JoyToKeyWndProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
#endif

// buttons
#ifdef ONE_BOARD_MICRO_COMPUTER
void create_buttons(HWND hWnd);
void draw_button(HDC hDC, UINT index, UINT pressed);
#endif

// misc
bool win8_or_later = false;

void disable_dwm()
{
	HMODULE hLibrary = LoadLibrary(_T("dwmapi.dll"));
	if(hLibrary) {
		typedef HRESULT (WINAPI *DwmEnableCompositionFunction)(__in UINT uCompositionAction);
		DwmEnableCompositionFunction lpfnDwmEnableComposition;
		lpfnDwmEnableComposition = reinterpret_cast<DwmEnableCompositionFunction>(::GetProcAddress(hLibrary, "DwmEnableComposition"));
		if(lpfnDwmEnableComposition) {
			lpfnDwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
		}
		FreeLibrary(hLibrary);
	}
}

#ifdef USE_SOCKET
void update_socket(int ch, WPARAM wParam, LPARAM lParam)
{
	if(WSAGETSELECTERROR(lParam) != 0) {
		emu->disconnect_socket(ch);
		emu->notify_socket_disconnected(ch);
		return;
	}
	if(emu->get_socket(ch) != (int)wParam) {
		return;
	}
	switch(WSAGETSELECTEVENT(lParam)) {
	case FD_CONNECT:
		emu->notify_socket_connected(ch);
		break;
	case FD_CLOSE:
		emu->notify_socket_disconnected(ch);
		break;
	case FD_WRITE:
		emu->send_socket_data(ch);
		break;
	case FD_READ:
		emu->recv_socket_data(ch);
		break;
	}
}
#endif

#ifdef USE_STATE
const _TCHAR *state_file_path(int num)
{
	return create_local_path(_T("%s.sta%d"), _T(CONFIG_NAME), num);
}
#endif

// ----------------------------------------------------------------------------
// window main
// ----------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int iCmdShow)
{
	// get os version
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx((OSVERSIONINFO*)&osvi);
	win8_or_later = (osvi.dwPlatformId == 2 && (osvi.dwMajorVersion > 6 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 2)));
	
	// load config
	load_config(create_local_path(_T("%s.ini"), _T(CONFIG_NAME)));
	
	// create window
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = (WNDPROC)WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
//	wndclass.hCursor = 0;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wndclass.lpszClassName = _T("CWINDOW");
	RegisterClass(&wndclass);
	
	// get window position
	RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
	AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, TRUE);
	HDC hdcScr = GetDC(NULL);
	desktop_width = GetDeviceCaps(hdcScr, HORZRES);
	desktop_height = GetDeviceCaps(hdcScr, VERTRES);
	desktop_bpp = GetDeviceCaps(hdcScr, BITSPIXEL);
	ReleaseDC(NULL, hdcScr);
	int dest_x = (int)((desktop_width - (rect.right - rect.left)) / 2);
	int dest_y = (int)((desktop_height - (rect.bottom - rect.top)) / 2);
	//dest_x = (dest_x < 0) ? 0 : dest_x;
	dest_y = (dest_y < 0) ? 0 : dest_y;
	
	// show window
	HWND hWnd = CreateWindow(_T("CWINDOW"), _T(DEVICE_NAME), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
	                         dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);
	
	// show menu bar
	show_menu_bar(hWnd);
	
	// show status bar
	if(config.show_status_bar) {
		show_status_bar(hWnd);
	}
	
	int status_bar_height = get_status_bar_height();
	RECT rect_tmp;
	GetClientRect(hWnd, &rect_tmp);
	if(rect_tmp.bottom != WINDOW_HEIGHT + status_bar_height) {
		rect.bottom += WINDOW_HEIGHT + status_bar_height - rect_tmp.bottom;
		dest_y = (int)((desktop_height - (rect.bottom - rect.top)) / 2);
		dest_y = (dest_y < 0) ? 0 : dest_y;
		SetWindowPos(hWnd, NULL, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
	}
	
	// enumerate screen mode
	enum_screen_mode();
	
	// restore screen mode
	if(config.window_mode >= 0 && config.window_mode < MAX_WINDOW) {
		PostMessage(hWnd, WM_COMMAND, ID_SCREEN_WINDOW + config.window_mode, 0L);
	} else if(config.window_mode >= MAX_WINDOW && config.window_mode < screen_mode_count + MAX_WINDOW) {
		PostMessage(hWnd, WM_COMMAND, ID_SCREEN_FULLSCREEN + config.window_mode - MAX_WINDOW, 0L);
	} else {
		config.window_mode = 0;
		PostMessage(hWnd, WM_COMMAND, ID_SCREEN_WINDOW, 0L);
	}
	
	// accelerator
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	
	// initialize emulation core
	emu = new EMU(hWnd, hInstance);
	emu->set_host_window_size(WINDOW_WIDTH, WINDOW_HEIGHT, true);
#if USE_RETROACHIEVEMENTS
    RA_InitUI(emu);
#endif
	
#ifdef SUPPORT_DRAG_DROP
	// open command line path
	if(szCmdLine[0]) {
		if(szCmdLine[0] == _T('"')) {
			int len = _tcslen(szCmdLine);
			szCmdLine[len - 1] = _T('\0');
			szCmdLine++;
		}
		_TCHAR path[_MAX_PATH];
		get_long_full_path_name(szCmdLine, path, _MAX_PATH);
		open_any_file(path);
	}
#endif
	
	// set priority
	SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
	
	// main loop
	int total_frames = 0, draw_frames = 0, skip_frames = 0;
	DWORD next_time = 0;
	bool prev_skip = false;
	DWORD update_fps_time = 0;
	DWORD update_status_bar_time = 0;
	DWORD disable_screen_saver_time = 0;
	MSG msg;
	
	while(1) {
		// check window message
		if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if(!GetMessage(&msg, NULL, 0, 0)) {
#ifdef _DEBUG
				_CrtDumpMemoryLeaks();
#endif
				ExitProcess(0);	// trick
				return msg.wParam;
			}
			if(!TranslateAccelerator(hWnd, hAccel, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else if(emu) {
			// drive machine
			int run_frames = emu->run();
			total_frames += run_frames;
			
			// timing controls
			int sleep_period = 0;
			bool now_skip = (config.full_speed || emu->is_frame_skippable()) && !emu->is_video_recording() && !emu->is_sound_recording();
			
			if((prev_skip && !now_skip) || next_time == 0) {
				next_time = timeGetTime();
			}
			if(!now_skip) {
				static int accum = 0;
				accum += emu->get_frame_interval();
				int interval = accum >> 10;
				accum -= interval << 10;
				next_time += interval;
			}
			prev_skip = now_skip;
			
			if(next_time > timeGetTime()) {
				// update window if enough time
				draw_frames += emu->draw_screen();
				skip_frames = 0;
#if USE_RETROACHIEVEMENTS
                if (status_bar_visible) {
                    InvalidateRect(hStatus, NULL, true);
                    SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | SBT_OWNERDRAW | SBT_NOBORDERS, (LPARAM)NULL);
                }
#endif
				
				// sleep 1 frame priod if need
				DWORD current_time = timeGetTime();
				if((int)(next_time - current_time) >= 10) {
					sleep_period = next_time - current_time;
				}
			} else if(++skip_frames > (int)emu->get_frame_rate()) {
				// update window at least once per 1 sec in virtual machine time
				draw_frames += emu->draw_screen();
#if USE_RETROACHIEVEMENTS
                if (status_bar_visible) {
                    InvalidateRect(hStatus, NULL, true);
                    SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | SBT_OWNERDRAW | SBT_NOBORDERS, (LPARAM)NULL);
                }
#endif
				skip_frames = 0;
				next_time = timeGetTime();
			}
			Sleep(sleep_period);
			
			DWORD current_time = timeGetTime();

#if !USE_RETROACHIEVEMENTS
            // calc frame rate
			if(update_fps_time <= current_time) {
				if(update_fps_time != 0) {
					if(emu->message_count > 0) {
						SetWindowText(hWnd, create_string(_T("%s - %s"), _T(DEVICE_NAME), emu->message));
						emu->message_count--;
					} else if(now_skip) {
						int ratio = (int)(100.0 * (double)total_frames / emu->get_frame_rate() + 0.5);
						SetWindowText(hWnd, create_string(_T("%s - Skip Frames (%d %%)"), _T(DEVICE_NAME), ratio));
					} else {
						int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);
						SetWindowText(hWnd, create_string(_T("%s - %d fps (%d %%)"), _T(DEVICE_NAME), draw_frames, ratio));
					}
					update_fps_time += 1000;
					total_frames = draw_frames = 0;
				}
				update_fps_time = current_time + 1000;
			}
#endif
			
			// update status bar
#if !USE_RETROACHIEVEMENTS
			if(update_status_bar_time <= current_time) {
				if(hStatus != NULL && status_bar_visible) {
					if(get_status_bar_updated()) {
						SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | SBT_OWNERDRAW, (LPARAM)NULL);
//						InvalidateRect(hStatus, NULL, FALSE);
					}
				}
				update_status_bar_time = current_time + 200;
			}
#endif
			
			// disable screen saver
			if(disable_screen_saver_time <= current_time) {
				SetThreadExecutionState(ES_DISPLAY_REQUIRED);
				INPUT input[1];
				ZeroMemory(input, sizeof(INPUT));
				input[0].type = INPUT_MOUSE;
				input[0].mi.dwFlags = MOUSEEVENTF_MOVE;
				input[0].mi.dx = 0;
				input[0].mi.dy = 0;
				SendInput(1, input, sizeof(INPUT)); 
				disable_screen_saver_time = current_time + 30000;
			}
		}
	}
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}

// ----------------------------------------------------------------------------
// window procedure
// ----------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static HINSTANCE hInstance;
	static HIMC himcPrev = 0;
	
	switch(iMsg) {
	case WM_CREATE:
		if(config.disable_dwm && win8_or_later) {
			disable_dwm();
		}
#ifdef ONE_BOARD_MICRO_COMPUTER
		create_buttons(hWnd);
#endif
#ifdef SUPPORT_DRAG_DROP
		DragAcceptFiles(hWnd, TRUE);
#endif
		hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
		timeBeginPeriod(1);
		break;
	case WM_CLOSE:
#ifdef USE_NOTIFY_POWER_OFF
		// notify power off
		if(emu) {
			static bool notified = false;
			if(!notified) {
				emu->notify_power_off();
				notified = true;
				return 0;
			}
		}
#elif USE_RETROACHIEVEMENTS
        if (!RA_ConfirmLoadNewRom(true))
            return 0;
#endif
		// release window
		if(now_fullscreen) {
			ChangeDisplaySettings(NULL, 0);
		}
		now_fullscreen = false;
		if(hMenu != NULL && IsMenu(hMenu)) {
			DestroyMenu(hMenu);
			hMenu = NULL;
		}
		if(hStatus != NULL) {
			DestroyWindow(hStatus);
			hStatus = NULL;
		}
		DestroyWindow(hWnd);
		// release emulation core
		if(emu) {
			delete emu;
			emu = NULL;
		}
		save_config(create_local_path(_T("%s.ini"), _T(CONFIG_NAME)));
		timeEndPeriod(1);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_ACTIVATE:
		// thanks PC8801MA‰ü
		if(LOWORD(wParam) != WA_INACTIVE) {
			himcPrev = ImmAssociateContext(hWnd, 0);
		} else {
			ImmAssociateContext(hWnd, himcPrev);
		}
		break;
	case WM_SIZE:
		if(hStatus != NULL) {
			SendMessage(hStatus, WM_SIZE, wParam, lParam);
		}
#ifdef ONE_BOARD_MICRO_COMPUTER
		if(emu) {
			emu->reload_bitmap();
		}
#endif
		break;
	case WM_KILLFOCUS:
		if(emu) {
			emu->key_lost_focus();
		}
		break;
	case WM_PAINT:
		if(emu) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
#ifdef ONE_BOARD_MICRO_COMPUTER
			// check if self invalidate or not
			int left = 0, top = 0, right = 0, bottom = 0;
			emu->get_invalidated_rect(&left, &top, &right, &bottom);
			if(ps.rcPaint.left != left || ps.rcPaint.top != top || ps.rcPaint.right != right || ps.rcPaint.bottom != bottom) {
				emu->reload_bitmap();
			}
#endif
			emu->update_screen(hdc);
			EndPaint(hWnd, &ps);
		}
		return 0;
	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
			if(lpDrawItem->CtlID == ID_STATUS) {
				if(emu) {
					update_status_bar(hInstance, lpDrawItem);
				}
			} else {
				#ifdef ONE_BOARD_MICRO_COMPUTER
//					if(lpDrawItem->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
						draw_button(lpDrawItem->hDC, (UINT)wParam - ID_BUTTON, lpDrawItem->itemState & ODS_SELECTED);
//					}
				#endif
			}
		}
		return TRUE;
	case WM_MOVING:
		if(emu) {
			emu->suspend();
		}
		break;
	case WM_KEYDOWN:
		if(emu) {
			bool extended = ((HIWORD(lParam) & 0x100) != 0);
			bool repeat = ((HIWORD(lParam) & 0x4000) != 0);
			emu->key_down(LOBYTE(wParam), extended, repeat);
		}
		break;
	case WM_KEYUP:
		if(emu) {
			bool extended = ((HIWORD(lParam) & 0x100) != 0);
			emu->key_up(LOBYTE(wParam), extended);
		}
		break;
	case WM_SYSKEYDOWN:
		if(emu) {
			bool extended = ((HIWORD(lParam) & 0x100) != 0);
			bool repeat = ((HIWORD(lParam) & 0x4000) != 0);
			emu->key_down(LOBYTE(wParam), extended, repeat);
		}
		return 0;	// not activate menu when hit ALT/F10
	case WM_SYSKEYUP:
		if(emu) {
			bool extended = ((HIWORD(lParam) & 0x100) != 0);
			emu->key_up(LOBYTE(wParam), extended);
		}
		return 0;	// not activate menu when hit ALT/F10
	case WM_CHAR:
		if(emu) {
			emu->key_char(LOBYTE(wParam));
		}
		break;
	case WM_SYSCHAR:
		return 0;	// not activate menu when hit ALT/F10
	case WM_INITMENUPOPUP:
		if(emu) {
			emu->suspend();
		}
		update_menu(hWnd, (HMENU)wParam);
		break;
	case WM_ENTERMENULOOP:
		now_menuloop = true;
		break;
	case WM_EXITMENULOOP:
		if(now_fullscreen && now_menuloop) {
			hide_menu_bar(hWnd);
		}
		now_menuloop = false;
		break;
	case WM_MOUSEMOVE:
		if(now_fullscreen && !now_menuloop) {
			POINTS p = MAKEPOINTS(lParam);
			if(p.y == 0) {
				show_menu_bar(hWnd);
			} else if(p.y > 32) {
				hide_menu_bar(hWnd);
			}
		}
		break;
	case WM_RESIZE:
		if(emu) {
			if(now_fullscreen) {
				emu->set_host_window_size(-1, -1, false);
			} else {
				set_window(hWnd, config.window_mode);
			}
		}
		break;
#ifdef SUPPORT_DRAG_DROP
	case WM_DROPFILES:
		if(emu) {
			open_dropped_file((HDROP)wParam);
		}
		break;
#endif
#ifdef USE_SOCKET
	case WM_SOCKET0: case WM_SOCKET1: case WM_SOCKET2: case WM_SOCKET3:
		if(emu) {
			update_socket(iMsg - WM_SOCKET0, wParam, lParam);
		}
		break;
#endif
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case ID_RESET:
			if(emu) {
				emu->reset();
			}
			break;
#ifdef USE_SPECIAL_RESET
		case ID_SPECIAL_RESET:
			if(emu) {
				emu->special_reset();
			}
			break;
#endif
		case ID_CPU_POWER0: case ID_CPU_POWER1: case ID_CPU_POWER2: case ID_CPU_POWER3: case ID_CPU_POWER4:
			config.cpu_power = LOWORD(wParam) - ID_CPU_POWER0;
			if(emu) {
				emu->update_config();
			}
			break;
		case ID_FULL_SPEED:
			config.full_speed = !config.full_speed;
			break;
#ifdef USE_AUTO_KEY
		case ID_AUTOKEY_START:
			if(emu) {
				start_auto_key();
			}
			break;
		case ID_AUTOKEY_STOP:
			if(emu) {
				emu->stop_auto_key();
			}
			break;
		case ID_ROMAJI_TO_KANA:
			if(emu) {
				if(!config.romaji_to_kana) {
					emu->set_auto_key_char(1); // start
					config.romaji_to_kana = true;
				} else {
					emu->set_auto_key_char(0); // end
					config.romaji_to_kana = false;
				}
			}
			break;
#endif
#ifdef USE_DEBUGGER
		case ID_OPEN_DEBUGGER0: case ID_OPEN_DEBUGGER1: case ID_OPEN_DEBUGGER2: case ID_OPEN_DEBUGGER3:
		case ID_OPEN_DEBUGGER4: case ID_OPEN_DEBUGGER5: case ID_OPEN_DEBUGGER6: case ID_OPEN_DEBUGGER7:
			if(emu) {
				emu->open_debugger(LOWORD(wParam) - ID_OPEN_DEBUGGER0);
			}
			break;
		case ID_CLOSE_DEBUGGER:
			if(emu) {
				emu->close_debugger();
			}
			break;
#endif
#ifdef USE_STATE
		case ID_SAVE_STATE0: case ID_SAVE_STATE1: case ID_SAVE_STATE2: case ID_SAVE_STATE3: case ID_SAVE_STATE4:
		case ID_SAVE_STATE5: case ID_SAVE_STATE6: case ID_SAVE_STATE7: case ID_SAVE_STATE8: case ID_SAVE_STATE9:
			if(emu) {
				emu->save_state(state_file_path(LOWORD(wParam) - ID_SAVE_STATE0));
			}
			break;
		case ID_LOAD_STATE0: case ID_LOAD_STATE1: case ID_LOAD_STATE2: case ID_LOAD_STATE3: case ID_LOAD_STATE4:
		case ID_LOAD_STATE5: case ID_LOAD_STATE6: case ID_LOAD_STATE7: case ID_LOAD_STATE8: case ID_LOAD_STATE9:
			if(emu) {
				emu->load_state(state_file_path(LOWORD(wParam) - ID_LOAD_STATE0));
			}
			break;
#endif
		case ID_EXIT:
			SendMessage(hWnd, WM_CLOSE, 0, 0L);
			break;
#ifdef USE_BOOT_MODE
		case ID_VM_BOOT_MODE0: case ID_VM_BOOT_MODE1: case ID_VM_BOOT_MODE2: case ID_VM_BOOT_MODE3:
		case ID_VM_BOOT_MODE4: case ID_VM_BOOT_MODE5: case ID_VM_BOOT_MODE6: case ID_VM_BOOT_MODE7:
			config.boot_mode = LOWORD(wParam) - ID_VM_BOOT_MODE0;
			if(emu) {
				emu->update_config();
			}
			break;
#endif
#ifdef USE_CPU_TYPE
		case ID_VM_CPU_TYPE0: case ID_VM_CPU_TYPE1: case ID_VM_CPU_TYPE2: case ID_VM_CPU_TYPE3:
		case ID_VM_CPU_TYPE4: case ID_VM_CPU_TYPE5: case ID_VM_CPU_TYPE6: case ID_VM_CPU_TYPE7:
			config.cpu_type = LOWORD(wParam) - ID_VM_CPU_TYPE0;
			// need to recreate vm class instance
//			if(emu) {
//				emu->update_config();
//			}
			break;
#endif
#ifdef USE_DIPSWITCH
		case ID_VM_DIPSWITCH0:  case ID_VM_DIPSWITCH1:  case ID_VM_DIPSWITCH2:  case ID_VM_DIPSWITCH3: 
		case ID_VM_DIPSWITCH4:  case ID_VM_DIPSWITCH5:  case ID_VM_DIPSWITCH6:  case ID_VM_DIPSWITCH7: 
		case ID_VM_DIPSWITCH8:  case ID_VM_DIPSWITCH9:  case ID_VM_DIPSWITCH10: case ID_VM_DIPSWITCH11:
		case ID_VM_DIPSWITCH12: case ID_VM_DIPSWITCH13: case ID_VM_DIPSWITCH14: case ID_VM_DIPSWITCH15:
		case ID_VM_DIPSWITCH16: case ID_VM_DIPSWITCH17: case ID_VM_DIPSWITCH18: case ID_VM_DIPSWITCH19:
		case ID_VM_DIPSWITCH20: case ID_VM_DIPSWITCH21: case ID_VM_DIPSWITCH22: case ID_VM_DIPSWITCH23:
		case ID_VM_DIPSWITCH24: case ID_VM_DIPSWITCH25: case ID_VM_DIPSWITCH26: case ID_VM_DIPSWITCH27:
		case ID_VM_DIPSWITCH28: case ID_VM_DIPSWITCH29: case ID_VM_DIPSWITCH30: case ID_VM_DIPSWITCH31:
			config.dipswitch ^= (1 << (LOWORD(wParam) - ID_VM_DIPSWITCH0));
			break;
#endif
#ifdef USE_DEVICE_TYPE
		case ID_VM_DEVICE_TYPE0: case ID_VM_DEVICE_TYPE1: case ID_VM_DEVICE_TYPE2: case ID_VM_DEVICE_TYPE3:
		case ID_VM_DEVICE_TYPE4: case ID_VM_DEVICE_TYPE5: case ID_VM_DEVICE_TYPE6: case ID_VM_DEVICE_TYPE7:
			config.device_type = LOWORD(wParam) - ID_VM_DEVICE_TYPE0;
			break;
#endif
#ifdef USE_DRIVE_TYPE
		case ID_VM_DRIVE_TYPE0: case ID_VM_DRIVE_TYPE1: case ID_VM_DRIVE_TYPE2: case ID_VM_DRIVE_TYPE3:
		case ID_VM_DRIVE_TYPE4: case ID_VM_DRIVE_TYPE5: case ID_VM_DRIVE_TYPE6: case ID_VM_DRIVE_TYPE7:
			config.drive_type = LOWORD(wParam) - ID_VM_DRIVE_TYPE0;
			if(emu) {
				emu->update_config();
			}
			break;
#endif
#ifdef USE_KEYBOARD_TYPE
		case ID_VM_KEYBOARD_TYPE0: case ID_VM_KEYBOARD_TYPE1: case ID_VM_KEYBOARD_TYPE2: case ID_VM_KEYBOARD_TYPE3:
		case ID_VM_KEYBOARD_TYPE4: case ID_VM_KEYBOARD_TYPE5: case ID_VM_KEYBOARD_TYPE6: case ID_VM_KEYBOARD_TYPE7:
			config.keyboard_type = LOWORD(wParam) - ID_VM_KEYBOARD_TYPE0;
			if(emu) {
				emu->update_config();
			}
			break;
#endif
#ifdef USE_MOUSE_TYPE
		case ID_VM_MOUSE_TYPE0: case ID_VM_MOUSE_TYPE1: case ID_VM_MOUSE_TYPE2: case ID_VM_MOUSE_TYPE3:
		case ID_VM_MOUSE_TYPE4: case ID_VM_MOUSE_TYPE5: case ID_VM_MOUSE_TYPE6: case ID_VM_MOUSE_TYPE7:
			config.mouse_type = LOWORD(wParam) - ID_VM_MOUSE_TYPE0;
			if(emu) {
				emu->update_config();
			}
			break;
#endif
#ifdef USE_JOYSTICK_TYPE
		case ID_VM_JOYSTICK_TYPE0: case ID_VM_JOYSTICK_TYPE1: case ID_VM_JOYSTICK_TYPE2: case ID_VM_JOYSTICK_TYPE3:
		case ID_VM_JOYSTICK_TYPE4: case ID_VM_JOYSTICK_TYPE5: case ID_VM_JOYSTICK_TYPE6: case ID_VM_JOYSTICK_TYPE7:
			config.joystick_type = LOWORD(wParam) - ID_VM_JOYSTICK_TYPE0;
			if(emu) {
				emu->update_config();
			}
			break;
#endif
#ifdef USE_SOUND_TYPE
		case ID_VM_SOUND_TYPE0: case ID_VM_SOUND_TYPE1: case ID_VM_SOUND_TYPE2: case ID_VM_SOUND_TYPE3:
		case ID_VM_SOUND_TYPE4: case ID_VM_SOUND_TYPE5: case ID_VM_SOUND_TYPE6: case ID_VM_SOUND_TYPE7:
			config.sound_type = LOWORD(wParam) - ID_VM_SOUND_TYPE0;
			//if(emu) {
			//	emu->update_config();
			//}
			break;
#endif
#ifdef USE_FLOPPY_DISK
		case ID_VM_SOUND_NOISE_FDD:
			config.sound_noise_fdd = !config.sound_noise_fdd;
			if(emu) {
				emu->update_config();
			}
			break;
#endif
#ifdef USE_TAPE
		case ID_VM_SOUND_NOISE_CMT:
			config.sound_noise_cmt = !config.sound_noise_cmt;
			if(emu) {
				emu->update_config();
			}
			break;
		case ID_VM_SOUND_PLAY_TAPE:
			config.sound_play_tape = !config.sound_play_tape;
			break;
#endif
#ifdef USE_MONITOR_TYPE
		case ID_VM_MONITOR_TYPE0: case ID_VM_MONITOR_TYPE1: case ID_VM_MONITOR_TYPE2: case ID_VM_MONITOR_TYPE3:
		case ID_VM_MONITOR_TYPE4: case ID_VM_MONITOR_TYPE5: case ID_VM_MONITOR_TYPE6: case ID_VM_MONITOR_TYPE7:
			config.monitor_type = LOWORD(wParam) - ID_VM_MONITOR_TYPE0;
			if(emu) {
#ifdef ONE_BOARD_MICRO_COMPUTER
				emu->reload_bitmap();
				emu->invalidate_screen();
#endif
				emu->update_config();
			}
			break;
#endif
#ifdef USE_SCANLINE
		case ID_VM_MONITOR_SCANLINE:
			config.scan_line = !config.scan_line;
			if(emu) {
				emu->update_config();
			}
			break;
#endif
#ifdef USE_PRINTER_TYPE
		case ID_VM_PRINTER_TYPE0: case ID_VM_PRINTER_TYPE1: case ID_VM_PRINTER_TYPE2: case ID_VM_PRINTER_TYPE3:
		case ID_VM_PRINTER_TYPE4: case ID_VM_PRINTER_TYPE5: case ID_VM_PRINTER_TYPE6: case ID_VM_PRINTER_TYPE7:
			config.printer_type = LOWORD(wParam) - ID_VM_PRINTER_TYPE0;
			break;
#endif
		case ID_HOST_REC_MOVIE_60FPS: case ID_HOST_REC_MOVIE_30FPS: case ID_HOST_REC_MOVIE_15FPS:
			if(emu) {
				static const int fps[3] = {60, 30, 15};
				emu->start_record_sound();
				if(!emu->start_record_video(fps[LOWORD(wParam) - ID_HOST_REC_MOVIE_60FPS])) {
					emu->stop_record_sound();
				}
			}
			break;
		case ID_HOST_REC_SOUND:
			if(emu) {
				emu->start_record_sound();
			}
			break;
		case ID_HOST_REC_STOP:
			if(emu) {
				emu->stop_record_video();
				emu->stop_record_sound();
			}
			break;
		case ID_HOST_CAPTURE_SCREEN:
			if(emu) {
				emu->capture_screen();
			}
			break;
		case ID_HOST_USE_D3D9:
			config.use_d3d9 = !config.use_d3d9;
			if(emu) {
				emu->set_host_window_size(-1, -1, !now_fullscreen);
			}
			break;
		case ID_HOST_WAIT_VSYNC:
			config.wait_vsync = !config.wait_vsync;
			if(emu) {
				emu->set_host_window_size(-1, -1, !now_fullscreen);
			}
			break;
		case ID_HOST_USE_DINPUT:
			config.use_dinput = !config.use_dinput;
			break;
		case ID_HOST_DISABLE_DWM:
			config.disable_dwm = !config.disable_dwm;
			break;
		case ID_HOST_SHOW_STATUS_BAR:
			config.show_status_bar = !config.show_status_bar;
			if(emu) {
				if(!now_fullscreen) {
					set_window(hWnd, prev_window_mode);
				}
			}
			break;
		case ID_SCREEN_WINDOW + 0: case ID_SCREEN_WINDOW + 1: case ID_SCREEN_WINDOW + 2: case ID_SCREEN_WINDOW + 3: case ID_SCREEN_WINDOW + 4:
		case ID_SCREEN_WINDOW + 5: case ID_SCREEN_WINDOW + 6: case ID_SCREEN_WINDOW + 7: case ID_SCREEN_WINDOW + 8: case ID_SCREEN_WINDOW + 9:
			if(emu) {
				set_window(hWnd, LOWORD(wParam) - ID_SCREEN_WINDOW);
			}
			break;
		case ID_SCREEN_FULLSCREEN +  0: case ID_SCREEN_FULLSCREEN +  1: case ID_SCREEN_FULLSCREEN +  2: case ID_SCREEN_FULLSCREEN +  3: case ID_SCREEN_FULLSCREEN +  4:
		case ID_SCREEN_FULLSCREEN +  5: case ID_SCREEN_FULLSCREEN +  6: case ID_SCREEN_FULLSCREEN +  7: case ID_SCREEN_FULLSCREEN +  8: case ID_SCREEN_FULLSCREEN +  9:
		case ID_SCREEN_FULLSCREEN + 10: case ID_SCREEN_FULLSCREEN + 11: case ID_SCREEN_FULLSCREEN + 12: case ID_SCREEN_FULLSCREEN + 13: case ID_SCREEN_FULLSCREEN + 14:
		case ID_SCREEN_FULLSCREEN + 15: case ID_SCREEN_FULLSCREEN + 16: case ID_SCREEN_FULLSCREEN + 17: case ID_SCREEN_FULLSCREEN + 18: case ID_SCREEN_FULLSCREEN + 19:
		case ID_SCREEN_FULLSCREEN + 20: case ID_SCREEN_FULLSCREEN + 21: case ID_SCREEN_FULLSCREEN + 22: case ID_SCREEN_FULLSCREEN + 23: case ID_SCREEN_FULLSCREEN + 24:
		case ID_SCREEN_FULLSCREEN + 25: case ID_SCREEN_FULLSCREEN + 26: case ID_SCREEN_FULLSCREEN + 27: case ID_SCREEN_FULLSCREEN + 28: case ID_SCREEN_FULLSCREEN + 29:
		case ID_SCREEN_FULLSCREEN + 30: case ID_SCREEN_FULLSCREEN + 31: case ID_SCREEN_FULLSCREEN + 32: case ID_SCREEN_FULLSCREEN + 33: case ID_SCREEN_FULLSCREEN + 34:
		case ID_SCREEN_FULLSCREEN + 35: case ID_SCREEN_FULLSCREEN + 36: case ID_SCREEN_FULLSCREEN + 37: case ID_SCREEN_FULLSCREEN + 38: case ID_SCREEN_FULLSCREEN + 39:
		case ID_SCREEN_FULLSCREEN + 40: case ID_SCREEN_FULLSCREEN + 41: case ID_SCREEN_FULLSCREEN + 42: case ID_SCREEN_FULLSCREEN + 43: case ID_SCREEN_FULLSCREEN + 44:
		case ID_SCREEN_FULLSCREEN + 45: case ID_SCREEN_FULLSCREEN + 46: case ID_SCREEN_FULLSCREEN + 47: case ID_SCREEN_FULLSCREEN + 48: case ID_SCREEN_FULLSCREEN + 49:
			if(emu && !now_fullscreen) {
				set_window(hWnd, LOWORD(wParam) - ID_SCREEN_FULLSCREEN + MAX_WINDOW);
			}
			break;
		case ID_SCREEN_WINDOW_STRETCH:
		case ID_SCREEN_WINDOW_ASPECT:
			config.window_stretch_type = LOWORD(wParam) - ID_SCREEN_WINDOW_STRETCH;
			if(emu) {
				if(!now_fullscreen) {
					set_window(hWnd, prev_window_mode);
				}
			}
			break;
		case ID_SCREEN_FULLSCREEN_DOTBYDOT:
		case ID_SCREEN_FULLSCREEN_STRETCH:
		case ID_SCREEN_FULLSCREEN_ASPECT:
		case ID_SCREEN_FULLSCREEN_FILL:
			config.fullscreen_stretch_type = LOWORD(wParam) - ID_SCREEN_FULLSCREEN_DOTBYDOT;
			if(emu) {
				if(now_fullscreen) {
					emu->set_host_window_size(-1, -1, false);
				}
			}
			break;
//#ifdef USE_SCREEN_ROTATE
		case ID_SCREEN_ROTATE_0: case ID_SCREEN_ROTATE_90: case ID_SCREEN_ROTATE_180: case ID_SCREEN_ROTATE_270:
			config.rotate_type = LOWORD(wParam) - ID_SCREEN_ROTATE_0;
			if(emu) {
				if(now_fullscreen) {
					emu->set_host_window_size(-1, -1, false);
				} else {
					set_window(hWnd, prev_window_mode);
				}
			}
			break;
//#endif
#ifdef USE_SCREEN_FILTER
		case ID_FILTER_RGB:
			config.filter_type = SCREEN_FILTER_RGB;
			break;
		case ID_FILTER_RF:
			config.filter_type = SCREEN_FILTER_RF;
			break;
		case ID_FILTER_NONE:
			config.filter_type = SCREEN_FILTER_NONE;
			break;
#endif
		case ID_SOUND_FREQ0: case ID_SOUND_FREQ1: case ID_SOUND_FREQ2: case ID_SOUND_FREQ3:
		case ID_SOUND_FREQ4: case ID_SOUND_FREQ5: case ID_SOUND_FREQ6: case ID_SOUND_FREQ7:
			config.sound_frequency = LOWORD(wParam) - ID_SOUND_FREQ0;
			if(emu) {
				emu->update_config();
			}
			break;
		case ID_SOUND_LATE0: case ID_SOUND_LATE1: case ID_SOUND_LATE2: case ID_SOUND_LATE3: case ID_SOUND_LATE4:
			config.sound_latency = LOWORD(wParam) - ID_SOUND_LATE0;
			if(emu) {
				emu->update_config();
			}
			break;
		case ID_SOUND_STRICT_RENDER: case ID_SOUND_LIGHT_RENDER:
			config.sound_strict_rendering = (LOWORD(wParam) == ID_SOUND_STRICT_RENDER);
			if(emu) {
				emu->update_config();
			}
			break;
#ifdef USE_SOUND_VOLUME
		case ID_SOUND_VOLUME:
			DialogBoxParam((HINSTANCE)GetModuleHandle(0), MAKEINTRESOURCE(IDD_VOLUME), hWnd, VolumeWndProc, 0);
			break;
#endif
#ifdef USE_JOYSTICK
		case ID_INPUT_JOYSTICK0: case ID_INPUT_JOYSTICK1: case ID_INPUT_JOYSTICK2: case ID_INPUT_JOYSTICK3:
		case ID_INPUT_JOYSTICK4: case ID_INPUT_JOYSTICK5: case ID_INPUT_JOYSTICK6: case ID_INPUT_JOYSTICK7:
			{
				LONG index = LOWORD(wParam) - ID_INPUT_JOYSTICK0;
				DialogBoxParam((HINSTANCE)GetModuleHandle(0), MAKEINTRESOURCE(IDD_JOYSTICK), hWnd, JoyWndProc, (LPARAM)&index);
			}
			break;
		case ID_INPUT_JOYTOKEY:
			{
				LONG index = 0;
				DialogBoxParam((HINSTANCE)GetModuleHandle(0), MAKEINTRESOURCE(IDD_JOYTOKEY), hWnd, JoyToKeyWndProc, (LPARAM)&index);
			}
			break;
#endif
#ifdef USE_VIDEO_CAPTURE
		case ID_CAPTURE_FILTER:
			if(emu) {
				emu->show_capture_dev_filter();
			}
			break;
		case ID_CAPTURE_PIN:
			if(emu) {
				emu->show_capture_dev_pin();
			}
			break;
		case ID_CAPTURE_SOURCE:
			if(emu) {
				emu->show_capture_dev_source();
			}
			break;
		case ID_CAPTURE_CLOSE:
			if(emu) {
				emu->close_capture_dev();
			}
			break;
		case ID_CAPTURE_DEVICE + 0: case ID_CAPTURE_DEVICE + 1: case ID_CAPTURE_DEVICE + 2: case ID_CAPTURE_DEVICE + 3:
		case ID_CAPTURE_DEVICE + 4: case ID_CAPTURE_DEVICE + 5: case ID_CAPTURE_DEVICE + 6: case ID_CAPTURE_DEVICE + 7:
			if(emu) {
				emu->open_capture_dev(LOWORD(wParam) - ID_CAPTURE_DEVICE, false);
			}
			break;
#endif
#ifdef USE_CART
	#if USE_CART >= 1
		#define CART_MENU_ITEMS(drv, ID_OPEN_CART, ID_CLOSE_CART, ID_RECENT_CART) \
		case ID_OPEN_CART: \
			if(emu) { \
				open_cart_dialog(hWnd, drv); \
			} \
			break; \
		case ID_CLOSE_CART: \
			if(emu) { \
				emu->close_cart(drv); \
			} \
			break; \
		case ID_RECENT_CART + 0: case ID_RECENT_CART + 1: case ID_RECENT_CART + 2: case ID_RECENT_CART + 3: \
		case ID_RECENT_CART + 4: case ID_RECENT_CART + 5: case ID_RECENT_CART + 6: case ID_RECENT_CART + 7: \
			if(emu) { \
				open_recent_cart(drv, LOWORD(wParam) - ID_RECENT_CART); \
			} \
			break;
		CART_MENU_ITEMS(0, ID_OPEN_CART1, ID_CLOSE_CART1, ID_RECENT_CART1)
	#endif
	#if USE_CART >= 2
		CART_MENU_ITEMS(1, ID_OPEN_CART2, ID_CLOSE_CART2, ID_RECENT_CART2)
	#endif
#endif
#ifdef USE_FLOPPY_DISK
	#if USE_FLOPPY_DISK >= 1
		#define FD_MENU_ITEMS(drv, ID_OPEN_FD, ID_CLOSE_FD, ID_OPEN_BLANK_2D_FD, ID_OPEN_BLANK_2DD_FD, ID_OPEN_BLANK_2HD_FD, ID_WRITE_PROTECT_FD, ID_CORRECT_TIMING_FD, ID_IGNORE_CRC_FD, ID_RECENT_FD, ID_SELECT_D88_BANK, ID_EJECT_D88_BANK) \
		case ID_OPEN_FD: \
			if(emu) { \
				open_floppy_disk_dialog(hWnd, drv); \
			} \
			break; \
		case ID_CLOSE_FD: \
			if(emu) { \
				close_floppy_disk(drv); \
			} \
			break; \
		case ID_OPEN_BLANK_2D_FD: \
			if(emu) { \
				open_blank_floppy_disk_dialog(hWnd, drv, 0x00); \
			} \
			break; \
		case ID_OPEN_BLANK_2DD_FD: \
			if(emu) { \
				open_blank_floppy_disk_dialog(hWnd, drv, 0x10); \
			} \
			break; \
		case ID_OPEN_BLANK_2HD_FD: \
			if(emu) { \
				open_blank_floppy_disk_dialog(hWnd, drv, 0x20); \
			} \
			break; \
		case ID_WRITE_PROTECT_FD: \
			if(emu) { \
				emu->is_floppy_disk_protected(drv, !emu->is_floppy_disk_protected(drv)); \
			} \
			break; \
		case ID_CORRECT_TIMING_FD: \
			config.correct_disk_timing[drv] = !config.correct_disk_timing[drv]; \
			break; \
		case ID_IGNORE_CRC_FD: \
			config.ignore_disk_crc[drv] = !config.ignore_disk_crc[drv]; \
			break; \
		case ID_RECENT_FD + 0: case ID_RECENT_FD + 1: case ID_RECENT_FD + 2: case ID_RECENT_FD + 3: \
		case ID_RECENT_FD + 4: case ID_RECENT_FD + 5: case ID_RECENT_FD + 6: case ID_RECENT_FD + 7: \
			if(emu) { \
				open_recent_floppy_disk(drv, LOWORD(wParam) - ID_RECENT_FD); \
			} \
			break; \
		case ID_SELECT_D88_BANK +  0: case ID_SELECT_D88_BANK +  1: case ID_SELECT_D88_BANK +  2: case ID_SELECT_D88_BANK +  3: \
		case ID_SELECT_D88_BANK +  4: case ID_SELECT_D88_BANK +  5: case ID_SELECT_D88_BANK +  6: case ID_SELECT_D88_BANK +  7: \
		case ID_SELECT_D88_BANK +  8: case ID_SELECT_D88_BANK +  9: case ID_SELECT_D88_BANK + 10: case ID_SELECT_D88_BANK + 11: \
		case ID_SELECT_D88_BANK + 12: case ID_SELECT_D88_BANK + 13: case ID_SELECT_D88_BANK + 14: case ID_SELECT_D88_BANK + 15: \
		case ID_SELECT_D88_BANK + 16: case ID_SELECT_D88_BANK + 17: case ID_SELECT_D88_BANK + 18: case ID_SELECT_D88_BANK + 19: \
		case ID_SELECT_D88_BANK + 20: case ID_SELECT_D88_BANK + 21: case ID_SELECT_D88_BANK + 22: case ID_SELECT_D88_BANK + 23: \
		case ID_SELECT_D88_BANK + 24: case ID_SELECT_D88_BANK + 25: case ID_SELECT_D88_BANK + 26: case ID_SELECT_D88_BANK + 27: \
		case ID_SELECT_D88_BANK + 28: case ID_SELECT_D88_BANK + 29: case ID_SELECT_D88_BANK + 30: case ID_SELECT_D88_BANK + 31: \
		case ID_SELECT_D88_BANK + 32: case ID_SELECT_D88_BANK + 33: case ID_SELECT_D88_BANK + 34: case ID_SELECT_D88_BANK + 35: \
		case ID_SELECT_D88_BANK + 36: case ID_SELECT_D88_BANK + 37: case ID_SELECT_D88_BANK + 38: case ID_SELECT_D88_BANK + 39: \
		case ID_SELECT_D88_BANK + 40: case ID_SELECT_D88_BANK + 41: case ID_SELECT_D88_BANK + 42: case ID_SELECT_D88_BANK + 43: \
		case ID_SELECT_D88_BANK + 44: case ID_SELECT_D88_BANK + 45: case ID_SELECT_D88_BANK + 46: case ID_SELECT_D88_BANK + 47: \
		case ID_SELECT_D88_BANK + 48: case ID_SELECT_D88_BANK + 49: case ID_SELECT_D88_BANK + 50: case ID_SELECT_D88_BANK + 51: \
		case ID_SELECT_D88_BANK + 52: case ID_SELECT_D88_BANK + 53: case ID_SELECT_D88_BANK + 54: case ID_SELECT_D88_BANK + 55: \
		case ID_SELECT_D88_BANK + 56: case ID_SELECT_D88_BANK + 57: case ID_SELECT_D88_BANK + 58: case ID_SELECT_D88_BANK + 59: \
		case ID_SELECT_D88_BANK + 60: case ID_SELECT_D88_BANK + 61: case ID_SELECT_D88_BANK + 62: case ID_SELECT_D88_BANK + 63: \
			if(emu) { \
				select_d88_bank(drv, LOWORD(wParam) - ID_SELECT_D88_BANK); \
			} \
			break; \
		case ID_EJECT_D88_BANK: \
			if(emu) { \
				select_d88_bank(drv, -1); \
			} \
			break;
		FD_MENU_ITEMS(0, ID_OPEN_FD1, ID_CLOSE_FD1, ID_OPEN_BLANK_2D_FD1, ID_OPEN_BLANK_2DD_FD1, ID_OPEN_BLANK_2HD_FD1, ID_WRITE_PROTECT_FD1, ID_CORRECT_TIMING_FD1, ID_IGNORE_CRC_FD1, ID_RECENT_FD1, ID_SELECT_D88_BANK1, ID_EJECT_D88_BANK1)
	#endif
	#if USE_FLOPPY_DISK >= 2
		FD_MENU_ITEMS(1, ID_OPEN_FD2, ID_CLOSE_FD2, ID_OPEN_BLANK_2D_FD2, ID_OPEN_BLANK_2DD_FD2, ID_OPEN_BLANK_2HD_FD2, ID_WRITE_PROTECT_FD2, ID_CORRECT_TIMING_FD2, ID_IGNORE_CRC_FD2, ID_RECENT_FD2, ID_SELECT_D88_BANK2, ID_EJECT_D88_BANK2)
	#endif
	#if USE_FLOPPY_DISK >= 3
		FD_MENU_ITEMS(2, ID_OPEN_FD3, ID_CLOSE_FD3, ID_OPEN_BLANK_2D_FD3, ID_OPEN_BLANK_2DD_FD3, ID_OPEN_BLANK_2HD_FD3, ID_WRITE_PROTECT_FD3, ID_CORRECT_TIMING_FD3, ID_IGNORE_CRC_FD3, ID_RECENT_FD3, ID_SELECT_D88_BANK3, ID_EJECT_D88_BANK3)
	#endif
	#if USE_FLOPPY_DISK >= 4
		FD_MENU_ITEMS(3, ID_OPEN_FD4, ID_CLOSE_FD4, ID_OPEN_BLANK_2D_FD4, ID_OPEN_BLANK_2DD_FD4, ID_OPEN_BLANK_2HD_FD4, ID_WRITE_PROTECT_FD4, ID_CORRECT_TIMING_FD4, ID_IGNORE_CRC_FD4, ID_RECENT_FD4, ID_SELECT_D88_BANK4, ID_EJECT_D88_BANK4)
	#endif
	#if USE_FLOPPY_DISK >= 5
		FD_MENU_ITEMS(4, ID_OPEN_FD5, ID_CLOSE_FD5, ID_OPEN_BLANK_2D_FD5, ID_OPEN_BLANK_2DD_FD5, ID_OPEN_BLANK_2HD_FD5, ID_WRITE_PROTECT_FD5, ID_CORRECT_TIMING_FD5, ID_IGNORE_CRC_FD5, ID_RECENT_FD5, ID_SELECT_D88_BANK5, ID_EJECT_D88_BANK5)
	#endif
	#if USE_FLOPPY_DISK >= 6
		FD_MENU_ITEMS(5, ID_OPEN_FD6, ID_CLOSE_FD6, ID_OPEN_BLANK_2D_FD6, ID_OPEN_BLANK_2DD_FD6, ID_OPEN_BLANK_2HD_FD6, ID_WRITE_PROTECT_FD6, ID_CORRECT_TIMING_FD6, ID_IGNORE_CRC_FD6, ID_RECENT_FD6, ID_SELECT_D88_BANK6, ID_EJECT_D88_BANK6)
	#endif
	#if USE_FLOPPY_DISK >= 7
		FD_MENU_ITEMS(6, ID_OPEN_FD7, ID_CLOSE_FD7, ID_OPEN_BLANK_2D_FD7, ID_OPEN_BLANK_2DD_FD7, ID_OPEN_BLANK_2HD_FD7, ID_WRITE_PROTECT_FD7, ID_CORRECT_TIMING_FD7, ID_IGNORE_CRC_FD7, ID_RECENT_FD7, ID_SELECT_D88_BANK7, ID_EJECT_D88_BANK7)
	#endif
	#if USE_FLOPPY_DISK >= 8
		FD_MENU_ITEMS(7, ID_OPEN_FD8, ID_CLOSE_FD8, ID_OPEN_BLANK_2D_FD8, ID_OPEN_BLANK_2DD_FD8, ID_OPEN_BLANK_2HD_FD8, ID_WRITE_PROTECT_FD8, ID_CORRECT_TIMING_FD8, ID_IGNORE_CRC_FD8, ID_RECENT_FD8, ID_SELECT_D88_BANK8, ID_EJECT_D88_BANK8)
	#endif
#endif
#ifdef USE_QUICK_DISK
	#if USE_QUICK_DISK >= 1
		#define QD_MENU_ITEMS(drv, ID_OPEN_QD, ID_CLOSE_QD, ID_RECENT_QD) \
		case ID_OPEN_QD: \
			if(emu) { \
				open_quick_disk_dialog(hWnd, drv); \
			} \
			break; \
		case ID_CLOSE_QD: \
			if(emu) { \
				emu->close_quick_disk(drv); \
			} \
			break; \
		case ID_RECENT_QD + 0: case ID_RECENT_QD + 1: case ID_RECENT_QD + 2: case ID_RECENT_QD + 3: \
		case ID_RECENT_QD + 4: case ID_RECENT_QD + 5: case ID_RECENT_QD + 6: case ID_RECENT_QD + 7: \
			if(emu) { \
				open_recent_quick_disk(drv, LOWORD(wParam) - ID_RECENT_QD); \
			} \
			break;
		QD_MENU_ITEMS(0, ID_OPEN_QD1, ID_CLOSE_QD1, ID_RECENT_QD1)
	#endif
	#if USE_QUICK_DISK >= 2
		QD_MENU_ITEMS(1, ID_OPEN_QD2, ID_CLOSE_QD2, ID_RECENT_QD2)
	#endif
#endif
#ifdef USE_HARD_DISK
	#if USE_HARD_DISK >= 1
		#define HD_MENU_ITEMS(drv, ID_OPEN_HD, ID_CLOSE_HD, ID_RECENT_HD) \
		case ID_OPEN_HD: \
			if(emu) { \
				open_hard_disk_dialog(hWnd, drv); \
			} \
			break; \
		case ID_CLOSE_HD: \
			if(emu) { \
				emu->close_hard_disk(drv); \
			} \
			break; \
		case ID_RECENT_HD + 0: case ID_RECENT_HD + 1: case ID_RECENT_HD + 2: case ID_RECENT_HD + 3: \
		case ID_RECENT_HD + 4: case ID_RECENT_HD + 5: case ID_RECENT_HD + 6: case ID_RECENT_HD + 7: \
			if(emu) { \
				open_recent_hard_disk(drv, LOWORD(wParam) - ID_RECENT_HD); \
			} \
			break;
		HD_MENU_ITEMS(0, ID_OPEN_HD1, ID_CLOSE_HD1, ID_RECENT_HD1)
	#endif
	#if USE_HARD_DISK >= 2
		HD_MENU_ITEMS(1, ID_OPEN_HD2, ID_CLOSE_HD2, ID_RECENT_HD2)
	#endif
	#if USE_HARD_DISK >= 3
		HD_MENU_ITEMS(2, ID_OPEN_HD3, ID_CLOSE_HD3, ID_RECENT_HD3)
	#endif
	#if USE_HARD_DISK >= 4
		HD_MENU_ITEMS(3, ID_OPEN_HD4, ID_CLOSE_HD4, ID_RECENT_HD4)
	#endif
	#if USE_HARD_DISK >= 5
		HD_MENU_ITEMS(4, ID_OPEN_HD5, ID_CLOSE_HD5, ID_RECENT_HD5)
	#endif
	#if USE_HARD_DISK >= 6
		HD_MENU_ITEMS(5, ID_OPEN_HD6, ID_CLOSE_HD6, ID_RECENT_HD6)
	#endif
	#if USE_HARD_DISK >= 7
		HD_MENU_ITEMS(6, ID_OPEN_HD7, ID_CLOSE_HD7, ID_RECENT_HD7)
	#endif
	#if USE_HARD_DISK >= 8
		HD_MENU_ITEMS(7, ID_OPEN_HD8, ID_CLOSE_HD8, ID_RECENT_HD8)
	#endif
#endif
#ifdef USE_TAPE
	#if USE_TAPE >= 1
		#define TAPE_MENU_ITEMS(drv, ID_PLAY_TAPE, ID_REC_TAPE, ID_CLOSE_TAPE, ID_PLAY_BUTTON, ID_STOP_BUTTON, ID_FAST_FORWARD, ID_FAST_REWIND, ID_APSS_FORWARD, ID_APSS_REWIND, ID_USE_WAVE_SHAPER, ID_DIRECT_LOAD_MZT, ID_TAPE_BAUD_LOW, ID_TAPE_BAUD_HIGH, ID_RECENT_TAPE) \
		case ID_PLAY_TAPE: \
			if(emu) { \
				open_tape_dialog(hWnd, drv, true); \
			} \
			break; \
		case ID_REC_TAPE: \
			if(emu) { \
				open_tape_dialog(hWnd, drv, false); \
			} \
			break; \
		case ID_CLOSE_TAPE: \
			if(emu) { \
				emu->close_tape(drv); \
			} \
			break; \
		case ID_PLAY_BUTTON: \
			if(emu) { \
				emu->push_play(drv); \
			} \
			break; \
		case ID_STOP_BUTTON: \
			if(emu) { \
				emu->push_stop(drv); \
			} \
			break; \
		case ID_FAST_FORWARD: \
			if(emu) { \
				emu->push_fast_forward(drv); \
			} \
			break; \
		case ID_FAST_REWIND: \
			if(emu) { \
				emu->push_fast_rewind(drv); \
			} \
			break; \
		case ID_APSS_FORWARD: \
			if(emu) { \
				emu->push_apss_forward(drv); \
			} \
			break; \
		case ID_APSS_REWIND: \
			if(emu) { \
				emu->push_apss_rewind(drv); \
			} \
			break; \
		case ID_USE_WAVE_SHAPER: \
			config.wave_shaper[drv] = !config.wave_shaper[drv]; \
			break; \
		case ID_DIRECT_LOAD_MZT: \
			config.direct_load_mzt[drv] = !config.direct_load_mzt[drv]; \
			break; \
		case ID_TAPE_BAUD_LOW: \
			config.baud_high[drv] = false; \
			break; \
		case ID_TAPE_BAUD_HIGH: \
			config.baud_high[drv] = true; \
			break; \
		case ID_RECENT_TAPE + 0: case ID_RECENT_TAPE + 1: case ID_RECENT_TAPE + 2: case ID_RECENT_TAPE + 3: \
		case ID_RECENT_TAPE + 4: case ID_RECENT_TAPE + 5: case ID_RECENT_TAPE + 6: case ID_RECENT_TAPE + 7: \
			if(emu) { \
				open_recent_tape(drv, LOWORD(wParam) - ID_RECENT_TAPE); \
			} \
			break;
		TAPE_MENU_ITEMS(0, ID_PLAY_TAPE1, ID_REC_TAPE1, ID_CLOSE_TAPE1, ID_PLAY_BUTTON1, ID_STOP_BUTTON1, ID_FAST_FORWARD1, ID_FAST_REWIND1, ID_APSS_FORWARD1, ID_APSS_REWIND1, ID_USE_WAVE_SHAPER1, ID_DIRECT_LOAD_MZT1, ID_TAPE_BAUD_LOW1, ID_TAPE_BAUD_HIGH1, ID_RECENT_TAPE1)
	#endif
	#if USE_TAPE >= 2
		TAPE_MENU_ITEMS(1, ID_PLAY_TAPE2, ID_REC_TAPE2, ID_CLOSE_TAPE2, ID_PLAY_BUTTON2, ID_STOP_BUTTON2, ID_FAST_FORWARD2, ID_FAST_REWIND2, ID_APSS_FORWARD2, ID_APSS_REWIND2, ID_USE_WAVE_SHAPER2, ID_DIRECT_LOAD_MZT2, ID_TAPE_BAUD_LOW2, ID_TAPE_BAUD_HIGH2, ID_RECENT_TAPE2)
	#endif
#endif
#ifdef USE_COMPACT_DISC
	#if USE_COMPACT_DISC >= 1
		#define COMPACT_DISC_MENU_ITEMS(drv, ID_OPEN_COMPACT_DISC, ID_CLOSE_COMPACT_DISC, ID_RECENT_COMPACT_DISC) \
		case ID_OPEN_COMPACT_DISC: \
			if(emu) { \
				open_compact_disc_dialog(hWnd, drv); \
			} \
			break; \
		case ID_CLOSE_COMPACT_DISC: \
			if(emu) { \
				emu->close_compact_disc(drv); \
			} \
			break; \
		case ID_RECENT_COMPACT_DISC + 0: case ID_RECENT_COMPACT_DISC + 1: case ID_RECENT_COMPACT_DISC + 2: case ID_RECENT_COMPACT_DISC + 3: \
		case ID_RECENT_COMPACT_DISC + 4: case ID_RECENT_COMPACT_DISC + 5: case ID_RECENT_COMPACT_DISC + 6: case ID_RECENT_COMPACT_DISC + 7: \
			if(emu) { \
				open_recent_compact_disc(drv, LOWORD(wParam) - ID_RECENT_COMPACT_DISC); \
			} \
			break;
		COMPACT_DISC_MENU_ITEMS(0, ID_OPEN_COMPACT_DISC1, ID_CLOSE_COMPACT_DISC1, ID_RECENT_COMPACT_DISC1)
	#endif
	#if USE_COMPACT_DISC >= 2
		COMPACT_DISC_MENU_ITEMS(1, ID_OPEN_COMPACT_DISC2, ID_CLOSE_COMPACT_DISC2, ID_RECENT_COMPACT_DISC2)
	#endif
#endif
#ifdef USE_LASER_DISC
	#if USE_LASER_DISC >= 1
		#define LASER_DISC_MENU_ITEMS(drv, ID_OPEN_LASER_DISC, ID_CLOSE_LASER_DISC, ID_RECENT_LASER_DISC) \
		case ID_OPEN_LASER_DISC: \
			if(emu) { \
				open_laser_disc_dialog(hWnd, drv); \
			} \
			break; \
		case ID_CLOSE_LASER_DISC: \
			if(emu) { \
				emu->close_laser_disc(drv); \
			} \
			break; \
		case ID_RECENT_LASER_DISC + 0: case ID_RECENT_LASER_DISC + 1: case ID_RECENT_LASER_DISC + 2: case ID_RECENT_LASER_DISC + 3: \
		case ID_RECENT_LASER_DISC + 4: case ID_RECENT_LASER_DISC + 5: case ID_RECENT_LASER_DISC + 6: case ID_RECENT_LASER_DISC + 7: \
			if(emu) { \
				open_recent_laser_disc(drv, LOWORD(wParam) - ID_RECENT_LASER_DISC); \
			} \
			break;
		LASER_DISC_MENU_ITEMS(0, ID_OPEN_LASER_DISC1, ID_CLOSE_LASER_DISC1, ID_RECENT_LASER_DISC1)
	#endif
	#if USE_LASER_DISC >= 2
		LASER_DISC_MENU_ITEMS(1, ID_OPEN_LASER_DISC2, ID_CLOSE_LASER_DISC2, ID_RECENT_LASER_DISC2)
	#endif
#endif
#ifdef USE_BINARY_FILE
	#if USE_BINARY_FILE >= 1
		#define BINARY_MENU_ITEMS(drv, ID_LOAD_BINARY, ID_SAVE_BINARY, ID_RECENT_BINARY) \
		case ID_LOAD_BINARY: \
			if(emu) { \
				open_binary_dialog(hWnd, drv, true); \
			} \
			break; \
		case ID_SAVE_BINARY: \
			if(emu) { \
				open_binary_dialog(hWnd, drv, false); \
			} \
			break; \
		case ID_RECENT_BINARY + 0: case ID_RECENT_BINARY + 1: case ID_RECENT_BINARY + 2: case ID_RECENT_BINARY + 3: \
		case ID_RECENT_BINARY + 4: case ID_RECENT_BINARY + 5: case ID_RECENT_BINARY + 6: case ID_RECENT_BINARY + 7: \
			if(emu) { \
				open_recent_binary(drv, LOWORD(wParam) - ID_RECENT_BINARY); \
			} \
			break;
		BINARY_MENU_ITEMS(0, ID_LOAD_BINARY1, ID_SAVE_BINARY1, ID_RECENT_BINARY1)
	#endif
	#if USE_BINARY_FILE >= 2
		BINARY_MENU_ITEMS(1, ID_LOAD_BINARY2, ID_SAVE_BINARY2, ID_RECENT_BINARY2)
	#endif
#endif
#ifdef USE_BUBBLE
	#if USE_BUBBLE >= 1
		#define BUBBLE_CASETTE_MENU_ITEMS(drv, ID_OPEN_BUBBLE, ID_CLOSE_BUBBLE, ID_RECENT_BUBBLE) \
		case ID_OPEN_BUBBLE: \
			if(emu) { \
				open_bubble_casette_dialog(hWnd, drv); \
			} \
			break; \
		case ID_CLOSE_BUBBLE: \
			if(emu) { \
				emu->close_bubble_casette(drv); \
			} \
			break; \
		case ID_RECENT_BUBBLE + 0: case ID_RECENT_BUBBLE + 1: case ID_RECENT_BUBBLE + 2: case ID_RECENT_BUBBLE + 3: \
		case ID_RECENT_BUBBLE + 4: case ID_RECENT_BUBBLE + 5: case ID_RECENT_BUBBLE + 6: case ID_RECENT_BUBBLE + 7: \
			if(emu) { \
				open_recent_bubble_casette(drv, LOWORD(wParam) - ID_RECENT_BUBBLE); \
			} \
			break;
		BUBBLE_CASETTE_MENU_ITEMS(0, ID_OPEN_BUBBLE1, ID_CLOSE_BUBBLE1, ID_RECENT_BUBBLE1)
	#endif
	#if USE_BUBBLE >= 2
		BUBBLE_CASETTE_MENU_ITEMS(1, ID_OPEN_BUBBLE2, ID_CLOSE_BUBBLE2, ID_RECENT_BUBBLE2)
	#endif
#endif
		case ID_ACCEL_SCREEN:
			if(emu) {
				emu->suspend();
				set_window(hWnd, now_fullscreen ? prev_window_mode : -1);
			}
			break;
#ifdef USE_MOUSE
		case ID_ACCEL_MOUSE:
			if(emu) {
				emu->toggle_mouse();
			}
			break;
#endif
		case ID_ACCEL_SPEED:
			config.full_speed = !config.full_speed;
			break;
#ifdef USE_AUTO_KEY
		case ID_ACCEL_ROMAJI:
			if(emu) {
				if(!config.romaji_to_kana) {
					emu->set_auto_key_char(1); // start
					config.romaji_to_kana = true;
				} else {
					emu->set_auto_key_char(0); // end
					config.romaji_to_kana = false;
				}
			}
			break;
#endif
#ifdef ONE_BOARD_MICRO_COMPUTER
		case ID_BUTTON +  0: case ID_BUTTON +  1: case ID_BUTTON +  2: case ID_BUTTON +  3:
		case ID_BUTTON +  4: case ID_BUTTON +  5: case ID_BUTTON +  6: case ID_BUTTON +  7:
		case ID_BUTTON +  8: case ID_BUTTON +  9: case ID_BUTTON + 10: case ID_BUTTON + 11:
		case ID_BUTTON + 12: case ID_BUTTON + 13: case ID_BUTTON + 14: case ID_BUTTON + 15:
		case ID_BUTTON + 16: case ID_BUTTON + 17: case ID_BUTTON + 18: case ID_BUTTON + 19:
		case ID_BUTTON + 20: case ID_BUTTON + 21: case ID_BUTTON + 22: case ID_BUTTON + 23:
		case ID_BUTTON + 24: case ID_BUTTON + 25: case ID_BUTTON + 26: case ID_BUTTON + 27:
		case ID_BUTTON + 28: case ID_BUTTON + 29: case ID_BUTTON + 30: case ID_BUTTON + 31:
			if(emu) {
				emu->press_button(LOWORD(wParam) - ID_BUTTON);
			}
			break;
#endif
#if USE_RETROACHIEVEMENTS
        default:
            if (emu) {
                RA_HandleMenuEvent(wParam);
            }
#endif
		}
        break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

// ----------------------------------------------------------------------------
// menu
// ----------------------------------------------------------------------------

void update_control_menu(HMENU hMenu)
{
	if(config.cpu_power >= 0 && config.cpu_power < 5) {
		CheckMenuRadioItem(hMenu, ID_CPU_POWER0, ID_CPU_POWER4, ID_CPU_POWER0 + config.cpu_power, MF_BYCOMMAND);
	}
	CheckMenuItem(hMenu, ID_FULL_SPEED, config.full_speed ? MF_CHECKED : MF_UNCHECKED);
#ifdef USE_AUTO_KEY
	bool now_paste = true, now_stop = true;
	if(emu) {
		now_paste = emu->is_auto_key_running();
		now_stop = !now_paste;
	}
	EnableMenuItem(hMenu, ID_AUTOKEY_START, now_paste ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, ID_AUTOKEY_STOP, now_stop ? MF_GRAYED : MF_ENABLED);
	CheckMenuItem(hMenu, ID_ROMAJI_TO_KANA, config.romaji_to_kana ? MF_CHECKED : MF_UNCHECKED);
#endif
#ifdef USE_DEBUGGER
	for(int i = 0; i < 8; i++) {
		EnableMenuItem(hMenu, ID_OPEN_DEBUGGER0 + i, emu && !emu->now_debugging && emu->is_debugger_enabled(i) ? MF_ENABLED : MF_GRAYED);
	}
	EnableMenuItem(hMenu, ID_CLOSE_DEBUGGER, emu &&  emu->now_debugging                                ? MF_ENABLED : MF_GRAYED);
#endif
}

#ifdef USE_STATE
void update_save_state_menu(HMENU hMenu)
{
	HANDLE hFile;
	FILETIME ftWrite;
	FILETIME ftLocal;
	SYSTEMTIME st;
	_TCHAR buf[64];
	MENUITEMINFO info;
	
	ZeroMemory(&info, sizeof(info));
	info.cbSize = sizeof(info);
	info.fMask = MIIM_TYPE;
	info.fType = MFT_STRING;
	info.dwTypeData = buf;
	
	for(int i = 0; i < 10; i++) {
		if((hFile = CreateFile(state_file_path(i), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE){
			GetFileTime(hFile, NULL, NULL, &ftWrite);
			FileTimeToLocalFileTime(&ftWrite, &ftLocal);
			FileTimeToSystemTime(&ftLocal, &st);
//			if(st.wHour < 12) {
//				my_stprintf_s(buf, 64, _T("%d:  %04d-%02d-%02d  %02d:%02d:%02d AM"), i, st.wYear, st.wMonth, st.wDay, st.wHour % 12, st.wMinute, st.wSecond);
//			} else {
//				my_stprintf_s(buf, 64, _T("%d:  %04d-%02d-%02d  %02d:%02d:%02d PM"), i, st.wYear, st.wMonth, st.wDay, st.wHour % 12, st.wMinute, st.wSecond);
//			}
			my_stprintf_s(buf, 64, _T("%d:  %04d-%02d-%02d  %02d:%02d:%02d"), i, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			CloseHandle(hFile);
		} else {
//			my_stprintf_s(buf, 64, _T("%d: ----/--/-- --:--:--"), i);
			my_stprintf_s(buf, 64, _T("%d: (No Data)"), i);
		}
		SetMenuItemInfo(hMenu, ID_SAVE_STATE0 + i, FALSE, &info);
	}
}

void update_load_state_menu(HMENU hMenu)
{
	HANDLE hFile;
	FILETIME ftWrite;
	FILETIME ftLocal;
	SYSTEMTIME st;
	_TCHAR buf[64];
	MENUITEMINFO info;
	
	ZeroMemory(&info, sizeof(info));
	info.cbSize = sizeof(info);
	info.fMask = MIIM_TYPE;
	info.fType = MFT_STRING;
	info.dwTypeData = buf;
	
	for(int i = 0; i < 10; i++) {
		if((hFile = CreateFile(state_file_path(i), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE){
			GetFileTime(hFile, NULL, NULL, &ftWrite);
			FileTimeToLocalFileTime(&ftWrite, &ftLocal);
			FileTimeToSystemTime(&ftLocal, &st);
//			if(st.wHour < 12) {
//				my_stprintf_s(buf, 64, _T("%d:  %04d-%02d-%02d  %02d:%02d:%02d AM"), i, st.wYear, st.wMonth, st.wDay, st.wHour % 12, st.wMinute, st.wSecond);
//			} else {
//				my_stprintf_s(buf, 64, _T("%d:  %04d-%02d-%02d  %02d:%02d:%02d PM"), i, st.wYear, st.wMonth, st.wDay, st.wHour % 12, st.wMinute, st.wSecond);
//			}
			my_stprintf_s(buf, 64, _T("%d:  %04d-%02d-%02d  %02d:%02d:%02d"), i, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			EnableMenuItem(hMenu, ID_LOAD_STATE0 + i, MF_ENABLED);
			CloseHandle(hFile);
		} else {
//			my_stprintf_s(buf, 64, _T("%d: ----/--/-- --:--:--"), i);
			my_stprintf_s(buf, 64, _T("%d: (No Data)"), i);
			EnableMenuItem(hMenu, ID_LOAD_STATE0 + i, MF_GRAYED);
		}
		SetMenuItemInfo(hMenu, ID_LOAD_STATE0 + i, FALSE, &info);
	}
}
#endif

#ifdef USE_CART
void update_cart_menu(HMENU hMenu, int drv, UINT ID_RECENT_CART, UINT ID_CLOSE_CART)
{
	bool flag = false;
	for(int i = 0; i < MAX_HISTORY; i++) {
		DeleteMenu(hMenu, ID_RECENT_CART + i, MF_BYCOMMAND);
	}
	for(int i = 0; i < MAX_HISTORY; i++) {
		if(_tcsicmp(config.recent_cart_path[drv][i], _T(""))) {
			AppendMenu(hMenu, MF_STRING, ID_RECENT_CART + i, config.recent_cart_path[drv][i]);
			flag = true;
		}
	}
	if(!flag) {
		AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_CART, _T("None"));
	}
	EnableMenuItem(hMenu, ID_CLOSE_CART, emu->is_cart_inserted(drv) ? MF_ENABLED : MF_GRAYED);
}
#endif

#ifdef USE_FLOPPY_DISK
void update_floppy_disk_menu(HMENU hMenu, int drv, UINT ID_RECENT_FD, UINT ID_D88_FILE_PATH, UINT ID_SELECT_D88_BANK, UINT ID_EJECT_D88_BANK, UINT ID_CLOSE_FD, UINT ID_WRITE_PROTECT_FD, UINT ID_CORRECT_TIMING_FD, UINT ID_IGNORE_CRC_FD)
{
	static int recent_menu_pos[] = {-1, -1, -1, -1, -1, -1, -1, -1};
	if(recent_menu_pos[drv] == -1) {
		int count = GetMenuItemCount(hMenu);
		for(int i = 0; i < count; i++) {
			if(GetMenuItemID(hMenu, i) == ID_RECENT_FD) {
				recent_menu_pos[drv] = i;
				break;
			}
		}
	}
	bool flag = false;
	
	while(DeleteMenu(hMenu, recent_menu_pos[drv], MF_BYPOSITION) != 0) {}
	if(emu->d88_file[drv].bank_num > 1) {
		AppendMenu(hMenu, MF_STRING | MF_DISABLED, ID_D88_FILE_PATH, emu->d88_file[drv].path);
		for(int i = 0; i < emu->d88_file[drv].bank_num; i++) {
			AppendMenu(hMenu, MF_STRING | (emu->d88_file[drv].cur_bank == i ? MF_CHECKED : 0), ID_SELECT_D88_BANK + i, create_string(_T("%d: %s"), i + 1, emu->d88_file[drv].disk_name[i]));
		}
		AppendMenu(hMenu, MF_STRING | (emu->d88_file[drv].cur_bank == -1 ? MF_CHECKED : 0), ID_EJECT_D88_BANK, _T("0: (Disk Ejected)"));
		AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	}
	for(int i = 0; i < MAX_HISTORY; i++) {
		if(_tcsicmp(config.recent_floppy_disk_path[drv][i], _T(""))) {
			AppendMenu(hMenu, MF_STRING, ID_RECENT_FD + i, config.recent_floppy_disk_path[drv][i]);
			flag = true;
		}
	}
	if(!flag) {
		AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_FD, _T("None"));
	}
	EnableMenuItem(hMenu, ID_CLOSE_FD, emu->is_floppy_disk_inserted(drv) || (emu->d88_file[drv].bank_num > 1 && emu->d88_file[drv].cur_bank == -1) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_WRITE_PROTECT_FD, emu->is_floppy_disk_inserted(drv) ? MF_ENABLED : MF_GRAYED);
	CheckMenuItem(hMenu, ID_WRITE_PROTECT_FD, emu->is_floppy_disk_protected(drv) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_CORRECT_TIMING_FD, config.correct_disk_timing[drv] ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_IGNORE_CRC_FD, config.ignore_disk_crc[drv] ? MF_CHECKED : MF_UNCHECKED);
}
#endif

#ifdef USE_QUICK_DISK
void update_quick_disk_menu(HMENU hMenu, int drv, UINT ID_RECENT_QD, UINT ID_CLOSE_QD)
{
	bool flag = false;
	for(int i = 0; i < MAX_HISTORY; i++) {
		DeleteMenu(hMenu, ID_RECENT_QD + i, MF_BYCOMMAND);
	}
	for(int i = 0; i < MAX_HISTORY; i++) {
		if(_tcsicmp(config.recent_quick_disk_path[drv][i], _T(""))) {
			AppendMenu(hMenu, MF_STRING, ID_RECENT_QD + i, config.recent_quick_disk_path[drv][i]);
			flag = true;
		}
	}
	if(!flag) {
		AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_QD, _T("None"));
	}
	EnableMenuItem(hMenu, ID_CLOSE_QD, emu->is_quick_disk_inserted(drv) ? MF_ENABLED : MF_GRAYED);
}
#endif

#ifdef USE_HARD_DISK
void update_hard_disk_menu(HMENU hMenu, int drv, UINT ID_RECENT_HD, UINT ID_CLOSE_HD)
{
	bool flag = false;
	for(int i = 0; i < MAX_HISTORY; i++) {
		DeleteMenu(hMenu, ID_RECENT_HD + i, MF_BYCOMMAND);
	}
	for(int i = 0; i < MAX_HISTORY; i++) {
		if(_tcsicmp(config.recent_hard_disk_path[drv][i], _T(""))) {
			AppendMenu(hMenu, MF_STRING, ID_RECENT_HD + i, config.recent_hard_disk_path[drv][i]);
			flag = true;
		}
	}
	if(!flag) {
		AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_HD, _T("None"));
	}
	EnableMenuItem(hMenu, ID_CLOSE_HD, emu->is_hard_disk_inserted(drv) ? MF_ENABLED : MF_GRAYED);
}
#endif

#ifdef USE_TAPE
void update_tape_menu(HMENU hMenu, int drv, UINT ID_RECENT_TAPE, UINT ID_CLOSE_TAPE, UINT ID_PLAY_BUTTON, UINT ID_STOP_BUTTON, UINT ID_FAST_FORWARD, UINT ID_FAST_REWIND, UINT ID_APSS_FORWARD, UINT ID_APSS_REWIND, UINT ID_USE_WAVE_SHAPER, UINT ID_DIRECT_LOAD_MZT, UINT ID_TAPE_BAUD_LOW, UINT ID_TAPE_BAUD_HIGH)
{
	bool flag = false;
	for(int i = 0; i < MAX_HISTORY; i++) {
		DeleteMenu(hMenu, ID_RECENT_TAPE + i, MF_BYCOMMAND);
	}
	for(int i = 0; i < MAX_HISTORY; i++) {
		if(_tcsicmp(config.recent_tape_path[drv][i], _T(""))) {
			AppendMenu(hMenu, MF_STRING, ID_RECENT_TAPE + i, config.recent_tape_path[drv][i]);
			flag = true;
		}
	}
	if(!flag) {
		AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_TAPE, _T("None"));
	}
	EnableMenuItem(hMenu, ID_CLOSE_TAPE, emu->is_tape_inserted(drv) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_PLAY_BUTTON, emu->is_tape_inserted(drv) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_STOP_BUTTON, emu->is_tape_inserted(drv) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_FAST_FORWARD, emu->is_tape_inserted(drv) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_FAST_REWIND, emu->is_tape_inserted(drv) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_APSS_FORWARD, emu->is_tape_inserted(drv) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_APSS_REWIND, emu->is_tape_inserted(drv) ? MF_ENABLED : MF_GRAYED);
	CheckMenuItem(hMenu, ID_USE_WAVE_SHAPER, config.wave_shaper[drv] ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DIRECT_LOAD_MZT, config.direct_load_mzt[drv] ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuRadioItem(hMenu, ID_TAPE_BAUD_LOW, ID_TAPE_BAUD_HIGH, !config.baud_high[drv] ? ID_TAPE_BAUD_LOW : ID_TAPE_BAUD_HIGH, MF_BYCOMMAND);
}
#endif

#ifdef USE_COMPACT_DISC
void update_compact_disc_menu(HMENU hMenu, int drv, UINT ID_RECENT_COMPACT_DISC, UINT ID_CLOSE_COMPACT_DISC)
{
	bool flag = false;
	for(int i = 0; i < MAX_HISTORY; i++) {
		DeleteMenu(hMenu, ID_RECENT_COMPACT_DISC + i, MF_BYCOMMAND);
	}
	for(int i = 0; i < MAX_HISTORY; i++) {
		if(_tcsicmp(config.recent_compact_disc_path[drv][i], _T(""))) {
			AppendMenu(hMenu, MF_STRING, ID_RECENT_COMPACT_DISC + i, config.recent_compact_disc_path[drv][i]);
			flag = true;
		}
	}
	if(!flag) {
		AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_COMPACT_DISC, _T("None"));
	}
	EnableMenuItem(hMenu, ID_CLOSE_COMPACT_DISC, emu->is_compact_disc_inserted(drv) ? MF_ENABLED : MF_GRAYED);
}
#endif

#ifdef USE_LASER_DISC
void update_laser_disc_menu(HMENU hMenu, int drv, UINT ID_RECENT_LASER_DISC, UINT ID_CLOSE_LASER_DISC)
{
	bool flag = false;
	for(int i = 0; i < MAX_HISTORY; i++) {
		DeleteMenu(hMenu, ID_RECENT_LASER_DISC + i, MF_BYCOMMAND);
	}
	for(int i = 0; i < MAX_HISTORY; i++) {
		if(_tcsicmp(config.recent_laser_disc_path[drv][i], _T(""))) {
			AppendMenu(hMenu, MF_STRING, ID_RECENT_LASER_DISC + i, config.recent_laser_disc_path[drv][i]);
			flag = true;
		}
	}
	if(!flag) {
		AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_LASER_DISC, _T("None"));
	}
	EnableMenuItem(hMenu, ID_CLOSE_LASER_DISC, emu->is_laser_disc_inserted(drv) ? MF_ENABLED : MF_GRAYED);
}
#endif

#ifdef USE_BINARY_FILE
void update_binary_menu(HMENU hMenu, int drv, UINT ID_RECENT_BINARY)
{
	bool flag = false;
	for(int i = 0; i < MAX_HISTORY; i++) {
		DeleteMenu(hMenu, ID_RECENT_BINARY + i, MF_BYCOMMAND);
	}
	for(int i = 0; i < MAX_HISTORY; i++) {
		if(_tcsicmp(config.recent_binary_path[drv][i], _T(""))) {
			AppendMenu(hMenu, MF_STRING, ID_RECENT_BINARY + i, config.recent_binary_path[drv][i]);
			flag = true;
		}
	}
	if(!flag) {
		AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_BINARY, _T("None"));
	}
}
#endif

#ifdef USE_BUBBLE
void update_bubble_casette_menu(HMENU hMenu, int drv, UINT ID_RECENT_BUBBLE)
{
	bool flag = false;
	for(int i = 0; i < MAX_HISTORY; i++) {
		DeleteMenu(hMenu, ID_RECENT_BUBBLE + i, MF_BYCOMMAND);
	}
	for(int i = 0; i < MAX_HISTORY; i++) {
		if(_tcsicmp(config.recent_bubble_casette_path[drv][i], _T(""))) {
			AppendMenu(hMenu, MF_STRING, ID_RECENT_BUBBLE + i, config.recent_bubble_casette_path[drv][i]);
			flag = true;
		}
	}
	if(!flag) {
		AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_RECENT_BUBBLE, _T("None"));
	}
}
#endif

#ifdef USE_BOOT_MODE
void update_vm_boot_menu(HMENU hMenu)
{
	if(config.boot_mode >= 0 && config.boot_mode < USE_BOOT_MODE) {
		CheckMenuRadioItem(hMenu, ID_VM_BOOT_MODE0, ID_VM_BOOT_MODE0 + USE_BOOT_MODE - 1, ID_VM_BOOT_MODE0 + config.boot_mode, MF_BYCOMMAND);
	}
}
#endif

#ifdef USE_CPU_TYPE
void update_vm_cpu_menu(HMENU hMenu)
{
	if(config.cpu_type >= 0 && config.cpu_type < USE_CPU_TYPE) {
		CheckMenuRadioItem(hMenu, ID_VM_CPU_TYPE0, ID_VM_CPU_TYPE0 + USE_CPU_TYPE - 1, ID_VM_CPU_TYPE0 + config.cpu_type, MF_BYCOMMAND);
	}
}
#endif

#ifdef USE_DIPSWITCH
void update_vm_dipswitch_menu(HMENU hMenu)
{
	for(int i = 0; i < 32; i++) {
		CheckMenuItem(hMenu, ID_VM_DIPSWITCH0 + i, (config.dipswitch & (1 << i)) ? MF_CHECKED : MF_UNCHECKED);
	}
}
#endif

#ifdef USE_DEVICE_TYPE
void update_vm_device_menu(HMENU hMenu)
{
	if(config.device_type >= 0 && config.device_type < USE_DEVICE_TYPE) {
		CheckMenuRadioItem(hMenu, ID_VM_DEVICE_TYPE0, ID_VM_DEVICE_TYPE0 + USE_DEVICE_TYPE - 1, ID_VM_DEVICE_TYPE0 + config.device_type, MF_BYCOMMAND);
	}
}
#endif

#ifdef USE_DRIVE_TYPE
void update_vm_drive_menu(HMENU hMenu)
{
	if(config.drive_type >= 0 && config.drive_type < USE_DRIVE_TYPE) {
		CheckMenuRadioItem(hMenu, ID_VM_DRIVE_TYPE0, ID_VM_DRIVE_TYPE0 + USE_DRIVE_TYPE - 1, ID_VM_DRIVE_TYPE0 + config.drive_type, MF_BYCOMMAND);
	}
}
#endif

#ifdef USE_KEYBOARD_TYPE
void update_vm_keyboard_menu(HMENU hMenu)
{
	if(config.keyboard_type >= 0 && config.keyboard_type < USE_KEYBOARD_TYPE) {
		CheckMenuRadioItem(hMenu, ID_VM_KEYBOARD_TYPE0, ID_VM_KEYBOARD_TYPE0 + USE_KEYBOARD_TYPE - 1, ID_VM_KEYBOARD_TYPE0 + config.keyboard_type, MF_BYCOMMAND);
	}
}
#endif

#ifdef USE_MOUSE_TYPE
void update_vm_mouse_menu(HMENU hMenu)
{
	if(config.mouse_type >= 0 && config.mouse_type < USE_MOUSE_TYPE) {
		CheckMenuRadioItem(hMenu, ID_VM_MOUSE_TYPE0, ID_VM_MOUSE_TYPE0 + USE_MOUSE_TYPE - 1, ID_VM_MOUSE_TYPE0 + config.mouse_type, MF_BYCOMMAND);
	}
}
#endif

#ifdef USE_JOYSTICK_TYPE
void update_vm_joystick_menu(HMENU hMenu)
{
	if(config.joystick_type >= 0 && config.joystick_type < USE_JOYSTICK_TYPE) {
		CheckMenuRadioItem(hMenu, ID_VM_JOYSTICK_TYPE0, ID_VM_JOYSTICK_TYPE0 + USE_JOYSTICK_TYPE - 1, ID_VM_JOYSTICK_TYPE0 + config.joystick_type, MF_BYCOMMAND);
	}
}
#endif

#if defined(USE_SOUND_TYPE) || defined(USE_FLOPPY_DISK) || defined(USE_TAPE)
void update_vm_sound_menu(HMENU hMenu)
{
#ifdef USE_SOUND_TYPE
	if(config.sound_type >= 0 && config.sound_type < USE_SOUND_TYPE) {
		CheckMenuRadioItem(hMenu, ID_VM_SOUND_TYPE0, ID_VM_SOUND_TYPE0 + USE_SOUND_TYPE - 1, ID_VM_SOUND_TYPE0 + config.sound_type, MF_BYCOMMAND);
	}
#endif
#ifdef USE_FLOPPY_DISK
	CheckMenuItem(hMenu, ID_VM_SOUND_NOISE_FDD, config.sound_noise_fdd ? MF_CHECKED : MF_UNCHECKED);
#endif
#ifdef USE_TAPE
	CheckMenuItem(hMenu, ID_VM_SOUND_NOISE_CMT, config.sound_noise_cmt ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_VM_SOUND_PLAY_TAPE, config.sound_play_tape ? MF_CHECKED : MF_UNCHECKED);
#endif
}
#endif

#if defined(USE_MONITOR_TYPE) || defined(USE_SCANLINE)
void update_vm_monitor_menu(HMENU hMenu)
{
#ifdef USE_MONITOR_TYPE
	if(config.monitor_type >= 0 && config.monitor_type < USE_MONITOR_TYPE) {
		CheckMenuRadioItem(hMenu, ID_VM_MONITOR_TYPE0, ID_VM_MONITOR_TYPE0 + USE_MONITOR_TYPE - 1, ID_VM_MONITOR_TYPE0 + config.monitor_type, MF_BYCOMMAND);
	}
#endif
#ifdef USE_SCANLINE
	CheckMenuItem(hMenu, ID_VM_MONITOR_SCANLINE, config.scan_line ? MF_CHECKED : MF_UNCHECKED);
#endif
}
#endif

#ifdef USE_PRINTER_TYPE
void update_vm_printer_menu(HMENU hMenu)
{
	if(config.printer_type >= 0 && config.printer_type < USE_PRINTER_TYPE) {
		CheckMenuRadioItem(hMenu, ID_VM_PRINTER_TYPE0, ID_VM_PRINTER_TYPE0 + USE_PRINTER_TYPE - 1, ID_VM_PRINTER_TYPE0 + config.printer_type, MF_BYCOMMAND);
	}
}
#endif

void update_host_menu(HMENU hMenu)
{
	bool now_rec = true, now_stop = true;
	if(emu) {
		now_rec = emu->is_video_recording() || emu->is_sound_recording();
		now_stop = !now_rec;
	}
	EnableMenuItem(hMenu, ID_HOST_REC_MOVIE_60FPS, now_rec ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, ID_HOST_REC_MOVIE_30FPS, now_rec ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, ID_HOST_REC_MOVIE_15FPS, now_rec ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, ID_HOST_REC_SOUND, now_rec ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, ID_HOST_REC_STOP, now_stop ? MF_GRAYED : MF_ENABLED);
	
	CheckMenuItem(hMenu, ID_HOST_USE_D3D9, config.use_d3d9 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_HOST_WAIT_VSYNC, config.wait_vsync ? MF_CHECKED : MF_UNCHECKED);
	EnableMenuItem(hMenu, ID_HOST_WAIT_VSYNC, config.use_d3d9 ? MF_ENABLED : MF_GRAYED);
	
	CheckMenuItem(hMenu, ID_HOST_USE_DINPUT, config.use_dinput ? MF_CHECKED : MF_UNCHECKED);
	
	CheckMenuItem(hMenu, ID_HOST_DISABLE_DWM, config.disable_dwm ? MF_CHECKED : MF_UNCHECKED);
	EnableMenuItem(hMenu, ID_HOST_DISABLE_DWM, win8_or_later ? MF_ENABLED : MF_GRAYED);
	
	CheckMenuItem(hMenu, ID_HOST_SHOW_STATUS_BAR, config.show_status_bar ? MF_CHECKED : MF_UNCHECKED);
}

#ifndef ONE_BOARD_MICRO_COMPUTER
void update_host_screen_menu(HMENU hMenu)
{
	UINT position = 0;
	UINT last = ID_SCREEN_WINDOW;
	_TCHAR buf[64];
	MENUITEMINFO info;
	
	ZeroMemory(&info, sizeof(info));
	info.cbSize = sizeof(info);
	info.fMask = MIIM_TYPE;
	info.fType = MFT_STRING;
	info.dwTypeData = buf;
	
	for(int i = 0; i < MAX_WINDOW; i++) {
		DeleteMenu(hMenu, ID_SCREEN_WINDOW + i, MF_BYCOMMAND);
	}
	for(int i = 0; i < MAX_FULLSCREEN; i++) {
		DeleteMenu(hMenu, ID_SCREEN_FULLSCREEN + i, MF_BYCOMMAND);
	}
	for(int i = 0; i < MAX_WINDOW; i++) {
		if(i == 0 || (emu && emu->get_window_mode_width(i) <= desktop_width && emu->get_window_mode_height(i) <= desktop_height)) {
			double power = emu->get_window_mode_power(i);
			if(power == (double)(int)power) {
				my_stprintf_s(buf, 64, _T("Window x%d"), (int)power);
			} else {
				my_stprintf_s(buf, 64, _T("Window x%.1f"), power); // x1.5
			}
			InsertMenu(hMenu, position++, MF_BYPOSITION | MF_STRING, ID_SCREEN_WINDOW + i, buf);
			last = ID_SCREEN_WINDOW + i;
		}
	}
	for(int i = 0; i < MAX_FULLSCREEN; i++) {
		if(i < screen_mode_count) {
			my_stprintf_s(buf, 64, _T("Fullscreen %dx%d"), screen_mode_width[i], screen_mode_height[i]);
			InsertMenu(hMenu, position++, MF_BYPOSITION | MF_STRING, ID_SCREEN_FULLSCREEN + i, buf);
			EnableMenuItem(hMenu, ID_SCREEN_FULLSCREEN + i, now_fullscreen ? MF_GRAYED : MF_ENABLED);
			last = ID_SCREEN_FULLSCREEN + i;
		}
	}
	if(config.window_mode >= 0 && config.window_mode < MAX_WINDOW) {
		CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW, last, ID_SCREEN_WINDOW + config.window_mode, MF_BYCOMMAND);
	} else if(config.window_mode >= MAX_WINDOW && config.window_mode < screen_mode_count + MAX_WINDOW) {
		CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW, last, ID_SCREEN_FULLSCREEN + config.window_mode - MAX_WINDOW, MF_BYCOMMAND);
	}
	
	my_stprintf_s(buf, 64, _T("Window: Aspect Ratio %d:%d"), emu->get_vm_window_width(), emu->get_vm_window_height());
	SetMenuItemInfo(hMenu, ID_SCREEN_WINDOW_STRETCH, FALSE, &info);
	my_stprintf_s(buf, 64, _T("Window: Aspect Ratio %d:%d"), emu->get_vm_window_width_aspect(), emu->get_vm_window_height_aspect());
	SetMenuItemInfo(hMenu, ID_SCREEN_WINDOW_ASPECT, FALSE, &info);
	CheckMenuRadioItem(hMenu, ID_SCREEN_WINDOW_STRETCH, ID_SCREEN_WINDOW_ASPECT, ID_SCREEN_WINDOW_STRETCH + config.window_stretch_type, MF_BYCOMMAND);
	
	my_stprintf_s(buf, 64, _T("Fullscreen: Dot By Dot"));
	SetMenuItemInfo(hMenu, ID_SCREEN_FULLSCREEN_DOTBYDOT, FALSE, &info);
	my_stprintf_s(buf, 64, _T("Fullscreen: Stretch (Aspect Ratio %d:%d)"), emu->get_vm_window_width(), emu->get_vm_window_height());
	SetMenuItemInfo(hMenu, ID_SCREEN_FULLSCREEN_STRETCH, FALSE, &info);
	my_stprintf_s(buf, 64, _T("Fullscreen: Stretch (Aspect Ratio %d:%d)"), emu->get_vm_window_width_aspect(), emu->get_vm_window_height_aspect());
	SetMenuItemInfo(hMenu, ID_SCREEN_FULLSCREEN_ASPECT, FALSE, &info);
	my_stprintf_s(buf, 64, _T("Fullscreen: Stretch (Fill)"));
	SetMenuItemInfo(hMenu, ID_SCREEN_FULLSCREEN_FILL, FALSE, &info);
	CheckMenuRadioItem(hMenu, ID_SCREEN_FULLSCREEN_DOTBYDOT, ID_SCREEN_FULLSCREEN_FILL, ID_SCREEN_FULLSCREEN_DOTBYDOT + config.fullscreen_stretch_type, MF_BYCOMMAND);
	
//#ifdef USE_SCREEN_ROTATE
	CheckMenuRadioItem(hMenu, ID_SCREEN_ROTATE_0, ID_SCREEN_ROTATE_270, ID_SCREEN_ROTATE_0 + config.rotate_type, MF_BYCOMMAND);
//#endif
}
#endif

#ifdef USE_SCREEN_FILTER
void update_host_filter_menu(HMENU hMenu)
{
	switch(config.filter_type) {
	case SCREEN_FILTER_RGB:
		CheckMenuRadioItem(hMenu, ID_FILTER_RGB, ID_FILTER_NONE, ID_FILTER_RGB,  MF_BYCOMMAND);
		break;
	case SCREEN_FILTER_RF:
		CheckMenuRadioItem(hMenu, ID_FILTER_RGB, ID_FILTER_NONE, ID_FILTER_RF,   MF_BYCOMMAND);
		break;
	default:
		CheckMenuRadioItem(hMenu, ID_FILTER_RGB, ID_FILTER_NONE, ID_FILTER_NONE, MF_BYCOMMAND);
		break;
	}
}
#endif

void update_host_sound_menu(HMENU hMenu)
{
	if(config.sound_frequency >= 0 && config.sound_frequency < 8) {
		CheckMenuRadioItem(hMenu, ID_SOUND_FREQ0, ID_SOUND_FREQ7, ID_SOUND_FREQ0 + config.sound_frequency, MF_BYCOMMAND);
	}
	if(config.sound_latency >= 0 && config.sound_latency < 5) {
		CheckMenuRadioItem(hMenu, ID_SOUND_LATE0, ID_SOUND_LATE4, ID_SOUND_LATE0 + config.sound_latency, MF_BYCOMMAND);
	}
	CheckMenuRadioItem(hMenu, ID_SOUND_STRICT_RENDER, ID_SOUND_LIGHT_RENDER, config.sound_strict_rendering ? ID_SOUND_STRICT_RENDER : ID_SOUND_LIGHT_RENDER, MF_BYCOMMAND);
}

void update_host_input_menu(HMENU hMenu)
{
}

#ifdef USE_VIDEO_CAPTURE
void update_host_capture_menu(HMENU hMenu)
{
	int num_devs = emu->get_num_capture_devs();
	int cur_index = emu->get_cur_capture_dev_index();
	
	for(int i = 0; i < 8; i++) {
		DeleteMenu(hMenu, ID_CAPTURE_DEVICE + i, MF_BYCOMMAND);
	}
	for(int i = 0; i < 8; i++) {
		if(num_devs >= i + 1) {
			AppendMenu(hMenu, MF_STRING, ID_CAPTURE_DEVICE + i, emu->get_capture_dev_name(i));
		}
	}
	if(num_devs == 0) {
		AppendMenu(hMenu, MF_GRAYED | MF_STRING, ID_CAPTURE_DEVICE, _T("None"));
	}
	if(cur_index != -1) {
		CheckMenuRadioItem(hMenu, ID_CAPTURE_DEVICE, ID_CAPTURE_DEVICE + num_devs - 1, ID_CAPTURE_DEVICE + cur_index, MF_BYCOMMAND);
	}
	EnableMenuItem(hMenu, ID_CAPTURE_FILTER, (cur_index != -1) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_CAPTURE_PIN, (cur_index != -1) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_CAPTURE_SOURCE, (cur_index != -1) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_CAPTURE_CLOSE, (cur_index != -1) ? MF_ENABLED : MF_GRAYED);
}
#endif

void update_menu(HWND hWnd, HMENU hMenu)
{
	int count = GetMenuItemCount(hMenu);
	UINT id = -1;
	for(int pos = 0; pos < count; pos++) {
		if((id = GetMenuItemID(hMenu, pos)) != -1) {
			break;
		}
	}
	if(id >= ID_CONTROL_MENU_START && id <= ID_CONTROL_MENU_END) {
		update_control_menu(hMenu);
#ifdef USE_STATE
	} else if(id >= ID_SAVE_MENU_START && id <= ID_SAVE_MENU_END) {
		update_save_state_menu(hMenu);
	} else if(id >= ID_LOAD_MENU_START && id <= ID_LOAD_MENU_END) {
		update_load_state_menu(hMenu);
#endif
#ifdef USE_CART
#if USE_CART >= 1
	} else if(id >= ID_CART1_MENU_START && id <= ID_CART1_MENU_END) {
		update_cart_menu(hMenu, 0, ID_RECENT_CART1, ID_CLOSE_CART1);
#endif
#if USE_CART >= 2
	} else if(id >= ID_CART2_MENU_START && id <= ID_CART2_MENU_END) {
		update_cart_menu(hMenu, 1, ID_RECENT_CART2, ID_CLOSE_CART2);
#endif
#endif
#ifdef USE_FLOPPY_DISK
#if USE_FLOPPY_DISK >= 1
	} else if(id >= ID_FD1_MENU_START && id <= ID_FD1_MENU_END) {
		update_floppy_disk_menu(hMenu, 0, ID_RECENT_FD1, ID_D88_FILE_PATH1, ID_SELECT_D88_BANK1, ID_EJECT_D88_BANK1, ID_CLOSE_FD1, ID_WRITE_PROTECT_FD1, ID_CORRECT_TIMING_FD1, ID_IGNORE_CRC_FD1);
#endif
#if USE_FLOPPY_DISK >= 2
	} else if(id >= ID_FD2_MENU_START && id <= ID_FD2_MENU_END) {
		update_floppy_disk_menu(hMenu, 1, ID_RECENT_FD2, ID_D88_FILE_PATH2, ID_SELECT_D88_BANK2, ID_EJECT_D88_BANK2, ID_CLOSE_FD2, ID_WRITE_PROTECT_FD2, ID_CORRECT_TIMING_FD2, ID_IGNORE_CRC_FD2);
#endif
#if USE_FLOPPY_DISK >= 3
	} else if(id >= ID_FD3_MENU_START && id <= ID_FD3_MENU_END) {
		update_floppy_disk_menu(hMenu, 2, ID_RECENT_FD3, ID_D88_FILE_PATH3, ID_SELECT_D88_BANK3, ID_EJECT_D88_BANK3, ID_CLOSE_FD3, ID_WRITE_PROTECT_FD3, ID_CORRECT_TIMING_FD3, ID_IGNORE_CRC_FD3);
#endif
#if USE_FLOPPY_DISK >= 4
	} else if(id >= ID_FD4_MENU_START && id <= ID_FD4_MENU_END) {
		update_floppy_disk_menu(hMenu, 3, ID_RECENT_FD4, ID_D88_FILE_PATH4, ID_SELECT_D88_BANK4, ID_EJECT_D88_BANK4, ID_CLOSE_FD4, ID_WRITE_PROTECT_FD4, ID_CORRECT_TIMING_FD4, ID_IGNORE_CRC_FD4);
#endif
#if USE_FLOPPY_DISK >= 5
	} else if(id >= ID_FD5_MENU_START && id <= ID_FD5_MENU_END) {
		update_floppy_disk_menu(hMenu, 4, ID_RECENT_FD5, ID_D88_FILE_PATH5, ID_SELECT_D88_BANK5, ID_EJECT_D88_BANK5, ID_CLOSE_FD5, ID_WRITE_PROTECT_FD5, ID_CORRECT_TIMING_FD5, ID_IGNORE_CRC_FD5);
#endif
#if USE_FLOPPY_DISK >= 6
	} else if(id >= ID_FD6_MENU_START && id <= ID_FD6_MENU_END) {
		update_floppy_disk_menu(hMenu, 5, ID_RECENT_FD6, ID_D88_FILE_PATH6, ID_SELECT_D88_BANK6, ID_EJECT_D88_BANK6, ID_CLOSE_FD6, ID_WRITE_PROTECT_FD6, ID_CORRECT_TIMING_FD6, ID_IGNORE_CRC_FD6);
#endif
#if USE_FLOPPY_DISK >= 7
	} else if(id >= ID_FD7_MENU_START && id <= ID_FD7_MENU_END) {
		update_floppy_disk_menu(hMenu, 6, ID_RECENT_FD7, ID_D88_FILE_PATH7, ID_SELECT_D88_BANK7, ID_EJECT_D88_BANK7, ID_CLOSE_FD7, ID_WRITE_PROTECT_FD7, ID_CORRECT_TIMING_FD7, ID_IGNORE_CRC_FD7);
#endif
#if USE_FLOPPY_DISK >= 8
	} else if(id >= ID_FD8_MENU_START && id <= ID_FD8_MENU_END) {
		update_floppy_disk_menu(hMenu, 7, ID_RECENT_FD8, ID_D88_FILE_PATH8, ID_SELECT_D88_BANK8, ID_EJECT_D88_BANK8, ID_CLOSE_FD8, ID_WRITE_PROTECT_FD8, ID_CORRECT_TIMING_FD8, ID_IGNORE_CRC_FD8);
#endif
#endif
#ifdef USE_QUICK_DISK
#if USE_QUICK_DISK >= 1
	} else if(id >= ID_QD1_MENU_START && id <= ID_QD1_MENU_END) {
		update_quick_disk_menu(hMenu, 0, ID_RECENT_QD1, ID_CLOSE_QD1);
#endif
#if USE_QUICK_DISK >= 2
	} else if(id >= ID_QD2_MENU_START && id <= ID_QD2_MENU_END) {
		update_quick_disk_menu(hMenu, 1, ID_RECENT_QD2, ID_CLOSE_QD2);
#endif
#endif
#ifdef USE_HARD_DISK
#if USE_HARD_DISK >= 1
	} else if(id >= ID_HD1_MENU_START && id <= ID_HD1_MENU_END) {
		update_hard_disk_menu(hMenu, 0, ID_RECENT_HD1, ID_CLOSE_HD1);
#endif
#if USE_HARD_DISK >= 2
	} else if(id >= ID_HD2_MENU_START && id <= ID_HD2_MENU_END) {
		update_hard_disk_menu(hMenu, 1, ID_RECENT_HD2, ID_CLOSE_HD2);
#endif
#if USE_HARD_DISK >= 3
	} else if(id >= ID_HD3_MENU_START && id <= ID_HD3_MENU_END) {
		update_hard_disk_menu(hMenu, 2, ID_RECENT_HD3, ID_CLOSE_HD3);
#endif
#if USE_HARD_DISK >= 4
	} else if(id >= ID_HD4_MENU_START && id <= ID_HD4_MENU_END) {
		update_hard_disk_menu(hMenu, 3, ID_RECENT_HD4, ID_CLOSE_HD4);
#endif
#if USE_HARD_DISK >= 5
	} else if(id >= ID_HD5_MENU_START && id <= ID_HD5_MENU_END) {
		update_hard_disk_menu(hMenu, 4, ID_RECENT_HD5, ID_CLOSE_HD5);
#endif
#if USE_HARD_DISK >= 6
	} else if(id >= ID_HD6_MENU_START && id <= ID_HD6_MENU_END) {
		update_hard_disk_menu(hMenu, 5, ID_RECENT_HD6, ID_CLOSE_HD6);
#endif
#if USE_HARD_DISK >= 7
	} else if(id >= ID_HD7_MENU_START && id <= ID_HD7_MENU_END) {
		update_hard_disk_menu(hMenu, 6, ID_RECENT_HD7, ID_CLOSE_HD7);
#endif
#if USE_HARD_DISK >= 8
	} else if(id >= ID_HD8_MENU_START && id <= ID_HD8_MENU_END) {
		update_hard_disk_menu(hMenu, 7, ID_RECENT_HD8, ID_CLOSE_HD8);
#endif
#endif
#ifdef USE_TAPE
#if USE_TAPE >= 1
	} else if(id >= ID_TAPE1_MENU_START && id <= ID_TAPE1_MENU_END) {
		update_tape_menu(hMenu, 0, ID_RECENT_TAPE1, ID_CLOSE_TAPE1, ID_PLAY_BUTTON1, ID_STOP_BUTTON1, ID_FAST_FORWARD1, ID_FAST_REWIND1, ID_APSS_FORWARD1, ID_APSS_REWIND1, ID_USE_WAVE_SHAPER1, ID_DIRECT_LOAD_MZT1, ID_TAPE_BAUD_LOW1, ID_TAPE_BAUD_HIGH1);
#endif
#if USE_TAPE >= 2
	} else if(id >= ID_TAPE2_MENU_START && id <= ID_TAPE2_MENU_END) {
		update_tape_menu(hMenu, 1, ID_RECENT_TAPE2, ID_CLOSE_TAPE2, ID_PLAY_BUTTON2, ID_STOP_BUTTON2, ID_FAST_FORWARD2, ID_FAST_REWIND2, ID_APSS_FORWARD2, ID_APSS_REWIND2, ID_USE_WAVE_SHAPER2, ID_DIRECT_LOAD_MZT2, ID_TAPE_BAUD_LOW2, ID_TAPE_BAUD_HIGH2);
#endif
#endif
#ifdef USE_COMPACT_DISC
#if USE_COMPACT_DISC >= 1
	} else if(id >= ID_COMPACT_DISC1_MENU_START && id <= ID_COMPACT_DISC1_MENU_END) {
		update_compact_disc_menu(hMenu, 0, ID_RECENT_COMPACT_DISC1, ID_CLOSE_COMPACT_DISC1);
#endif
#if USE_COMPACT_DISC >= 2
	} else if(id >= ID_COMPACT_DISC2_MENU_START && id <= ID_COMPACT_DISC2_MENU_END) {
		update_compact_disc_menu(hMenu, 1, ID_RECENT_COMPACT_DISC2, ID_CLOSE_COMPACT_DISC2);
#endif
#endif
#ifdef USE_LASER_DISC
#if USE_LASER_DISC >= 1
	} else if(id >= ID_LASER_DISC1_MENU_START && id <= ID_LASER_DISC1_MENU_END) {
		update_laser_disc_menu(hMenu, 0, ID_RECENT_LASER_DISC1, ID_CLOSE_LASER_DISC1);
#endif
#if USE_LASER_DISC >= 2
	} else if(id >= ID_LASER_DISC2_MENU_START && id <= ID_LASER_DISC2_MENU_END) {
		update_laser_disc_menu(hMenu, 1, ID_RECENT_LASER_DISC2, ID_CLOSE_LASER_DISC2);
#endif
#endif
#ifdef USE_BINARY_FILE
#if USE_BINARY_FILE >= 1
	} else if(id >= ID_BINARY1_MENU_START && id <= ID_BINARY1_MENU_END) {
		update_binary_menu(hMenu, 0, ID_RECENT_BINARY1);
#endif
#if USE_BINARY_FILE >= 2
	} else if(id >= ID_BINARY2_MENU_START && id <= ID_BINARY2_MENU_END) {
		update_binary_menu(hMenu, 1, ID_RECENT_BINARY2);
#endif
#endif
#ifdef USE_BUBBLE
#if USE_BUBBLE >= 1
	} else if(id >= ID_BUBBLE1_MENU_START && id <= ID_BUBBLE1_MENU_END) {
		update_bubble_casette_menu(hMenu, 0, ID_RECENT_BUBBLE1);
#endif
#if USE_BUBBLE >= 2
	} else if(id >= ID_BUBBLE2_MENU_START && id <= ID_BUBBLE2_MENU_END) {
		update_bubble_casette_menu(hMenu, 1, ID_RECENT_BUBBLE2);
#endif
#endif
#ifdef USE_BOOT_MODE
	} else if(id >= ID_VM_BOOT_MENU_START && id <= ID_VM_BOOT_MENU_END) {
		 update_vm_boot_menu(hMenu);
#endif
#ifdef USE_CPU_TYPE
	} else if(id >= ID_VM_CPU_MENU_START && id <= ID_VM_CPU_MENU_END) {
		 update_vm_cpu_menu(hMenu);
#endif
#ifdef USE_DIPSWITCH
	} else if(id >= ID_VM_DIPSWITCH_MENU_START && id <= ID_VM_DIPSWITCH_MENU_END) {
		 update_vm_dipswitch_menu(hMenu);
#endif
#ifdef USE_DEVICE_TYPE
	} else if(id >= ID_VM_DEVICE_MENU_START && id <= ID_VM_DEVICE_MENU_END) {
		 update_vm_device_menu(hMenu);
#endif
#ifdef USE_DRIVE_TYPE
	} else if(id >= ID_VM_DRIVE_MENU_START && id <= ID_VM_DRIVE_MENU_END) {
		 update_vm_drive_menu(hMenu);
#endif
#ifdef USE_KEYBOARD_TYPE
	} else if(id >= ID_VM_KEYBOARD_MENU_START && id <= ID_VM_KEYBOARD_MENU_END) {
		 update_vm_keyboard_menu(hMenu);
#endif
#ifdef USE_MOUSE_TYPE
	} else if(id >= ID_VM_MOUSE_MENU_START && id <= ID_VM_MOUSE_MENU_END) {
		 update_vm_mouse_menu(hMenu);
#endif
#ifdef USE_JOYSTICK_TYPE
	} else if(id >= ID_VM_JOYSTICK_MENU_START && id <= ID_VM_JOYSTICK_MENU_END) {
		 update_vm_joystick_menu(hMenu);
#endif
#if defined(USE_SOUND_TYPE) || defined(USE_FLOPPY_DISK) || defined(USE_TAPE) || defined(USE_DIPSWITCH)
	} else if(id >= ID_VM_SOUND_MENU_START && id <= ID_VM_SOUND_MENU_END) {
		 update_vm_sound_menu(hMenu);
#ifdef USE_DIPSWITCH
		// dipswitch may be in sound menu
		 update_vm_dipswitch_menu(hMenu);
#endif
#endif
#if defined(USE_MONITOR_TYPE) || defined(USE_SCANLINE)
	} else if(id >= ID_VM_MONITOR_MENU_START && id <= ID_VM_MONITOR_MENU_END) {
		 update_vm_monitor_menu(hMenu);
#endif
#ifdef USE_PRINTER_TYPE
	} else if(id >= ID_VM_PRINTER_MENU_START && id <= ID_VM_PRINTER_MENU_START) {
		 update_vm_printer_menu(hMenu);
#endif
	} else if(id >= ID_HOST_MENU_START && id <= ID_HOST_MENU_END) {
		update_host_menu(hMenu);
#ifndef ONE_BOARD_MICRO_COMPUTER
	} else if(id >= ID_SCREEN_MENU_START && id <= ID_SCREEN_MENU_END) {
		update_host_screen_menu(hMenu);
#endif
#ifdef USE_SCREEN_FILTER
	} else if(id >= ID_FILTER_MENU_START && id <= ID_FILTER_MENU_END) {
		update_host_filter_menu(hMenu);
#endif
	} else if(id >= ID_SOUND_MENU_START && id <= ID_SOUND_MENU_END) {
		update_host_sound_menu(hMenu);
	} else if(id >= ID_INPUT_MENU_START && id <= ID_INPUT_MENU_END) {
		update_host_input_menu(hMenu);
#ifdef USE_VIDEO_CAPTURE
	} else if(id >= ID_CAPTURE_MENU_START && id <= ID_CAPTURE_MENU_END) {
		update_host_capture_menu(hMenu);
#endif
	}
	DrawMenuBar(hWnd);
}

void show_menu_bar(HWND hWnd)
{
	if(!(hMenu != NULL && IsMenu(hMenu))) {
		hMenu = LoadMenu((HINSTANCE)GetModuleHandle(0), MAKEINTRESOURCE(IDR_MENU1));
		SetMenu(hWnd, hMenu);
	}
}

void hide_menu_bar(HWND hWnd)
{
	if(hMenu != NULL && IsMenu(hMenu)) {
		SetMenu(hWnd, NULL);
		DestroyMenu(hMenu);
		hMenu = NULL;
	}
}

// ----------------------------------------------------------------------------
// status bar
// ----------------------------------------------------------------------------

void show_status_bar(HWND hWnd)
{
//#if defined(USE_FLOPPY_DISK) || defined(USE_QUICK_DISK) || defined(USE_HARD_DISK) || defined(USE_COMPACT_DISC) || defined(USE_LASER_DISC) || (defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY))
	if(hStatus == NULL) {
		InitCommonControls();
		hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE | CCS_BOTTOM, NULL, hWnd, ID_STATUS);
		SendMessage(hStatus, SB_SETTEXT, (WPARAM)0 | SBT_OWNERDRAW, (LPARAM)NULL);
	}
	ShowWindow(hStatus, SW_SHOW);
//#endif
	status_bar_visible = true;
}

void hide_status_bar(HWND hWnd)
{
	if(hStatus != NULL) {
		ShowWindow(hStatus, SW_HIDE);
	}
	status_bar_visible = false;
}

int get_status_bar_height()
{
	if(hStatus != NULL && status_bar_visible) {
		RECT rect;
		GetWindowRect(hStatus, &rect);
		return rect.bottom - rect.top;
	}
	return 0;
}

bool get_status_bar_updated()
{
	bool updated = false;
	
#ifdef USE_FLOPPY_DISK
	uint32_t new_fd_status = emu->is_floppy_disk_accessed();
	if(fd_status != new_fd_status) {
		updated = true;
		fd_status = new_fd_status;
	}
#endif
#ifdef USE_QUICK_DISK
	uint32_t new_qd_status = emu->is_quick_disk_accessed();
	if(qd_status != new_qd_status) {
		updated = true;
		qd_status = new_qd_status;
	}
#endif
#ifdef USE_HARD_DISK
	uint32_t new_hd_status = emu->is_hard_disk_accessed();
	if(hd_status != new_hd_status) {
		updated = true;
		hd_status = new_hd_status;
	}
#endif
#ifdef USE_COMPACT_DISC
	uint32_t new_cd_status = emu->is_compact_disc_accessed();
	if(cd_status != new_cd_status) {
		updated = true;
		cd_status = new_cd_status;
	}
#endif
#ifdef USE_LASER_DISC
	uint32_t new_ld_status = emu->is_laser_disc_accessed();
	if(ld_status != new_ld_status) {
		updated = true;
		ld_status = new_ld_status;
	}
#endif
#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
	_TCHAR new_tape_status[array_length(tape_status)] = {0};
	for(int drv = 0; drv < USE_TAPE; drv++) {
		const _TCHAR* message = emu->get_tape_message(drv);
		if(message != NULL) {
			my_tcscpy_s(new_tape_status, array_length(new_tape_status), message);
			break;
		}
	}
	if(_tcsicmp(tape_status, new_tape_status) != 0) {
		updated = true;
		my_tcscpy_s(tape_status, array_length(tape_status), new_tape_status);
	}
#endif
	return updated;
}

void update_status_bar(HINSTANCE hInstance, LPDRAWITEMSTRUCT lpDrawItem)
{
	TEXTMETRIC tm;
	GetTextMetrics(lpDrawItem->hDC, &tm);
	int text_top = (lpDrawItem->rcItem.top + lpDrawItem->rcItem.bottom) / 2 - tm.tmHeight / 2;
	int draw_left = lpDrawItem->rcItem.left;
	
	SetTextColor(lpDrawItem->hDC, RGB(0, 0, 0));
	SetBkMode(lpDrawItem->hDC, TRANSPARENT);
	
	#if defined(USE_FLOPPY_DISK) || defined(USE_QUICK_DISK) || defined(USE_HARD_DISK) || defined(USE_COMPACT_DISC) || defined(USE_LASER_DISC)
	{
		HDC hdcMem = CreateCompatibleDC(lpDrawItem->hDC);
		HBITMAP hBitmap[2];
		BITMAP bmp[2];
		hBitmap[0] = LoadBitmap(hInstance, _T("IDI_BITMAP_ACCESS_OFF"));
		hBitmap[1] = LoadBitmap(hInstance, _T("IDI_BITMAP_ACCESS_ON" ));
		
		if(hBitmap[0] != NULL && hBitmap[1] != NULL) {
			GetObject(hBitmap[0], sizeof(BITMAP), &bmp[0]);
			GetObject(hBitmap[1], sizeof(BITMAP), &bmp[1]);
			
			int bmp_width = bmp[0].bmWidth;
			int bmp_height = bmp[0].bmHeight;
			int bmp_top = (lpDrawItem->rcItem.top + lpDrawItem->rcItem.bottom) / 2 - bmp_height / 2;
			SIZE size;
			
			#ifdef USE_FLOPPY_DISK
				TextOut(lpDrawItem->hDC, draw_left, text_top, _T("FD:"), 3);
				GetTextExtentPoint32(lpDrawItem->hDC, _T("FD:"), 3, &size);
				draw_left += size.cx + 4;
				
				for(int i = 0; i < USE_FLOPPY_DISK; i++) {
					int idx = (fd_status >> i) & 1;
					SelectObject(hdcMem, hBitmap[idx]);
					TransparentBlt(lpDrawItem->hDC, draw_left, bmp_top, bmp_width, bmp_height, hdcMem, 0, 0, bmp_width, bmp_height, 0);
					draw_left += bmp_width + 2;
				}
				draw_left += 8;
			#endif
			#ifdef USE_QUICK_DISK
				TextOut(lpDrawItem->hDC, draw_left, text_top, _T("QD:"), 3);
				GetTextExtentPoint32(lpDrawItem->hDC, _T("QD:"), 3, &size);
				draw_left += size.cx + 4;
				
				for(int i = 0; i < USE_QUICK_DISK; i++) {
					int idx = (qd_status >> i) & 1;
					SelectObject(hdcMem, hBitmap[idx]);
					TransparentBlt(lpDrawItem->hDC, draw_left, bmp_top, bmp_width, bmp_height, hdcMem, 0, 0, bmp_width, bmp_height, 0);
					draw_left += bmp_width + 2;
				}
				draw_left += 8;
			#endif
			#ifdef USE_HARD_DISK
				TextOut(lpDrawItem->hDC, draw_left, text_top, _T("HD:"), 3);
				GetTextExtentPoint32(lpDrawItem->hDC, _T("HD:"), 3, &size);
				draw_left += size.cx + 4;
				
				for(int i = 0; i < USE_HARD_DISK; i++) {
					int idx = (hd_status >> i) & 1;
					SelectObject(hdcMem, hBitmap[idx]);
					TransparentBlt(lpDrawItem->hDC, draw_left, bmp_top, bmp_width, bmp_height, hdcMem, 0, 0, bmp_width, bmp_height, 0);
					draw_left += bmp_width + 2;
				}
				draw_left += 8;
			#endif
			#ifdef USE_COMPACT_DISC
				TextOut(lpDrawItem->hDC, draw_left, text_top, _T("CD:"), 3);
				GetTextExtentPoint32(lpDrawItem->hDC, _T("CD:"), 3, &size);
				draw_left += size.cx + 4;
				
				for(int i = 0; i < USE_COMPACT_DISC; i++) {
					int idx = (cd_status >> i) & 1;
					SelectObject(hdcMem, hBitmap[idx]);
					TransparentBlt(lpDrawItem->hDC, draw_left, bmp_top, bmp_width, bmp_height, hdcMem, 0, 0, bmp_width, bmp_height, 0);
					draw_left += bmp_width + 2;
				}
				draw_left += 8;
			#endif
			#ifdef USE_LASER_DISC
				TextOut(lpDrawItem->hDC, draw_left, text_top, _T("LD:"), 3);
				GetTextExtentPoint32(lpDrawItem->hDC, _T("LD:"), 3, &size);
				draw_left += size.cx + 4;
				
				for(int i = 0; i < USE_LASER_DISC; i++) {
					int idx = (ld_status >> i) & 1;
					SelectObject(hdcMem, hBitmap[idx]);
					TransparentBlt(lpDrawItem->hDC, draw_left, bmp_top, bmp_width, bmp_height, hdcMem, 0, 0, bmp_width, bmp_height, 0);
					draw_left += bmp_width + 2;
				}
				draw_left += 8;
			#endif
		}
		if(hBitmap[0] != NULL) {
			DeleteObject(hBitmap[0]);
		}
		if(hBitmap[1] != NULL) {
			DeleteObject(hBitmap[1]);
		}
		DeleteDC(hdcMem);
	}
	#endif
	#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
	{
		SIZE size;
		
		TextOut(lpDrawItem->hDC, draw_left, text_top, _T("CMT:"), 4);
		GetTextExtentPoint32(lpDrawItem->hDC, _T("CMT:"), 4, &size);
		draw_left += size.cx + 4;
		TextOut(lpDrawItem->hDC, draw_left, text_top, tape_status, _tcslen(tape_status));
	}
	#endif
}

// ----------------------------------------------------------------------------
// file
// ----------------------------------------------------------------------------

#define UPDATE_HISTORY(path, recent) { \
	int index = MAX_HISTORY - 1; \
	for(int i = 0; i < MAX_HISTORY; i++) { \
		if(_tcsicmp(recent[i], path) == 0) { \
			index = i; \
			break; \
		} \
	} \
	for(int i = index; i > 0; i--) { \
		my_tcscpy_s(recent[i], _MAX_PATH, recent[i - 1]); \
	} \
	my_tcscpy_s(recent[0], _MAX_PATH, path); \
}

#ifdef USE_CART
void open_cart_dialog(HWND hWnd, int drv)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
#if defined(_COLECOVISION)
		_T("Supported Files (*.rom;*.bin;*.hex;*.col)\0*.rom;*.bin;*.hex;*.col\0All Files (*.*)\0*.*\0\0"),
		_T("Game Cartridge"),
#elif defined(_GAMEGEAR)
		_T("Supported Files (*.rom;*.bin;*.hex;*.gg;*.col)\0*.rom;*.bin;*.hex;*.gg;*.col\0All Files (*.*)\0*.*\0\0"),
		_T("Game Cartridge"),
#elif defined(_MASTERSYSTEM)
		_T("Supported Files (*.rom;*.bin;*.hex;*.sms)\0*.rom;*.bin;*.hex;*.sms\0All Files (*.*)\0*.*\0\0"),
		_T("Game Cartridge"),
#elif defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
		_T("Supported Files (*.rom;*.bin;*.hex;*.60)\0*.rom;*.bin;*.hex;*.60\0All Files (*.*)\0*.*\0\0"),
		_T("Game Cartridge"),
#elif defined(_PCENGINE) || defined(_X1TWIN)
		_T("Supported Files (*.rom;*.bin;*.hex;*.pce)\0*.rom;*.bin;*.hex;*.pce\0All Files (*.*)\0*.*\0\0"),
		_T("HuCARD"),
#elif defined(_SC3000)
		_T("Supported Files (*.rom;*.bin;*.hex;*.sc;*.sg)\0*.rom;*.bin;*.hex;*.sc;*.sg\0All Files (*.*)\0*.*\0\0"),
		_T("Game Cartridge"),
#else
		_T("Supported Files (*.rom;*.bin;*.hex)\0*.rom;*.bin;*.hex\0All Files (*.*)\0*.*\0\0"), 
		_T("Game Cartridge"),
#endif
		NULL,
		config.initial_cart_dir, _MAX_PATH
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_cart_path[drv]);
		my_tcscpy_s(config.initial_cart_dir, _MAX_PATH, get_parent_dir(path));
		emu->open_cart(drv, path);
	}
}

void open_recent_cart(int drv, int index)
{
	_TCHAR path[_MAX_PATH];
	my_tcscpy_s(path, _MAX_PATH, config.recent_cart_path[drv][index]);
	for(int i = index; i > 0; i--) {
		my_tcscpy_s(config.recent_cart_path[drv][i], _MAX_PATH, config.recent_cart_path[drv][i - 1]);
	}
	my_tcscpy_s(config.recent_cart_path[drv][0], _MAX_PATH, path);
	emu->open_cart(drv, path);
}
#endif

#ifdef USE_FLOPPY_DISK
void open_floppy_disk_dialog(HWND hWnd, int drv)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
		_T("Supported Files (*.d88;*.d77;*.1dd;*.td0;*.imd;*.dsk;*.nfd;*.fdi;*.hdm;*.hd5;*.hd4;*.hdb;*.dd9;*.dd6;*.tfd;*.xdf;*.2d;*.sf7;*.img;*.ima;*.vfd)\0*.d88;*.d77;*.1dd;*.td0;*.imd;*.dsk;*.nfd;*.fdi;*.hdm;*.hd5;*.hd4;*.hdb;*.dd9;*.dd6;*.tfd;*.xdf;*.2d;*.sf7;*.img;*.ima;*.vfd\0All Files (*.*)\0*.*\0\0"),
		_T("Floppy Disk"),
		NULL,
		config.initial_floppy_disk_dir, _MAX_PATH
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_floppy_disk_path[drv]);
		my_tcscpy_s(config.initial_floppy_disk_dir, _MAX_PATH, get_parent_dir(path));
		open_floppy_disk(drv, path, 0);
	}
}

void open_blank_floppy_disk_dialog(HWND hWnd, int drv, uint8_t type)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
		_T("Supported Files (*.d88;*.d77)\0*.d88;*.d77\0All Files (*.*)\0*.*\0\0"),
		_T("Floppy Disk"),
		create_date_file_name(_T("d88")),
		config.initial_floppy_disk_dir, _MAX_PATH
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_floppy_disk_path[drv]);
		my_tcscpy_s(config.initial_floppy_disk_dir, _MAX_PATH, get_parent_dir(path));
		emu->create_bank_floppy_disk(path, type);
		open_floppy_disk(drv, path, 0);
	}
}

void open_floppy_disk(int drv, const _TCHAR* path, int bank)
{
	emu->d88_file[drv].bank_num = 0;
	emu->d88_file[drv].cur_bank = -1;
	
	if(check_file_extension(path, _T(".d88")) || check_file_extension(path, _T(".d77")) || check_file_extension(path, _T(".1dd"))) {
		FILEIO *fio = new FILEIO();
		if(fio->Fopen(path, FILEIO_READ_BINARY)) {
			try {
				fio->Fseek(0, FILEIO_SEEK_END);
				uint32_t file_size = fio->Ftell(), file_offset = 0;
				while(file_offset + 0x2b0 <= file_size && emu->d88_file[drv].bank_num < MAX_D88_BANKS) {
					fio->Fseek(file_offset, FILEIO_SEEK_SET);
#ifdef _UNICODE
					char tmp[18];
					fio->Fread(tmp, 17, 1);
					tmp[17] = 0;
					MultiByteToWideChar(CP_ACP, 0, tmp, -1, emu->d88_file[drv].disk_name[emu->d88_file[drv].bank_num], 18);
#else
					fio->Fread(emu->d88_file[drv].disk_name[emu->d88_file[drv].bank_num], 17, 1);
					emu->d88_file[drv].disk_name[emu->d88_file[drv].bank_num][17] = 0;
#endif
					fio->Fseek(file_offset + 0x1c, SEEK_SET);
					file_offset += fio->FgetUint32_LE();
					emu->d88_file[drv].bank_num++;
				}
				my_tcscpy_s(emu->d88_file[drv].path, _MAX_PATH, path);
				emu->d88_file[drv].cur_bank = bank;
			} catch(...) {
				emu->d88_file[drv].bank_num = 0;
			}
			fio->Fclose();
		}
		delete fio;
	}
	emu->open_floppy_disk(drv, path, bank);
#if USE_FLOPPY_DISK >= 2
	if((drv & 1) == 0 && drv + 1 < USE_FLOPPY_DISK && bank + 1 < emu->d88_file[drv].bank_num) {
		open_floppy_disk(drv + 1, path, bank + 1);
	}
#endif
}

void open_recent_floppy_disk(int drv, int index)
{
	_TCHAR path[_MAX_PATH];
	my_tcscpy_s(path, _MAX_PATH, config.recent_floppy_disk_path[drv][index]);
	for(int i = index; i > 0; i--) {
		my_tcscpy_s(config.recent_floppy_disk_path[drv][i], _MAX_PATH, config.recent_floppy_disk_path[drv][i - 1]);
	}
	my_tcscpy_s(config.recent_floppy_disk_path[drv][0], _MAX_PATH, path);
	open_floppy_disk(drv, path, 0);
}

void select_d88_bank(int drv, int index)
{
	if(emu->d88_file[drv].cur_bank != index) {
		emu->open_floppy_disk(drv, emu->d88_file[drv].path, index);
		emu->d88_file[drv].cur_bank = index;
	}
}

void close_floppy_disk(int drv)
{
	emu->close_floppy_disk(drv);
	emu->d88_file[drv].bank_num = 0;
	emu->d88_file[drv].cur_bank = -1;

}
#endif

#ifdef USE_QUICK_DISK
void open_quick_disk_dialog(HWND hWnd, int drv)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
		_T("Supported Files (*.mzt;*.q20;*.qdf)\0*.mzt;*.q20;*.qdf\0All Files (*.*)\0*.*\0\0"),
		_T("Quick Disk"),
		NULL,
		config.initial_quick_disk_dir, _MAX_PATH
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_quick_disk_path[drv]);
		my_tcscpy_s(config.initial_quick_disk_dir, _MAX_PATH, get_parent_dir(path));
		emu->open_quick_disk(drv, path);
	}
}

void open_recent_quick_disk(int drv, int index)
{
	_TCHAR path[_MAX_PATH];
	my_tcscpy_s(path, _MAX_PATH, config.recent_quick_disk_path[drv][index]);
	for(int i = index; i > 0; i--) {
		my_tcscpy_s(config.recent_quick_disk_path[drv][i], _MAX_PATH, config.recent_quick_disk_path[drv][i - 1]);
	}
	my_tcscpy_s(config.recent_quick_disk_path[drv][0], _MAX_PATH, path);
	emu->open_quick_disk(drv, path);
}
#endif

#ifdef USE_HARD_DISK
void open_hard_disk_dialog(HWND hWnd, int drv)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
		_T("Supported Files (*.thd;*.nhd;*.hdi;*.hdd;*.dat)\0*.thd;*.nhd;*.hdi;*.hdd;*.dat\0All Files (*.*)\0*.*\0\0"),
		_T("Hard Disk"),
		NULL,
		config.initial_hard_disk_dir, _MAX_PATH
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_hard_disk_path[drv]);
		my_tcscpy_s(config.initial_hard_disk_dir, _MAX_PATH, get_parent_dir(path));
		emu->open_hard_disk(drv, path);
	}
}

void open_recent_hard_disk(int drv, int index)
{
	_TCHAR path[_MAX_PATH];
	my_tcscpy_s(path, _MAX_PATH, config.recent_hard_disk_path[drv][index]);
	for(int i = index; i > 0; i--) {
		my_tcscpy_s(config.recent_hard_disk_path[drv][i], _MAX_PATH, config.recent_hard_disk_path[drv][i - 1]);
	}
	my_tcscpy_s(config.recent_hard_disk_path[drv][0], _MAX_PATH, path);
	emu->open_hard_disk(drv, path);
}
#endif

#ifdef USE_TAPE
void open_tape_dialog(HWND hWnd, int drv, bool play)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
#if defined(_PC6001) || defined(_PC6001MK2) || defined(_PC6001MK2SR) || defined(_PC6601) || defined(_PC6601SR)
		play ? _T("Supported Files (*.wav;*.cas;*.p6;*.p6t;*.gz)\0*.wav;*.cas;*.p6;*.p6t;*.gz\0All Files (*.*)\0*.*\0\0")
		     : _T("Supported Files (*.wav;*.cas;*.p6;*.p6t)\0*.wav;*.cas;*.p6;*.p6t\0All Files (*.*)\0*.*\0\0"),
#elif defined(_PC8001) || defined(_PC8001MK2) || defined(_PC8001SR) || defined(_PC8801) || defined(_PC8801MK2) || defined(_PC8801MA) || defined(_PC98DO)
		play ? _T("Supported Files (*.cas;*.cmt;*.n80;*.t88)\0*.cas;*.cmt;*.n80;*.t88\0All Files (*.*)\0*.*\0\0")
		     : _T("Supported Files (*.cas;*.cmt)\0*.cas;*.cmt\0All Files (*.*)\0*.*\0\0"),
#elif defined(_MZ80A) || defined(_MZ80K) || defined(_MZ1200) || defined(_MZ700) || defined(_MZ800) || defined(_MZ1500)
		play ? _T("Supported Files (*.wav;*.cas;*.mzt;*.mzf;*.m12;*.gz)\0*.wav;*.cas;*.mzt;*.mzf;*.m12;*.gz\0All Files (*.*)\0*.*\0\0")
		     : _T("Supported Files (*.wav;*.cas)\0*.wav;*.cas\0All Files (*.*)\0*.*\0\0"),
#elif defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
		play ? _T("Supported Files (*.wav;*.cas;*.mzt;*.mzf;*.mti;*.mtw;*.dat;*.gz)\0*.wav;*.cas;*.mzt;*.mzf;*.mti;*.mtw;*.dat;*.gz\0All Files (*.*)\0*.*\0\0")
		     : _T("Supported Files (*.wav;*.cas)\0*.wav;*.cas\0All Files (*.*)\0*.*\0\0"),
#elif defined(_X1) || defined(_X1TWIN) || defined(_X1TURBO) || defined(_X1TURBOZ)
		play ? _T("Supported Files (*.wav;*.cas;*.tap;*.gz)\0*.wav;*.cas;*.tap;*.gz\0All Files (*.*)\0*.*\0\0")
		     : _T("Supported Files (*.wav;*.cas;*.tap)\0*.wav;*.cas;*.tap\0All Files (*.*)\0*.*\0\0"),
#elif defined(_FM8) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		play ? _T("Supported Files (*.wav;*.cas;*.t77;*.gz)\0*.wav;*.cas;*.t77;*.gz\0All Files (*.*)\0*.*\0\0")
		     : _T("Supported Files (*.wav;*.cas;*.t77)\0*.wav;*.cas;*.t77\0All Files (*.*)\0*.*\0\0"),
#elif defined(_BMJR)
		play ? _T("Supported Files (*.wav;*.cas;*.bin;*.gz)\0*.wav;*.cas;*.bin;*.gz\0All Files (*.*)\0*.*\0\0")
		     : _T("Supported Files (*.wav;*.cas)\0*.wav;*.cas\0All Files (*.*)\0*.*\0\0"),
#elif defined(_TK80BS)
		drv == 1 ? _T("Supported Files (*.cas;*.cmt)\0*.cas;*.cmt\0All Files (*.*)\0*.*\0\0")
		: play   ? _T("Supported Files (*.wav;*.cas;*.gz)\0*.wav;*.cas;*.gz\0All Files (*.*)\0*.*\0\0")
		         : _T("Supported Files (*.wav;*.cas)\0*.wav;*.cas\0All Files (*.*)\0*.*\0\0"),
#elif !defined(TAPE_BINARY_ONLY)
		play ? _T("Supported Files (*.wav;*.cas;*.gz)\0*.wav;*.cas;*.gz\0All Files (*.*)\0*.*\0\0")
		     : _T("Supported Files (*.wav;*.cas)\0*.wav;*.cas\0All Files (*.*)\0*.*\0\0"),
#else
		_T("Supported Files (*.cas;*.cmt)\0*.cas;*.cmt\0All Files (*.*)\0*.*\0\0"),
#endif
		play ? _T("Data Recorder Tape [Play]") : _T("Data Recorder Tape [Rec]"),
		NULL,
		config.initial_tape_dir, _MAX_PATH
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_tape_path[drv]);
		my_tcscpy_s(config.initial_tape_dir, _MAX_PATH, get_parent_dir(path));
		if(play) {
			emu->play_tape(drv, path);
		} else {
			emu->rec_tape(drv, path);
		}
	}
}

void open_recent_tape(int drv, int index)
{
	_TCHAR path[_MAX_PATH];
	my_tcscpy_s(path, _MAX_PATH, config.recent_tape_path[drv][index]);
	for(int i = index; i > 0; i--) {
		my_tcscpy_s(config.recent_tape_path[drv][i], _MAX_PATH, config.recent_tape_path[drv][i - 1]);
	}
	my_tcscpy_s(config.recent_tape_path[drv][0], _MAX_PATH, path);
	emu->play_tape(drv, path);
}
#endif

#ifdef USE_COMPACT_DISC
void open_compact_disc_dialog(HWND hWnd, int drv)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
		_T("Supported Files (*.ccd;*.cue)\0*.ccd;*.cue\0All Files (*.*)\0*.*\0\0"),
		_T("Compact Disc"),
		NULL,
		config.initial_compact_disc_dir, _MAX_PATH
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_compact_disc_path[drv]);
		my_tcscpy_s(config.initial_compact_disc_dir, _MAX_PATH, get_parent_dir(path));
		emu->open_compact_disc(drv, path);
	}
}

void open_recent_compact_disc(int drv, int index)
{
	_TCHAR path[_MAX_PATH];
	my_tcscpy_s(path, _MAX_PATH, config.recent_compact_disc_path[drv][index]);
	for(int i = index; i > 0; i--) {
		my_tcscpy_s(config.recent_compact_disc_path[drv][i], _MAX_PATH, config.recent_compact_disc_path[drv][i - 1]);
	}
	my_tcscpy_s(config.recent_compact_disc_path[drv][0], _MAX_PATH, path);
	emu->open_compact_disc(drv, path);
}
#endif

#ifdef USE_LASER_DISC
void open_laser_disc_dialog(HWND hWnd, int drv)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
		_T("Supported Files (*.avi;*.mpg;*.mpeg;*.mp4;*.wmv;*.ogv)\0*.avi;*.mpg;*.mpeg;*.mp4;*.wmv;*.ogv\0All Files (*.*)\0*.*\0\0"),
		_T("Laser Disc"),
		NULL,
		config.initial_laser_disc_dir, _MAX_PATH
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_laser_disc_path[drv]);
		my_tcscpy_s(config.initial_laser_disc_dir, _MAX_PATH, get_parent_dir(path));
		emu->open_laser_disc(drv, path);
	}
}

void open_recent_laser_disc(int drv, int index)
{
	_TCHAR path[_MAX_PATH];
	my_tcscpy_s(path, _MAX_PATH, config.recent_laser_disc_path[drv][index]);
	for(int i = index; i > 0; i--) {
		my_tcscpy_s(config.recent_laser_disc_path[drv][i], _MAX_PATH, config.recent_laser_disc_path[drv][i - 1]);
	}
	my_tcscpy_s(config.recent_laser_disc_path[drv][0], _MAX_PATH, path);
	emu->open_laser_disc(drv, path);
}
#endif

#ifdef USE_BINARY_FILE
void open_binary_dialog(HWND hWnd, int drv, bool load)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
		_T("Supported Files (*.ram;*.bin;*.hex)\0*.ram;*.bin;*.hex\0All Files (*.*)\0*.*\0\0"),
#if defined(_PASOPIA) || defined(_PASOPIA7)
		_T("RAM Pack Cartridge"),
#else
		_T("Memory Dump"),
#endif
		NULL,
		config.initial_binary_dir, _MAX_PATH
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_binary_path[drv]);
		my_tcscpy_s(config.initial_binary_dir, _MAX_PATH, get_parent_dir(path));
		if(load) {
			emu->load_binary(drv, path);
		} else {
			emu->save_binary(drv, path);
		}
	}
}

void open_recent_binary(int drv, int index)
{
	_TCHAR path[_MAX_PATH];
	my_tcscpy_s(path, _MAX_PATH, config.recent_binary_path[drv][index]);
	for(int i = index; i > 0; i--) {
		my_tcscpy_s(config.recent_binary_path[drv][i], _MAX_PATH, config.recent_binary_path[drv][i - 1]);
	}
	my_tcscpy_s(config.recent_binary_path[drv][0], _MAX_PATH, path);
	emu->load_binary(drv, path);
}
#endif

#ifdef USE_BUBBLE
void open_bubble_casette_dialog(HWND hWnd, int drv)
{
	_TCHAR* path = get_open_file_name(
		hWnd,
		_T("Supported Files (*.b77;*.bbl)\0*.b77;*.bbl\0All Files (*.*)\0*.*\0\0"),
		_T("Bubble Casette"),
		NULL,
		config.initial_bubble_casette_dir, _MAX_PATH
	);
	if(path) {
		UPDATE_HISTORY(path, config.recent_bubble_casette_path[drv]);
		my_tcscpy_s(config.initial_bubble_casette_dir, _MAX_PATH, get_parent_dir(path));
		emu->open_bubble_casette(drv, path, 0);
	}
}

void open_recent_bubble_casette(int drv, int index)
{
	_TCHAR path[_MAX_PATH];
	my_tcscpy_s(path, _MAX_PATH, config.recent_bubble_casette_path[drv][index]);
	for(int i = index; i > 0; i--) {
		my_tcscpy_s(config.recent_bubble_casette_path[drv][i], _MAX_PATH, config.recent_bubble_casette_path[drv][i - 1]);
	}
	my_tcscpy_s(config.recent_bubble_casette_path[drv][0], _MAX_PATH, path);
	emu->open_bubble_casette(drv, path, 0);
}
#endif

#ifdef SUPPORT_DRAG_DROP
void open_dropped_file(HDROP hDrop)
{
	if(DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0) == 1) {
		_TCHAR path[_MAX_PATH];
		DragQueryFile(hDrop, 0, path, _MAX_PATH);
		open_any_file(path);
	}
	DragFinish(hDrop);
}

void open_any_file(const _TCHAR* path)
{
#if defined(USE_CART)
	if(check_file_extension(path, _T(".rom")) || 
	   check_file_extension(path, _T(".bin")) || 
	   check_file_extension(path, _T(".hex")) || 
	   check_file_extension(path, _T(".sc" )) || 
	   check_file_extension(path, _T(".sg" )) || 
	   check_file_extension(path, _T(".gg" )) || 
	   check_file_extension(path, _T(".col")) || 
	   check_file_extension(path, _T(".sms")) || 
	   check_file_extension(path, _T(".60" )) || 
	   check_file_extension(path, _T(".pce"))) {
		UPDATE_HISTORY(path, config.recent_cart_path[0]);
		my_tcscpy_s(config.initial_cart_dir, _MAX_PATH, get_parent_dir(path));
		emu->open_cart(0, path);
		return;
	}
#endif
#if defined(USE_FLOPPY_DISK)
	if(check_file_extension(path, _T(".d88")) || 
	   check_file_extension(path, _T(".d77")) || 
	   check_file_extension(path, _T(".1dd")) || 
	   check_file_extension(path, _T(".td0")) || 
	   check_file_extension(path, _T(".imd")) || 
	   check_file_extension(path, _T(".dsk")) || 
	   check_file_extension(path, _T(".nfd")) || 
	   check_file_extension(path, _T(".fdi")) || 
	   check_file_extension(path, _T(".hdm")) || 
	   check_file_extension(path, _T(".hd5")) || 
	   check_file_extension(path, _T(".hd4")) || 
	   check_file_extension(path, _T(".hdb")) || 
	   check_file_extension(path, _T(".dd9")) || 
	   check_file_extension(path, _T(".dd6")) || 
	   check_file_extension(path, _T(".tfd")) || 
	   check_file_extension(path, _T(".xdf")) || 
	   check_file_extension(path, _T(".2d" )) || 
	   check_file_extension(path, _T(".sf7")) || 
	   check_file_extension(path, _T(".img")) || 
	   check_file_extension(path, _T(".ima")) || 
	   check_file_extension(path, _T(".vfd"))) {
		UPDATE_HISTORY(path, config.recent_floppy_disk_path[0]);
		my_tcscpy_s(config.initial_floppy_disk_dir, _MAX_PATH, get_parent_dir(path));
		open_floppy_disk(0, path, 0);
		return;
	}
#endif
#if defined(USE_HARD_DISK)
	if(check_file_extension(path, _T(".thd")) || 
	   check_file_extension(path, _T(".nhd")) || 
	   check_file_extension(path, _T(".hdi")) || 
	   check_file_extension(path, _T(".hdd"))) {
		UPDATE_HISTORY(path, config.recent_hard_disk_path[0]);
		my_tcscpy_s(config.initial_hard_disk_dir, _MAX_PATH, get_parent_dir(path));
		emu->open_hard_disk(0, path);
		return;
	}
#endif
#if defined(USE_TAPE)
	if(check_file_extension(path, _T(".wav")) || 
	   check_file_extension(path, _T(".cas")) || 
	   check_file_extension(path, _T(".p6" )) || 
	   check_file_extension(path, _T(".p6t")) || 
	   check_file_extension(path, _T(".cmt")) || 
	   check_file_extension(path, _T(".n80")) || 
	   check_file_extension(path, _T(".t88")) || 
	   check_file_extension(path, _T(".mzt")) || 
	   check_file_extension(path, _T(".mzf")) || 
	   check_file_extension(path, _T(".m12")) || 
	   check_file_extension(path, _T(".mti")) || 
	   check_file_extension(path, _T(".mtw")) || 
	   check_file_extension(path, _T(".tap")) || 
	   check_file_extension(path, _T(".t77"))) {
		UPDATE_HISTORY(path, config.recent_tape_path[0]);
		my_tcscpy_s(config.initial_tape_dir, _MAX_PATH, get_parent_dir(path));
		emu->play_tape(0, path);
		return;
	}
#endif
#if defined(USE_COMPACT_DISC)
	if(check_file_extension(path, _T(".ccd" )) || 
	   check_file_extension(path, _T(".cue" ))) {
		UPDATE_HISTORY(path, config.recent_compact_disc_path[0]);
		my_tcscpy_s(config.initial_compact_disc_dir, _MAX_PATH, get_parent_dir(path));
		emu->open_compact_disc(0, path);
		return;
	}
#endif
#if defined(USE_LASER_DISC)
	if(check_file_extension(path, _T(".avi" )) || 
	   check_file_extension(path, _T(".mpg" )) || 
	   check_file_extension(path, _T(".mpeg")) || 
	   check_file_extension(path, _T(".mp4" )) || 
	   check_file_extension(path, _T(".wmv" )) || 
	   check_file_extension(path, _T(".ogv" ))) {
		UPDATE_HISTORY(path, config.recent_laser_disc_path[0]);
		my_tcscpy_s(config.initial_laser_disc_dir, _MAX_PATH, get_parent_dir(path));
		emu->open_laser_disc(0, path);
		return;
	}
#endif
#if defined(USE_BINARY_FILE)
	if(check_file_extension(path, _T(".ram")) || 
	   check_file_extension(path, _T(".bin")) || 
	   check_file_extension(path, _T(".hex"))) {
		UPDATE_HISTORY(path, config.recent_binary_path[0]);
		my_tcscpy_s(config.initial_binary_dir, _MAX_PATH, get_parent_dir(path));
		emu->load_binary(0, path);
		return;
	}
#endif
#if defined(USE_BUBBLE)
	if(check_file_extension(path, _T(".b77")) || 
	   check_file_extension(path, _T(".bbl"))) {
		UPDATE_HISTORY(path, config.recent_bubble_casette_path[0]);
		my_tcscpy_s(config.initial_bubble_casette_dir, _MAX_PATH, get_parent_dir(path));
		emu->open_bubble_casette(0, path, 0);
		return;
	}
#endif
}
#endif

_TCHAR* get_open_file_name(HWND hWnd, const _TCHAR* filter, const _TCHAR* title, const _TCHAR* new_file, _TCHAR* dir, size_t dir_len)
{
	static _TCHAR path[_MAX_PATH];
	_TCHAR tmp[_MAX_PATH] = _T("");
	OPENFILENAME OpenFileName;
	
	if(new_file != NULL) {
		my_tcscpy_s(tmp, _MAX_PATH, new_file);
	}
	memset(&OpenFileName, 0, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof(OPENFILENAME);
	OpenFileName.hwndOwner = hWnd;
	OpenFileName.lpstrFilter = filter;
	OpenFileName.lpstrFile = tmp;
	OpenFileName.nMaxFile = _MAX_PATH;
	OpenFileName.lpstrTitle = title;
	if(dir[0]) {
		OpenFileName.lpstrInitialDir = dir;
	} else {
		_TCHAR app[_MAX_PATH];
		GetModuleFileName(NULL, app, _MAX_PATH);
		OpenFileName.lpstrInitialDir = get_parent_dir(app);
	}
	if(GetOpenFileName(&OpenFileName)) {
		get_long_full_path_name(OpenFileName.lpstrFile, path, _MAX_PATH);
		my_tcscpy_s(dir, dir_len, get_parent_dir(path));
		return path;
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// screen
// ----------------------------------------------------------------------------

void enum_screen_mode()
{
	screen_mode_count = 0;
	
	if(WINDOW_WIDTH <= 320 && WINDOW_HEIGHT <= 240) {
		// FIXME: need to check if your video card really supports 320x240 resolution
		screen_mode_width[0] = 320;
		screen_mode_height[0] = 240;
		screen_mode_count = 1;
	}
	for(int i = 0;; i++) {
		DEVMODE dev;
		ZeroMemory(&dev, sizeof(dev));
		dev.dmSize = sizeof(dev);
		if(EnumDisplaySettings(NULL, i, &dev) == 0) {
			break;
		}
		if(dev.dmPelsWidth >= WINDOW_WIDTH && dev.dmPelsHeight >= WINDOW_HEIGHT) {
			if(dev.dmPelsWidth >= 640 && dev.dmPelsHeight >= 480) {
				bool found = false;
				for(int j = 0; j < screen_mode_count; j++) {
					if(screen_mode_width[j] == dev.dmPelsWidth && screen_mode_height[j] == dev.dmPelsHeight) {
						found = true;
						break;
					}
				}
				if(!found) {
					screen_mode_width[screen_mode_count] = dev.dmPelsWidth;
					screen_mode_height[screen_mode_count] = dev.dmPelsHeight;
					if(++screen_mode_count == MAX_FULLSCREEN) {
						break;
					}
				}
			}
		}
	}
	for(int i = 0; i < screen_mode_count - 1; i++) {
		for(int j = i + 1; j < screen_mode_count; j++) {
			if(screen_mode_width[i] > screen_mode_width[j] || (screen_mode_width[i] == screen_mode_width[j] && screen_mode_height[i] > screen_mode_height[j])) {
				int width = screen_mode_width[i];
				screen_mode_width[i] = screen_mode_width[j];
				screen_mode_width[j] = width;
				int height = screen_mode_height[i];
				screen_mode_height[i] = screen_mode_height[j];
				screen_mode_height[j] = height;
			}
		}
	}
	if(screen_mode_count == 0) {
		screen_mode_width[0] = desktop_width;
		screen_mode_height[0] = desktop_height;
		screen_mode_count = 1;
	}
}

void set_window(HWND hWnd, int mode)
{
	static LONG style = WS_VISIBLE;
//	WINDOWPLACEMENT place;
//	place.length = sizeof(WINDOWPLACEMENT);
	
	if(mode >= 0 && mode < MAX_WINDOW) {
		// window
		int width = emu->get_window_mode_width(mode);
		int height = emu->get_window_mode_height(mode);
		RECT rect = {0, 0, width, height};
		AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, TRUE);
		int dest_x = (int)((desktop_width - (rect.right - rect.left)) / 2);
		int dest_y = (int)((desktop_height - (rect.bottom - rect.top)) / 2);
		//dest_x = (dest_x < 0) ? 0 : dest_x;
		dest_y = (dest_y < 0) ? 0 : dest_y;
		
		if(now_fullscreen) {
			ChangeDisplaySettings(NULL, 0);
			SetWindowLong(hWnd, GWL_STYLE, style);
			SetWindowPos(hWnd, HWND_TOP, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
			now_fullscreen = false;
			
			// show menu bar
			show_menu_bar(hWnd);
			
		} else {
			SetWindowPos(hWnd, NULL, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		}
		if(config.show_status_bar) {
			// show status bar
			show_status_bar(hWnd);
		} else {
			// hide status bar
			hide_status_bar(hWnd);
		}
		
		int status_bar_height = get_status_bar_height();
		RECT rect_tmp;
		GetClientRect(hWnd, &rect_tmp);
		if(rect_tmp.bottom != height + status_bar_height) {
			rect.bottom += height + status_bar_height - rect_tmp.bottom;
			dest_y = (int)((desktop_height - (rect.bottom - rect.top)) / 2);
			dest_y = (dest_y < 0) ? 0 : dest_y;
			SetWindowPos(hWnd, NULL, dest_x, dest_y, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		}
		config.window_mode = prev_window_mode = mode;
		
		// set screen size to emu class
		emu->set_host_window_size(width, height, true);
	} else if(!now_fullscreen) {
		// fullscreen
		int width = (mode == -1) ? desktop_width : screen_mode_width[mode - MAX_WINDOW];
		int height = (mode == -1) ? desktop_height : screen_mode_height[mode - MAX_WINDOW];
		
		DEVMODE dev;
		ZeroMemory(&dev, sizeof(dev));
		dev.dmSize = sizeof(dev);
		dev.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		dev.dmBitsPerPel = desktop_bpp;
		dev.dmPelsWidth = width;
		dev.dmPelsHeight = height;
		
		if(ChangeDisplaySettings(&dev, CDS_TEST) == DISP_CHANGE_SUCCESSFUL) {
//			GetWindowPlacement(hWnd, &place);
			ChangeDisplaySettings(&dev, CDS_FULLSCREEN);
			style = GetWindowLong(hWnd, GWL_STYLE);
			SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE);
			SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);
			SetCursorPos(width / 2, height / 2);
			now_fullscreen = true;
			if(mode == -1) {
				for(int i = 0; i < screen_mode_count; i++) {
					if(screen_mode_width[i] == desktop_width && screen_mode_height[i] == desktop_height) {
						mode = i + MAX_WINDOW;
						break;
					}
				}
			}
			config.window_mode = mode;
			
			// hide menu bar
			hide_menu_bar(hWnd);
			
			// hide status bar
			hide_status_bar(hWnd);
			
			// set screen size to emu class
			emu->set_host_window_size(width, height, false);
		}
	}
}

// ----------------------------------------------------------------------------
// input
// ----------------------------------------------------------------------------

#ifdef USE_AUTO_KEY
void start_auto_key()
{
	if(OpenClipboard(NULL)) {
		HANDLE hClip = GetClipboardData(CF_TEXT);
		if(hClip) {
			char* buf = (char*)GlobalLock(hClip);
			int size = strlen(buf);
			
			if(size > 0) {
				emu->stop_auto_key();
				emu->set_auto_key_list(buf, size);
				emu->start_auto_key();
			}
			GlobalUnlock(hClip);
		}
		CloseClipboard();
	}
}
#endif

// ----------------------------------------------------------------------------
// dialog
// ----------------------------------------------------------------------------

#ifdef USE_SOUND_VOLUME
BOOL CALLBACK VolumeWndProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) {
	case WM_CLOSE:
		EndDialog(hDlg, IDCANCEL);
		break;
	case WM_INITDIALOG:
		for(int i = 0; i < USE_SOUND_VOLUME; i++) {
			SetDlgItemText(hDlg, IDC_VOLUME_CAPTION0 + i, sound_device_caption[i]);
			SendDlgItemMessage(hDlg, IDC_VOLUME_PARAM_L0 + i, TBM_SETTICFREQ, 5, 0);
			SendDlgItemMessage(hDlg, IDC_VOLUME_PARAM_R0 + i, TBM_SETTICFREQ, 5, 0);
			SendDlgItemMessage(hDlg, IDC_VOLUME_PARAM_L0 + i, TBM_SETRANGE, TRUE, MAKELPARAM(-40, 0));
			SendDlgItemMessage(hDlg, IDC_VOLUME_PARAM_R0 + i, TBM_SETRANGE, TRUE, MAKELPARAM(-40, 0));
			SendDlgItemMessage(hDlg, IDC_VOLUME_PARAM_L0 + i, TBM_SETPOS, TRUE, max(-40, min(0, config.sound_volume_l[i])));
			SendDlgItemMessage(hDlg, IDC_VOLUME_PARAM_R0 + i, TBM_SETPOS, TRUE, max(-40, min(0, config.sound_volume_r[i])));
//			EnableWindow(GetDlgItem(hDlg, IDC_VOLUME_CAPTION0 + i), TRUE);
//			EnableWindow(GetDlgItem(hDlg, IDC_VOLUME_PARAM_L0 + i), TRUE);
//			EnableWindow(GetDlgItem(hDlg, IDC_VOLUME_PARAM_R0 + i), TRUE);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			for(int i = 0; i < USE_SOUND_VOLUME; i++) {
				config.sound_volume_l[i] = SendDlgItemMessage(hDlg, IDC_VOLUME_PARAM_L0 + i, TBM_GETPOS, 0, 0);
				config.sound_volume_r[i] = SendDlgItemMessage(hDlg, IDC_VOLUME_PARAM_R0 + i, TBM_GETPOS, 0, 0);
				emu->set_sound_device_volume(i, config.sound_volume_l[i], config.sound_volume_r[i]);
			}
			EndDialog(hDlg, IDOK);
			break;
		case IDC_VOLUME_RESET:
			for(int i = 0; i < USE_SOUND_VOLUME; i++) {
				SendDlgItemMessage(hDlg, IDC_VOLUME_PARAM_L0 + i, TBM_SETPOS, TRUE, 0);
				SendDlgItemMessage(hDlg, IDC_VOLUME_PARAM_R0 + i, TBM_SETPOS, TRUE, 0);
			}
			break;
		default:
			return FALSE;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
#endif

#ifdef USE_JOYSTICK
// from http://homepage3.nifty.com/ic/help/rmfunc/vkey.htm
static const _TCHAR *vk_names[] = {
	_T("VK_$00"),			_T("VK_LBUTTON"),		_T("VK_RBUTTON"),		_T("VK_CANCEL"),		
	_T("VK_MBUTTON"),		_T("VK_XBUTTON1"),		_T("VK_XBUTTON2"),		_T("VK_$07"),			
	_T("VK_BACK"),			_T("VK_TAB"),			_T("VK_$0A"),			_T("VK_$0B"),			
	_T("VK_CLEAR"),			_T("VK_RETURN"),		_T("VK_$0E"),			_T("VK_$0F"),			
	_T("VK_SHIFT"),			_T("VK_CONTROL"),		_T("VK_MENU"),			_T("VK_PAUSE"),			
	_T("VK_CAPITAL"),		_T("VK_KANA"),			_T("VK_$16"),			_T("VK_JUNJA"),			
	_T("VK_FINAL"),			_T("VK_KANJI"),			_T("VK_$1A"),			_T("VK_ESCAPE"),		
	_T("VK_CONVERT"),		_T("VK_NONCONVERT"),		_T("VK_ACCEPT"),		_T("VK_MODECHANGE"),		
	_T("VK_SPACE"),			_T("VK_PRIOR"),			_T("VK_NEXT"),			_T("VK_END"),			
	_T("VK_HOME"),			_T("VK_LEFT"),			_T("VK_UP"),			_T("VK_RIGHT"),			
	_T("VK_DOWN"),			_T("VK_SELECT"),		_T("VK_PRINT"),			_T("VK_EXECUTE"),		
	_T("VK_SNAPSHOT"),		_T("VK_INSERT"),		_T("VK_DELETE"),		_T("VK_HELP"),			
	_T("VK_0"),			_T("VK_1"),			_T("VK_2"),			_T("VK_3"),			
	_T("VK_4"),			_T("VK_5"),			_T("VK_6"),			_T("VK_7"),			
	_T("VK_8"),			_T("VK_9"),			_T("VK_$3A"),			_T("VK_$3B"),			
	_T("VK_$3C"),			_T("VK_$3D"),			_T("VK_$3E"),			_T("VK_$3F"),			
	_T("VK_$40"),			_T("VK_A"),			_T("VK_B"),			_T("VK_C"),			
	_T("VK_D"),			_T("VK_E"),			_T("VK_F"),			_T("VK_G"),			
	_T("VK_H"),			_T("VK_I"),			_T("VK_J"),			_T("VK_K"),			
	_T("VK_L"),			_T("VK_M"),			_T("VK_N"),			_T("VK_O"),			
	_T("VK_P"),			_T("VK_Q"),			_T("VK_R"),			_T("VK_S"),			
	_T("VK_T"),			_T("VK_U"),			_T("VK_V"),			_T("VK_W"),			
	_T("VK_X"),			_T("VK_Y"),			_T("VK_Z"),			_T("VK_LWIN"),			
	_T("VK_RWIN"),			_T("VK_APPS"),			_T("VK_$5E"),			_T("VK_SLEEP"),			
	_T("VK_NUMPAD0"),		_T("VK_NUMPAD1"),		_T("VK_NUMPAD2"),		_T("VK_NUMPAD3"),		
	_T("VK_NUMPAD4"),		_T("VK_NUMPAD5"),		_T("VK_NUMPAD6"),		_T("VK_NUMPAD7"),		
	_T("VK_NUMPAD8"),		_T("VK_NUMPAD9"),		_T("VK_MULTIPLY"),		_T("VK_ADD"),			
	_T("VK_SEPARATOR"),		_T("VK_SUBTRACT"),		_T("VK_DECIMAL"),		_T("VK_DIVIDE"),		
	_T("VK_F1"),			_T("VK_F2"),			_T("VK_F3"),			_T("VK_F4"),			
	_T("VK_F5"),			_T("VK_F6"),			_T("VK_F7"),			_T("VK_F8"),			
	_T("VK_F9"),			_T("VK_F10"),			_T("VK_F11"),			_T("VK_F12"),			
	_T("VK_F13"),			_T("VK_F14"),			_T("VK_F15"),			_T("VK_F16"),			
	_T("VK_F17"),			_T("VK_F18"),			_T("VK_F19"),			_T("VK_F20"),			
	_T("VK_F21"),			_T("VK_F22"),			_T("VK_F23"),			_T("VK_F24"),			
	_T("VK_$88"),			_T("VK_$89"),			_T("VK_$8A"),			_T("VK_$8B"),			
	_T("VK_$8C"),			_T("VK_$8D"),			_T("VK_$8E"),			_T("VK_$8F"),			
	_T("VK_NUMLOCK"),		_T("VK_SCROLL"),		_T("VK_$92"),			_T("VK_$93"),			
	_T("VK_$94"),			_T("VK_$95"),			_T("VK_$96"),			_T("VK_$97"),			
	_T("VK_$98"),			_T("VK_$99"),			_T("VK_$9A"),			_T("VK_$9B"),			
	_T("VK_$9C"),			_T("VK_$9D"),			_T("VK_$9E"),			_T("VK_$9F"),			
	_T("VK_LSHIFT"),		_T("VK_RSHIFT"),		_T("VK_LCONTROL"),		_T("VK_RCONTROL"),		
	_T("VK_LMENU"),			_T("VK_RMENU"),			_T("VK_BROWSER_BACK"),		_T("VK_BROWSER_FORWARD"),	
	_T("VK_BROWSER_REFRESH"),	_T("VK_BROWSER_STOP"),		_T("VK_BROWSER_SEARCH"),	_T("VK_BROWSER_FAVORITES"),	
	_T("VK_BROWSER_HOME"),		_T("VK_VOLUME_MUTE"),		_T("VK_VOLUME_DOWN"),		_T("VK_VOLUME_UP"),		
	_T("VK_MEDIA_NEXT_TRACK"),	_T("VK_MEDIA_PREV_TRACK"),	_T("VK_MEDIA_STOP"),		_T("VK_MEDIA_PLAY_PAUSE"),	
	_T("VK_LAUNCH_MAIL"),		_T("VK_LAUNCH_MEDIA_SELECT"),	_T("VK_LAUNCH_APP1"),		_T("VK_LAUNCH_APP2"),		
	_T("VK_$B8"),			_T("VK_$B9"),			_T("VK_OEM_1"),			_T("VK_OEM_PLUS"),		
	_T("VK_OEM_COMMA"),		_T("VK_OEM_MINUS"),		_T("VK_OEM_PERIOD"),		_T("VK_OEM_2"),			
	_T("VK_OEM_3"),			_T("VK_$C1"),			_T("VK_$C2"),			_T("VK_$C3"),			
	_T("VK_$C4"),			_T("VK_$C5"),			_T("VK_$C6"),			_T("VK_$C7"),			
	_T("VK_$C8"),			_T("VK_$C9"),			_T("VK_$CA"),			_T("VK_$CB"),			
	_T("VK_$CC"),			_T("VK_$CD"),			_T("VK_$CE"),			_T("VK_$CF"),			
	_T("VK_$D0"),			_T("VK_$D1"),			_T("VK_$D2"),			_T("VK_$D3"),			
	_T("VK_$D4"),			_T("VK_$D5"),			_T("VK_$D6"),			_T("VK_$D7"),			
	_T("VK_$D8"),			_T("VK_$D9"),			_T("VK_$DA"),			_T("VK_OEM_4"),			
	_T("VK_OEM_5"),			_T("VK_OEM_6"),			_T("VK_OEM_7"),			_T("VK_OEM_8"),			
	_T("VK_$E0"),			_T("VK_OEM_AX"),		_T("VK_OEM_102"),		_T("VK_ICO_HELP"),		
	_T("VK_ICO_00"),		_T("VK_PROCESSKEY"),		_T("VK_ICO_CLEAR"),		_T("VK_PACKET"),		
	_T("VK_$E8"),			_T("VK_OEM_RESET"),		_T("VK_OEM_JUMP"),		_T("VK_OEM_PA1"),		
	_T("VK_OEM_PA2"),		_T("VK_OEM_PA3"),		_T("VK_OEM_WSCTRL"),		_T("VK_OEM_CUSEL"),		
	_T("VK_OEM_ATTN"),		_T("VK_OEM_FINISH"),		_T("VK_OEM_COPY"),		_T("VK_OEM_AUTO"),		
	_T("VK_OEM_ENLW"),		_T("VK_OEM_BACKTAB"),		_T("VK_ATTN"),			_T("VK_CRSEL"),			
	_T("VK_EXSEL"),			_T("VK_EREOF"),			_T("VK_PLAY"),			_T("VK_ZOOM"),			
	_T("VK_NONAME"),		_T("VK_PA1"),			_T("VK_OEM_CLEAR"),		_T("VK_$FF"),			
};

static const _TCHAR *joy_button_names[32] = {
	_T("Up"),
	_T("Down"),
	_T("Left"),
	_T("Right"),
	_T("Button #1"),
	_T("Button #2"),
	_T("Button #3"),
	_T("Button #4"),
	_T("Button #5"),
	_T("Button #6"),
	_T("Button #7"),
	_T("Button #8"),
	_T("Button #9"),
	_T("Button #10"),
	_T("Button #11"),
	_T("Button #12"),
	_T("Button #13"),
	_T("Button #14"),
	_T("Button #15"),
	_T("Button #16"),
	_T("Z-Axis Low"),
	_T("Z-Axis High"),
	_T("R-Axis Low"),
	_T("R-Axis High"),
	_T("U-Axis Low"),
	_T("U-Axis High"),
	_T("V-Axis Low"),
	_T("V-Axis High"),
	_T("POV 0deg"),
	_T("POV 90deg"),
	_T("POV 180deg"),
	_T("POV 270deg"),
};

HWND hJoyDlg;
HWND hJoyEdit[16];
WNDPROC JoyOldProc[16];
int joy_stick_index;
int joy_button_index;
int joy_button_params[16];
uint32_t joy_status[4];

#define get_joy_range(min_value, max_value, lo_value, hi_value) \
{ \
	uint64_t center = ((uint64_t)min_value + (uint64_t)max_value) / 2; \
	lo_value = (DWORD)((center + (uint64_t)min_value) / 2); \
	hi_value = (DWORD)((center + (uint64_t)max_value) / 2); \
}

uint32_t get_joy_status(int index)
{
	JOYCAPS joycaps;
	JOYINFOEX joyinfo;
	uint32_t status = 0;
	
	if(joyGetDevCaps(index, &joycaps, sizeof(JOYCAPS)) == JOYERR_NOERROR) {
		joyinfo.dwSize = sizeof(JOYINFOEX);
		joyinfo.dwFlags = JOY_RETURNALL;
		if(joyGetPosEx(index, &joyinfo) == JOYERR_NOERROR) {
			if(joycaps.wNumAxes >= 2) {
				DWORD dwYposLo, dwYposHi;
				get_joy_range(joycaps.wYmin, joycaps.wYmax, dwYposLo, dwYposHi);
				if(joyinfo.dwYpos < dwYposLo) status |= 0x00000001;	// up
				if(joyinfo.dwYpos > dwYposHi) status |= 0x00000002;	// down
			}
			if(joycaps.wNumAxes >= 1) {
				DWORD dwXposLo, dwXposHi;
				get_joy_range(joycaps.wXmin, joycaps.wXmax, dwXposLo, dwXposHi);
				if(joyinfo.dwXpos < dwXposLo) status |= 0x00000004;	// left
				if(joyinfo.dwXpos > dwXposHi) status |= 0x00000008;	// right
			}
			if(joycaps.wNumAxes >= 3) {
				DWORD dwZposLo, dwZposHi;
				get_joy_range(joycaps.wZmin, joycaps.wZmax, dwZposLo, dwZposHi);
				if(joyinfo.dwZpos < dwZposLo) status |= 0x00100000;
				if(joyinfo.dwZpos > dwZposHi) status |= 0x00200000;
			}
			if(joycaps.wNumAxes >= 4) {
				DWORD dwRposLo, dwRposHi;
				get_joy_range(joycaps.wRmin, joycaps.wRmax, dwRposLo, dwRposHi);
				if(joyinfo.dwRpos < dwRposLo) status |= 0x00400000;
				if(joyinfo.dwRpos > dwRposHi) status |= 0x00800000;
			}
			if(joycaps.wNumAxes >= 5) {
				DWORD dwUposLo, dwUposHi;
				get_joy_range(joycaps.wUmin, joycaps.wUmax, dwUposLo, dwUposHi);
				if(joyinfo.dwUpos < dwUposLo) status |= 0x01000000;
				if(joyinfo.dwUpos > dwUposHi) status |= 0x02000000;
			}
			if(joycaps.wNumAxes >= 6) {
				DWORD dwVposLo, dwVposHi;
				get_joy_range(joycaps.wVmin, joycaps.wVmax, dwVposLo, dwVposHi);
				if(joyinfo.dwVpos < dwVposLo) status |= 0x04000000;
				if(joyinfo.dwVpos > dwVposHi) status |= 0x08000000;
			}
			if(joyinfo.dwPOV != 0xffff) {
				static const uint32_t dir[8] = {
					0x10000000 + 0x00000000,
					0x10000000 + 0x20000000,
					0x00000000 + 0x20000000,
					0x40000000 + 0x20000000,
					0x40000000 + 0x00000000,
					0x40000000 + 0x80000000,
					0x00000000 + 0x80000000,
					0x10000000 + 0x80000000,
				};
				for(int i = 0; i < 9; i++) {
					if(joyinfo.dwPOV < (DWORD)(2250 + 4500 * i)) {
						status |= dir[i & 7];
						break;
					}
				}
			}
			DWORD dwButtonsMask = (1 << min(16, joycaps.wNumButtons)) - 1;
			status |= ((joyinfo.dwButtons & dwButtonsMask) << 4);
		}
	}
	return status;
}

void set_joy_button_text(int index)
{
	if(joy_button_params[index] < 0) {
		SetDlgItemText(hJoyDlg, IDC_JOYSTICK_PARAM0 + index, vk_names[-joy_button_params[index]]);
	} else if(joy_stick_index == -1) {
			SetDlgItemText(hJoyDlg, IDC_JOYSTICK_PARAM0 + index, _T("(None)"));
	} else {
		SetDlgItemText(hJoyDlg, IDC_JOYSTICK_PARAM0 + index, create_string(_T("Joystick #%d - %s"), (joy_button_params[index] >> 5) + 1, joy_button_names[joy_button_params[index] & 0x1f]));
	}
}

LRESULT CALLBACK JoySubProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	int index = -1;
	for(int i = 0; i < 16; i++) {
		if(hWnd == hJoyEdit[i]) {
			index = i;
			break;
		}
	}
	if(index == -1) {
		return 0L;
	}
	switch(iMsg) {
	case WM_CHAR:
		return 0L;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if(joy_stick_index == -1 && LOBYTE(wParam) == VK_BACK) {
			joy_button_params[index] = 0;
		} else {
			joy_button_params[index] = -(int)LOBYTE(wParam);
		}
		set_joy_button_text(index);
		if(hJoyEdit[++index] == NULL) {
			index = 0;
		}
		SetFocus(hJoyEdit[index]);
		return 0L;
	case WM_SETFOCUS:
		joy_button_index = index;
		break;
	default:
		break;
	}
	return CallWindowProc(JoyOldProc[index], hWnd, iMsg, wParam, lParam);
}

BOOL CALLBACK JoyWndProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) {
	case WM_CLOSE:
		EndDialog(hDlg, IDCANCEL);
		break;
	case WM_INITDIALOG:
		hJoyDlg = hDlg;
		joy_stick_index = (int)(*(LONG*)lParam);
#if !USE_RETROACHIEVEMENTS
		SetWindowText(hDlg, create_string(_T("Joystick #%d"), joy_stick_index + 1));
#endif
		for(int i = 0; i < 16; i++) {
			joy_button_params[i] = config.joy_buttons[joy_stick_index][i];
			if((hJoyEdit[i] = GetDlgItem(hDlg, IDC_JOYSTICK_PARAM0 + i)) != NULL) {
#ifdef USE_JOY_BUTTON_CAPTIONS
				if(i < array_length(joy_button_captions)) {
					SetDlgItemText(hDlg, IDC_JOYSTICK_CAPTION0 + i, joy_button_captions[i]);
				} else
#endif
				SetDlgItemText(hDlg, IDC_JOYSTICK_CAPTION0 + i, joy_button_names[i]);
				set_joy_button_text(i);
				JoyOldProc[i] = (WNDPROC)GetWindowLong(hJoyEdit[i], GWL_WNDPROC);
				SetWindowLong(hJoyEdit[i], GWL_WNDPROC, (LONG)JoySubProc);
			}
		}
		memset(joy_status, 0, sizeof(joy_status));
		SetTimer(hDlg, 1, 100, NULL);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			for(int i = 0; i < 16; i++) {
				config.joy_buttons[joy_stick_index][i] = joy_button_params[i];
			}
			EndDialog(hDlg, IDOK);
			break;
		case IDC_JOYSTICK_RESET:
			for(int i = 0; i < 16; i++) {
				joy_button_params[i] = (joy_stick_index << 5) | i;
				set_joy_button_text(i);
			}
			break;
		default:
			return FALSE;
		}
		break;
	case WM_TIMER:
		for(int i = 0; i < 4; i++) {
			uint32_t status = get_joy_status(i);
			for(int j = 0; j < 32; j++) {
				uint32_t bit = 1 << j;
				if((joy_status[i] & bit) && !(status & bit)) {
					joy_button_params[joy_button_index] = (i << 5) | j;
					set_joy_button_text(joy_button_index);
					if(hJoyEdit[++joy_button_index] == NULL) {
						joy_button_index = 0;
					}
					SetFocus(hJoyEdit[joy_button_index]);
					break;
				}
			}
			joy_status[i] = status;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK JoyToKeyWndProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) {
	case WM_CLOSE:
		EndDialog(hDlg, IDCANCEL);
		break;
	case WM_INITDIALOG:
		hJoyDlg = hDlg;
		joy_stick_index = -1;//(int)(*(LONG*)lParam);
//		SetWindowText(hDlg, create_string(_T("Joystick To Keyboard #%d"), joy_stick_index + 1));
		SendMessage(GetDlgItem(hDlg, IDC_JOYTOKEY_CHECK0), BM_SETCHECK, (WPARAM)config.use_joy_to_key, 0L);
		SendMessage(GetDlgItem(hDlg, IDC_JOYTOKEY_RADIO0), BM_SETCHECK, (WPARAM)(config.joy_to_key_type == 0), 0L);
		SendMessage(GetDlgItem(hDlg, IDC_JOYTOKEY_RADIO1), BM_SETCHECK, (WPARAM)(config.joy_to_key_type == 1), 0L);
		SendMessage(GetDlgItem(hDlg, IDC_JOYTOKEY_RADIO2), BM_SETCHECK, (WPARAM)(config.joy_to_key_type == 2), 0L);
		SendMessage(GetDlgItem(hDlg, IDC_JOYTOKEY_CHECK1), BM_SETCHECK, (WPARAM)config.joy_to_key_numpad5, 0L);
		for(int i = 0; i < 16; i++) {
			joy_button_params[i] = config.joy_to_key_buttons[i];
			if((hJoyEdit[i] = GetDlgItem(hDlg, IDC_JOYSTICK_PARAM0 + i)) != NULL) {
				set_joy_button_text(i);
				JoyOldProc[i] = (WNDPROC)GetWindowLong(hJoyEdit[i], GWL_WNDPROC);
				SetWindowLong(hJoyEdit[i], GWL_WNDPROC, (LONG)JoySubProc);
			}
		}
		memset(joy_status, 0, sizeof(joy_status));
		SetTimer(hDlg, 1, 100, NULL);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			config.use_joy_to_key = (IsDlgButtonChecked(hDlg, IDC_JOYTOKEY_CHECK0) == BST_CHECKED);
			if(IsDlgButtonChecked(hDlg, IDC_JOYTOKEY_RADIO0) == BST_CHECKED) {
				config.joy_to_key_type = 0;
			} else if(IsDlgButtonChecked(hDlg, IDC_JOYTOKEY_RADIO1) == BST_CHECKED) {
				config.joy_to_key_type = 1;
			} else {
				config.joy_to_key_type = 2;
			}
			config.joy_to_key_numpad5 = (IsDlgButtonChecked(hDlg, IDC_JOYTOKEY_CHECK1) == BST_CHECKED);
			for(int i = 0; i < 16; i++) {
				config.joy_to_key_buttons[i] = joy_button_params[i];
			}
			EndDialog(hDlg, IDOK);
			break;
		case IDC_JOYSTICK_RESET:
			SendMessage(GetDlgItem(hDlg, IDC_JOYTOKEY_CHECK0), BM_SETCHECK, (WPARAM)false, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_JOYTOKEY_RADIO0), BM_SETCHECK, (WPARAM)false, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_JOYTOKEY_RADIO1), BM_SETCHECK, (WPARAM)false, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_JOYTOKEY_RADIO2), BM_SETCHECK, (WPARAM)true,  0L);
			SendMessage(GetDlgItem(hDlg, IDC_JOYTOKEY_CHECK1), BM_SETCHECK, (WPARAM)false, 0L);
			for(int i = 0; i < 16; i++) {
				joy_button_params[i] = (i == 0) ? -('Z') : (i == 1) ? -('X') : 0;
				set_joy_button_text(i);
			}
			break;
		default:
			return FALSE;
		}
		break;
	case WM_TIMER:
		for(int i = 0; i < 1; i++) {
			uint32_t status = get_joy_status(i);
			for(int j = 0; j < 16; j++) {
				uint32_t bit = 1 << (j + 4);
				if((joy_status[i] & bit) && !(status & bit)) {
					if(hJoyEdit[j] != NULL) {
						joy_button_index = j;
						SetFocus(hJoyEdit[joy_button_index]);
					}
					break;
				}
			}
			joy_status[i] = status;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
#endif

// ----------------------------------------------------------------------------
// button
// ----------------------------------------------------------------------------

#ifdef ONE_BOARD_MICRO_COMPUTER
#define MAX_FONT_SIZE 32
HWND hButton[MAX_BUTTONS];
WNDPROC ButtonOldProc[MAX_BUTTONS];

LRESULT CALLBACK ButtonSubProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	for(int i = 0; i < MAX_BUTTONS; i++) {
		if(hWnd == hButton[i]) {
			switch(iMsg) {
			case WM_KEYDOWN:
				if(emu) {
					bool extended = ((HIWORD(lParam) & 0x100) != 0);
					bool repeat = ((HIWORD(lParam) & 0x4000) != 0);
					emu->key_down(LOBYTE(wParam), extended, repeat);
				}
				break;
			case WM_SYSKEYDOWN:
				if(emu) {
					bool extended = ((HIWORD(lParam) & 0x100) != 0);
					bool repeat = ((HIWORD(lParam) & 0x4000) != 0);
					emu->key_down(LOBYTE(wParam), extended, repeat);
				}
				return 0;	// not activate menu when hit ALT/F10
			case WM_KEYUP:
				if(emu) {
					bool extended = ((HIWORD(lParam) & 0x100) != 0);
					emu->key_up(LOBYTE(wParam), extended);
				}
				break;
			case WM_SYSKEYUP:
				if(emu) {
					bool extended = ((HIWORD(lParam) & 0x100) != 0);
					emu->key_up(LOBYTE(wParam), extended);
				}
				return 0;	// not activate menu when hit ALT/F10
			case WM_CHAR:
				if(emu) {
					emu->key_char(LOBYTE(wParam));
				}
				break;
			}
			return CallWindowProc(ButtonOldProc[i], hWnd, iMsg, wParam, lParam);
		}
	}
	return 0;
}

void create_buttons(HWND hWnd)
{
	for(int i = 0; i < MAX_BUTTONS; i++) {
		hButton[i] = CreateWindow(_T("BUTTON"), NULL/*vm_buttons[i].caption*/,
		                          WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW/*(_tcsstr(vm_buttons[i].caption, _T("\n")) ? BS_MULTILINE : 0)*/,
		                          vm_buttons[i].x, vm_buttons[i].y,
		                          vm_buttons[i].width, vm_buttons[i].height,
		                          hWnd, (HMENU)(ID_BUTTON + i), (HINSTANCE)GetModuleHandle(0), NULL);
		ButtonOldProc[i] = (WNDPROC)(LONG_PTR)GetWindowLong(hButton[i], GWL_WNDPROC);
		SetWindowLong(hButton[i], GWL_WNDPROC, (LONG)(LONG_PTR)ButtonSubProc);
	}
}

void draw_button(HDC hDC, UINT index, UINT pressed)
{
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);
	_TCHAR name[24];
	
	_stprintf(name, _T("IDI_BITMAP_BUTTON%02d"), index);
#if 1
	// load png from resource
	HRSRC hResource = FindResource(hInstance, name, _T("IMAGE"));
	if(hResource != NULL) {
		const void* pResourceData = LockResource(LoadResource(hInstance, hResource));
		if(pResourceData != NULL) {
			DWORD dwResourceSize = SizeofResource(hInstance, hResource);
			HGLOBAL hResourceBuffer = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);
			if(hResourceBuffer != NULL) {
				void* pResourceBuffer = GlobalLock(hResourceBuffer);
				if(pResourceBuffer != NULL) {
					CopyMemory(pResourceBuffer, pResourceData, dwResourceSize);
					IStream* pIStream = NULL;
					if(CreateStreamOnHGlobal(hResourceBuffer, FALSE, &pIStream) == S_OK) {
						Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromStream(pIStream);
						if(pBitmap != NULL) {
							Gdiplus::Graphics graphics(hDC);
							if(pressed) {
								graphics.DrawImage(pBitmap, 2,  2, pBitmap->GetWidth() - 2, pBitmap->GetHeight() - 2);
							} else {
								graphics.DrawImage(pBitmap, 1, 1);
							}
							delete pBitmap;
						}
					}
				}
				GlobalUnlock(hResourceBuffer);
			}
			GlobalFree(hResourceBuffer);
		}
	}
#else
	// load bitmap from resource
	HDC hmdc = CreateCompatibleDC(hDC);
	HBITMAP hBitmap = LoadBitmap(hInstance, name);
	BITMAP bmp;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);
	int w = (int)bmp.bmWidth;
	int h = (int)bmp.bmHeight;
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hmdc, hBitmap);
	if(pressed) {
		StretchBlt(hDC, 2, 2, w - 2, h - 2, hmdc, 0, 0, w, h, SRCCOPY);
	} else {
		BitBlt(hDC, 1, 1, w, h, hmdc, 0, 0, SRCCOPY);
	}
	SelectObject(hmdc, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hmdc);
#endif
}
#endif

