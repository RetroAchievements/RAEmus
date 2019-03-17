/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ common header ]
*/

#ifndef _VM_H_
#define _VM_H_

// GIJUTSU-HYORON-SHA Babbase-2nd
#ifdef _BABBAGE2ND
#include "babbage2nd/babbage2nd.h"
#endif

// HITACHI BASIC Master Jr
#ifdef _BMJR
#include "bmjr/bmjr.h"
#endif

// Systems Formulate BUBCOM80
#ifdef _BUBCOM80
#include "bubcom80/bubcom80.h"
#endif

// COLECO ColecoVision
#ifdef _COLECOVISION
#include "colecovision/colecovision.h"
#endif

// TOSHIBA EX-80
#ifdef _EX80
#include "ex80/ex80.h"
#endif

// Nintendo Family BASIC
#ifdef _FAMILYBASIC
#include "familybasic/familybasic.h"
#endif

// FUJITSU FM-8
#ifdef _FM8
#include "fm7/fm7.h"
#endif

// FUJITSU FM-7
#ifdef _FM7
#include "fm7/fm7.h"
#endif

// FUJITSU FM-NEW7
#ifdef _FMNEW7
#include "fm7/fm7.h"
#endif

// FUJITSU FM-77
#ifdef _FM77
#include "fm7/fm7.h"
#endif

// FUJITSU FM-77L2
#ifdef _FM77L2
#include "fm7/fm7.h"
#endif

// FUJITSU FM-77L4
#ifdef _FM77L4
#include "fm7/fm7.h"
#endif

// FUJITSU FM77AV
#ifdef _FM77AV
#include "fm7/fm7.h"
#endif

// FUJITSU FM77AV20
#ifdef _FM77AV20
#include "fm7/fm7.h"
#endif

// FUJITSU FM77AV40
#ifdef _FM77AV40
#include "fm7/fm7.h"
#endif

// FUJITSU FM77AV20EX
#ifdef _FM77AV20EX
#include "fm7/fm7.h"
#endif

// FUJITSU FM77AV40EX
#ifdef _FM77AV40EX
#include "fm7/fm7.h"
#endif

// FUJITSU FM77AV40SX
#ifdef _FM77AV40SX
#include "fm7/fm7.h"
#endif

// FUJITSU FM16beta
#ifdef _FM16BETA
#include "fm16beta/fm16beta.h"
#endif

// FUJITSU FM16pi
#ifdef _FM16PI
#include "fm16pi/fm16pi.h"
#endif

// FUJITSU FMR-30
#ifdef _FMR30
#include "fmr30/fmr30.h"
#endif

// FUJITSU FMR-50
#ifdef _FMR50
#include "fmr50/fmr50.h"
#endif

// FUJITSU FMR-60
#ifdef _FMR60
#include "fmr50/fmr50.h"
#endif

// CASIO FP-200
#ifdef _FP200
#include "fp200/fp200.h"
#endif

// CASIO FP-1100
#ifdef _FP1100
#include "fp1100/fp1100.h"
#endif

// Panasonic FS-A1
#ifdef _FSA1
#include "msx/msx_ex.h"
#endif

// SEGA GAME GEAR
#ifdef _GAMEGEAR
#include "gamegear/gamegear.h"
#endif

// EPSON HC-20
#ifdef _HC20
#include "hc20/hc20.h"
#endif

// EPSON HC-40
#ifdef _HC40
#include "hc40/hc40.h"
#endif

// EPSON HC-80
#ifdef _HC80
#include "hc80/hc80.h"
#endif

// TOSHIBA HX-20 + FDD
#ifdef _HX20
#include "msx/msx_ex.h"
#endif

// TOSHIBA J-3100GT
#ifdef _J3100GT
#include "j3100/j3100.h"
#endif

// TOSHIBA J-3100SL
#ifdef _J3100SL
#include "j3100/j3100.h"
#endif

