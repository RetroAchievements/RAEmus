/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	[ win32 screen ]
*/

#include "osd.h"

#if USE_RETROACHIEVEMENTS
#include "../retroachievements.h"
#endif

#define REC_VIDEO_SUCCESS	1
#define REC_VIDEO_FULL		2
#define REC_VIDEO_ERROR		3

void OSD::initialize_screen()
{
	host_window_width = WINDOW_WIDTH;
	host_window_height = WINDOW_HEIGHT;
	host_window_mode = true;
	
	vm_screen_width = SCREEN_WIDTH;
	vm_screen_height = SCREEN_HEIGHT;
	vm_window_width = WINDOW_WIDTH;
	vm_window_height = WINDOW_HEIGHT;
	vm_window_width_aspect = WINDOW_WIDTH_ASPECT;
	vm_window_height_aspect = WINDOW_HEIGHT_ASPECT;
	
	memset(&vm_screen_buffer, 0, sizeof(bitmap_t));
#ifdef USE_SCREEN_FILTER
	memset(&filtered_screen_buffer, 0, sizeof(bitmap_t));
	memset(&tmp_filtered_screen_buffer, 0, sizeof(bitmap_t));
#endif
//#ifdef USE_SCREEN_ROTATE
	memset(&rotated_screen_buffer, 0, sizeof(bitmap_t));
//#endif
	memset(&stretched_screen_buffer, 0, sizeof(bitmap_t));
	memset(&shrinked_screen_buffer, 0, sizeof(bitmap_t));
	memset(&video_screen_buffer, 0, sizeof(bitmap_t));
	
	lpd3d9 = NULL;
	lpd3d9Device = NULL;
	lpd3d9Surface = NULL;
	lpd3d9OffscreenSurface = NULL;
	
	now_record_video = false;
	pAVIStream = NULL;
	pAVICompressed = NULL;
	pAVIFile = NULL;
	
	first_draw_screen = false;
	first_invalidate = true;
	self_invalidate = false;
}

void OSD::release_screen()
{
	stop_record_video();
	
	release_d3d9();
	release_screen_buffer(&vm_screen_buffer);
#ifdef USE_SCREEN_FILTER
	release_screen_buffer(&filtered_screen_buffer);
	release_screen_buffer(&tmp_filtered_screen_buffer);
#endif
//#ifdef USE_SCREEN_ROTATE
	release_screen_buffer(&rotated_screen_buffer);
//#endif
	release_screen_buffer(&stretched_screen_buffer);
	release_screen_buffer(&shrinked_screen_buffer);
	release_screen_buffer(&video_screen_buffer);
}

double OSD::get_window_mode_power(int mode)
{
	if(mode + WINDOW_MODE_BASE == 2) {
		return 1.5;
	} else if(mode + WINDOW_MODE_BASE > 2) {
		return mode + WINDOW_MODE_BASE - 1;
	}
	return mode + WINDOW_MODE_BASE;
}

int OSD::get_window_mode_width(int mode)
{
//#ifdef USE_SCREEN_ROTATE
	if(config.rotate_type == 1 || config.rotate_type == 3) {
		return (int)((config.window_stretch_type == 0 ? vm_window_height : vm_window_height_aspect) * get_window_mode_power(mode));
	}
//#endif
	return (int)((config.window_stretch_type == 0 ? vm_window_width : vm_window_width_aspect) * get_window_mode_power(mode));
}

int OSD::get_window_mode_height(int mode)
{
//#ifdef USE_SCREEN_ROTATE
	if(config.rotate_type == 1 || config.rotate_type == 3) {
		return (int)((config.window_stretch_type == 0 ? vm_window_width : vm_window_width_aspect) * get_window_mode_power(mode));
	}
//#endif
	return (int)((config.window_stretch_type == 0 ? vm_window_height : vm_window_height_aspect) * get_window_mode_power(mode));
}

void OSD::set_host_window_size(int window_width, int window_height, bool window_mode)
{
	if(window_width != -1) {
		host_window_width = window_width;
	}
	if(window_height != -1) {
		host_window_height = window_height;
	}
	host_window_mode = window_mode;
	
	first_draw_screen = false;
	first_invalidate = true;
}

void OSD::set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect)
{
	if(vm_screen_width != screen_width || vm_screen_height != screen_height) {
		if(window_width == -1) {
			window_width = screen_width;
		}
		if(window_height == -1) {
			window_height = screen_height;
		}
		if(window_width_aspect == -1) {
			window_width_aspect = window_width;
		}
		if(window_height_aspect == -1) {
			window_height_aspect = window_height;
		}
		vm_screen_width = screen_width;
		vm_screen_height = screen_height;
		
		if(vm_window_width != window_width || vm_window_height != window_height || vm_window_width_aspect != window_width_aspect || vm_window_height_aspect != window_height_aspect) {
			vm_window_width = window_width;
			vm_window_height = window_height;
			vm_window_width_aspect = window_width_aspect;
			vm_window_height_aspect = window_height_aspect;
			
			// change the window size
			PostMessage(main_window_handle, WM_RESIZE, 0L, 0L);
		} else {
			// to make sure
			set_host_window_size(-1, -1, host_window_mode);
		}
	}
	if(vm_screen_buffer.width != vm_screen_width || vm_screen_buffer.height != vm_screen_height) {
		if(now_record_video) {
			stop_record_video();
//			stop_record_sound();
		}
		initialize_screen_buffer(&vm_screen_buffer, vm_screen_width, vm_screen_height, COLORONCOLOR);
	}
}

void OSD::set_vm_screen_lines(int lines)
{
//	set_vm_screen_size(vm_screen_width, lines, vm_window_width, vm_window_height, vm_window_width_aspect, vm_screen_height);
}

scrntype_t* OSD::get_vm_screen_buffer(int y)
{
	return vm_screen_buffer.get_buffer(y);
}

