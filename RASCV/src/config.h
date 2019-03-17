/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ config ]
*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#if defined(_USE_AGAR) || defined(_USE_QT)
#include <string>
#endif
#include "vm/vm.h"
#include "fileio.h"

#if defined(_USE_QT)
	enum {
		CONFIG_RENDER_TYPE_STD = 0,
		CONFIG_RENDER_TYPE_TV,
		CONFIG_RENDER_TYPE_END
	};
	enum {
		CONFIG_RENDER_PLATFORM_OPENGL_MAIN = 0,
		CONFIG_RENDER_PLATFORM_OPENGL_CORE = 1,
		CONFIG_RENDER_PLATFORM_OPENGL_ES   = 2,
		CONFIG_RENDER_PLATFORM_QT3D        = 8,
		CONFIG_RENDER_PLATFORM_VULKAN      = 16,
		CONFIG_RENDER_PLATFORM_DIRECTDRAW  = 24,
		CONFIG_RENDER_PLATFORM_SDLFB       = 25,
		CONFIG_RENDER_PLATFORM_DIRECTX     = 32,
	};
#endif

#define MAX_HISTORY	8

#ifdef USE_SHARED_DLL
	#define USE_CART_TMP		8
	#define USE_FLOPPY_DISK_TMP	16
	#define USE_QUICK_DISK_TMP	8
	#define USE_HARD_DISK_TMP	8
	#define USE_TAPE_TMP		8
	#define USE_COMPACT_DISC_TMP	8
	#define USE_LASER_DISC_TMP	8
	#define USE_BINARY_FILE_TMP	8
	#define USE_BUBBLE_TMP		16
	#define MAX_VOLUME_TMP		32
#else
	#define USE_CART_TMP		USE_CART
	#define USE_FLOPPY_DISK_TMP	USE_FLOPPY_DISK
	#define USE_QUICK_DISK_TMP	USE_QUICK_DISK
	#define USE_HARD_DISK_TMP	USE_HARD_DISK
	#define USE_TAPE_TMP		USE_TAPE
	#define USE_COMPACT_DISC_TMP	USE_COMPACT_DISC
	#define USE_LASER_DISC_TMP	USE_LASER_DISC
	#define USE_BINARY_FILE_TMP	USE_BINARY_FILE
	#define USE_BUBBLE_TMP		USE_BUBBLE
	#ifdef USE_SOUND_VOLUME
		#define MAX_VOLUME_TMP	USE_SOUND_VOLUME
	#endif
#endif

void DLL_PREFIX initialize_config();
void DLL_PREFIX load_config(const _TCHAR* config_path);
void DLL_PREFIX save_config(const _TCHAR* config_path);
bool DLL_PREFIX process_config_state(void *f, bool loading);

typedef struct {
	// control
	#if defined(USE_SHARED_DLL) || defined(USE_BOOT_MODE)
		int boot_mode;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_CPU_TYPE)
		int cpu_type;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_DIPSWITCH)
		uint32_t dipswitch;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_DEVICE_TYPE)
		int device_type;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_DRIVE_TYPE)
		int drive_type;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_KEYBOARD_TYPE)
		int keyboard_type;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_MOUSE_TYPE)
		int mouse_type;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_JOYSTICK_TYPE)
		int joystick_type;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_SOUND_TYPE)
		int sound_type;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_MONITOR_TYPE)
		int monitor_type;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_SCANLINE)
		bool scan_line;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_PRINTER_TYPE)
		int printer_type;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_FLOPPY_DISK)
		bool correct_disk_timing[/*USE_FLOPPY_DISK_TMP*/16];
		bool ignore_disk_crc[/*USE_FLOPPY_DISK_TMP*/16];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_TAPE)
		bool wave_shaper[USE_TAPE_TMP];
		bool direct_load_mzt[USE_TAPE_TMP];
		bool baud_high[USE_TAPE_TMP];
	#endif
	bool compress_state;
	int cpu_power;
	bool full_speed;
	
	// recent files
	#if defined(USE_SHARED_DLL) || defined(USE_CART)
		_TCHAR initial_cart_dir[_MAX_PATH];
		_TCHAR recent_cart_path[USE_CART_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_FLOPPY_DISK)
		_TCHAR initial_floppy_disk_dir[_MAX_PATH];
		_TCHAR recent_floppy_disk_path[USE_FLOPPY_DISK_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_QUICK_DISK)
		_TCHAR initial_quick_disk_dir[_MAX_PATH];
		_TCHAR recent_quick_disk_path[USE_QUICK_DISK_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_HARD_DISK)
		_TCHAR initial_hard_disk_dir[_MAX_PATH];
		_TCHAR recent_hard_disk_path[USE_HARD_DISK_TMP][MAX_HISTORY][_MAX_PATH];
		_TCHAR last_hard_disk_path[USE_HARD_DISK_TMP][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_TAPE)
		_TCHAR initial_tape_dir[_MAX_PATH];
		_TCHAR recent_tape_path[USE_TAPE_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_COMPACT_DISC)
		_TCHAR initial_compact_disc_dir[_MAX_PATH];
		_TCHAR recent_compact_disc_path[USE_COMPACT_DISC_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_LASER_DISC)
		_TCHAR initial_laser_disc_dir[_MAX_PATH];
		_TCHAR recent_laser_disc_path[USE_LASER_DISC_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_BINARY_FILE)
		_TCHAR initial_binary_dir[_MAX_PATH];
		_TCHAR recent_binary_path[USE_BINARY_FILE_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_BUBBLE)
		_TCHAR initial_bubble_casette_dir[_MAX_PATH];
		_TCHAR recent_bubble_casette_path[USE_BUBBLE_TMP][MAX_HISTORY][_MAX_PATH];
	#endif
	
	// screen
	int window_mode;
	int window_stretch_type;
	int fullscreen_stretch_type;