// National JR-100
#ifdef _JR100
#include "jr100/jr100.h"
#endif

// National JR-200
#ifdef _JR200
#include "jr200/jr200.h"
#endif

// National JR-800
#ifdef _JR800
#include "jr800/jr800.h"
#endif

// IBM Japan Ltd PC/JX
#ifdef _JX
#include "jx/jx.h"
#endif

// SORD m5
#ifdef _M5
#include "m5/m5.h"
#endif

// SEIKO MAP-1010
#ifdef _MAP1010
#include "phc25/phc25.h"
#endif

// SEGA MASTER SYSTEM
#ifdef _MASTERSYSTEM
#include "gamegear/mastersystem.h"
#endif

// ASCII MSX
#ifdef _MSX1
//#include "msx/msx.h"
#include "msx/msx_ex.h"
#endif

// ASCII MSX2
#ifdef _MSX2
//#include "msx/msx.h"
#include "msx/msx_ex.h"
#endif

// ASCII MSX2+
#ifdef _MSX2P
#include "msx/msx_ex.h"
#endif

// MITSUBISHI Elec. MULTI8
#ifdef _MULTI8
#include "multi8/multi8.h"
#endif

// Japan Electronics College MYCOMZ-80A
#ifdef _MYCOMZ80A
#include "mycomz80a/mycomz80a.h"
#endif

// SHARP MZ-80A
#ifdef _MZ80A
#include "mz80k/mz80k.h"
#endif

// SHARP MZ-80B
#ifdef _MZ80B
#include "mz2500/mz80b.h"
#endif

// SHARP MZ-80K
#ifdef _MZ80K
#include "mz80k/mz80k.h"
#endif

// SHARP MZ-700
#ifdef _MZ700
#include "mz700/mz700.h"
#endif

// SHARP MZ-800
#ifdef _MZ800
#include "mz700/mz700.h"
#endif

// SHARP MZ-1200
#ifdef _MZ1200
#include "mz80k/mz80k.h"
#endif

// SHARP MZ-1500
#ifdef _MZ1500
#include "mz700/mz700.h"
#endif

// SHARP MZ-2200
#ifdef _MZ2200
#include "mz2500/mz80b.h"
#endif

// SHARP MZ-2500
#ifdef _MZ2500
#include "mz2500/mz2500.h"
#endif

// SHARP MZ-2800
#ifdef _MZ2800
#include "mz2800/mz2800.h"
#endif

// SHARP MZ-3500
#ifdef _MZ3500
#include "mz3500/mz3500.h"
#endif

// SHARP MZ-5500
#ifdef _MZ5500
#include "mz5500/mz5500.h"
#endif

// SHARP MZ-6500
#ifdef _MZ6500
#include "mz5500/mz5500.h"
#endif

// SHARP MZ-6550
#ifdef _MZ6550
#include "mz5500/mz5500.h"
#endif

// NEC N5200
#ifdef _N5200
#include "n5200/n5200.h"
#endif

// TOSHIBA PASOPIA
#ifdef _PASOPIA
#include "pasopia/pasopia.h"
#endif

// TOSHIBA PASOPIA 7
#ifdef _PASOPIA7
#include "pasopia7/pasopia7.h"
#endif

// NEC PC-2001
#ifdef _PC2001
#include "pc2001/pc2001.h"
#endif

// NEC PC-6001
#ifdef _PC6001
#include "pc6001/pc6001.h"
#endif

// NEC PC-6001mkII
#ifdef _PC6001MK2
#include "pc6001/pc6001.h"
#endif

// NEC PC-6001mkIISR
#ifdef _PC6001MK2SR
#include "pc6001/pc6001.h"
#endif

// NEC PC-6601
#ifdef _PC6601
#include "pc6001/pc6001.h"
#endif

// NEC PC-6601SR
#ifdef _PC6601SR
#include "pc6001/pc6001.h"
#endif

