/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	[ win32 dependent ]
*/

#include "osd.h"

void OSD::initialize(int rate, int samples)
{
	// check os version
	OSVERSIONINFO os_info;
	os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&os_info);
	vista_or_later = (os_info.dwPlatformId == 2 && (os_info.dwMajorVersion > 6 || (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion >= 0)));
	
	GdiplusStartup(&gdiToken, &gdiSI, NULL);
	initialize_input();
	initialize_screen();
	initialize_sound(rate, samples);
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	CoInitialize(NULL);
	initialize_video();
#endif
#ifdef USE_SOCKET
	initialize_socket();
#endif
}

void OSD::release()
{
	release_input();
	release_screen();
	release_sound();
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	release_video();
	CoUninitialize();
#endif
#ifdef USE_SOCKET
	release_socket();
#endif
	GdiplusShutdown(gdiToken);
}

void OSD::power_off()
{
	PostMessage(main_window_handle, WM_CLOSE, 0, 0L);
}

void OSD::suspend()
{
#ifdef USE_MOVIE_PLAYER
	if(now_movie_play && !now_movie_pause) {
		pause_movie();
		now_movie_pause = false;
	}
#endif
	mute_sound();
}

void OSD::restore()
{
#ifdef USE_MOVIE_PLAYER
	if(now_movie_play && !now_movie_pause) {
		play_movie();
	}
#endif
}

void OSD::lock_vm()
{
	lock_count++;
}

void OSD::unlock_vm()
{
	if(--lock_count <= 0) {
		force_unlock_vm();
	}
}

void OSD::force_unlock_vm()
{
	lock_count = 0;
}

void OSD::sleep(uint32_t ms)
{
	Sleep(ms);
}

#ifdef USE_DEBUGGER
FARPROC hWndProc = NULL;
OSD *my_osd = NULL;

LRESULT CALLBACK MyWndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch(iMsg) {
	case WM_CLOSE:
		return 0;
	case WM_PAINT:
		if(my_osd) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
#ifdef ONE_BOARD_MICRO_COMPUTER
			my_osd->reload_bitmap();
#endif
			my_osd->update_screen(hdc);
			EndPaint(hWnd, &ps);
		}
		return 0;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

void OSD::start_waiting_in_debugger()
{
	HMENU hMenu = GetMenu(main_window_handle);
	
	if(hMenu != NULL) {
		for(int i = 0;; i++) {
			if(EnableMenuItem(hMenu, i, MF_BYPOSITION | MF_GRAYED) == -1) {
				break;
			}
		}
	}
	hWndProc = (FARPROC)GetWindowLong(main_window_handle, GWL_WNDPROC);
	SetWindowLong(main_window_handle, GWL_WNDPROC, (LONG)MyWndProc);
	my_osd = this;
}

void OSD::finish_waiting_in_debugger()
{
	HMENU hMenu = GetMenu(main_window_handle);
	
	if(hMenu != NULL) {
		for(int i = 0;; i++) {
			if(EnableMenuItem(hMenu, i, MF_BYPOSITION | MF_ENABLED) == -1) {
				break;
			}
		}
	}
	SetWindowLong(main_window_handle, GWL_WNDPROC, (LONG)hWndProc);
	my_osd = NULL;
}

void OSD::process_waiting_in_debugger()
{
	MSG msg;
	
	while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		if(GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}
#endif