int OSD::draw_screen()
{
	// check avi file recording timing
	if(now_record_video && rec_video_run_frames <= 0) {
		return 0;
	}
	
	// draw screen
	if(vm_screen_buffer.width != vm_screen_width || vm_screen_buffer.height != vm_screen_height) {
		if(now_record_video) {
			stop_record_video();
//			stop_record_sound();
		}
		initialize_screen_buffer(&vm_screen_buffer, vm_screen_width, vm_screen_height, COLORONCOLOR);
	}
#ifdef USE_SCREEN_FILTER
	screen_skip_line = false;
#endif
	vm->draw_screen();
	
#ifndef ONE_BOARD_MICRO_COMPUTER
	// screen size was changed in vm->draw_screen()
	if(vm_screen_buffer.width != vm_screen_width || vm_screen_buffer.height != vm_screen_height) {
		return 0;
	}
	draw_screen_buffer = &vm_screen_buffer;
	
	// calculate screen size
//#ifdef USE_SCREEN_ROTATE
	int tmp_width_aspect = (config.rotate_type == 1 || config.rotate_type == 3) ? vm_window_height_aspect : vm_window_width_aspect;
	int tmp_height_aspect = (config.rotate_type == 1 || config.rotate_type == 3) ? vm_window_width_aspect : vm_window_height_aspect;
	int tmp_width = (config.rotate_type == 1 || config.rotate_type == 3) ? vm_window_height : vm_window_width;
	int tmp_height = (config.rotate_type == 1 || config.rotate_type == 3) ? vm_window_width : vm_window_height;
//#else
//	#define tmp_width_aspect vm_window_width_aspect
//	#define tmp_height_aspect vm_window_height_aspect
//	#define tmp_width vm_window_width
//	#define tmp_height vm_window_height
//#endif
	
	if(host_window_mode) {
		// window mode
		draw_screen_width = host_window_width;
		draw_screen_height = host_window_height;
	} else {
		// fullscreen mode
		if(config.fullscreen_stretch_type == 0) {
			// dot by dot
			int tmp_pow_x = host_window_width / tmp_width;
			int tmp_pow_y = host_window_height / tmp_height;
			int tmp_pow = 1;
			if(tmp_pow_y >= tmp_pow_x && tmp_pow_x > 1) {
				tmp_pow = tmp_pow_x;
			} else if(tmp_pow_x >= tmp_pow_y && tmp_pow_y > 1) {
				tmp_pow = tmp_pow_y;
			}
			draw_screen_width = tmp_width * tmp_pow;
			draw_screen_height = tmp_height * tmp_pow;
		} else if(config.fullscreen_stretch_type == 1) {
			// stretch (no aspect)
			draw_screen_width = (host_window_height * tmp_width) / tmp_height;
			draw_screen_height = host_window_height;
			if(draw_screen_width > host_window_width) {
				draw_screen_width = host_window_width;
				draw_screen_height = (host_window_width * tmp_height) / tmp_width;
			}
		} else if(config.fullscreen_stretch_type == 2) {
			// stretch (aspect)
			draw_screen_width = (host_window_height * tmp_width_aspect) / tmp_height_aspect;
			draw_screen_height = host_window_height;
			if(draw_screen_width > host_window_width) {
				draw_screen_width = host_window_width;
				draw_screen_height = (host_window_width * tmp_height_aspect) / tmp_width_aspect;
			}
		} else if(config.fullscreen_stretch_type == 3) {
			// stretch (fill)
			draw_screen_width = host_window_width;
			draw_screen_height = host_window_height;
		}
	}
	int dest_pow_x = (int)ceil((double)draw_screen_width / (double)tmp_width);
	int dest_pow_y = (int)ceil((double)draw_screen_height / (double)tmp_height);
//#ifdef USE_SCREEN_ROTATE
	int tmp_pow_x = (config.rotate_type == 1 || config.rotate_type == 3) ? dest_pow_y : dest_pow_x;
	int tmp_pow_y = (config.rotate_type == 1 || config.rotate_type == 3) ? dest_pow_x : dest_pow_y;
//#else
//	#define tmp_pow_x dest_pow_x
//	#define tmp_pow_y dest_pow_y
//#endif
	
#ifdef USE_SCREEN_FILTER
	// apply crt filter
	if(config.filter_type == SCREEN_FILTER_RGB) {
		if(filtered_screen_buffer.width != vm_screen_width * tmp_pow_x || filtered_screen_buffer.height != vm_screen_height * tmp_pow_y) {
			initialize_screen_buffer(&filtered_screen_buffer, vm_screen_width * tmp_pow_x, vm_screen_height * tmp_pow_y, COLORONCOLOR);
		}
		apply_rgb_filter_to_screen_buffer(draw_screen_buffer, &filtered_screen_buffer);
		draw_screen_buffer = &filtered_screen_buffer;
	} else if(config.filter_type == SCREEN_FILTER_RF) {
		// FIXME
	}
#endif
//#ifdef USE_SCREEN_ROTATE
	// rotate screen
	if(config.rotate_type == 1 || config.rotate_type == 3) {
		if(rotated_screen_buffer.width != draw_screen_buffer->height || rotated_screen_buffer.height != draw_screen_buffer->width) {
			initialize_screen_buffer(&rotated_screen_buffer, draw_screen_buffer->height, draw_screen_buffer->width, COLORONCOLOR);
		}
		rotate_screen_buffer(draw_screen_buffer, &rotated_screen_buffer);
		draw_screen_buffer = &rotated_screen_buffer;
	} else if(config.rotate_type == 2) {
		if(rotated_screen_buffer.width != draw_screen_buffer->width || rotated_screen_buffer.height != draw_screen_buffer->height) {
			initialize_screen_buffer(&rotated_screen_buffer, draw_screen_buffer->width, draw_screen_buffer->height, COLORONCOLOR);
		}
		rotate_screen_buffer(draw_screen_buffer, &rotated_screen_buffer);
		draw_screen_buffer = &rotated_screen_buffer;
	}
//#endif
	// stretch screen
	if(draw_screen_buffer->width != tmp_width * dest_pow_x || draw_screen_buffer->height != tmp_height * dest_pow_y) {
		if(stretched_screen_buffer.width != tmp_width * dest_pow_x || stretched_screen_buffer.height != tmp_height * dest_pow_y) {
			initialize_screen_buffer(&stretched_screen_buffer, tmp_width * dest_pow_x, tmp_height * dest_pow_y, COLORONCOLOR);
		}
		stretch_screen_buffer(draw_screen_buffer, &stretched_screen_buffer);
		draw_screen_buffer = &stretched_screen_buffer;
	}
	
	// initialize d3d9 surface
	static bool prev_use_d3d9 = config.use_d3d9;
	static bool prev_wait_vsync = config.wait_vsync;
	static int prev_window_width = 0, prev_window_height = 0;
	static int prev_screen_width = 0, prev_screen_height = 0;
	
	if(prev_use_d3d9 != config.use_d3d9 || prev_wait_vsync != config.wait_vsync || prev_window_width != host_window_width || prev_window_height != host_window_height) {
		if(config.use_d3d9) {
			config.use_d3d9 = initialize_d3d9();
		} else {
			release_d3d9();
		}
		prev_use_d3d9 = config.use_d3d9;
		prev_wait_vsync = config.wait_vsync;
		prev_window_width = host_window_width;
		prev_window_height = host_window_height;
		prev_screen_width = prev_screen_height = 0;
	}
	if(prev_screen_width != draw_screen_buffer->width || prev_screen_height != draw_screen_buffer->height) {
		if(config.use_d3d9) {
			config.use_d3d9 = initialize_d3d9_surface(draw_screen_buffer);
		}
		prev_screen_width = draw_screen_buffer->width;
		prev_screen_height = draw_screen_buffer->height;
	}
	
	if(config.use_d3d9) {
		// copy screen to d3d9 offscreen surface
		copy_to_d3d9_surface(draw_screen_buffer);
	} else {
		if(draw_screen_buffer->width != draw_screen_width || draw_screen_buffer->height != draw_screen_height) {
			if(shrinked_screen_buffer.width != draw_screen_width || shrinked_screen_buffer.height != draw_screen_height) {
				initialize_screen_buffer(&shrinked_screen_buffer, draw_screen_width, draw_screen_height, HALFTONE);
			}
			stretch_screen_buffer(draw_screen_buffer, &shrinked_screen_buffer);
			draw_screen_buffer = &shrinked_screen_buffer;
		}
	}
#endif
	
	// invalidate window
#ifdef ONE_BOARD_MICRO_COMPUTER
	if(first_invalidate) {
//		InvalidateRect(main_window_handle, NULL, TRUE);
		RECT rect = { 0, 0, host_window_width, host_window_height };
		InvalidateRect(main_window_handle, &rect, TRUE);
	} else {
#ifdef MAX_DRAW_RANGES
		for(int i = 0; i < MAX_DRAW_RANGES; i++) {
#else
		for(int i = 0; i < vm->max_draw_ranges(); i++) { // for TK-80BS
#endif
			int x = vm_ranges[i].x;
			int y = vm_ranges[i].y;
			int w = vm_ranges[i].width;
			int h = vm_ranges[i].height;
			RECT rect = { x, y, x + w, y + h };
			InvalidateRect(main_window_handle, &rect, FALSE);
		}
	}
#else
//	InvalidateRect(main_window_handle, NULL, first_invalidate);
	RECT rect = { 0, 0, host_window_width, host_window_height };
	InvalidateRect(main_window_handle, &rect, first_invalidate);
#endif
	UpdateWindow(main_window_handle);
#if USE_RETROACHIEVEMENTS
    RA_RenderOverlayFrame(NULL);
#endif
	first_draw_screen = self_invalidate = true;
	
	// record avi file
	if(now_record_video) {
		return add_video_frames();
	} else {
		return 1;
	}
}

void OSD::invalidate_screen()
{
//	InvalidateRect(main_window_handle, NULL, TRUE);
	RECT rect = { 0, 0, host_window_width, host_window_height };
	InvalidateRect(main_window_handle, &rect, TRUE);
}

void OSD::update_screen(HDC hdc)
{
#ifdef ONE_BOARD_MICRO_COMPUTER
#ifndef BITMAP_OFFSET_X
#define BITMAP_OFFSET_X 0
#endif
#ifndef BITMAP_OFFSET_Y
#define BITMAP_OFFSET_Y 0
#endif
	if(first_invalidate || !self_invalidate) {
#if 1
		// load png from resource
		HRSRC hResource = FindResource(instance_handle, _T("IDI_BITMAP_BOARD"), _T("IMAGE"));
		if(hResource != NULL) {
			const void* pResourceData = LockResource(LoadResource(instance_handle, hResource));
			if(pResourceData != NULL) {
				DWORD dwResourceSize = SizeofResource(instance_handle, hResource);
				HGLOBAL hResourceBuffer = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);
				if(hResourceBuffer != NULL) {
					void* pResourceBuffer = GlobalLock(hResourceBuffer);
					if(pResourceBuffer != NULL) {
						CopyMemory(pResourceBuffer, pResourceData, dwResourceSize);
						IStream* pIStream = NULL;
						if(CreateStreamOnHGlobal(hResourceBuffer, FALSE, &pIStream) == S_OK) {
							Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromStream(pIStream);
							if(pBitmap != NULL) {
								Gdiplus::Graphics graphics(hdc);
								graphics.DrawImage(pBitmap, BITMAP_OFFSET_X, BITMAP_OFFSET_Y);
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
		HDC hmdc = CreateCompatibleDC(hdc);
		HBITMAP hBitmap = LoadBitmap(instance_handle, _T("IDI_BITMAP_BOARD"));
		BITMAP bmp;
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		int w = (int)bmp.bmWidth;
		int h = (int)bmp.bmHeight;
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hmdc, hBitmap);
		BitBlt(hdc, BITMAP_OFFSET_X, BITMAP_OFFSET_Y, w, h, hmdc, 0, 0, SRCCOPY);
		SelectObject(hmdc, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hmdc);
#endif
	}
	if(first_draw_screen) {
		// 7-seg LEDs
#ifdef MAX_DRAW_RANGES
		for(int i = 0; i < MAX_DRAW_RANGES; i++) {
#else
		for(int i = 0; i < vm->max_draw_ranges(); i++) { // for TK-80BS
#endif
			int x = vm_ranges[i].x;
			int y = vm_ranges[i].y;
			int w = vm_ranges[i].width;
			int h = vm_ranges[i].height;
			BitBlt(hdc, x, y, w, h, vm_screen_buffer.hdcDib, x, y, SRCCOPY);
		}
		first_invalidate = self_invalidate = false;
	}
#else
	if(first_draw_screen) {
		int dest_x = (host_window_width - draw_screen_width) / 2;
		int dest_y = (host_window_height - draw_screen_height) / 2;
		
		if(config.use_d3d9) {
			LPDIRECT3DSURFACE9 lpd3d9BackSurface = NULL;
			if(lpd3d9Device != NULL && lpd3d9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &lpd3d9BackSurface) == D3D_OK && lpd3d9BackSurface != NULL) {
				RECT rectSrc = { 0, 0, draw_screen_buffer->width, draw_screen_buffer->height };
				RECT rectDst = { dest_x, dest_y, dest_x + draw_screen_width, dest_y + draw_screen_height };
				RECT rectWin = { 0, 0, host_window_width, host_window_height };
				bool stretch_screen = !(draw_screen_buffer->width == draw_screen_width && draw_screen_buffer->height == draw_screen_height);
				
				lpd3d9Device->UpdateSurface(lpd3d9OffscreenSurface, NULL, lpd3d9Surface, NULL);
				lpd3d9Device->StretchRect(lpd3d9Surface, &rectSrc, lpd3d9BackSurface, &rectDst, stretch_screen ? D3DTEXF_LINEAR : D3DTEXF_POINT);
				lpd3d9BackSurface->Release();				
				lpd3d9Device->Present(&rectWin, &rectWin, NULL, NULL);
			}
		} else {
			BitBlt(hdc, dest_x, dest_y, draw_screen_width, draw_screen_height, draw_screen_buffer->hdcDib, 0, 0, SRCCOPY);
		}
		first_invalidate = self_invalidate = false;
	}
#endif
}

void OSD::initialize_screen_buffer(bitmap_t *buffer, int width, int height, int mode)
{
	release_screen_buffer(buffer);
	
	HDC hdc = GetDC(main_window_handle);
	buffer->hdcDib = CreateCompatibleDC(hdc);
#if defined(_RGB565)
	// thanks PC8801MA‰ü
	buffer->lpBuf = (LPBYTE)GlobalAlloc(GPTR, sizeof(BITMAPINFOHEADER) + sizeof(DWORD) * 3);
#else
	buffer->lpBuf = (LPBYTE)GlobalAlloc(GPTR, sizeof(BITMAPINFO));
#endif
	buffer->lpDib = (LPBITMAPINFO)(buffer->lpBuf);
	memset(&buffer->lpDib->bmiHeader, 0, sizeof(BITMAPINFOHEADER));
	buffer->lpDib->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	buffer->lpDib->bmiHeader.biWidth = width;
	buffer->lpDib->bmiHeader.biHeight = height;
	buffer->lpDib->bmiHeader.biPlanes = 1;
#if defined(_RGB555)
	buffer->lpDib->bmiHeader.biBitCount = 16;
	buffer->lpDib->bmiHeader.biCompression = BI_RGB;
	buffer->lpDib->bmiHeader.biSizeImage = width * height * 2;
#elif defined(_RGB565)
	buffer->lpDib->bmiHeader.biBitCount = 16;
	buffer->lpDib->bmiHeader.biCompression = BI_BITFIELDS;
	LPDWORD lpBf = (LPDWORD)buffer->lpDib->bmiColors;
	lpBf[0] = 0x1f << 11;
	lpBf[1] = 0x3f << 5;
	lpBf[2] = 0x1f << 0;
	buffer->lpDib->bmiHeader.biSizeImage = width * height * 2;
#elif defined(_RGB888)
	buffer->lpDib->bmiHeader.biBitCount = 32;
	buffer->lpDib->bmiHeader.biCompression = BI_RGB;
	buffer->lpDib->bmiHeader.biSizeImage = width * height * 4;
#endif
	buffer->lpDib->bmiHeader.biXPelsPerMeter = 0;
	buffer->lpDib->bmiHeader.biYPelsPerMeter = 0;
	buffer->lpDib->bmiHeader.biClrUsed = 0;
	buffer->lpDib->bmiHeader.biClrImportant = 0;
	buffer->hBmp = CreateDIBSection(hdc, buffer->lpDib, DIB_RGB_COLORS, (PVOID*)&(buffer->lpBmp), NULL, 0);
	buffer->hOldBmp = (HBITMAP)SelectObject(buffer->hdcDib, buffer->hBmp);
	ReleaseDC(main_window_handle, hdc);
	SetStretchBltMode(buffer->hdcDib, mode);
	
	buffer->width = width;
	buffer->height = height;
}

void OSD::release_screen_buffer(bitmap_t *buffer)
{
	if(buffer->initialized()) {
		if(buffer->hdcDib != NULL && buffer->hOldBmp != NULL) {
			SelectObject(buffer->hdcDib, buffer->hOldBmp);
		}
		if(buffer->hBmp != NULL) {
			DeleteObject(buffer->hBmp);
			buffer->hBmp = NULL;
		}
		if(buffer->lpBuf != NULL) {
			GlobalFree(buffer->lpBuf);
			buffer->lpBuf = NULL;
		}
		if(buffer->hdcDib != NULL) {
			DeleteDC(buffer->hdcDib);
			buffer->hdcDib = NULL;
		}
	}
	memset(buffer, 0, sizeof(bitmap_t));
}

#ifdef USE_SCREEN_FILTER
#define _3_8(v) (((((v) * 3) >> 3) * 180) >> 8)
#define _5_8(v) (((((v) * 3) >> 3) * 180) >> 8)
#define _8_8(v) (((v) * 180) >> 8)

static uint8_t r0[2048], g0[2048], b0[2048], t0[2048];
static uint8_t r1[2048], g1[2048], b1[2048];

void OSD::apply_rgb_filter_to_screen_buffer(bitmap_t *source, bitmap_t *dest)
{
	if(source->width * 6 == dest->width && source->height * 6 == dest->height) {
		// FM-77AV: 320x200 -> 640x400 -> 1920x1200
		if(tmp_filtered_screen_buffer.width != source->width * 2 || tmp_filtered_screen_buffer.height != source->height * 2) {
			initialize_screen_buffer(&tmp_filtered_screen_buffer, source->width * 2, source->height * 2, COLORONCOLOR);
		}
		stretch_screen_buffer(source, &tmp_filtered_screen_buffer);
		screen_skip_line = true;
		apply_rgb_filter_x3_y3(&tmp_filtered_screen_buffer, dest);
	} else if(source->width * 3 == dest->width && source->height * 6 == dest->height) {
		// FM-77AV: 640x200 -> 640x400 -> 1920x1200
		if(tmp_filtered_screen_buffer.width != source->width || tmp_filtered_screen_buffer.height != source->height * 2) {
			initialize_screen_buffer(&tmp_filtered_screen_buffer, source->width, source->height * 2, COLORONCOLOR);
		}
		stretch_screen_buffer(source, &tmp_filtered_screen_buffer);
		screen_skip_line = true;
		apply_rgb_filter_x3_y3(&tmp_filtered_screen_buffer, dest);
	} else if(source->width * 4 == dest->width && source->height * 4 == dest->height) {
		// FM-77AV: 320x200 -> 640x400 -> 1280x800
		if(tmp_filtered_screen_buffer.width != source->width * 2 || tmp_filtered_screen_buffer.height != source->height * 2) {
			initialize_screen_buffer(&tmp_filtered_screen_buffer, source->width * 2, source->height * 2, COLORONCOLOR);
		}
		stretch_screen_buffer(source, &tmp_filtered_screen_buffer);
		screen_skip_line = true;
		apply_rgb_filter_x2_y2(&tmp_filtered_screen_buffer, dest);
	} else if(source->width * 2 == dest->width && source->height * 4 == dest->height) {
		// FM-77AV: 640x200 -> 640x400 -> 1280x800
		if(tmp_filtered_screen_buffer.width != source->width || tmp_filtered_screen_buffer.height != source->height * 2) {
			initialize_screen_buffer(&tmp_filtered_screen_buffer, source->width, source->height * 2, COLORONCOLOR);
		}
		stretch_screen_buffer(source, &tmp_filtered_screen_buffer);
		screen_skip_line = true;
		apply_rgb_filter_x2_y2(&tmp_filtered_screen_buffer, dest);
	} else if(source->width * 3 == dest->width && source->height * 3 == dest->height) {
		apply_rgb_filter_x3_y3(source, dest);
	} else if(source->width * 3 == dest->width && source->height * 2 == dest->height) {
		apply_rgb_filter_x3_y2(source, dest);
	} else if(source->width * 2 == dest->width && source->height * 3 == dest->height) {
		apply_rgb_filter_x2_y3(source, dest);
	} else if(source->width * 2 == dest->width && source->height * 2 == dest->height) {
		apply_rgb_filter_x2_y2(source, dest);
	} else if(source->width != dest->width || source->height != dest->height) {
		if(tmp_filtered_screen_buffer.width != source->width || tmp_filtered_screen_buffer.height != source->height) {
			initialize_screen_buffer(&tmp_filtered_screen_buffer, source->width, source->height, COLORONCOLOR);
		}
		apply_rgb_filter_x1_y1(source, &tmp_filtered_screen_buffer);
		stretch_screen_buffer(&tmp_filtered_screen_buffer, dest);
	} else {
		apply_rgb_filter_x1_y1(source, dest);
	}
}

void OSD::apply_rgb_filter_x3_y3(bitmap_t *source, bitmap_t *dest)
{
	if(!screen_skip_line) {
		for(int y = 0, yy = 0; y < source->height; y++, yy += 3) {
			scrntype_t* src = source->get_buffer(y);
			scrntype_t* out1 = dest->get_buffer(yy + 0);
			scrntype_t* out2 = dest->get_buffer(yy + 1);
			scrntype_t* out3 = dest->get_buffer(yy + 2);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype_t c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 3) {
				uint32_t r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32_t g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32_t b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = out2[xx    ] = (32 + _8_8(r)) << 16;
				out1[xx + 1] = out2[xx + 1] = (32 + _8_8(g)) << 8;
				out1[xx + 2] = out2[xx + 2] = (32 + _8_8(b));
				if(t0[x]) {
					out3[xx    ] = (32 + _8_8(r)) << 16;
					out3[xx + 1] = (32 + _8_8(g)) << 8;
					out3[xx + 2] = (32 + _8_8(b));
				} else {
					out3[xx    ] = (32 + _5_8(r)) << 16;
					out3[xx + 1] = (32 + _5_8(g)) << 8;
					out3[xx + 2] = (32 + _5_8(b));
				}
			}
		}
	} else {
		for(int y = 0, yy = 0; y < source->height; y += 2, yy += 6) {
			scrntype_t* src = source->get_buffer(y);
			scrntype_t* out1 = dest->get_buffer(yy + 0);
			scrntype_t* out2 = dest->get_buffer(yy + 1);
			scrntype_t* out3 = dest->get_buffer(yy + 2);
			scrntype_t* out4 = dest->get_buffer(yy + 3);
			scrntype_t* out5 = dest->get_buffer(yy + 4);
			scrntype_t* out6 = dest->get_buffer(yy + 5);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype_t c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 3) {
				uint32_t r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32_t g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32_t b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = out2[xx    ] = out3[xx    ] = out4[xx    ] = (32 + _8_8(r)) << 16;
				out1[xx + 1] = out2[xx + 1] = out3[xx + 1] = out4[xx + 1] = (32 + _8_8(g)) << 8;
				out1[xx + 2] = out2[xx + 2] = out3[xx + 2] = out4[xx + 2] = (32 + _8_8(b));
				if(t0[x]) {
					out5[xx    ] = out6[xx    ] = (32 + _8_8(r)) << 16;
					out5[xx + 1] = out6[xx + 1] = (32 + _8_8(g)) << 8;
					out5[xx + 2] = out6[xx + 2] = (32 + _8_8(b));
				} else {
					out5[xx    ] = out6[xx    ] = (32 + _5_8(r)) << 16;
					out5[xx + 1] = out6[xx + 1] = (32 + _5_8(g)) << 8;
					out5[xx + 2] = out6[xx + 2] = (32 + _5_8(b));
				}
			}
		}
	}
}

void OSD::apply_rgb_filter_x3_y2(bitmap_t *source, bitmap_t *dest)
{
	if(!screen_skip_line) {
		for(int y = 0, yy = 0; y < source->height; y++, yy += 2) {
			scrntype_t* src = source->get_buffer(y);
			scrntype_t* out1 = dest->get_buffer(yy + 0);
			scrntype_t* out2 = dest->get_buffer(yy + 1);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype_t c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 3) {
				uint32_t r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32_t g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32_t b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = (32 + _8_8(r)) << 16;
				out1[xx + 1] = (32 + _8_8(g)) << 8;
				out1[xx + 2] = (32 + _8_8(b));
				if(t0[x]) {
					out2[xx    ] = (32 + _8_8(r)) << 16;
					out2[xx + 1] = (32 + _8_8(g)) << 8;
					out2[xx + 2] = (32 + _8_8(b));
				} else {
					out2[xx    ] = (32 + _5_8(r)) << 16;
					out2[xx + 1] = (32 + _5_8(g)) << 8;
					out2[xx + 2] = (32 + _5_8(b));
				}
			}
		}
	} else {
		for(int y = 0, yy = 0; y < source->height; y += 2, yy += 4) {
			scrntype_t* src = source->get_buffer(y);
			scrntype_t* out1 = dest->get_buffer(yy + 0);
			scrntype_t* out2 = dest->get_buffer(yy + 1);
			scrntype_t* out3 = dest->get_buffer(yy + 2);
			scrntype_t* out4 = dest->get_buffer(yy + 3);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype_t c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 3) {
				uint32_t r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32_t g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32_t b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = out2[xx    ] = out3[xx    ] = (32 + _8_8(r)) << 16;
				out1[xx + 1] = out2[xx + 1] = out3[xx + 1] = (32 + _8_8(g)) << 8;
				out1[xx + 2] = out2[xx + 2] = out3[xx + 2] = (32 + _8_8(b));
				if(t0[x]) {
					out4[xx    ] = (32 + _8_8(r)) << 16;
					out4[xx + 1] = (32 + _8_8(g)) << 8;
					out4[xx + 2] = (32 + _8_8(b));
				} else {
					out4[xx    ] = (32 + _5_8(r)) << 16;
					out4[xx + 1] = (32 + _5_8(g)) << 8;
					out4[xx + 2] = (32 + _5_8(b));
				}
			}
		}
	}
}

void OSD::apply_rgb_filter_x2_y3(bitmap_t *source, bitmap_t *dest)
{
	if(!screen_skip_line) {
		for(int y = 0, yy = 0; y < source->height; y++, yy += 3) {
			scrntype_t* src = source->get_buffer(y);
			scrntype_t* out1 = dest->get_buffer(yy + 0);
			scrntype_t* out2 = dest->get_buffer(yy + 1);
			scrntype_t* out3 = dest->get_buffer(yy + 2);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype_t c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 2) {
				uint32_t r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32_t g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32_t b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = out2[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
				out1[xx + 1] = out2[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				if(t0[x]) {
					out3[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
					out3[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				} else {
					out3[xx    ] = RGB_COLOR(32 + _3_8(r), 32 + _3_8(g), 32 + _3_8(b));
					out3[xx + 1] = RGB_COLOR(16 + _3_8(r), 16 + _3_8(g), 16 + _3_8(b));
				}
			}
		}
	} else {
		for(int y = 0, yy = 0; y < source->height; y += 2, yy += 6) {
			scrntype_t* src = source->get_buffer(y);
			scrntype_t* out1 = dest->get_buffer(yy + 0);
			scrntype_t* out2 = dest->get_buffer(yy + 1);
			scrntype_t* out3 = dest->get_buffer(yy + 2);
			scrntype_t* out4 = dest->get_buffer(yy + 3);
			scrntype_t* out5 = dest->get_buffer(yy + 4);
			scrntype_t* out6 = dest->get_buffer(yy + 5);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype_t c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 2) {
				uint32_t r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32_t g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32_t b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = out2[xx    ] = out3[xx    ] = out4[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
				out1[xx + 1] = out2[xx + 1] = out3[xx + 1] = out4[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				if(t0[x]) {
					out5[xx    ] = out6[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
					out5[xx + 1] = out6[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				} else {
					out5[xx    ] = out6[xx    ] = RGB_COLOR(32 + _3_8(r), 32 + _3_8(g), 32 + _3_8(b));
					out5[xx + 1] = out6[xx + 1] = RGB_COLOR(16 + _3_8(r), 16 + _3_8(g), 16 + _3_8(b));
				}
			}
		}
	}
}

void OSD::apply_rgb_filter_x2_y2(bitmap_t *source, bitmap_t *dest)
{
	if(!screen_skip_line) {
		for(int y = 0, yy = 0; y < source->height; y++, yy += 2) {
			scrntype_t* src = source->get_buffer(y);
			scrntype_t* out1 = dest->get_buffer(yy + 0);
			scrntype_t* out2 = dest->get_buffer(yy + 1);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype_t c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 2) {
				uint32_t r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32_t g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32_t b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
				out1[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				if(t0[x]) {
					out2[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
					out2[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				} else {
					out2[xx    ] = RGB_COLOR(32 + _3_8(r), 32 + _3_8(g), 32 + _3_8(b));
					out2[xx + 1] = RGB_COLOR(16 + _3_8(r), 16 + _3_8(g), 16 + _3_8(b));
				}
			}
		}
	} else {
		for(int y = 0, yy = 0; y < source->height; y += 2, yy += 4) {
			scrntype_t* src = source->get_buffer(y);
			scrntype_t* out1 = dest->get_buffer(yy + 0);
			scrntype_t* out2 = dest->get_buffer(yy + 1);
			scrntype_t* out3 = dest->get_buffer(yy + 2);
			scrntype_t* out4 = dest->get_buffer(yy + 3);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype_t c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1, xx = 0; x <= source->width; x++, xx += 2) {
				uint32_t r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32_t g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32_t b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[xx    ] = out2[xx    ] = out3[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
				out1[xx + 1] = out2[xx + 1] = out3[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				if(t0[x]) {
					out4[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
					out4[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
				} else {
					out4[xx    ] = RGB_COLOR(32 + _3_8(r), 32 + _3_8(g), 32 + _3_8(b));
					out4[xx + 1] = RGB_COLOR(16 + _3_8(r), 16 + _3_8(g), 16 + _3_8(b));
				}
			}
		}
	}
}

void OSD::apply_rgb_filter_x1_y1(bitmap_t *source, bitmap_t *dest)
{
	if(!screen_skip_line) {
		for(int y = 0; y < source->height; y++) {
			scrntype_t* src = source->get_buffer(y);
			scrntype_t* out1 = dest->get_buffer(y + 0);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype_t c = src[x - 1];
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1; x <= source->width; x++) {
				uint32_t r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32_t g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32_t b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[x - 1] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
			}
		}
	} else {
		for(int y = 0; y < source->height; y += 2) {
			scrntype_t* src = source->get_buffer(y);
			scrntype_t* out1 = dest->get_buffer(y + 0);
			scrntype_t* out2 = dest->get_buffer(y + 1);
			
			for(int x = 1; x <= source->width; x++) {
				scrntype_t c = src[x - 1];
				t0[x] = A_OF_COLOR(c);
				r0[x] = R_OF_COLOR(c);
				g0[x] = G_OF_COLOR(c);
				b0[x] = B_OF_COLOR(c);
				r1[x] = r0[x] >> 3;
				g1[x] = g0[x] >> 3;
				b1[x] = b0[x] >> 3;
			}
			for(int x = 1; x <= source->width; x++) {
				uint32_t r = r1[x - 1] + r0[x] + r1[x + 1];
				uint32_t g = g1[x - 1] + g0[x] + g1[x + 1];
				uint32_t b = b1[x - 1] + b0[x] + b1[x + 1];
				out1[x - 1] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
				if(t0[x]) {
					out2[x - 1] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
				} else {
					out2[x - 1] = RGB_COLOR(32 + _3_8(r), 32 + _3_8(g), 32 + _3_8(b));
				}
			}
		}
	}
}
#endif

//#ifdef USE_SCREEN_ROTATE
void OSD::rotate_screen_buffer(bitmap_t *source, bitmap_t *dest)
{
	if(config.rotate_type == 1) {
		// turn right 90deg
		if(source->width == dest->height && source->height == dest->width) {
			for(int y = 0; y < source->height; y++) {
				scrntype_t* source_buffer = source->get_buffer(y);
				int offset = dest->width - y - 1;
				
				for(int x = 0; x < source->width; x++) {
					dest->get_buffer(x)[offset] = source_buffer[x];
				}
			}
		}
	} else if(config.rotate_type == 2) {
		// turn right 180deg
		if(source->width == dest->width && source->height == dest->height) {
			for(int y = 0; y < source->height; y++) {
				scrntype_t* source_buffer = source->get_buffer(y);
				scrntype_t* dest_buffer = dest->get_buffer(dest->height - y - 1);
				int offset = dest->width - 1;
				
				for(int x = 0; x < source->width; x++) {
					dest_buffer[offset - x] = source_buffer[x];
				}
			}
		}
	} else if(config.rotate_type == 3) {
		// turn right 270deg
		if(source->width == dest->height && source->height == dest->width) {
			for(int y = 0; y < source->height; y++) {
				scrntype_t* source_buffer = source->get_buffer(y);
				int offset = dest->height - 1;
				
				for(int x = 0; x < source->width; x++) {
					dest->get_buffer(offset - x)[y] = source_buffer[x];
				}
			}
		}
	}
}
//#endif

void OSD::stretch_screen_buffer(bitmap_t *source, bitmap_t *dest)
{
	if((dest->width % source->width) == 0 && (dest->height % source->height) == 0) {
		// faster than StretchBlt()
		int pow_x = dest->width / source->width;
		int pow_y = dest->height / source->height;
		
		for(int y = 0, yy = 0; y < source->height; y++, yy += pow_y) {
			scrntype_t* source_buffer = source->get_buffer(y);
			scrntype_t* dest_buffer = dest->get_buffer(yy);
			
			if(pow_x != 1) {
				scrntype_t* tmp_buffer = dest_buffer;
				for(int x = 0; x < source->width; x++) {
					scrntype_t c = source_buffer[x];
					for(int px = 0; px < pow_x; px++) {
						tmp_buffer[px] = c;
					}
					tmp_buffer += pow_x;
				}
			} else {
				// about 10% faster than memcpy()
				for(int x = 0; x < source->width; x++) {
					dest_buffer[x] = source_buffer[x];
				}
			}
			if(pow_y != 1) {
				for(int py = 1; py < pow_y; py++) {
					// about 10% faster than memcpy()
					scrntype_t* tmp_buffer = dest->get_buffer(yy + py);
					for(int x = 0; x < dest->width; x++) {
						tmp_buffer[x] = dest_buffer[x];
					}
				}
			}
		}
	} else {
		StretchBlt(dest->hdcDib, 0, 0, dest->width, dest->height, source->hdcDib, 0, 0, source->width, source->height, SRCCOPY);
	}
}

#if defined(_RGB555)
	#define D3DFMT_TMP D3DFMT_X1R5G5B5
#elif defined(_RGB565)
	#define D3DFMT_TMP D3DFMT_R5G6B5
#elif defined(_RGB888)
	#define D3DFMT_TMP D3DFMT_X8R8G8B8
#endif

bool OSD::initialize_d3d9()
{
	release_d3d9();
	
	if((lpd3d9 = Direct3DCreate9(D3D_SDK_VERSION)) != NULL) {
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.BackBufferWidth = host_window_width;
		d3dpp.BackBufferHeight = host_window_height;
		d3dpp.BackBufferFormat = D3DFMT_TMP;//D3DFMT_UNKNOWN;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = main_window_handle;
		d3dpp.Windowed = TRUE;
		d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		d3dpp.PresentationInterval = config.wait_vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
		
		// create d3d9 device
		HRESULT hr = lpd3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, main_window_handle, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &lpd3d9Device);
		if(hr != D3D_OK) {
			hr = lpd3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, main_window_handle, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &lpd3d9Device);
			if(hr != D3D_OK) {
				hr = lpd3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, main_window_handle, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &lpd3d9Device);
			}
		}
		if(hr == D3D_OK) {
			return true;
		}
		MessageBox(main_window_handle, _T("Failed to create a Direct3D9 device"), _T(DEVICE_NAME), MB_OK | MB_ICONWARNING);
		release_d3d9();
	} else {
		MessageBox(main_window_handle, _T("Failed to initialize Direct3D9"), _T(DEVICE_NAME), MB_OK | MB_ICONWARNING);
	}
	return false;
}

bool OSD::initialize_d3d9_surface(bitmap_t *buffer)
{
	release_d3d9_surface();
	
	if(lpd3d9Device != NULL) {
		HRESULT hr = lpd3d9Device->CreateOffscreenPlainSurface(buffer->width, buffer->height, D3DFMT_TMP, D3DPOOL_DEFAULT, &lpd3d9Surface, NULL);
		if(hr == D3D_OK) {
			hr = lpd3d9Device->CreateOffscreenPlainSurface(buffer->width, buffer->height, D3DFMT_TMP, D3DPOOL_SYSTEMMEM, &lpd3d9OffscreenSurface, NULL);
		}
		if(hr == D3D_OK) {
			lpd3d9Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 0.0, 0);
			return true;
		}
		MessageBox(main_window_handle, _T("Failed to create a Direct3D9 offscreen surface"), _T(DEVICE_NAME), MB_OK | MB_ICONWARNING);
		release_d3d9();
	}
	return false;
}

void OSD::release_d3d9()
{
	release_d3d9_surface();
	
	if(lpd3d9Device != NULL) {
		lpd3d9Device->Release();
		lpd3d9Device = NULL;
	}
	if(lpd3d9 != NULL) {
		lpd3d9->Release();
		lpd3d9 = NULL;
	}
}

void OSD::release_d3d9_surface()
{
	if(lpd3d9OffscreenSurface != NULL) {
		lpd3d9OffscreenSurface->Release();
		lpd3d9OffscreenSurface = NULL;
	}
	if(lpd3d9Surface != NULL) {
		lpd3d9Surface->Release();
		lpd3d9Surface = NULL;
	}
}

void OSD::copy_to_d3d9_surface(bitmap_t *buffer)
{
	if(lpd3d9OffscreenSurface != NULL) {
		// lock offscreen surface
		D3DLOCKED_RECT pLockedRect;
		if(lpd3d9OffscreenSurface->LockRect(&pLockedRect, NULL, 0) == D3D_OK) {
			scrntype_t *lpd3d9Buffer = (scrntype_t *)pLockedRect.pBits;
			scrntype_t *out = lpd3d9Buffer;
			for(int y = 0; y < buffer->height; y++) {
				scrntype_t* src = buffer->get_buffer(y);
				for(int i = 0; i < buffer->width; i++) {
					out[i] = src[i];
				}
				out += buffer->width;
			}
			
			// unlock offscreen surface
			lpd3d9Buffer = NULL;
			lpd3d9OffscreenSurface->UnlockRect();
		}
	}
	
}

void OSD::capture_screen()
{
//	write_bitmap_to_file(&vm_screen_buffer, create_date_file_path(_T("bmp")));
	write_bitmap_to_file(&vm_screen_buffer, create_date_file_path(_T("png")));
}

bool OSD::start_record_video(int fps)
{
	if(fps > 0) {
		rec_video_fps = fps;
		rec_video_run_frames = rec_video_frames = 0;
	}
	bool show_dialog = (fps > 0);
	
	// initialize vfw
	create_date_file_path(video_file_path, _MAX_PATH, _T("avi"));
	AVIFileInit();
	if(AVIFileOpen(&pAVIFile, video_file_path, OF_WRITE | OF_CREATE, NULL) != AVIERR_OK) {
		return false;
	}
	if(video_screen_buffer.width != vm_screen_buffer.width || video_screen_buffer.height != vm_screen_buffer.height) {
		initialize_screen_buffer(&video_screen_buffer, vm_screen_buffer.width, vm_screen_buffer.height, COLORONCOLOR);
	}
	
	// stream header
	AVISTREAMINFO strhdr;
	memset(&strhdr, 0, sizeof(strhdr));
	strhdr.fccType = streamtypeVIDEO;	// vids
	strhdr.fccHandler = 0;
	strhdr.dwScale = 1;
	strhdr.dwRate = rec_video_fps;
	strhdr.dwSuggestedBufferSize = video_screen_buffer.lpDib->bmiHeader.biSizeImage;
	SetRect(&strhdr.rcFrame, 0, 0, video_screen_buffer.width, video_screen_buffer.height);
	if(AVIFileCreateStream(pAVIFile, &pAVIStream, &strhdr) != AVIERR_OK) {
		stop_record_video();
		return false;
	}
	
	// compression
	AVICOMPRESSOPTIONS FAR * pOpts[1];
	pOpts[0] = &AVIOpts;
	if(show_dialog && !AVISaveOptions(main_window_handle, ICMF_CHOOSE_KEYFRAME | ICMF_CHOOSE_DATARATE, 1, &pAVIStream, (LPAVICOMPRESSOPTIONS FAR *)&pOpts)) {
		AVISaveOptionsFree(1, (LPAVICOMPRESSOPTIONS FAR *)&pOpts);
		stop_record_video();
		return false;
	}
	if(AVIMakeCompressedStream(&pAVICompressed, pAVIStream, &AVIOpts, NULL) != AVIERR_OK) {
		stop_record_video();
		return false;
	}
	if(AVIStreamSetFormat(pAVICompressed, 0, &video_screen_buffer.lpDib->bmiHeader, video_screen_buffer.lpDib->bmiHeader.biSize + video_screen_buffer.lpDib->bmiHeader.biClrUsed * sizeof(RGBQUAD)) != AVIERR_OK) {
		stop_record_video();
		return false;
	}
	dwAVIFileSize = 0;
	lAVIFrames = 0;
	
	hVideoThread = (HANDLE)0;
	rec_video_thread_param.pAVICompressed = pAVICompressed;
	rec_video_thread_param.lpBmp = video_screen_buffer.lpBmp;
	rec_video_thread_param.pbmInfoHeader = &video_screen_buffer.lpDib->bmiHeader;
	rec_video_thread_param.dwAVIFileSize = 0;
	rec_video_thread_param.lAVIFrames = 0;
	rec_video_thread_param.frames = 0;
	rec_video_thread_param.result = 0;
	
	now_record_video = true;
	return true;
}

void OSD::stop_record_video()
{
	// release thread
	if(hVideoThread != (HANDLE)0) {
		WaitForSingleObject(hVideoThread, INFINITE);
		hVideoThread = (HANDLE)0;
	}
	
	// release vfw
	if(pAVIStream) {
		AVIStreamClose(pAVIStream);
	}
	if(pAVICompressed) {
		AVIStreamClose(pAVICompressed);
	}
	if(pAVIFile) {
		AVIFileClose(pAVIFile);
		AVIFileExit();
	}
	pAVIStream = NULL;
	pAVICompressed = NULL;
	pAVIFile = NULL;
	
	// repair header
	if(now_record_video) {
		FILE* fp = NULL;
		if((fp = _tfopen(video_file_path, _T("r+b"))) != NULL) {
			// copy fccHandler
			uint8_t buf[4];
			fseek(fp, 0xbc, SEEK_SET);
			if(ftell(fp) == 0xbc) {
				fread(buf, 4, 1, fp);
				fseek(fp, 0x70, SEEK_SET);
				fwrite(buf, 4, 1, fp);
			}
			fclose(fp);
		}
	}
	now_record_video = false;
}

void OSD::restart_record_video()
{
	bool tmp = now_record_video;
	stop_record_video();
	if(tmp) {
		start_record_video(-1);
	}
}

void OSD::add_extra_frames(int extra_frames)
{
	rec_video_run_frames += extra_frames;
}

unsigned __stdcall rec_video_thread(void *lpx)
{
	volatile rec_video_thread_param_t *p = (rec_video_thread_param_t *)lpx;
	LONG lBytesWritten;
	int result = REC_VIDEO_SUCCESS;
	
	while(p->frames > 0) {
		if(AVIStreamWrite(p->pAVICompressed, p->lAVIFrames++, 1, (LPBYTE)p->lpBmp, p->pbmInfoHeader->biSizeImage, AVIIF_KEYFRAME, NULL, &lBytesWritten) == AVIERR_OK) {
			p->frames--;
			// if avi file size > (2GB - 16MB), create new avi file
			if((p->dwAVIFileSize += lBytesWritten) >= 2130706432) {
				result = REC_VIDEO_FULL;
				break;
			}
		} else {
			result = REC_VIDEO_ERROR;
			break;
		}
	}
	p->result = result;
	_endthreadex(0);
	return 0;
}

int OSD::add_video_frames()
{
	static double frames = 0;
	static int prev_video_fps = -1;
	int counter = 0;
	static double prev_vm_fps = -1;
	double vm_fps = vm->get_frame_rate();
	
	if(prev_video_fps != rec_video_fps || prev_vm_fps != vm_fps) {
		prev_video_fps = rec_video_fps;
		prev_vm_fps = vm_fps;
		frames = vm_fps / rec_video_fps;
	}
	while(rec_video_run_frames > 0) {
		rec_video_run_frames -= frames;
		rec_video_frames += frames;
		counter++;
	}
	if(counter != 0) {
		if(hVideoThread != (HANDLE)0) {
			if(rec_video_thread_param.result == 0) {
				WaitForSingleObject(hVideoThread, INFINITE);
			}
			hVideoThread = (HANDLE)0;
			
			if(rec_video_thread_param.result == REC_VIDEO_FULL) {
				stop_record_video();
				if(!start_record_video(-1)) {
					return 0;
				}
			} else if(rec_video_thread_param.result == REC_VIDEO_ERROR) {
				stop_record_video();
				return 0;
			}
		}
//		BitBlt(vm_screen_buffer.hdcDib, 0, 0, vm_screen_buffer.width, vm_screen_buffer.height, video_screen_buffer.hdcDib, 0, 0, SRCCOPY);
		memcpy(video_screen_buffer.lpBmp, vm_screen_buffer.lpBmp, sizeof(scrntype_t) * vm_screen_buffer.width * vm_screen_buffer.height);
		
		rec_video_thread_param.frames += counter;
		rec_video_thread_param.result = 0;
		if((hVideoThread = (HANDLE)_beginthreadex(NULL, 0, rec_video_thread, &rec_video_thread_param, 0, NULL)) == (HANDLE)0) {
			stop_record_video();
			return 0;
		}
	}
	return counter;
}

#ifdef USE_PRINTER
void OSD::create_bitmap(bitmap_t *bitmap, int width, int height)
{
	initialize_screen_buffer(bitmap, width, height, HALFTONE);
}

void OSD::release_bitmap(bitmap_t *bitmap)
{
	release_screen_buffer(bitmap);
}

void OSD::create_font(font_t *font, const _TCHAR *family, int width, int height, int rotate, bool bold, bool italic)
{
	LOGFONT logfont;
	memset(&logfont, 0, sizeof(logfont));
	logfont.lfEscapement = font->rotate = rotate;
	logfont.lfOrientation = 0;
	logfont.lfWeight = (font->bold = bold) ? FW_BOLD : FW_NORMAL;
	logfont.lfItalic = (font->italic = italic) ? TRUE : FALSE;
	logfont.lfUnderline = FALSE;
	logfont.lfStrikeOut = FALSE;
	logfont.lfCharSet = SHIFTJIS_CHARSET;
	logfont.lfOutPrecision = OUT_TT_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = DEFAULT_QUALITY; 
	logfont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	if(_tcsicmp(family, _T("Gothic")) == 0) {
		my_tcscpy_s(logfont.lfFaceName, LF_FACESIZE, _T("MS Gothic"));
		my_tcscpy_s(font->family, 64, _T("Gothic"));
	} if(_tcsicmp(family, _T("PGothic")) == 0) {
		my_tcscpy_s(logfont.lfFaceName, LF_FACESIZE, _T("MS PGothic"));
		my_tcscpy_s(font->family, 64, _T("Gothic"));
	} else if(_tcsicmp(family, _T("Mincho")) == 0) {
		my_tcscpy_s(logfont.lfFaceName, LF_FACESIZE, _T("MS Mincho"));
		my_tcscpy_s(font->family, 64, _T("Mincho"));
	} else if(_tcsicmp(family, _T("PMincho")) == 0) {
		my_tcscpy_s(logfont.lfFaceName, LF_FACESIZE, _T("MS PMincho"));
		my_tcscpy_s(font->family, 64, _T("Mincho"));
	} else {
		my_tcscpy_s(logfont.lfFaceName, LF_FACESIZE, _T("MS Gothic"));
		my_tcscpy_s(font->family, 64, _T("Gothic"));
	}
	logfont.lfHeight = font->height = height;
	logfont.lfWidth = font->width = width;
	font->hFont = CreateFontIndirect(&logfont);
}

void OSD::release_font(font_t *font)
{
	if(font->initialized()) {
		DeleteObject(font->hFont);
		font->hFont = NULL;
	}
}

void OSD::create_pen(pen_t *pen, int width, uint8_t r, uint8_t g, uint8_t b)
{
	pen->hPen = CreatePen(PS_SOLID, (pen->width = width), RGB((pen->r = r), (pen->g = g), (pen->b = b)));
}

void OSD::release_pen(pen_t *pen)
{
	if(pen->initialized()) {
		DeleteObject(pen->hPen);
		pen->hPen = NULL;
	}
}

void OSD::clear_bitmap(bitmap_t *bitmap, uint8_t r, uint8_t g, uint8_t b)
{
	draw_rectangle_to_bitmap(bitmap, 0, 0, bitmap->width, bitmap->height, r, g, b);
}

int OSD::get_text_width(bitmap_t *bitmap, font_t *font, const char *text)
{
	HFONT hFontOld = (HFONT)SelectObject(bitmap->hdcDib, font->hFont);
	SIZE size;
#ifdef _UNICODE
	_TCHAR unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, text, -1, unicode, 1024);
	GetTextExtentPoint32(bitmap->hdcDib, unicode, wcslen(unicode), &size);
#else
	GetTextExtentPoint32(bitmap->hdcDib, text, strlen(text), &size);
#endif
	SelectObject(bitmap->hdcDib, hFontOld);
	return (int)size.cx;
}

void OSD::draw_text_to_bitmap(bitmap_t *bitmap, font_t *font, int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b)
{
	HFONT hFontOld = (HFONT)SelectObject(bitmap->hdcDib, font->hFont);
	SetBkMode(bitmap->hdcDib, TRANSPARENT);
	SetTextColor(bitmap->hdcDib, RGB(r, g, b));
#ifdef _UNICODE
	_TCHAR unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, text, -1, unicode, 1024);
	ExtTextOut(bitmap->hdcDib, x, y, NULL, NULL, unicode, wcslen(unicode), NULL);
#else
	ExtTextOut(bitmap->hdcDib, x, y, NULL, NULL, text, strlen(text), NULL);
#endif
	SelectObject(bitmap->hdcDib, hFontOld);
}

void OSD::draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen, int sx, int sy, int ex, int ey)
{
	HPEN hPenOld = (HPEN)SelectObject(bitmap->hdcDib, pen->hPen);
	MoveToEx(bitmap->hdcDib, sx, sy, NULL);
	LineTo(bitmap->hdcDib, ex, ey);
	SelectObject(bitmap->hdcDib, hPenOld);
}

void OSD::draw_rectangle_to_bitmap(bitmap_t *bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b)
{
	for(int yy = 0; yy < height; yy++) {
		for(int xx = 0; xx < width; xx++) {
			draw_point_to_bitmap(bitmap, x + xx, y + yy, r, g, b);
		}
	}
}

void OSD::draw_point_to_bitmap(bitmap_t *bitmap, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	if(x >= 0 && x < bitmap->width && y >= 0 && y < bitmap->height) {
		scrntype_t *dest = bitmap->get_buffer(y);
		dest[x] = RGB_COLOR(r, g, b);
	}
}

void OSD::stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y, int dest_width, int dest_height, bitmap_t *source, int source_x, int source_y, int source_width, int source_height)
{
	if(dest_width == source_width && dest_height == source_height) {
		BitBlt(dest->hdcDib, dest_x, dest_y, dest_width, dest_height, source->hdcDib, source_x, source_y, SRCCOPY);
	} else {
		StretchBlt(dest->hdcDib, dest_x, dest_y, dest_width, dest_height, source->hdcDib, source_x, source_y, source_width, source_height, SRCCOPY);
	}
}
#endif

bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0, size = 0;
	
	GetImageEncodersSize(&num, &size);
	if(size == 0) {
		return false;
	}
	ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)malloc(size);
	if(pImageCodecInfo == NULL) {
		return false;
	}
	GetImageEncoders(num, size, pImageCodecInfo);
	
	for(UINT j = 0; j < num; ++j) {
		if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return true;
		}
	}
	free(pImageCodecInfo);
	return false;
}