// NEC PC-8001
#ifdef _PC8001
#include "pc8801/pc8801.h"
#endif

// NEC PC-8001mkII
#ifdef _PC8001MK2
#include "pc8801/pc8801.h"
#endif

// NEC PC-8001mkIISR
#ifdef _PC8001SR
#include "pc8801/pc8801.h"
#endif

// NEC PC-8201
#ifdef _PC8201
#include "pc8201/pc8201.h"
#endif

// NEC PC-8201A
#ifdef _PC8201A
#include "pc8201/pc8201.h"
#endif

// NEC PC-8801
#ifdef _PC8801
#include "pc8801/pc8801.h"
#endif

// NEC PC-8801mkII
#ifdef _PC8801MK2
#include "pc8801/pc8801.h"
#endif

// NEC PC-8801MA
#ifdef _PC8801MA
#include "pc8801/pc8801.h"
#endif

// NEC PC-9801
#ifdef _PC9801
#include "pc9801/pc9801.h"
#endif

// NEC PC-9801E/F/M
#ifdef _PC9801E
#include "pc9801/pc9801.h"
#endif

// NEC PC-9801U
#ifdef _PC9801U
#include "pc9801/pc9801.h"
#endif

// NEC PC-9801VF
#ifdef _PC9801VF
#include "pc9801/pc9801.h"
#endif

// NEC PC-9801VM
#ifdef _PC9801VM
#include "pc9801/pc9801.h"
#endif

// NEC PC-9801VX
#ifdef _PC9801VX
#include "pc9801/pc9801.h"
#endif

// NEC PC-9801RA
#ifdef _PC9801RA
#include "pc9801/pc9801.h"
#endif

// NEC PC-98DO
#ifdef _PC98DO
#include "pc9801/pc9801.h"
#endif

// NEC PC-98HA
#ifdef _PC98HA
#include "pc98ha/pc98ha.h"
#endif

// NEC PC-98LT
#ifdef _PC98LT
#include "pc98ha/pc98ha.h"
#endif

// NEC PC-98RL
#ifdef _PC98RL
#include "pc9801/pc9801.h"
#endif

// NEC PC-98XA
#ifdef _PC98XA
#include "pc9801/pc9801.h"
#endif

// NEC PC-98XL
#ifdef _PC98XL
#include "pc9801/pc9801.h"
#endif

// NEC PC-100
#ifdef _PC100
#include "pc100/pc100.h"
#endif

// NEC-HE PC Engine
#ifdef _PCENGINE
#include "pcengine/pcengine.h"
#endif

// SANYO PHC-20
#ifdef _PHC20
#include "phc20/phc20.h"
#endif

// SANYO PHC-25
#ifdef _PHC25
#include "phc25/phc25.h"
#endif

// CASIO PV-1000
#ifdef _PV1000
#include "pv1000/pv1000.h"
#endif

// CASIO PV-2000
#ifdef _PV2000
#include "pv2000/pv2000.h"
#endif

// PIONEER PX-7
#ifdef _PX7
//#include "msx/msx.h"
#include "msx/msx_ex.h"
#endif

// TOMY PYUTA
#ifdef _PYUTA
#include "pyuta/pyuta.h"
#endif

// EPSON QC-10
#ifdef _QC10
#include "qc10/qc10.h"
#endif

// BANDAI RX-78
#ifdef _RX78
#include "rx78/rx78.h"
#endif

// SEGA SC-3000
#ifdef _SC3000
#include "sc3000/sc3000.h"
#endif

// EPOCH Super Cassette Vision
#ifdef _SCV
#include "scv/scv.h"
#endif

// SHARP SM-B-80TE
#ifdef _SMB80TE
#include "smb80te/smb80te.h"
#endif

// SONY SMC-70
#ifdef _SMC70
#include "smc777/smc777.h"
#endif

// SONY SMC-777
#ifdef _SMC777
#include "smc777/smc777.h"
#endif

