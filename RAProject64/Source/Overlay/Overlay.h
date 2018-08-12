#ifndef RA_DDRAW_H
#define RA_DDRAW_H

#ifndef DIRECTDRAW_VERSION
#define DIRECTDRAW_VERSION         0x0500
#endif

#include <ddraw.h>

LRESULT CALLBACK OverlayWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
void CreateOverlay( HWND hwnd, HWND status );
void UpdateOverlay( HDC hdc, RECT rect );
void InitOverlayRender( HWND hwnd, HWND status );
void RenderAchievementsOverlay();




#endif