void OSD::write_bitmap_to_file(bitmap_t *bitmap, const _TCHAR *file_path)
{
	if(check_file_extension(file_path, _T(".bmp"))) {
		// save as bmp file
		BITMAPFILEHEADER bmFileHeader = { (WORD)(TEXT('B') | TEXT('M') << 8) };
		bmFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		bmFileHeader.bfSize = bmFileHeader.bfOffBits + bitmap->lpDib->bmiHeader.biSizeImage;
		
		DWORD dwSize;
		HANDLE hFile = CreateFile(file_path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		WriteFile(hFile, &bmFileHeader, sizeof(BITMAPFILEHEADER), &dwSize, NULL);
		WriteFile(hFile, bitmap->lpDib, sizeof(BITMAPINFOHEADER), &dwSize, NULL);
		WriteFile(hFile, bitmap->lpBmp, bitmap->lpDib->bmiHeader.biSizeImage, &dwSize, NULL);
		CloseHandle(hFile);
	} else if(check_file_extension(file_path, _T(".png"))) {
		// save as png file
		CLSID encoderClsid;
		if(GetEncoderClsid(L"image/png", &encoderClsid)) {
			Bitmap image(bitmap->hBmp, (HPALETTE)GetStockObject(DEFAULT_PALETTE));
#ifdef _UNICODE
			image.Save(file_path, &encoderClsid, NULL);
#else
			WCHAR wszFilePath[_MAX_PATH];
			MultiByteToWideChar(CP_ACP, 0, file_path, -1, wszFilePath, _MAX_PATH);
			image.Save(wszFilePath, &encoderClsid, NULL);
#endif
		}
	}
}