//	#if defined(USE_SHARED_DLL) || defined(USE_SCREEN_ROTATE)
		int rotate_type;
//	#endif
	
	// filter
	#if defined(USE_SHARED_DLL) || defined(USE_SCREEN_FILTER)
		int filter_type;
	#endif
	
	// sound
	int sound_frequency;
	int sound_latency;
	bool sound_strict_rendering;
	#if defined(USE_SHARED_DLL) || defined(USE_FLOPPY_DISK)
		bool sound_noise_fdd;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_TAPE)
		bool sound_noise_cmt;
		bool sound_play_tape;
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_SOUND_VOLUME)
		int sound_volume_l[MAX_VOLUME_TMP];
		int sound_volume_r[MAX_VOLUME_TMP];
	#endif
	#if defined(USE_SHARED_DLL) || defined(_WIN32)
		_TCHAR mame2151_dll_path[_MAX_PATH];
		_TCHAR mame2608_dll_path[_MAX_PATH];
	#endif
	
	// input
	#if defined(USE_SHARED_DLL) || defined(USE_JOYSTICK)
		int joy_buttons[8][16];
		bool use_joy_to_key;
		int joy_to_key_type;
		bool joy_to_key_numpad5;
		int joy_to_key_buttons[16];
	#endif
	#if defined(USE_SHARED_DLL) || defined(USE_AUTO_KEY)
		bool romaji_to_kana;
	#endif
	
	// printer
	#if defined(USE_SHARED_DLL) || defined(USE_PRINTER)
		_TCHAR printer_dll_path[_MAX_PATH];
	#endif
	
	// debug
	bool print_statistics;
	bool special_debug_fdc;
	
	// win32
	#if defined(USE_SHARED_DLL) || defined(_WIN32)
		bool use_d3d9;
		bool wait_vsync;
		bool use_dinput;
		bool disable_dwm;
		bool show_status_bar;
	#endif
	
	// qt
	#if defined(USE_SHARED_DLL) || defined(_USE_QT)
		bool use_opengl_scanline;
		bool opengl_scanline_vert;
		bool opengl_scanline_horiz;
		bool use_opengl_filters;
		int opengl_filter_num;
		bool swap_kanji_pause;
		/*
		 * TYPE : 
		 *    0 : OpenGL/Main Profile
		 *    1 : OpenGL/Core Profile
		 *    2 : OpenGL ES
		 *    8 : Qt3D(Will not implement)
		 *   16 : Vulkan (Will not implement)
		 *   24 : DirectDraw (Will not implement)
		 *   25 : SDLFB(Will not implement)
		 *   32 : DirectX(Will not implement)
		 */
		int render_platform;
		int render_major_version;
		int render_minor_version;
		int rendering_type;
	#endif
} config_t;

extern DLL_PREFIX config_t config;

#endif