// NEC TK-80BS (COMPO BS/80)
#ifdef _TK80BS
#include "tk80bs/tk80bs.h"
#endif

// NEC TK-80
#ifdef _TK80
#include "tk80bs/tk80bs.h"
#endif

// NEC TK-85
#ifdef _TK85
#include "tk80bs/tk80bs.h"
#endif

// CANON X-07
#ifdef _X07
#include "x07/x07.h"
#endif

// SHARP X1
#ifdef _X1
#include "x1/x1.h"
#endif

// SHARP X1twin
#ifdef _X1TWIN
#include "x1/x1.h"
#endif

// SHARP X1turbo
#ifdef _X1TURBO
#include "x1/x1.h"
#endif

// SHARP X1turboZ
#ifdef _X1TURBOZ
#include "x1/x1.h"
#endif

// Yuasa Kyouiku System YALKY
#ifdef _YALKY
#include "yalky/yalky.h"
#endif

// YAMAHA YIS
#ifdef _YIS
#include "yis/yis.h"
#endif

// SHINKO SANGYO YS-6464A
#ifdef _YS6464A
#include "ys6464a/ys6464a.h"
#endif

// Homebrew Z80 TV GAME SYSTEM
#ifdef _Z80TVGAME
#include "z80tvgame/z80tvgame.h"
#endif

#ifndef WINDOW_MODE_BASE
	#define WINDOW_MODE_BASE 1
#endif
#ifndef WINDOW_WIDTH
	#ifdef SCREEN_FAKE_WIDTH
		#define WINDOW_WIDTH SCREEN_FAKE_WIDTH
	#else
		#define WINDOW_WIDTH SCREEN_WIDTH
	#endif
#endif
#ifndef WINDOW_HEIGHT
	#ifdef SCREEN_FAKE_HEIGHT
		#define WINDOW_HEIGHT SCREEN_FAKE_HEIGHT
	#else
		#define WINDOW_HEIGHT SCREEN_HEIGHT
	#endif
#endif
#ifndef WINDOW_WIDTH_ASPECT
	#define WINDOW_WIDTH_ASPECT WINDOW_WIDTH
#endif
#ifndef WINDOW_HEIGHT_ASPECT
	#define WINDOW_HEIGHT_ASPECT WINDOW_HEIGHT
#endif

#if defined(USE_CART) && !defined(BASE_CART_NUM)
	#define BASE_CART_NUM		1
#endif
#if defined(USE_FLOPPY_DISK) && !defined(BASE_FLOPPY_DISK_NUM)
	#define BASE_FLOPPY_DISK_NUM	1
#endif
#if defined(USE_QUICK_DISK) && !defined(BASE_QUICK_DISK_NUM)
	#define BASE_QUICK_DISK_NUM	1
#endif
#if defined(USE_HARD_DISK) && !defined(BASE_HARD_DISK_NUM)
	#define BASE_HARD_DISK_NUM	1
#endif
#if defined(USE_TAPE) && !defined(BASE_TAPE_NUM)
	#define BASE_TAPE_NUM		1
#endif
#if defined(USE_COMPACT_DISC) && !defined(BASE_COMPACT_DISC_NUM)
	#define BASE_COMPACT_DISC_NUM	1
#endif
#if defined(USE_LASER_DISC) && !defined(BASE_LASER_DISC_NUM)
	#define BASE_LASER_DISC_NUM	1
#endif
#if defined(USE_BINARY_FILE) && !defined(BASE_BINARY_FILE_NUM)
	#define BASE_BINARY_FILE_NUM	1
#endif
#if defined(USE_BUBBLE) && !defined(BASE_BUBBLE_NUM)
	#define BASE_BUBBLE_NUM		1
#endif

#ifndef KEY_KEEP_FRAMES
	#define KEY_KEEP_FRAMES 3
#endif

#endif
