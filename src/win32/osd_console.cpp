/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 console ]
*/

#include "osd.h"
#include "../res/resource.h"

BOOL WINAPI ctrl_c_handler(DWORD type)
{
	return TRUE;
}

void OSD::open_console(const _TCHAR* title)
{
	AllocConsole();
	SetConsoleTitle(title);
	SetConsoleCtrlHandler(ctrl_c_handler, TRUE);
	RemoveMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);
	
	hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	
	COORD coord;
	coord.X = 80;
	coord.Y = 4000;
	
	SetConsoleScreenBufferSize(hStdOut, coord);
	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
}

void OSD::close_console()
{
	SetConsoleCtrlHandler(ctrl_c_handler, FALSE);
	FreeConsole();
}

unsigned int OSD::get_console_code_page()
{
	return GetConsoleCP();
}

bool OSD::is_console_active()
{
	HWND hWnd = GetForegroundWindow();
	return (hWnd != NULL && hWnd == FindWindow(_T("ConsoleWindowClass"), NULL));
}

void OSD::set_console_text_attribute(unsigned short attr)
{
	unsigned short new_attr = 0;
	
	if(attr & OSD_CONSOLE_BLUE     ) new_attr |= FOREGROUND_BLUE;
	if(attr & OSD_CONSOLE_GREEN    ) new_attr |= FOREGROUND_GREEN;
	if(attr & OSD_CONSOLE_RED      ) new_attr |= FOREGROUND_RED;
	if(attr & OSD_CONSOLE_INTENSITY) new_attr |= FOREGROUND_INTENSITY;
	
	SetConsoleTextAttribute(hStdOut, attr);
}

void OSD::write_console(const _TCHAR* buffer, unsigned int length)
{
	DWORD dwWritten;
	WriteConsole(hStdOut, buffer, length, &dwWritten, NULL);
}

int OSD::read_console_input(_TCHAR* buffer, unsigned int length)
{
	INPUT_RECORD ir[16];
	DWORD dwRead;
	unsigned int count = 0;
	
	if(ReadConsoleInput(hStdIn, ir, min(16, length), &dwRead)) {
		for(unsigned int i = 0; i < dwRead; i++) {
			if((ir[i].EventType & KEY_EVENT) && ir[i].Event.KeyEvent.bKeyDown) {
#ifdef _UNICODE
				if(ir[i].Event.KeyEvent.uChar.UnicodeChar) {
					if(count < length) {
						buffer[count++] = ir[i].Event.KeyEvent.uChar.UnicodeChar;
					}
#else
				if(ir[i].Event.KeyEvent.uChar.AsciiChar) {
					if(count < length) {
						buffer[count++] = ir[i].Event.KeyEvent.uChar.AsciiChar;
					}
#endif
				} else if(ir[i].Event.KeyEvent.wVirtualKeyCode >= 0x25 && ir[i].Event.KeyEvent.wVirtualKeyCode <= 0x28) {
					static const _TCHAR cursor[] = {_T('D'), _T('A'), _T('C'), _T('B')}; // left, up, right, down
					if(count + 2 < length) {
						buffer[count++] = 0x1b;
						buffer[count++] = 0x5b;
						buffer[count++] = cursor[ir[i].Event.KeyEvent.wVirtualKeyCode - 0x25];
					}
				}
			}
		}
	}
	return count;
}

bool OSD::is_console_key_pressed(int vk)
{
	return ((GetAsyncKeyState(vk) & 0x8000) != 0);
}

void OSD::close_debugger_console()
{
	PostMessage(main_window_handle, WM_COMMAND, ID_CLOSE_DEBUGGER, 0L);
}

