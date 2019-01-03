/************************************************************************/
/*									*/
/* PC8801 メインシステム(本体側)					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "quasi88.h"
#include "initval.h"
#include "pc88main.h"

#include "pc88cpu.h"
#include "crtcdmac.h"
#include "screen.h"
#include "intr.h"
#include "keyboard.h"
#include "memory.h"
#include "pio.h"
#include "soundbd.h"
#include "fdc.h"		/* disk_ex_drv */

#include "event.h"
#include "emu.h"
#include "drive.h"
#include "snddrv.h"
#include "suspend.h"
#include "status.h"



static	OSD_FILE *fp_so = NULL;		/* シリアル出力用fp		*/
static	OSD_FILE *fp_si = NULL;		/*	   入力用fp		*/
static	OSD_FILE *fp_to = NULL;		/* テープ出力用  fp		*/
static	OSD_FILE *fp_ti = NULL;		/*       入力用  fp		*/
static	OSD_FILE *fp_prn= NULL;		/* プリンタ出力用fp		*/




int	boot_basic     =DEFAULT_BASIC;	/* 起動時の BASICモード		*/
int	boot_dipsw     =DEFAULT_DIPSW;	/* 起動時のディップ設定		*/
int	boot_from_rom  =DEFAULT_BOOT;	/* 起動デバイスの設定		*/
int	boot_clock_4mhz=DEFAULT_CLOCK;	/* 起動時の CPUクロック		*/

int	monitor_15k    =0x00;		/* 15k モニター 2:Yes 0:No	*/

z80arch	z80main_cpu;			/* Z80 CPU ( main system )	*/

int	high_mode;			/* 高速モード 1:Yes 0:No	*/


static	byte	dipsw_1;		/* IN[30] ディップスイッチ 1	*/
static	byte	dipsw_2;		/* IN[31] ディップスイッチ 2	*/
static	byte	ctrl_boot;		/* IN[40] ディスクブート情報	*/
static	byte	cpu_clock;		/* IN[6E] CPU クロック		*/

int	memory_bank;			/* OUT[5C-5F] IN[5C] メモリバンク*/

static	byte	common_out_data;	/* OUT[10] PRT/時計		*/
byte	misc_ctrl;			/* I/O[32] 各種Ctrl		*/
byte	ALU1_ctrl;			/* OUT[34] ALU Ctrl 1		*/
byte	ALU2_ctrl;			/* OUT[35] ALU Ctrl 2		*/
byte	ctrl_signal;			/* OUT[40] コントロール信号出力値保存*/
byte	baudrate_sw = DEFAULT_BAUDRATE;	/* I/O[6F] ボーレート		*/
word	window_offset;			/* I/O[70] WINDOW オフセット	*/
byte	ext_rom_bank;			/* I/O[71] 拡張ROM BANK		*/
byte	ext_ram_ctrl;			/* I/O[E2] 拡張RAM制御		*/
byte	ext_ram_bank;			/* I/O[E3] 拡張RAMセレクト	*/

static	pair	kanji1_addr;		/* OUT[E8-E9] 漢字ROM(第1) ADDR	*/
static	pair	kanji2_addr;		/* OUT[EC-ED] 漢字ROM(第2) ADDR	*/

byte	jisho_rom_bank;			/* OUT[F0] 辞書ROMセレクト	*/
byte	jisho_rom_ctrl;			/* OUT[F1] 辞書ROMバンク	*/


int	calendar_stop = FALSE;		/* 時計停止フラグ		*/
static	char	calendar_data[7] =	/* 時計停止時刻 (年月日曜時分秒)*/
{ 85, 0, 1, 0, 0, 0, 0, };


int	cmt_speed = 0;			/* テープ速度(BPS)、 0は自動	*/
int	cmt_intr  = TRUE;		/* 真で、テープ読込に割込使用	*/
int	cmt_wait  = TRUE;		/* 真で、テープ読込ウェイトあり	*/


int	highspeed_mode = FALSE;		/* 真で、高速 BASIC 処理あり 	*/


int	use_siomouse = FALSE;		/* 真で、シリアルマウスあり	*/


/* 以下はテープイメージのファイル依存情報なので、ステートセーブしない */

static	int	cmt_is_t88;		/* 真…T88、偽…CMT		*/
static	int	cmt_block_size;		/* データタグ内のサイズ(T88)	*/
static	long	cmt_size;		/* イメージのサイズ		*/
static	int	cmt_EOF = FALSE;	/* 真で、テープ入力 EOF   	*/
static	int	com_EOF = FALSE;	/* 真で、シリアル入力 EOF 	*/
static	long	com_size;		/* イメージのサイズ		*/


static byte sio_in_data( void );	/* IN[20] RS232C入力 (データ)	*/
static byte sio_in_status( void );	/* IN[21] RS232C入力 (制御)	*/
static byte in_ctrl_signal( void );	/* IN[40] コントロール信号入力	*/

static void sio_out_data( byte );	/* OUT[20] RS232C出力 (データ)	*/
static void sio_out_command( byte );	/* OUT[21] RS232C出力 (コマンド)*/
static void out_ctrl_signal( byte );	/* OUT[40] コントロール信号出力	*/

static void sio_tape_highspeed_load( void );
static void sio_set_intr_base( void );
static void sio_check_cmt_error( void );

#define	sio_tape_readable()	(fp_ti && !cmt_EOF)	/* テープ読込可？   */
#define	sio_tape_writable()	(fp_to)			/* テープ書込可？   */
#define	sio_serial_readable()	(fp_si && !com_EOF)	/* シリアル読込可？ */
#define	sio_serial_writable()	(fp_so)			/* シリアル書込可？ */


/************************************************************************/
/* メモリウェイト							*/
/************************************************************************/
/*
 * まだちゃんと対応するのは先になりそうですが、とりあえず
 *
 *	低速モードでの M1 ウェイト (フェッチ毎に 1ステート? )
 *	低速モードでの DMAのウェイト ( 1バイト9ステート? を VSYNC毎? )
 *	高速モードで、高速RAMでの M1 ウェイト (フェッチ毎に 1ステート? )
 *	サブシステムでの M1 ウェイト (フェッチ毎に 1ステート? )
 *
 * あたりを適当に入れてみることにします。
 */

#define	DMA_WAIT	(9)

static	int	mem_wait_highram = FALSE;


/************************************************************************/
/* PCG-8100								*/
/************************************************************************/
static	int	pcg_data;
static	int	pcg_addr;

static	void	pcg_out_data( byte data )
{
  pcg_data = data;
}

static	void	pcg_out_addr_low( byte addr )
{
  pcg_addr = (pcg_addr & 0xff00) | addr;
}

static	void	pcg_out_addr_high( byte addr )
{
  byte src;

  pcg_addr = (pcg_addr & 0x00ff) | ((int)addr << 8);

  if( addr & 0x10 ){	/* exec */

    if( addr & 0x20 ){ src = font_mem[ 0x400 + (pcg_addr&0x3ff) ]; } /* copy */
    else             { src = pcg_data; }			    /* store */

    font_pcg[ 0x400 + (pcg_addr&0x3ff) ] = src;
  }
}



/************************************************************************/
/* 高速 BASIC モード							*/
/************************************************************************/
/*
 * 高速 BASIC 処理は、peach氏により提供されました。
 */

static	word	ret_addr = 0xffff;
static	int	hs_icount = 0;

	int highspeed_flag = FALSE;	/* 現在、高速BASIC 処理中	*/
static	int highspeed_n88rom = FALSE;	/* MAIN-ROM バンク選択時、真	*/
					/* (この時、高速BASIC 処理可能)	*/

/* 高速 BASIC モードに入るときのアドレス (BIOS依存かも?) */
word highspeed_routine[] = {
    0x6e9a,			/* PSET   */
    0x6eae,			/* LINE   */
    0x6eca,			/* ROLL   */
    0x6ece,			/* CIRCLE */
    0x6eda,			/* PAINT  */
    0x7198,			/* GET@   */
    0x71a6,			/* PUT@   */
    EndofBasicAddr
};


/************************************************************************/
/* メモリアクセス							*/
/*					special thanks	笠松健一さん	*/
/*							peach さん	*/
/************************************************************************/
/*
   メインメモリはバンク切り替えによって、以下のようにマッピングされる。

   0000	+------++------+				+------+ +------+
	|      ||      |				|      | |      |+
	|      ||      |				|      | |      ||
	|      || MAIN |				|N-    | | EXT  ||
	|      || ROM  |				| BASIC| | RAM  ||
	|      ||      |				|  ROM | |      ||
   6000	+      ++      ++------++------++------++------++      + | (x4) ||
	|      ||      ||Ext.0 ||Ext.1 ||Ext.2 ||Ext.3 ||      | |      ||
   8000	+ MAIN ++------++------++------++------++------++------+ +------+|
	| RAM  ||Window|                                          +------+
   8400	+      ++------+
	|      |
	|      |
   C000	+      +	+------++------++------+                 +------+
	|      |	|      ||      ||      |		 |      |+
	|      |	| VRAM || VRAM || VRAM |		 | 辞書 ||
   F000	+      ++------+|   B  ||   R  ||   G  |		 | ROM  ||
	|      || High ||      ||      ||      |		 | (x32)||
   FFFF	+------++------++------++------++------+		 +------+|
								  +------+
   つまり、大きく分けると、以下の6つのエリアに分けられる。

	0000H〜5FFFH	MAIN RAM / MAIN ROM / N-BASIC ROM / 拡張RAM
	6000H〜7FFFH	MAIN RAM / MAIN ROM / 拡張ROM / N-BASIC ROM / 拡張RAM
	8000H〜83FFH	MAIN RAM / ウインドウ
	8400H〜BFFFH	MAIN RAM
	C000H〜EFFFH	MAIN RAM / VRAM / 辞書ROM
	F000H〜FFFFH	MAIN RAM / 高速RAM / VRAM / 辞書ROM

   バンク切り替えを行なった時に、各々のエリアがどのバンクに割り当てられたのか
   をチェックし、実際のメモリアクセスはその割り当て情報により行なう。


   注)
   Hモードにおいては、 0xf000 〜 0xffff 番地を以下のようにエミュレートする。

	高速RAMは    main_ram[ 0xf000 〜 0xffff ]      を使う
	メインRAMは  main_high_ram[ 0x0000 〜 0x0fff ] を使う

   これにより、テキスト表示処理は常に main_ram を参照すればよいことになる。
*/


static	byte	*read_mem_0000_5fff;	/* メインメモリ リードポインタ	*/
static	byte	*read_mem_6000_7fff;
static	byte	*read_mem_8000_83ff;
static	byte	*read_mem_c000_efff;
static	byte	*read_mem_f000_ffff;

static	byte	*write_mem_0000_7fff;	/* メインメモリ ライトポインタ	*/
static	byte	*write_mem_8000_83ff;
static	byte	*write_mem_c000_efff;
static	byte	*write_mem_f000_ffff;

/*------------------------------------------------------*/
/* address : 0x0000 〜 0x7fff の メモリ割り当て		*/
/*		ext_ram_ctrl, ext_ram_bank, grph_ctrl,	*/
/*		ext_rom_bank, misc_ctrl により変化	*/
/*------------------------------------------------------*/
#if 1
INLINE	void	main_memory_mapping_0000_7fff( void )
{
  highspeed_n88rom = FALSE;	/* デフォルト */

  switch( ext_ram_ctrl ){

  case 0x00:					/* 拡張RAM RW不可 */
    if( grph_ctrl&GRPH_CTRL_64RAM ){			/* 64KB RAM mode */
      read_mem_0000_5fff  = &main_ram[ 0x0000 ];
      read_mem_6000_7fff  = &main_ram[ 0x6000 ];
      write_mem_0000_7fff = &main_ram[ 0x0000 ];
    }else{						/* ROM/RAM mode */
      if( grph_ctrl&GRPH_CTRL_N ){				/* N BASIC */
	read_mem_0000_5fff = &main_rom_n[ 0x0000 ];
	read_mem_6000_7fff = &main_rom_n[ 0x6000 ];
      }else{							/*N88 BASIC*/
	read_mem_0000_5fff = &main_rom[ 0x0000 ];
	if( ext_rom_bank&EXT_ROM_NOT ){				/* 通常ROM */
	  read_mem_6000_7fff = &main_rom[ 0x6000 ];
	  highspeed_n88rom = TRUE;
	}else{							/* 拡張ROM */
	  read_mem_6000_7fff = &main_rom_ext[ misc_ctrl&MISC_CTRL_EBANK ][0];
	}
      }
      write_mem_0000_7fff = &main_ram[ 0x0000 ];
    }
    break;

  case 0x01:					/* 拡張RAM R可 W不可 */
    if( ext_ram_bank < use_extram*4 ){
      read_mem_0000_5fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
      read_mem_6000_7fff = &ext_ram[ ext_ram_bank ][ 0x6000 ];
    }else{
      read_mem_0000_5fff = dummy_rom;
      read_mem_6000_7fff = dummy_rom;
    }
    write_mem_0000_7fff = &main_ram[ 0x0000 ];
    break;

  case 0x10:					/* 拡張RAM R不可 W可  */
		/* buf fix by peach (thanks!) */
    if( grph_ctrl&GRPH_CTRL_64RAM ){			/* 64KB RAM mode */
      read_mem_0000_5fff  = &main_ram[ 0x0000 ];
      read_mem_6000_7fff  = &main_ram[ 0x6000 ];
    }else{						/* ROM/RAM mode */
      if( grph_ctrl&GRPH_CTRL_N ){				/* N BASIC */
	read_mem_0000_5fff = &main_rom_n[ 0x0000 ];
	read_mem_6000_7fff = &main_rom_n[ 0x6000 ];
      }else{							/*N88 BASIC*/
	read_mem_0000_5fff = &main_rom[ 0x0000 ];
	if( ext_rom_bank&EXT_ROM_NOT ){				/* 通常ROM */
	  read_mem_6000_7fff = &main_rom[ 0x6000 ];
	  highspeed_n88rom = TRUE;
	}else{							/* 拡張ROM */
	  read_mem_6000_7fff = &main_rom_ext[ misc_ctrl&MISC_CTRL_EBANK ][0];
	}
      }
    }
    if( ext_ram_bank < use_extram*4 ){
      write_mem_0000_7fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
    }else{
      write_mem_0000_7fff = dummy_ram;
    }
    break;

  case 0x11:					/* 拡張RAM RW可 */
    if( ext_ram_bank < use_extram*4 ){
      read_mem_0000_5fff  = &ext_ram[ ext_ram_bank ][ 0x0000 ];
      read_mem_6000_7fff  = &ext_ram[ ext_ram_bank ][ 0x6000 ];
      write_mem_0000_7fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
    }else{
      read_mem_0000_5fff  = dummy_rom;
      read_mem_6000_7fff  = dummy_rom;
      write_mem_0000_7fff = dummy_ram;
    }
    break;
  }
}

#else	/* こう、すっきりさせるほうがいい？ */

INLINE	void	main_memory_mapping_0000_7fff( void )
{
  highspeed_n88rom = FALSE;	/* デフォルト */

	/* リードは、指定したバンクに応じたメモリから */

				/* buf fix by peach (thanks!) */
  if( grph_ctrl&GRPH_CTRL_64RAM ){			/* 64KB RAM mode */
    read_mem_0000_5fff  = &main_ram[ 0x0000 ];
    read_mem_6000_7fff  = &main_ram[ 0x6000 ];
  }else{						/* ROM/RAM mode */
    if( grph_ctrl&GRPH_CTRL_N ){				/* N BASIC */
      read_mem_0000_5fff = &main_rom_n[ 0x0000 ];
      read_mem_6000_7fff = &main_rom_n[ 0x6000 ];
    }else{							/*N88 BASIC*/
      read_mem_0000_5fff = &main_rom[ 0x0000 ];
      if( ext_rom_bank&EXT_ROM_NOT ){				/* 通常ROM */
	read_mem_6000_7fff = &main_rom[ 0x6000 ];
	highspeed_n88rom = TRUE;
      }else{							/* 拡張ROM */
	read_mem_6000_7fff = &main_rom_ext[ misc_ctrl&MISC_CTRL_EBANK ][0];
      }
    }
  }

	/* ライトは、常にメインRAM へ */

  write_mem_0000_7fff = &main_ram[ 0x0000 ];



	/* 拡張RAMへのアクセス指定があれば、拡張RAMをリード・ライトする */

  if( ext_ram_ctrl & 0x01 ){				/* 拡張RAM R可 */
    if( ext_ram_bank < use_extram*4 ){
      read_mem_0000_5fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
      read_mem_6000_7fff = &ext_ram[ ext_ram_bank ][ 0x6000 ];
    }
  }

  if( ext_ram_ctrl & 0x10 ){				/* 拡張RAM W可 */
    if( ext_ram_bank < use_extram*4 ){
      write_mem_0000_7fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
    }
  }
}
#endif


/*------------------------------------------------------*/
/* address : 0x8000 〜 0x83ff の メモリ割り当て		*/
/*		grph_ctrl, window_offset により変化	*/
/*------------------------------------------------------*/
INLINE	void	main_memory_mapping_8000_83ff( void )
{
  if( grph_ctrl & ( GRPH_CTRL_64RAM | GRPH_CTRL_N ) ){
    read_mem_8000_83ff  = &main_ram[ 0x8000 ];
    write_mem_8000_83ff = &main_ram[ 0x8000 ];
  }else{
    if( high_mode ){
      if( window_offset <= 0xf000 - 0x400 ){
	read_mem_8000_83ff  = &main_ram[ window_offset ];
	write_mem_8000_83ff = &main_ram[ window_offset ];
      }else if( 0xf000 <= window_offset && window_offset <= 0x10000 - 0x400 ){
	read_mem_8000_83ff  = &main_high_ram[ window_offset - 0xf000 ];
	write_mem_8000_83ff = &main_high_ram[ window_offset - 0xf000 ];
      }else{
	read_mem_8000_83ff  = NULL;
	write_mem_8000_83ff = NULL;
      }
    }else{
      read_mem_8000_83ff  = &main_ram[ window_offset ];
      write_mem_8000_83ff = &main_ram[ window_offset ];
    }
  }
}


/*------------------------------------------------------*/
/* address : 0xc000 〜 0xffff の メモリ割り当て		*/
/*		jisho_rom_ctrl, jisho_rom_bank, 	*/
/*		misc_ctrl により変化			*/
/*------------------------------------------------------*/
INLINE	void	main_memory_mapping_c000_ffff( void )
{
  mem_wait_highram = FALSE;

  if( jisho_rom_ctrl ){
    read_mem_c000_efff = &main_ram[ 0xc000 ];
    if( high_mode && (misc_ctrl&MISC_CTRL_TEXT_MAIN) ){
      read_mem_f000_ffff = &main_high_ram[ 0x0000 ];
      mem_wait_highram = TRUE;
    }else{
      read_mem_f000_ffff = &main_ram[ 0xf000 ];
    }
  }else{
    read_mem_c000_efff = &jisho_rom[ jisho_rom_bank ][ 0x0000 ];
    read_mem_f000_ffff = &jisho_rom[ jisho_rom_bank ][ 0x3000 ];
  }

  write_mem_c000_efff = &main_ram[ 0xc000 ];
  if( high_mode && (misc_ctrl&MISC_CTRL_TEXT_MAIN) ){
    write_mem_f000_ffff = &main_high_ram[ 0x0000 ];
    mem_wait_highram = TRUE;
  }else{
    write_mem_f000_ffff = &main_ram[ 0xf000 ];
  }
}


/*------------------------------------------------------*/
/* address : 0xc000 〜 0xffff の メイン←→VARM切り替え	*/
/*		misc_ctrl, ALU2_ctrl,			*/
/*		memory_bank により変化			*/
/*------------------------------------------------------*/
static	int	vram_access_way;	/* vram アクセスの方法	*/
enum VramAccessWay{
  VRAM_ACCESS_BANK,
  VRAM_ACCESS_ALU,
  VRAM_NOT_ACCESS,
  EndofVramAcc
};

INLINE	void	main_memory_vram_mapping( void )
{
  if( misc_ctrl & MISC_CTRL_EVRAM ){		/* 拡張アクセスモード */

    /* ワードラゴンで使用 (port 35H の方はいらないかも…) by peach */
    memory_bank = MEMORY_BANK_MAIN;

    if( ALU2_ctrl & ALU2_CTRL_VACCESS ){		/* VRAM拡張アクセス */
      vram_access_way = VRAM_ACCESS_ALU;
    }else{						/* MAIN RAMアクセス */
      vram_access_way = VRAM_NOT_ACCESS;
    }
  }else{					/* 独立アクセスモード */
    if( memory_bank == MEMORY_BANK_MAIN ){		/* MAIN RAMアクセス */
      vram_access_way = VRAM_NOT_ACCESS;
    }else{						/* VRAMアクセス     */
      vram_access_way = VRAM_ACCESS_BANK;
    }
  }
}




/*------------------------------*/
/* 通常のＶＲＡＭリード		*/
/*------------------------------*/
INLINE	byte	vram_read( word addr )
{
  return main_vram[addr][ memory_bank ];
}

/*------------------------------*/
/* 通常のＶＲＡＭライト		*/
/*------------------------------*/
INLINE	void	vram_write( word addr, byte data )
{
  screen_set_dirty_flag(addr);

  main_vram[addr][ memory_bank ] = data;
}

/*------------------------------*/
/* ＡＬＵを介したＶＲＡＭリード	*/
/*------------------------------*/
typedef	union {
  bit8		c[4];
  bit32		l;
} ALU_memory;

static	ALU_memory	ALU_buf;
static	ALU_memory	ALU_comp;

#ifdef LSB_FIRST
#define	set_ALU_comp()						\
	do{							\
	  ALU_comp.l = 0;					\
	  if( (ALU2_ctrl&0x01)==0 ) ALU_comp.l |= 0x000000ff;	\
	  if( (ALU2_ctrl&0x02)==0 ) ALU_comp.l |= 0x0000ff00;	\
	  if( (ALU2_ctrl&0x04)==0 ) ALU_comp.l |= 0x00ff0000;	\
	}while(0)
#else
#define	set_ALU_comp()						\
	do{							\
	  ALU_comp.l = 0;					\
	  if( (ALU2_ctrl&0x01)==0 ) ALU_comp.l |= 0xff000000;	\
	  if( (ALU2_ctrl&0x02)==0 ) ALU_comp.l |= 0x00ff0000;	\
	  if( (ALU2_ctrl&0x04)==0 ) ALU_comp.l |= 0x0000ff00;	\
	}while(0)
#endif

INLINE	byte	ALU_read( word addr )
{
  ALU_memory	wk;

  ALU_buf.l  = (main_vram4)[addr];
  wk.l       = ALU_comp.l ^ ALU_buf.l;

  return  wk.c[0] & wk.c[1] & wk.c[2];
}

/*------------------------------*/
/* ＡＬＵを介したＶＲＡＭライト	*/
/*------------------------------*/
INLINE	void	ALU_write( word addr, byte data )
{
  int i, mode;

  screen_set_dirty_flag(addr);

  switch( ALU2_ctrl&ALU2_CTRL_MODE ){

  case 0x00:
    mode = ALU1_ctrl;
    for( i=0;  i<3;  i++, mode>>=1 ){
      switch( mode&0x11 ){
      case 0x00:  main_vram[addr][i] &= ~data;	break;
      case 0x01:  main_vram[addr][i] |=  data;	break;
      case 0x10:  main_vram[addr][i] ^=  data;	break;
      default:					break;
      }
    }
    break;

  case 0x10:
    (main_vram4)[addr] = ALU_buf.l;
    break;

  case 0x20:
    main_vram[addr][0] = ALU_buf.c[1];
    break;

  default:
    main_vram[addr][1] = ALU_buf.c[0];
    break;

  }
}


/*----------------------*/
/*    フェッチ		*/
/*----------------------*/

byte	main_fetch( word addr )
{

  /* かなり適当な、メモリウェイト処理 */

  if( memory_wait ){

    if( high_mode == FALSE ){		/* 低速モードの場合 */

      z80main_cpu.state0 += 1;			/* M1サイクルウェイト */

      if( dma_wait_count ){			/* DMAウェイトがあれば    */
	dma_wait_count --;			/* すこしずつ加算していく */
	z80main_cpu.state0 += DMA_WAIT;
      }

    }else{				/* 高速モードの場合 */

      if( addr>=0xf000 && mem_wait_highram ){	/* 高速RAMのフェッチは */
	z80main_cpu.state0 += 1;		/* M1サイクルウェイト  */
      }
    }

    /* VRAMアクセス時とか、8MHz時のウェイトもあるけど未実装 */
  }


  /* 高速 BASIC モード */		/* peach氏提供 */

  if (highspeed_mode){
    if (!(highspeed_flag) && highspeed_n88rom) {
      int i;
      for (i = 0; highspeed_routine[i] != EndofBasicAddr; i++) {
	if (addr == highspeed_routine[i]) {
	  highspeed_flag = TRUE;
	  ret_addr = main_mem_read(z80main_cpu.SP.W) +
	    	    (main_mem_read(z80main_cpu.SP.W + 1) << 8);
	  hs_icount= z80_state_intchk;

	  z80_state_intchk = HS_BASIC_COUNT*2;
	  /*printf("%x %d -> %d -> ",addr,hs_icount,z80_state_intchk);*/
	  break;
	}
      }
    } else if ((highspeed_flag) &&
	       (ret_addr == addr || z80main_cpu.state0 >= HS_BASIC_COUNT)) {
      ret_addr = 0xffff;
      /*printf("'%d'\n",z80_state_intchk);*/
      z80_state_intchk = hs_icount;
      if (z80main_cpu.state0 > z80_state_intchk) z80main_cpu.state0 = z80_state_intchk;
      highspeed_flag = FALSE;
    }
  }

  /* メモリリード */

  if     ( addr < 0x6000 ) return  read_mem_0000_5fff[ addr ];
  else if( addr < 0x8000 ) return  read_mem_6000_7fff[ addr & 0x1fff ];
  else if( addr < 0x8400 ){
    if( read_mem_8000_83ff ) return  read_mem_8000_83ff[ addr & 0x03ff ];
    else{
      addr = (addr & 0x03ff) + window_offset;
      if( addr < 0xf000 ) return  main_ram[ addr ];
      else                return  main_high_ram[ addr & 0x0fff ];
    }
  }
  else if( addr < 0xc000 ) return  main_ram[ addr ];
  else{
    switch( vram_access_way ){
    case VRAM_ACCESS_ALU:  return  ALU_read(  addr & 0x3fff );
    case VRAM_ACCESS_BANK: return  vram_read( addr & 0x3fff );
    default:
      if( addr < 0xf000 )  return  read_mem_c000_efff[ addr & 0x3fff ];
      else                 return  read_mem_f000_ffff[ addr & 0x0fff ];
    }
  }
}

/*----------------------*/
/*    メモリ・リード	*/
/*----------------------*/
byte	main_mem_read( word addr )
{
  if     ( addr < 0x6000 ) return  read_mem_0000_5fff[ addr ];
  else if( addr < 0x8000 ) return  read_mem_6000_7fff[ addr & 0x1fff ];
  else if( addr < 0x8400 ){
    if( read_mem_8000_83ff ) return  read_mem_8000_83ff[ addr & 0x03ff ];
    else{
      addr = (addr & 0x03ff) + window_offset;
      if( addr < 0xf000 ) return  main_ram[ addr ];
      else                return  main_high_ram[ addr & 0x0fff ];
    }
  }
  else if( addr < 0xc000 ) return  main_ram[ addr ];
  else{
    switch( vram_access_way ){
    case VRAM_ACCESS_ALU:  return  ALU_read(  addr & 0x3fff );
    case VRAM_ACCESS_BANK: return  vram_read( addr & 0x3fff );
    default:
      if( addr < 0xf000 )  return  read_mem_c000_efff[ addr & 0x3fff ];
      else                 return  read_mem_f000_ffff[ addr & 0x0fff ];
    }
  }
}

/*----------------------*/
/*     メモリ・ライト	*/
/*----------------------*/
void	main_mem_write( word addr, byte data )
{
  if     ( addr < 0x8000 ) write_mem_0000_7fff[ addr ]          = data;
  else if( addr < 0x8400 ){
    if( write_mem_8000_83ff ) write_mem_8000_83ff[ addr & 0x03ff ] = data;
    else{
      addr = (addr & 0x03ff) + window_offset;
      if( addr < 0xf000 ) main_ram[ addr ]               = data;
      else                main_high_ram[ addr & 0x0fff ] = data;
    }
  }
  else if( addr < 0xc000 ) main_ram[ addr ]                     = data;
  else{
    switch( vram_access_way ){
    case VRAM_ACCESS_ALU:  ALU_write( addr & 0x3fff, data );	break;
    case VRAM_ACCESS_BANK: vram_write( addr & 0x3fff, data );	break;
    default:
      if( addr < 0xf000 )  write_mem_c000_efff[ addr & 0x3fff ] = data;
      else                 write_mem_f000_ffff[ addr & 0x0fff ] = data;
    }
  }
}





/************************************************************************/
/* Ｉ／Ｏポートアクセス							*/
/************************************************************************/

/*----------------------*/
/*    ポート・ライト	*/
/*----------------------*/

void	main_io_out( byte port, byte data )
{
  byte chg;
  PC88_PALETTE_T new_pal;

  switch( port ){

	/* 高速テープロード / PCG */
  case 0x00:
    /*if( use_pcg )*/
      pcg_out_data( data );

    if( boot_basic != BASIC_N )
      sio_tape_highspeed_load();
    return;

  case 0x01:
    /*if( use_pcg )*/
      pcg_out_addr_low( data );
    return;

  case 0x02:
    /*if( use_pcg )*/
      pcg_out_addr_high( data );
    return;

  case 0x0c:
  case 0x0d:
  case 0x0e:
  case 0x0f:
    /* PCG のサウンド出力のポートらしい */
    return;


	/* プリンタ出力／カレンダクロック 出力データ */
  case 0x10:
    common_out_data = data;
    return;


	/* RS-232C／CMT 出力データ */
  case 0x20:
    sio_out_data( data );
    return;

	/* RS-232C／CMT 制御コマンド */
  case 0x21:
    sio_out_command( data );
    return;


	/* システムコントロール出力 */
  case 0x30:
    if( (sys_ctrl^data) & (SYS_CTRL_80) ){	/* SYS_CTRL_MONO は無視 */
      screen_set_dirty_all();			/* (テキストのカラーは  */
    }						/*  CRTC設定にて決定)   */

    if( sio_tape_readable() ){
      if( (sys_ctrl & 0x08) && !(data & 0x08) ) sio_check_cmt_error();
    }

    /* カセットモーターリレー音 */
    if ((~sys_ctrl &  data) & 0x08) { xmame_dev_sample_motoron();  }
    if (( sys_ctrl & ~data) & 0x08) { xmame_dev_sample_motoroff(); }

    sys_ctrl = data;
    sio_set_intr_base();
/*
printf("CMT %02x, %s: Motor %s: CDS %d\n",data,
       ((data&0x3)==0)?"  600":( ((data&0x30)==0x10)?" 1200":"RS232"),
       ((data&8)==0)?"Off":"On ",(data>>2)&1);
*/
    return;

	/* グラフィックコントロール出力 */
  case 0x31:
    chg = grph_ctrl ^ data;

		/* GRPH_CTRL_25 は無視 (テキスト25行は CRTC設定にて決定) */
    if( chg & (GRPH_CTRL_200|GRPH_CTRL_VDISP|GRPH_CTRL_COLOR) ){
      screen_set_dirty_all();

      if( chg & GRPH_CTRL_COLOR ){
	screen_set_dirty_palette();
      }
    }

    /* M88 ではこうなってる？ (peach) */
    /*if (chg & (GRPH_CTRL_64RAM|GRPH_CTRL_N))*/
    /*grph_ctrl = data & (GRPH_CTRL_64RAM|GRPH_CTRL_N);*/
    /*else grph_ctrl = data;*/

    grph_ctrl = data;
    set_text_display();
    main_memory_mapping_0000_7fff();
    main_memory_mapping_8000_83ff();
    return;

	/* 各種設定入出力 */
  case 0x32:
    chg = misc_ctrl ^ data;
    if( chg & MISC_CTRL_ANALOG ){
      screen_set_dirty_palette();
    }
    if( sound_port & SD_PORT_44_45 ){
      intr_sound_enable = (data & INTERRUPT_MASK_SOUND) ^ INTERRUPT_MASK_SOUND;
      if( highspeed_flag == FALSE ) CPU_REFRESH_INTERRUPT();
      /*if( intr_sound_enable == FALSE ) SOUND_flag = FALSE;*/
    }
    misc_ctrl = data;
    main_memory_mapping_0000_7fff();
    main_memory_mapping_c000_ffff();
    main_memory_vram_mapping();
    return;


	/* 拡張VRAM制御 */
  case 0x34:
    ALU1_ctrl = data;
    return;
  case 0x35:
    ALU2_ctrl = data;
    set_ALU_comp();

    /* クリムゾン３やワードラゴン,STAR TRADERなどで使用 */
    if (data & ALU2_CTRL_VACCESS) memory_bank = MEMORY_BANK_MAIN;
					/* bug fix by peach (thanks!) */

    main_memory_vram_mapping();
    return;


	/* コントロール信号出力 */
  case 0x40:
    out_ctrl_signal( data );
    return;


	/* サウンド出力 */
  case 0x44:
    if( sound_port & SD_PORT_44_45 ) sound_out_reg( data );
    return;
  case 0x45:
    if( sound_port & SD_PORT_44_45 ) sound_out_data( data );
    return;
  case 0x46:
    if( sound_port & SD_PORT_46_47 ) sound2_out_reg( data );
    return;
  case 0x47:
    if( sound_port & SD_PORT_46_47 ) sound2_out_data( data );
    return;


    	/* CRTC出力 */
  case 0x50:
    crtc_out_parameter( data );
/*printf("CRTC PARM %02x\n",data);*/
    return;
  case 0x51:
    crtc_out_command( data );
/*printf("CRTC CMD %02x\n",data);*/
    return;

	/* 背景色（デジタル）*/
  case 0x52:
    if( data&0x1 ) new_pal.blue  = 7;
    else           new_pal.blue  = 0;
    if( data&0x2 ) new_pal.red   = 7;
    else           new_pal.red   = 0;
    if( data&0x4 ) new_pal.green = 7;
    else           new_pal.green = 0;

    if( new_pal.blue  != vram_bg_palette.blue  ||
	new_pal.red   != vram_bg_palette.red   ||
        new_pal.green != vram_bg_palette.green ){
      vram_bg_palette.blue  = new_pal.blue;
      vram_bg_palette.red   = new_pal.red;
      vram_bg_palette.green = new_pal.green;
      screen_set_dirty_palette();
    }
    return;

	/* 画面重ね合わせ */
  case 0x53:
    grph_pile = data;
    set_text_display();
    screen_set_dirty_all();
    return;

	/* パレット設定 */
  case 0x54:
    if( (data & 0x80) &&
	(misc_ctrl & MISC_CTRL_ANALOG) ){	/* アナログモード */
      if( (data & 0x40) == 0 ){
	new_pal.blue  = (data     ) & 0x07;
	new_pal.red   = (data >> 3) & 0x07;
	new_pal.green = vram_bg_palette.green;
      }else{
	new_pal.blue  = vram_bg_palette.blue;
	new_pal.red   = vram_bg_palette.red;
	new_pal.green = (data     ) & 0x07;
      }
      if( new_pal.blue  != vram_bg_palette.blue  ||
	  new_pal.red   != vram_bg_palette.red   ||
          new_pal.green != vram_bg_palette.green ){
	vram_bg_palette.blue  = new_pal.blue;
	vram_bg_palette.red   = new_pal.red;
	vram_bg_palette.green = new_pal.green;
	screen_set_dirty_palette();
      }
      return;
    }	/* else no return; (.. continued) */
    /* FALLTHROUGH */
  case 0x55:
  case 0x56:
  case 0x57:
  case 0x58:
  case 0x59:
  case 0x5a:
  case 0x5b:
/*printf("PAL %02xH %02x\n",port,data );*/
    if( ! (misc_ctrl&MISC_CTRL_ANALOG) ){	/* デジタルモード */

      if( data&0x1 ) new_pal.blue  = 7;
      else           new_pal.blue  = 0;
      if( data&0x2 ) new_pal.red   = 7;
      else           new_pal.red   = 0;
      if( data&0x4 ) new_pal.green = 7;
      else           new_pal.green = 0;

    }else{					/* アナログモード */
      if( (data & 0x40) == 0 ){
	new_pal.blue  = (data      ) & 0x07;
	new_pal.red   = (data >> 3 ) & 0x07;
	new_pal.green = vram_palette[ port-0x54 ].green;
      }else{
	new_pal.green = (data      ) & 0x07;
	new_pal.red   = vram_palette[ port-0x54 ].red;
	new_pal.blue  = vram_palette[ port-0x54 ].blue;
      }
    }

    if( new_pal.blue  != vram_palette[ port-0x54 ].blue  ||
	new_pal.red   != vram_palette[ port-0x54 ].red   ||
	new_pal.green != vram_palette[ port-0x54 ].green ){
      vram_palette[ port-0x54 ].blue  = new_pal.blue;
      vram_palette[ port-0x54 ].red   = new_pal.red;
      vram_palette[ port-0x54 ].green = new_pal.green;
      screen_set_dirty_palette();
    }
    return;

    
	/* メモリバンク切替え */
  case 0x5c:
    memory_bank = MEMORY_BANK_GRAM0;
    main_memory_vram_mapping();
    return;
  case 0x5d:
    memory_bank = MEMORY_BANK_GRAM1;
    main_memory_vram_mapping();
    return;
  case 0x5e:
    memory_bank = MEMORY_BANK_GRAM2;
    main_memory_vram_mapping();
    return;
  case 0x5f:
    memory_bank = MEMORY_BANK_MAIN;
    main_memory_vram_mapping();
    return;

	/* DMAC出力 */

  case 0x60:
  case 0x62:
  case 0x64:
  case 0x66:
    dmac_out_address( (port-0x60)/2, data );
/*printf("DMAC %x ADDR %02x\n",(port-0x60)/2,data );*/
    return;
  case 0x61:
  case 0x63:
  case 0x65:
  case 0x67:
    dmac_out_counter( (port-0x61)/2, data );
/*printf("DMAC %x CNTR %02x\n",(port-0x61)/2,data );*/
    return;
  case 0x68:
    dmac_out_mode( data );
/*printf("DMAC MODE %02x\n",data );*/
    return;


	/* ボーレート */
  case 0x6f:
    if( ROM_VERSION >= '8' ) baudrate_sw = data;	/* FH/MH 以降に対応 */
    return;


	/* Window オフセットアドレス入出力 */
  case 0x70:
    window_offset = (word)data << 8;
    main_memory_mapping_8000_83ff();
    return;

	/* 拡張 ROM バンク */
  case 0x71:
    ext_rom_bank = data;
    main_memory_mapping_0000_7fff();
    return;

	/* Window オフセットアドレス インクリメント */

  case 0x78:
    window_offset += 0x100;
    main_memory_mapping_8000_83ff();
    return;



	/* サウンド出力(オプション) */
  case 0xa8:
    if( sound_port & SD_PORT_A8_AD ){
      sound_out_reg( data );
    }
    return;
  case 0xa9:
    if( sound_port & SD_PORT_A8_AD ){
      sound_out_data( data );
    }
    return;
  case 0xaa:
    if( sound_port & SD_PORT_A8_AD ){
      intr_sound_enable = (data & INTERRUPT_MASK_SOUND) ^ INTERRUPT_MASK_SOUND;
      if( highspeed_flag == FALSE ) CPU_REFRESH_INTERRUPT();
      /*if( intr_sound_enable == FALSE ) SOUND_flag = FALSE;*/
    }
    return;
  case 0xac:
    if( sound_port & SD_PORT_A8_AD ){
      sound2_out_reg( data );
    }
    return;
  case 0xad:
    if( sound_port & SD_PORT_A8_AD ){
      sound2_out_data( data );
    }
    return;


	/* 拡張 RAM 制御 */
  case 0xe2:
    if( use_extram ){
      ext_ram_ctrl = data & 0x11;
      main_memory_mapping_0000_7fff();
    }
    return;
  case 0xe3:
/*printf("OUT E3 <=  %02X\n",data);*/
    if( use_extram ){
      if( linear_ext_ram ){		/* 出力値の通りにバンクを割り振る */
	ext_ram_bank = data;
      }else{				/* 実機っぽく(?)バンクを割り振る */
	ext_ram_bank = 0xff;
	if (use_extram <= 4) {				/* 128KB*4以下 */
	    if ((data & 0x0f) < use_extram * 4) {
		ext_ram_bank = data & 0x0f;
	    }
	} else if (use_extram == 8) {			/* 1MB */
	    /* 設定 00-07h, 10-17h, 20-27h, 30-37h とする */
	    if ((data & 0xc8) == 0x00) {
		ext_ram_bank = ((data & 0x30) >> 1) | (data & 0x07);
	    }
	} else if (use_extram <= 10) {			/* 1MB + 128KB*2以下 */
	    /* 設定 08-0Fh, 18-1Fh, 28-2Fh, 38-3Fh とする */
	    if ((data & 0xc8) == 0x08) {
		ext_ram_bank = ((data & 0x30) >> 1) | (data & 0x07);
	    } else if ((data & 0x0f) < (use_extram - 8) * 4) {
		ext_ram_bank = (data & 0x0f) + 0x20;
	    }
	} else if (use_extram == 16) {			/* 2MB */
	    /* 設定 08-0Fh, 18-1Fh, 28-2Fh, 38-3Fh とする */
	    /* 設定 48-4Fh, 58-5Fh, 68-6Fh, 78-7Fh とする */
	    if ((data & 0x88) == 0x08) {
		ext_ram_bank = ((data & 0x70) >> 1) | (data & 0x07);
	    }
	}
      }
      main_memory_mapping_0000_7fff();
    }
    return;


	/* 割り込みレベルの設定 */
  case 0xe4:
    intr_priority = data & 0x08;
    if( intr_priority ) intr_level = 7;
    else                intr_level = data & 0x07;
    if( highspeed_flag == FALSE ){
      CPU_REFRESH_INTERRUPT();

      /* 'ASHE対策…… */	/* thanks! peach */
      z80main_cpu.skip_intr_chk = TRUE;
    }
    return;

	/* 割り込みマスク */
  case 0xe6:
    intr_sio_enable   = data & INTERRUPT_MASK_SIO;
    intr_vsync_enable = data & INTERRUPT_MASK_VSYNC;
    intr_rtc_enable   = data & INTERRUPT_MASK_RTC;

    if( intr_sio_enable   == FALSE ){RS232C_flag = FALSE; sio_data_clear(); }
    if( intr_vsync_enable == FALSE ) VSYNC_flag  = FALSE;
    if( intr_rtc_enable   == FALSE ) RTC_flag    = FALSE;

    if( highspeed_flag == FALSE ) CPU_REFRESH_INTERRUPT();
    return;


	/* 漢字ＲＯＭ アドレス設定 */
  case 0xe8:
    kanji1_addr.B.l = data;
    return;
  case 0xe9:
    kanji1_addr.B.h = data;
    return;

  case 0xea:
  case 0xeb:
    return;

  case 0xec:
    kanji2_addr.B.l = data;
    return;
  case 0xed:
    kanji2_addr.B.h = data;
    return;


	/* 辞書ROMの設定 */

  case 0xf0:
    if( use_jisho_rom ){
      jisho_rom_bank = data & JISHO_BANK;
      main_memory_mapping_c000_ffff();
    }
    return;
  case 0xf1:
    if( use_jisho_rom ){
      jisho_rom_ctrl = data & JISHO_NOT_SELECT; 
      main_memory_mapping_c000_ffff();
    }
    return;

	/* ＰＩＯ */

  case 0xfc:
    logpio(" %02x-->\n",data);
    pio_write_AB( PIO_SIDE_M, PIO_PORT_A, data );
    return;
  case 0xfd:
    logpio(" %02x==>\n",data);
    pio_write_AB( PIO_SIDE_M, PIO_PORT_B, data );
    return;
  case 0xfe:
    pio_write_C_direct( PIO_SIDE_M, data );
    return;
  case 0xff:
    if( data & 0x80 ) pio_set_mode( PIO_SIDE_M, data );
    else              pio_write_C( PIO_SIDE_M, data );
    return;




	/* その他のポート */

  case 0x90:  case 0x91:  case 0x92:  case 0x93:	/* CD-ROM */
  case 0x94:  case 0x95:  case 0x96:  case 0x97:
  case 0x98:  case 0x99:  case 0x9a:  case 0x9b:
  case 0x9c:  case 0x9d:  case 0x9e:  case 0x9f:

  case 0xa0:  case 0xa1:  case 0xa2:  case 0xa3:	/* MUSIC & NETWORK */

			  case 0xc2:  case 0xc3:	/* MUSIC */
  case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
  case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:
  case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:

  case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:	/* MUSIC & GP-IB*/
  case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
  case 0xd8:						/* GP-IB */

  case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:	/* MODEM */

  case 0xb4:  case 0xb5:				/* VIDEO ART */


  case 0xc1:				/* ??? Access in N88-BASIC ver 1.8 */
  case 0xf3:  case 0xf4:  case 0xf8:
  
  case 0xe7:				/* ??? Access in N-BASIC ver 1.8 */

    return;
  }


  if( verbose_io )printf("OUT data %02X to undecoeded port %02XH\n",data,port);

}

/*----------------------*/
/*    ポート・リード	*/
/*----------------------*/
byte	main_io_in( byte port )
{
  switch( port ){

	/* キーボード */
  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
  case 0x04:
  case 0x05:
  case 0x06:
  case 0x07:
  case 0x08:
  case 0x09:
  case 0x0a:
  case 0x0b:
  case 0x0c:
  case 0x0d:
  case 0x0e:
  case 0x0f:
    disk_ex_drv = 0;		/* キー入力でリセット */
#ifdef	USE_KEYBOARD_BUG				/* peach氏提供 */
    {
      int i;
      byte mkey, mkey_old;

      mkey = key_scan[port];
      do {
	mkey_old = mkey;
	for (i = 0; i < 0x10; i++) {
	  if (i != port && key_scan[i] != 0xff) {
	    /* [SHIFT],[CTRL],[GRAPH],[カナ]には適用しない */
	    if ((i == 0x08 && (mkey | key_scan[i] | 0xf0) != 0xff) ||
		(i != 0x08 && (mkey | key_scan[i])        != 0xff))
	      mkey &= key_scan[i];
	  }
	}
      } while (mkey_old != mkey);
      return(mkey);
    }
#else
    return key_scan[ port ];
#endif


	/* RS-232C／CMT 入力データ */
  case 0x20:
    return sio_in_data();


	/* RS-232C/CMT 制御 */
  case 0x21:
    return sio_in_status();


	/* ディップスイッチ入力 */

  case 0x30:
    return dipsw_1 | 0xc0;
  case 0x31:
    return dipsw_2;

	/* 各種設定入力 */
  case 0x32:
    return misc_ctrl;

	/* コントロール信号入力 */
  case 0x40:
    return in_ctrl_signal() | 0xc0 | 0x04;
 /* return in_ctrl_signal() | 0xc0;*/


	/* サウンド入力 */
	
  case 0x44:
    if( sound_port & SD_PORT_44_45 ) return sound_in_status( );
    else                             return 0xff;
  case 0x45:
    if( sound_port & SD_PORT_44_45 ) return sound_in_data( FALSE );
    else                             return 0xff;
  case 0x46:
    if( sound_port & SD_PORT_46_47 ) return sound2_in_status( );
    else                             return 0xff;
  case 0x47:
    if( sound_port & SD_PORT_46_47 ) return sound2_in_data( );
    else                             return 0xff;


    	/* CRTC入力 */
  case 0x50:
/*printf("READ CRTC parm\n");*/
    return crtc_in_parameter( );
  case 0x51:
/*printf("READ CRTC stat\n");*/
    return crtc_in_status( );


	/* メモリバンク */
  case 0x5c:
    return (1<<memory_bank) | 0xf8;


	/* DMAC入力 */

  case 0x60:
  case 0x62:
  case 0x64:
  case 0x66:
/*printf("READ DMAC addr\n");*/
    return dmac_in_address( (port-0x60)/2 );
  case 0x61:
  case 0x63:
  case 0x65:
  case 0x67:
/*printf("READ DMAC cntr\n");*/
    return dmac_in_counter( (port-0x61)/2 );
  case 0x68:
/*printf("READ DMAC stat\n");*/
    return dmac_in_status( );


	/* CPU クロック */
  case 0x6e:
    if( ROM_VERSION >= '8' ) return cpu_clock | 0x10;	/* FH/MH 以降に対応 */
    else		     return 0xff;


	/* ボーレート */
  case 0x6f:
    if( ROM_VERSION >= '8' ) return baudrate_sw | 0xf0;	/* FH/MH 以降に対応 */
    else		     return 0xff;


	/* Window オフセットアドレス入出力 */
  case 0x70:
    return window_offset >> 8;


	/* 拡張 ROM バンク */
  case 0x71:
    return ext_rom_bank;


	/* 拡張 RAM 制御 */
  case 0xe2:
    if( use_extram ) return ~ext_ram_ctrl | 0xee;
    return 0xff;
  case 0xe3:
    if( linear_ext_ram ){		/* 出力値の通りにバンクを割り振った */
      if( use_extram &&
	(ext_ram_bank < use_extram*4) ) return ext_ram_bank;
      return 0xff;
    }else{				/* 実機っぽく(?)バンクを割り振った */
	byte ret = 0xff;
	if (use_extram && (ext_ram_bank != 0xff)) {
	    if (use_extram <= 4) {			/* 128KB*4以下 */
		ret = (ext_ram_bank | 0xf0);
	    } else if (use_extram == 8) {		/* 1MB */
		/* 設定 00-07h, 10-17h, 20-27h, 30-37h とする */
		if (ext_ram_bank < 8) {
		    ret = (ext_ram_bank | 0xf0);
		} else {
		    ret = ((ext_ram_bank & 0x18) << 1) | (ext_ram_bank & 0x07);
		}
	    } else if (use_extram <= 10) {		/* 1MB + 128KB*2以下 */
		/* 設定 08-0Fh, 18-1Fh, 28-2Fh, 38-3Fh とする */
		if (ext_ram_bank < 0x20) {
		    ret = ((ext_ram_bank & 0x18) << 1) | 0x08 |
							 (ext_ram_bank & 0x07);
		} else {
		    ret = ((ext_ram_bank - 0x20) | 0xf0);
		}
	    } else if (use_extram == 16) {		/* 2MB */
		/* 設定 08-0Fh, 18-1Fh, 28-2Fh, 38-3Fh とする */
		/* 設定 48-4Fh, 58-5Fh, 68-6Fh, 78-7Fh とする */
		ret = ((ext_ram_bank & 0x38) << 1) | 0x08 |
							(ext_ram_bank & 0x07);
	    }
	}
/*printf("IN E3 -----> %02X\n",ret);*/
	return ret;
    }


	/* サウンド入力(オプション) */
  case 0xa8:
    if( sound_port & SD_PORT_A8_AD ) return sound_in_status( );
    else                             return 0xff;
  case 0xa9:
    if( sound_port & SD_PORT_A8_AD ) return sound_in_data( TRUE );
    else                             return 0xff;
  case 0xaa:
    if( sound_port & SD_PORT_A8_AD ) return intr_sound_enable | 0x7f;
    else                             return 0xff;
  case 0xac:
    if( sound_port & SD_PORT_A8_AD ) return sound2_in_status( );
    else                             return 0xff;
  case 0xad:
    if( sound_port & SD_PORT_A8_AD ) return sound2_in_data( );
    else                             return 0xff;



	/* 漢字ＲＯＭ フォント入力 */
  case 0xe8:
    return kanji_rom[0][kanji1_addr.W][1];
  case 0xe9:
    return kanji_rom[0][kanji1_addr.W][0];

  case 0xec:
    return kanji_rom[1][kanji2_addr.W][1];
  case 0xed:
    return kanji_rom[1][kanji2_addr.W][0];



	/* ＰＩＯ */

  case 0xfc:
    {
      byte data = pio_read_AB( PIO_SIDE_M, PIO_PORT_A );
      logpio(" %02x<--\n",data);
/*      {
	static byte debug_pio_halt[4] = { 0,0,0,0 };
	debug_pio_halt[0] = debug_pio_halt[1];
	debug_pio_halt[1] = debug_pio_halt[2];
	debug_pio_halt[2] = debug_pio_halt[3];
	debug_pio_halt[3] = data;
	if(debug_pio_halt[0]==0x20&&
	   debug_pio_halt[1]==0x3d&&
	   debug_pio_halt[2]==0x02&&
	   debug_pio_halt[3]==0x00) emu_mode=MONITOR;
      }*/
      return data;
    }
  case 0xfd:
    {
      byte data = pio_read_AB( PIO_SIDE_M, PIO_PORT_B );
      logpio(" %02x<==\n",data);
      return data;
    }
  case 0xfe:
    return pio_read_C( PIO_SIDE_M );




	/* その他のポート */

  case 0x90:  case 0x91:  case 0x92:  case 0x93:	/* CD-ROM */
  case 0x94:  case 0x95:  case 0x96:  case 0x97:
  case 0x98:  case 0x99:  case 0x9a:  case 0x9b:
  case 0x9c:  case 0x9d:  case 0x9e:  case 0x9f:

  case 0xa0:  case 0xa1:  case 0xa2:  case 0xa3:	/* MUSIC & NETWORK */

			  case 0xc2:  case 0xc3:	/* MUSIC */
  case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
  case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:
  case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:

  case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:	/* MUSIC & GP-IB*/
  case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
  case 0xd8:						/* GP-IB */

  case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:	/* MODEM */

  case 0xb4:  case 0xb5:				/* VIDEO ART */


  case 0xc1:				/* ??? Access in N88-BASIC ver 1.8 */
  case 0xf3:  case 0xf4:  case 0xf8:
  
    return 0xff;
#if 0
  case 0xf4:				/* ??? */
    return 0xff;
  case 0xf8:				/* ??? */
    return 0xff;
#endif
  }


  if( verbose_io )printf("IN        from undecoeded port %02XH\n",port);

  return 0xff;
}






/*===========================================================================*/
/* シリアルポート							     */
/*===========================================================================*/

static	int	sio_instruction;		/* USART のコマンド状態 */
static	byte	sio_mode;			/* USART の設定モード   */
static	byte	sio_command;			/* USART のコマンド	*/
static	int	sio_data_exist;			/* 読込未の SIOデータ有 */
static	byte	sio_data;			/* SIOデータ            */

static	int	com_X_flow = FALSE;		/* 真で、Xフロー制御中	*/

static	int	cmt_dummy_read_cnt = 0;		/* 割込未使用時のダミー	*/
static	int	cmt_skip;			/* 無効データ部の読み飛ばし */
static	int	cmt_skip_data;			/* 読み飛ばし直後のデータ   */

static	long	cmt_read_chars = 0;		/* 読み込んだ実バイト数	*/
static	long	cmt_stateload_chars = 0;	/* ステートロード時読込数*/
static	int	cmt_stateload_skip = 0;		/* ステートロード時 skip */

static	int	sio_getc( int is_cmt, int *tick );

void	sio_data_clear(void)
{
    sio_data_exist = FALSE;
}


/*-------- ロード用テープイメージファイルを "rb" で開く --------*/

int	sio_open_tapeload( const char *filename )
{
  sio_close_tapeload();

  if( (fp_ti = osd_fopen( FTYPE_TAPE_LOAD, filename, "rb" )) ){

    sio_set_intr_base();
    return sio_tape_rewind();

  }else{
    if (quasi88_is_menu() == FALSE)
      printf("\n[[[ %s : Tape load image can't open ]]]\n\n", filename );
  }
  cmt_stateload_chars = 0;
  return FALSE;
}
void	sio_close_tapeload( void )
{
  if( fp_ti ){ osd_fclose( fp_ti ); fp_ti = NULL; }
  sio_set_intr_base();

  cmt_read_chars = 0;
}

/*-------- セーブ用テープイメージファイルを "ab" で開く --------*/

int	sio_open_tapesave( const char *filename )
{
  sio_close_tapesave();

  if( (fp_to = osd_fopen( FTYPE_TAPE_SAVE, filename, "ab" )) ){

    return TRUE;

  }else{
    if (quasi88_is_menu() == FALSE)
      printf("\n[[[ %s : Tape save image can't open ]]]\n\n", filename );
  }
  return FALSE;
}
void	sio_close_tapesave( void )
{
  if( fp_to ){ osd_fclose( fp_to ); fp_to = NULL; }
}

/*-------- シリアル入力用のファイルを "rb" で開く --------*/

int	sio_open_serialin( const char *filename )
{
  sio_close_serialin();

  if( (fp_si = osd_fopen( FTYPE_COM_LOAD, filename, "rb" )) ){

    sio_set_intr_base();
    com_EOF = FALSE;

    if( osd_fseek( fp_si, 0, SEEK_END ) ) goto ERR;
    if( (com_size = osd_ftell( fp_si )) < 0 ) goto ERR;
    if( osd_fseek( fp_si, 0, SEEK_SET ) ) goto ERR;

    return TRUE;
  }
 ERR:
  if( fp_si ){
      printf("\n[[[ Serial input image access error ]]]\n\n" );
  }else{
    if (quasi88_is_menu() == FALSE)
      printf("\n[[[ %s : Serial input file can't open ]]]\n\n", filename );
  }
  sio_close_serialin();

  return FALSE;
}
void	sio_close_serialin( void )
{
  if( fp_si ){ osd_fclose( fp_si ); fp_si = NULL; }
  sio_set_intr_base();
  /* com_X_flow = FALSE; */
}

/*-------- シリアル出力用のファイルを "ab" で開く --------*/

int	sio_open_serialout( const char *filename )
{
  sio_close_serialout();

  if( (fp_so = osd_fopen( FTYPE_COM_SAVE, filename, "ab" )) ){

    return TRUE;

  }else{
    if (quasi88_is_menu() == FALSE)
      printf("\n[[[ %s : Serial output file can't open ]]]\n\n", filename );
  }
  return FALSE;
}
void	sio_close_serialout( void )
{
  if( fp_so ){ osd_fclose( fp_so ); fp_so = NULL; }
}

/*-------- シリアルマウスを初期化/終了する --------*/

void	sio_mouse_init(int initial)
{
  if (initial) {
    init_serial_mouse_data();
  }
  sio_set_intr_base();
}

/*-------- 開いているテープイメージを巻き戻す --------*/

#define T88_HEADER_STR		"PC-8801 Tape Image(T88)"
int	sio_tape_rewind( void )
{
  int size;
  char buf[ sizeof(T88_HEADER_STR) ];

  cmt_read_chars = 0;

  if( fp_ti ){
    if( osd_fseek( fp_ti, 0, SEEK_END ) ) goto ERR;
    if( (cmt_size = osd_ftell( fp_ti )) < 0 ) goto ERR;

    if( osd_fseek( fp_ti, 0, SEEK_SET ) ) goto ERR;

    size = osd_fread( buf, sizeof(char), sizeof(buf), fp_ti );
    if( size == sizeof(buf) &&
	memcmp( buf, T88_HEADER_STR, sizeof(buf) ) == 0 ){	/* T88 */
      cmt_is_t88     = TRUE;
      cmt_block_size = 0;
      cmt_EOF        = FALSE;
      cmt_skip       = 0;
    }else{							/* CMT */
      cmt_is_t88     = FALSE;
      cmt_EOF        = FALSE;
      cmt_skip       = 0;
      if( osd_fseek( fp_ti, 0, SEEK_SET ) ) goto ERR;
    }

    while( cmt_stateload_chars -- ){	/* ステートロード時は、テープ早送り */
      if( sio_getc( TRUE, NULL ) == EOF ){
	break;
      }
    }
    cmt_skip = cmt_stateload_skip;
    cmt_stateload_chars = 0;

/*printf("%d\n",osd_ftell(fp_ti));*/
    return TRUE;
  }

 ERR:
  if( fp_ti ){
    printf("\n[[[ Tape image access error ]]]\n\n" );
  }
  sio_close_tapeload();

  cmt_stateload_chars = 0;
  return FALSE;
}

/*-------- 開いているテープの現在位置を返す (何%読んだかの確認用) --------*/

int	sio_tape_pos( long *cur, long *end )
{
  long v;

  if( fp_ti ){
    if( cmt_EOF ){		/* 終端なら、位置=0/終端=0 にし、真を返す */
      *cur = 0;
      *end = 0;
      return TRUE;
    }else{			/* 途中なら、位置と終端をセットし真を返す */
      v = osd_ftell( fp_ti );
      if( v >= 0 ){
	*cur = v;
	*end = cmt_size;
	return TRUE;
      }
    }
  }
  *cur = 0;			/* 不明時は、位置=0/終端=0 にし、偽を返す */
  *end = 0;
  return FALSE;
}

int	sio_com_pos( long *cur, long *end )
{
  long v;

  if( fp_si ){
    if( com_EOF ){		/* 終端なら、位置=0/終端=0 にし、真を返す */
      *cur = 0;
      *end = 0;
      return TRUE;
    }else{			/* 途中なら、位置と終端をセットし真を返す */
      v = osd_ftell( fp_si );
      if( v >= 0 ){
	*cur = v;
	*end = com_size;
	return TRUE;
      }
    }
  }
  *cur = 0;			/* 不明時は、位置=0/終端=0 にし、偽を返す */
  *end = 0;
  return FALSE;
}





/*
 * 開いているsioイメージから1文字読み込む 
 */
static	int	sio_getc( int is_cmt, int *tick )
{
  int i, c, id, size, time;

  if( tick ) *tick = 0;

  if( is_cmt==FALSE ){			/* シリアル入力 */

    if (use_siomouse) {
      c = get_serial_mouse_data();
      return c;
    }

    if( fp_si==NULL ) return EOF;
    if( com_EOF )     return EOF;

    c = osd_fgetc( fp_si );
    if( c==EOF ){
      printf(" (( %s : Serial input file EOF ))\n", file_sin );
      status_message( 1, STATUS_WARN_TIME, "Serial input  [EOF]" );
      com_EOF = TRUE;
    }
    return c;

  }else{				/* テープ入力 */

    if( fp_ti==NULL ) return EOF;
    if( cmt_EOF )     return EOF;

    if( cmt_is_t88 == FALSE ){			/* CMT形式の場合 */

      c = osd_fgetc( fp_ti );

    }else{					/* T88形式の場合 */

      while( cmt_block_size == 0 ){

	if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	id = c;
	if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	id += c << 8;

	if( id==0x0000 ){				/* 終了タグ */
	  c = EOF; break;
	}
	else {
	  if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	  size = c;
	  if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	  size += c << 8;

	  if( id == 0x0101 ){				/* データタグ */

	    if( size < 12 ){ c = EOF; break; }

	    for( i=0; i<12; i++ ){	/* 情報は全て無視 */
	      if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	    }
	    if( c==EOF ) break;
	    cmt_block_size = size - 12;

	  }else{					

	    if( id == 0x0100 ||				/* ブランクタグ */
		id == 0x0102 ||				/* スペースタグ */
		id == 0x0103 ){				/* マークタグ   */

	      if( size != 8 ){ c = EOF; break; }

	      for( i=0; i<4; i++ ){	/* 開始時間は無視 */
		if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	      }
	      if( c==EOF ) break;
					/* 長さ時間は取得 */
	      if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	      time = c;
	      if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	      time += c << 8;
	      if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	      time += c << 16;
	      if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	      time += c << 24;

	      if( tick ) *tick += time;

	    }else{					/* 他のタグ(無視) */

	      for( i=0; i<size; i++ ){
		if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	      }
	      if( c==EOF ) break;

	    }
	  }
	}
      }

      cmt_block_size --;
      c = osd_fgetc( fp_ti );
    }

    if( c==EOF ){
      cmt_EOF = TRUE;
      status_message( 1, STATUS_WARN_TIME, "Tape Read  [EOF]");
    }else{
      cmt_read_chars ++;
    }

    return c;
  }
}

/* 
 * テープの読み込み途中にモータOFFされたら、読み込みエラー発生として
 * 1バイト読み飛ばす。データの途中かどうかは T88 でないとチェックできない。
 * こんなチェック、必要なのか？？
 */
static	void	sio_check_cmt_error( void )
{
  int c;
  if( sio_tape_readable() ){
    if( cmt_is_t88     &&	/* T88 かつ、データタグの途中の時のみ */
	cmt_skip == 0  &&
	cmt_block_size ){
      cmt_block_size --;
      c = osd_fgetc( fp_ti );

      if( verbose_proc )
	printf( "Tape read: lost 1 byte\n" );

      if( c==EOF ){
	cmt_EOF = TRUE;
	status_message(1, STATUS_WARN_TIME, "Tape Read  [EOF]");
      }else{
	cmt_read_chars ++;
      }
    }
  }
}






/*
 * 開いているsioイメージに1文字書き込む 
 */
static	int	sio_putc( int is_cmt, int c )
{
  OSD_FILE *fp;

  if( is_cmt==FALSE ){ fp = fp_so; }	/* シリアル出力 */
  else               { fp = fp_to; }	/* テープ出力 */
  
  if( fp ){
    osd_fputc( c, fp );
    osd_fflush( fp );
  }
  return c;
}


/*
 * 高速テープロード …… 詳細不明。こんな機能あったのか… 
 */
static	void	sio_tape_highspeed_load( void )
{
  int c, sum, addr, size;

  if( sio_tape_readable()==FALSE ) return;

			  /* マシン語ヘッダを探す */

  do{						/* 0x3a が出てくるまでリード */
    if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
  } while( c != 0x3a );
						/* 転送先アドレス H */
  if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
  sum = c;
  addr = c * 256;
						/* 転送先アドレス L */
  if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
  sum += c;
  addr += c;
						/* ヘッダ部サム */
  if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
  sum += c;
  if( (sum&0xff) != 0 ){ return; }


		/* あとはデータ部の繰り返し */

  while( TRUE ){

    do{						/* 0x3a が出てくるまでリード */
      if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
    } while( c != 0x3a );

						/* データ数 */
    if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
    sum  = c;
    size = c;
    if( c==0 ){						/* データ数==0で終端 */
      return;
    }

    for( ; size; size -- ){			/* データ数分、転送 */

      if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
      sum += c;
      main_mem_write( addr, c );
      addr ++;
    }
						/* データ部サム */
    if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
    sum += c;
    if( (sum&0xff) != 0 ){ return; }
  }
}



/*
 * RS232C割り込み周期のセット 
 */
				/* これらの変数は、設定から算出する */
static int sio_bps;		/* BPS */
static int sio_framesize;	/* StartBit + Bit長 + StopBit を適当に計算 */

static void sio_set_intr_base( void )
{
  static const int table[] = {
    75, 150, 300, 600, 1200, 2400, 4800, 9600, 19200,
  };

  /* イメージファイルセット済み かつ、
     受信 Enable かつ、
     RS232C (I/O 30h:bit5=ON) か、CMTかつモータON (I/O 30h:bit5=OFF,bit3=ON)
     の時に、周期的に割り込みを発生させる。その周期を計算する。 */

  if( (fp_si || fp_ti || use_siomouse) &&
      (sio_command & 0x04) &&
      ( (sys_ctrl & 0x20) || (sys_ctrl & 0x08) ) ){

    if( sys_ctrl & 0x20 ){		/* RS232C 指定時  */

      sio_bps = table[ baudrate_sw ];		/* BPS は ボーレート設定 */
      sio_framesize = 10;			/* フレーム長は10bit固定 */

    }else{				/* CMT 指定時 */

      if( cmt_speed == 0 ){			/* 通常は、*/
	if( sys_ctrl & 0x10 ) sio_bps = 1200;	/* I/O 30h:bit4=1 で 1200bps */
	else                  sio_bps =  600;	/*             =0 で  600bps */
      }else{
	sio_bps = cmt_speed;			/* 引数指定時はその値 */
      }
      sio_framesize = 11;			/* フレーム長は11bit固定 */
    }

  }else{				/* シリアル不可時 */
    sio_bps = 0;
    sio_framesize = 0;
  }

  interval_work_set_RS232C( sio_bps, sio_framesize );  
}


/*
 * T88 フォーマット の TICK時間を シリアル割り込み回数に換算する
 *
 * ブランク、スペース、マークタグは、1/4800s単位の絶対時間が記述してある。
 * なので、時間が n の場合、n/4800秒ウェイトをいれるといい感じになるはず。
 * つまり、 ( n / 4800 ) * CPU_CLOCK / rs232c_intr_base 回分、
 * RS232C割り込みを余分に待てばいい。この式は、置き換えると
 *	     ( n / 4800 ) * ( bps / framesize )
 * になる。まあいずれにせよかなり適当なウェイトではあるが。
*/
static	int	tick_2_intr_skip( int tick )
{
  if( sio_framesize == 0 ) return 0;

  return (tick * sio_bps / 4800 / sio_framesize) + 1;
}



/*
 *
 */
static	void	sio_init( void )
{
  sio_instruction = 0;
  sio_command     = 0;
  sio_mode        = 0;
  sio_data_exist  = FALSE;
  sio_data        = 0;		/* 初期値は0。とあるゲームの救済のため… ;_; */

  com_X_flow      = FALSE;

  cmt_dummy_read_cnt = 0;
}
/*
 *
 */
static	void	sio_out_command( byte data )
{
  if( sio_instruction==0 ){			/* 内部リセット直後は、 */
    sio_mode        = data;				/* モード受付け */
    sio_instruction = 1;

  }else{					/* それ以外はコマンド受付け */

    if( data & 0x40 ){					/* 内部リセット */
      sio_mode        = 0;
      sio_instruction = 0;
      sio_command     = 0x40;
      sio_data        = 0;
    }else{						/* その他       */
      sio_command     = data;
    }

    if( (sio_command & 0x04) == 0 ){			/* リセットor受信禁止*/
      sio_data_exist = FALSE;				/* なら、受信ワーク  */
      RS232C_flag   = FALSE;				/* をクリアする      */
    }

    sio_set_intr_base();
  }
/*printf("SIO %02x -> mode:%02x cmd:%02x\n",data,sio_mode,sio_command);*/
}
/*
 *
 */
static	void	sio_out_data( byte data )
{
  int is_cmt;

  if( (sio_command & 0x01) ){		/* 送信イネーブル */
    if( sys_ctrl & 0x20 ){			/* シリアル出力の場合 */
      is_cmt = FALSE;
      if     ( data==0x11 ){				/* ^Q 出力 */
	com_X_flow = FALSE;
      }else if( data==0x13 ){				/* ^S 出力 */
	com_X_flow = TRUE;
      }
    }else{					/* テープ出力の場合 */
      is_cmt = TRUE;
    }
    sio_putc( is_cmt, data );
  }
}
/*
 *
 */
static	byte	sio_in_data( void )
{
/*printf("->%02x ",sio_data);fflush(stdout);*/
  sio_data_exist = FALSE;
  RS232C_flag = FALSE;
  return sio_data;
}
/*
 *
 */
static	byte	sio_in_status( void )
{
  int c;
  byte	status = 0x80 | 0x04;		/* 送信バッファエンプティ */
                /* DSR| TxE */

  if( sio_command & 0x04 ){		/* 現在、受信イネーブルの場合 */

    if( (sys_ctrl & 0x20)==0 && 		/* テープで、SIO割り込みを  */
	sio_tape_readable() &&			/* 使わない場合、ここで読む */
	cmt_intr == FALSE ){

      cmt_dummy_read_cnt ++;			/* IN 21 を 2回実行する度に */
      if( cmt_dummy_read_cnt >= 2 ){
	cmt_dummy_read_cnt = 0;

	c = sio_getc( TRUE, 0 );		/* テープから1文字読む */
/*printf("[%03x]",c&0xfff);fflush(stdout);*/
	if( c != EOF ){
	  sio_data = (byte)c;
	  sio_data_exist = TRUE;
	}
      }
    }

    if( sio_data_exist ){			/* データがあれば */
      status |= 0x02;					/* 受信レディ */
             /* RxRDY */
    }
  }

  if( sio_command & 0x01 ){		/* 現在、送信イネーブルの場合 */
    if(( (sys_ctrl & 0x20) /*&& sio_serial_writable()*/ ) ||
       (!(sys_ctrl & 0x20)   && sio_tape_writable()     ) ){
      status |= 0x01;				/* 送信レディ */
             /* TxRDY */
    }
  }

  return	status;
}
/*
 *
 */
static	void	sio_term( void )
{
}



/*
 * RS-232C 受信割り込み処理
 */
int	sio_intr( void )
{
  int c = EOF;
  int tick;

  if( (sio_command & 0x04) &&		/* 現在、受信イネーブルで   */
      ! sio_data_exist ){		/* 読込未のデータがない場合 */

    if( sys_ctrl & 0x20 ){			/* シリアル入力 */

      if( com_X_flow ) return FALSE;
      c = sio_getc( FALSE, 0 );

    }else{					/* テープ入力(割込使用時のみ)*/
      if( cmt_intr ){

	if( cmt_skip==0 ){
	  if( cmt_wait ){
	    c = sio_getc( TRUE, &tick );
	    if( tick ){
	      cmt_skip = tick_2_intr_skip( tick );
	      if( cmt_skip!=0  ){
		cmt_skip_data = c;
		c = EOF;
	      }
	    }
	  }else{
	    c = sio_getc( TRUE, 0 );
	  }
	}else{						/* T88の場合は、    */
	  cmt_skip --;					/* 無効データ部分の */
	  if( cmt_skip==0 ){				/* 間、時間潰しする */
	    c = cmt_skip_data;
	  }
	}
      }
    }

    if( c!=EOF ){
      sio_data = (byte)c;
      sio_data_exist = TRUE;
/*printf("<%02x> ",sio_data);fflush(stdout);*/
      return TRUE;				/* RxRDY割り込み発生 */
    }
  }
  return FALSE;
}



/*
 *	状態チェック関数
 */
int	tape_exist( void )
{
  return (fp_ti || fp_to);
}

int	tape_readable( void )
{
  return (fp_ti) ? TRUE : FALSE;
}

int	tape_writable( void )
{
  return (fp_to) ? TRUE : FALSE;
}

int	tape_reading( void )
{
  return( fp_ti &&
	  (sio_command & 4) &&
	  ((sys_ctrl & 0x28) == 0x08) );
}

int	tape_writing( void )
{
  return( fp_to &&
	  (sio_command & 1) &&
	  ((sys_ctrl & 0x28) == 0x08) );
}



/*===========================================================================*/
/* パラレルポート							     */
/*===========================================================================*/

int	printer_open( const char *filename )
{
  printer_close();

  if( (fp_prn = osd_fopen( FTYPE_PRN, filename, "ab") ) ){

    return TRUE;

  }else{
    if (quasi88_is_menu() == FALSE)
      printf("\n[[[ %s : Printer output file can't open ]]]\n\n", filename );
  }
  return FALSE;
}
void	printer_close( void )
{
  if( fp_prn ){ osd_fclose( fp_prn ); fp_prn = NULL; }
}



void	printer_init( void )
{
}
void	printer_stlobe( void )
{
  if( fp_prn ){
    osd_fputc( common_out_data, fp_prn );
    osd_fflush( fp_prn );
  }
}
void	printer_term( void )
{
}


/*===========================================================================*/
/* カレンダクロック							     */
/*===========================================================================*/

static	Uchar	shift_reg[7];
static	Uchar	calendar_cdo;
static	int	calendar_diff;

static	void	get_calendar_work( void )
{
  struct tm t;

  if( calendar_stop==FALSE ){
    time_t now_time;
    struct tm *tp;

    now_time  = time( NULL );
    now_time += (time_t)calendar_diff;
    tp = localtime( &now_time );
    t = *tp;
  }else{
    t.tm_year = calendar_data[0] + 1900;
    t.tm_mon  = calendar_data[1];
    t.tm_mday = calendar_data[2];
    t.tm_wday = calendar_data[3];
    t.tm_hour = calendar_data[4];
    t.tm_min  = calendar_data[5];
    t.tm_sec  = calendar_data[6];
  }

  shift_reg[0] = ( t.tm_sec %10 <<4 );
  shift_reg[1] = ( t.tm_min %10 <<4 ) | ( t.tm_sec /10 );
  shift_reg[2] = ( t.tm_hour%10 <<4 ) | ( t.tm_min /10 );
  shift_reg[3] = ( t.tm_mday%10 <<4 ) | ( t.tm_hour/10 );
  shift_reg[4] = ( t.tm_wday    <<4 ) | ( t.tm_mday/10 );
  shift_reg[5] = ( (t.tm_year%100)%10 <<4 ) | ( t.tm_mon+1 );
  shift_reg[6] = ( (t.tm_year%100)/10 );
}
static	void	set_calendar_work( int x )
{
#define BCD2INT(b)	((((b)>>4)&0x0f)*10 + ((b)&0x0f))
  time_t now_time;
  time_t chg_time;
  struct tm *tp;
  struct tm t;
  int i;

  static const char *week[]=
  { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "???" };

  if(x==0){
    if( verbose_io )
      printf("Set Clock %02d/%02x(%s) %02x:%02x:%02x\n",
	     (shift_reg[4]>>4)&0x0f, shift_reg[3], week[ shift_reg[4]&0x07 ],
	     shift_reg[2], shift_reg[1], shift_reg[0]);

    now_time  = time( NULL );
    now_time += (time_t)calendar_diff;
    tp = localtime( &now_time );
    t.tm_year = tp->tm_year;

  }else{
    if( verbose_io )
      printf("Set Clock %02x/%02d/%02x(%s) %02x:%02x:%02x\n",
	     shift_reg[5],
	     (shift_reg[4]>>4)&0x0f, shift_reg[3], week[ shift_reg[4]&0x07 ],
	     shift_reg[2], shift_reg[1], shift_reg[0]);

    i = BCD2INT( shift_reg[5] );
    if( i >= 38 ) t.tm_year = 1900 + i -1900;
    else          t.tm_year = 2000 + i -1900;
  }

  t.tm_mon  = ((shift_reg[4]>>4)&0x0f) -1;
  t.tm_mday = BCD2INT( shift_reg[3] );
  t.tm_wday = shift_reg[4]&0x07;
  t.tm_hour = BCD2INT( shift_reg[2] );
  t.tm_min  = BCD2INT( shift_reg[1] );
  t.tm_sec  = BCD2INT( shift_reg[0] );
  t.tm_yday = 0;
  t.tm_isdst= 0;

  now_time = time( NULL );
  chg_time = mktime( &t );

  if( now_time != -1 && chg_time != -1  )
    calendar_diff = (int)difftime( chg_time, now_time );

#undef BCD2INT
}


void	calendar_init( void )
{
  int	i;
  for(i=0;i<7;i++) shift_reg[i] = 0;
  calendar_cdo = 0;

  calendar_diff = 0;
}

void	calendar_shift_clock( void )
{
  byte	x = ( common_out_data>>3 ) & 0x01;

  calendar_cdo = shift_reg[0] & 0x01;
  shift_reg[0] = ( shift_reg[0]>>1 ) | ( shift_reg[1]<<7 );
  shift_reg[1] = ( shift_reg[1]>>1 ) | ( shift_reg[2]<<7 );
  shift_reg[2] = ( shift_reg[2]>>1 ) | ( shift_reg[3]<<7 );
  shift_reg[3] = ( shift_reg[3]>>1 ) | ( shift_reg[4]<<7 );
  shift_reg[4] = ( shift_reg[4]>>1 ) | ( shift_reg[5]<<7 );
  shift_reg[5] = ( shift_reg[5]>>1 ) | ( shift_reg[6]<<7 );
  shift_reg[6] = ( shift_reg[6]>>1 ) | ( x<<3 );
}

void	calendar_stlobe( void )
{
  switch( common_out_data & 0x7 ){
  case 0:	/*calendar_init();*/	break;		/* 初期化 */
  case 1:	calendar_shift_clock();	break;		/* シフト */
  case 2:	calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		set_calendar_work(0);	break;
  case 3:	get_calendar_work();			/* 時刻取得 */
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();	break;
  case 4:	break;
  case 5:	break;
  case 6:	break;
  case 7:
    switch( shift_reg[6] & 0xf ){

    case 0:	/*calendar_init();*/	break;		/* 初期化 */
    case 1:	calendar_shift_clock();	break;		/* シフト */
    case 2:	set_calendar_work(1);	break;		/* 時刻設定 */
    case 3:	get_calendar_work();	break;		/* 時刻取得 */
    case 4:	break;
    case 5:	break;
    case 6:	break;
    case 7:	break;
    case 8:	break;
    case 9:	break;
    case 10:	break;
    case 11:	break;
    case 12:	break;
    case 13:	break;
    case 14:	break;
    case 15:	/*test_mode();*/	break;
    }
    break;
  }
}


/*===========================================================================*/
/* コントロール信号入出力						     */
/*===========================================================================*/

void	out_ctrl_signal( byte data )
{
  byte	trg_on  = ~ctrl_signal &  data;
  byte	trg_off =  ctrl_signal & ~data;

  if( trg_on  & 0x01 ) printer_stlobe();
  if( trg_off & 0x02 ) calendar_stlobe();
  if( trg_off & 0x04 ) calendar_shift_clock();

  if( data & 0x08 ) set_crtc_sync_bit();
  else		    clr_crtc_sync_bit();

  if( (trg_on & (0x80|0x20)) || (trg_off & (0x80|0x20)) ){
    xmame_dev_beep_out_data( data );
  }

  if( trg_on  & 0x40 ) keyboard_jop1_strobe();
  if( trg_off & 0x40 ) keyboard_jop1_strobe();

  ctrl_signal = data;
}

byte	in_ctrl_signal( void )
{
  return ((ctrl_vrtc    << 5 ) |
	  (calendar_cdo << 4 ) |
	   ctrl_boot           |
	   monitor_15k         );
}








/************************************************************************/
/* メモリの初期化 (電源投入時のみ)					*/
/************************************************************************/
#if 0
void	power_on_ram_init( void )
{
  int   addr, i;
  Uchar data;

		/* メイン RAM を特殊なパターンで埋める */

  for( addr = 0; addr < 0x10000; addr += 0x100 ){
    if( (addr&0x0d00)==0x0100 || (addr&0x0f00)==0x0500 ||
        (addr&0x0f00)==0x0a00 || (addr&0x0d00)==0x0c00 )  data = 0xff;
    else                                                  data = 0x00;

    if( addr&0x4000 ) data ^= 0xff;
    if( addr&0x8000 ) data ^= 0xff;
    if((addr&0xf000)==0xb000 ) data ^= 0xff;
#if 0
    if((addr&0xf000)==0xe000 ) data ^= 0xff; /* とりあえず反転 */
					     /* changed by peach */
#endif

    for(i=0;i<4;i++){
      memset( &main_ram[ addr + i*64     ], data,      16 );
      memset( &main_ram[ addr + i*64 +16 ], data^0xff, 16 );
      memset( &main_ram[ addr + i*64 +32 ], data,      16 );
      memset( &main_ram[ addr + i*64 +48 ], data^0xff, 16 );
      data ^= 0xff;
    }
  }
  if( high_mode ){
    for( i=0xf000; i<0x10000; i++ ) main_ram[i] ^= 0xff;
  }


		/* 高速 RAM(の裏) を特殊なパターンで埋める */

  memcpy( main_high_ram, &main_ram[0xf000], 0x1000 );
  for( addr=0xf000; addr<0x10000; addr++ ) main_ram[addr] ^= 0xff;
}


#else	/* FH ではこのような気がするけど……… */
void	power_on_ram_init( void )
{
  int   addr, i;
  Uchar data;

		/* メイン RAM を特殊なパターンで埋める */

  for( addr = 0; addr < 0x4000; addr += 0x100 ){

    if( ((addr & 0x0d00) == 0x0100) ||		/* x100, x300 */
	((addr & 0x0d00) == 0x0c00) ){		/* xc00, xe00 */
      data = 0xff;
    }else if( ((addr & 0x0f00) == 0x0500) &&
	      ((addr & 0x2000) == 0x0000) ){	/* 0500, 1500 */
      data = 0xff;
    }else if( ((addr & 0x0f00) == 0x0a00) &&
	      ((addr & 0x3000) != 0x0000) ){	/* 1a00, 2a00, 3a00 */
      data = 0xff;
    }else{
      data = 0x00;
    }

    for(i=0;i<4;i++){
      memset( &main_ram[ addr + i*64     ], data,      16 );
      memset( &main_ram[ addr + i*64 +16 ], data^0xff, 16 );
      memset( &main_ram[ addr + i*64 +32 ], data,      16 );
      memset( &main_ram[ addr + i*64 +48 ], data^0xff, 16 );
      data ^= 0xff;
    }
  }

  for( addr = 0x4000; addr < 0x8000; addr += 0x100 ){
    for( i=0; i<0x100; i++ ){
      main_ram[ addr + i ] = main_ram[ addr + i - 0x4000 ] ^ 0xff;
    }
  }

  for( addr = 0x8000; addr < 0x10000; addr += 0x100 ){
    memcpy( &main_ram[ addr ], &main_ram[ 0x7f00 - (addr - 0x8000) ], 0x100 );
  }

  memcpy( &main_high_ram[0], &main_ram[0xf000], 0x1000 );

/*
  memset( &main_ram[0x9000], 0x00, 0x100 );
  main_ram[0x90ff] = 0xff;
  memset( &main_ram[0xbf00], 0x00, 0x100 );
  main_ram[0xbfff] = 0xff;
*/
  for( addr = 0xff00; addr < 0xffff; addr ++ ){
    main_ram[ addr ] = 0xff;
  }
  main_ram[ 0xffff ] = 0x00;

  /* 参考までに、以下のソフトはRAMの初期値を使っているようだ。
     スキーム        : 0xffff が 0x00 であること
     天使たちの午後2 : 0xfff8〜0xffff の 全バイトのOR !=0x00 であること
     天使たちの午後  : 条件不明
  */
}
#endif










/************************************************************************/
/* PC88 メインシステム 初期化						*/
/************************************************************************/

static	void	bootup_work_init(void)
{
	/* V1モードのバージョンの小数点以下を強制変更する */

    if (set_version) ROM_VERSION = set_version;
    else             ROM_VERSION = rom_version;

	/* 起動デバイス(ROM/DISK)未定の時 */

    if (boot_from_rom == BOOT_AUTO) {
	if (disk_image_exist(0))	/* ディスク挿入時はDISK */
	    boot_from_rom = FALSE;
	else				/* それ以外は、    ROM  */
	    boot_from_rom = TRUE;
    }

	/* 起動時の BASICモード未定の時	  */

    if (boot_basic == BASIC_AUTO) {			
	if (ROM_VERSION >= '4')			/* SR 以降は、V2	  */
	    boot_basic = BASIC_V2;
	else					/* それ以前は、V1S	  */
	    boot_basic = BASIC_V1S;
    }

	/* サウンド(I/II)のポートを設定	 */

    if (sound_board == SOUND_II) {

	if      (ROM_VERSION >= '8')		/* FH/MH 以降は、44〜47H */
	    sound_port = SD_PORT_44_45 | SD_PORT_46_47;
	else if (ROM_VERSION >= '4')		/* SR 以降は、44〜45,A8〜ADH */
	    sound_port = SD_PORT_44_45 | SD_PORT_A8_AD;
	else					/* それ以前は、  A8〜ADH */
	    sound_port = SD_PORT_A8_AD;

    } else {

	if (ROM_VERSION >= '4')			/* SR以降は、44〜45H	 */
	    sound_port = SD_PORT_44_45;
	else					/* それ以前は、？？？	 */
	  /*sound_port = SD_PORT_A8_AD;*/
	    sound_port = 0;			/*	対応しないなら 0 */
    }
}


void	pc88main_init( int init )
{
  int i;

  bootup_work_init();

	/* CPU ワーク初期化 */

  if( init == INIT_POWERON  ||  init == INIT_RESET ){

    z80_reset( &z80main_cpu );
  }

  pc88main_bus_setup();

  z80main_cpu.intr_update = main_INT_update;
  z80main_cpu.intr_ack    = main_INT_chk;

  z80main_cpu.break_if_halt = FALSE;		/* for debug */
  z80main_cpu.PC_prev   = z80main_cpu.PC;	/* dummy for monitor */

#ifdef	DEBUGLOG
  z80main_cpu.log	= TRUE;
#else
  z80main_cpu.log	= FALSE;
#endif


	/* RAMを電源投入時パターンで初期化 */

  if( init == INIT_POWERON ){
    power_on_ram_init();
  }


	/* フォント初期化 */

  if( init == INIT_POWERON  ||  init == INIT_RESET ){

    memory_reset_font();
  }else{
    memory_set_font();
  }


	/* キーボード初期化 */

  if( init == INIT_POWERON  ||  init == INIT_STATELOAD ){
    keyboard_reset();
  }



  printer_init();			/* PRINTER は復元しない	*/

  if( init == INIT_POWERON  ||  init == INIT_RESET ){

    sio_init();

    main_INT_init();

    crtc_init();
    dmac_init();
    calendar_init();
    keyboard_jop1_reset();
    sound_board_init();
    pio_init();


    dipsw_1 = (boot_dipsw   ) & SW_1_MASK;
    dipsw_2 = (boot_dipsw>>8) & SW_2_MASK;

    switch( boot_basic ){
    case BASIC_N:
      dipsw_1 &= ~SW_N88;
      dipsw_2 |=  SW_V1;
      dipsw_2 &= ~SW_H;
      high_mode = FALSE;
      break;
    case BASIC_V1S:
      dipsw_1 |=  SW_N88;
      dipsw_2 |=  SW_V1;
      dipsw_2 &= ~SW_H;
      high_mode = FALSE;
      break;
    case BASIC_V1H:
      dipsw_1 |=  SW_N88;
      dipsw_2 |=  SW_V1;
      dipsw_2 |=  SW_H;
      high_mode = TRUE;
      break;
    case BASIC_V2:
      dipsw_1 |=  SW_N88;
      dipsw_2 &= ~SW_V1;
      dipsw_2 |=  SW_H;
      high_mode = TRUE;
      break;
    }

    ctrl_boot		= (boot_from_rom) ? SW_ROMBOOT : 0;
    memory_bank		= MEMORY_BANK_MAIN;
    cpu_clock		= (boot_clock_4mhz) ? SW_4MHZ : 0;

    sys_ctrl		= 0x31;
    grph_ctrl		= 0x31;
    misc_ctrl		= 0x90;
    ALU1_ctrl		= 0x77;
    ALU2_ctrl		= 0x00;
    ctrl_signal		= 0x0f;
    grph_pile		= 0x00;
  /*baudrate_sw		= 0;*/
    window_offset	= 0x0000;
    ext_rom_bank	= 0xff;

    ext_ram_ctrl	= 0;
    ext_ram_bank	= 0;

    jisho_rom_ctrl	= JISHO_NOT_SELECT;
    jisho_rom_bank	= 0;

    vram_bg_palette.blue  = 0;
    vram_bg_palette.red   = 0;
    vram_bg_palette.green = 0;
    for( i=0; i<8; i++ ){
      vram_palette[ i ].blue  = (i&1) ? 7 : 0;
      vram_palette[ i ].red   = (i&2) ? 7 : 0;
      vram_palette[ i ].green = (i&4) ? 7 : 0;
    }

    intr_level		= 7;
    intr_priority	= 0;
    intr_sio_enable	= 0x00;
    intr_vsync_enable	= 0x00;
    intr_rtc_enable	= 0x00;
  }

  main_memory_mapping_0000_7fff();
  main_memory_mapping_8000_83ff();
  main_memory_mapping_c000_ffff();
  main_memory_vram_mapping();


  /* CRTC/DMAC関連による初期化 */
  set_text_display();
  frameskip_blink_reset();

  /* シリアルマウス初期化 */
  if (use_siomouse) {
    sio_mouse_init(TRUE);
  }

  /* サウンドについて・・・ */
  if( init == INIT_STATELOAD ){
    sound_output_after_stateload();
  }

}


/************************************************************************/
/* PC88 メインシステム 終了						*/
/************************************************************************/
void	pc88main_term( void )
{
  printer_term();
  sio_term();
}










/************************************************************************/
/* ブレークポイント関連							*/
/************************************************************************/
INLINE	void	check_break_point( int type, word addr, byte data, char *str )
{
  int	i;

  if (quasi88_is_monitor())  return; /* モニターモード時はスルー */
  for( i=0; i<NR_BP; i++ ){
    if( break_point[BP_MAIN][i].type == type &&
        break_point[BP_MAIN][i].addr == addr ){
      printf( "*** Break at %04x *** "
	      "( MAIN - #%d [ %s %04XH , data = %02XH ]\n",
	      z80main_cpu.PC.W, i+1, str, addr, data );
      quasi88_debug();
      break;
    }
  }
}

byte	main_fetch_with_BP( word addr )
{
  byte	data = main_fetch( addr );
  check_break_point( BP_READ, addr, data, "FETCH from" );
  return data;
}

byte	main_mem_read_with_BP( word addr )
{
  byte	data = main_mem_read( addr );
  check_break_point( BP_READ, addr, data, "READ from" );
  return data;
}

void	main_mem_write_with_BP( word addr, byte data )
{
  main_mem_write( addr, data );
  check_break_point( BP_WRITE, addr, data, "WRITE to" );
}

byte	main_io_in_with_BP( byte port )
{
  byte	data =  main_io_in( port );
  check_break_point( BP_IN, port, data, "IN from" );
  return data;
}

void	main_io_out_with_BP( byte port, byte data )
{
  main_io_out( port, data );
  check_break_point( BP_OUT, port, data, "OUT to" );
}



/************************************************************************/
/*									*/
/************************************************************************/
void	pc88main_bus_setup( void )
{
#ifdef	USE_MONITOR

  int	i, buf[4];
  for( i=0; i<4; i++ ) buf[i]=0;
  for( i=0; i<NR_BP; i++ ){
    switch( break_point[BP_MAIN][i].type ){
    case BP_READ:	buf[0]++;	break;
    case BP_WRITE:	buf[1]++;	break;
    case BP_IN:		buf[2]++;	break;
    case BP_OUT:	buf[3]++;	break;
    }
  }
   
  if( memory_wait || highspeed_mode ){
    if( buf[0] ) z80main_cpu.fetch   = main_fetch_with_BP;
    else         z80main_cpu.fetch   = main_fetch;
  }else{
    if( buf[0] ) z80main_cpu.fetch   = main_mem_read_with_BP;
    else         z80main_cpu.fetch   = main_mem_read;
  }

  if( buf[0] ) z80main_cpu.mem_read  = main_mem_read_with_BP;
  else         z80main_cpu.mem_read  = main_mem_read;

  if( buf[1] ) z80main_cpu.mem_write = main_mem_write_with_BP;
  else         z80main_cpu.mem_write = main_mem_write;

  if( buf[2] ) z80main_cpu.io_read   = main_io_in_with_BP;
  else         z80main_cpu.io_read   = main_io_in;

  if( buf[3] ) z80main_cpu.io_write  = main_io_out_with_BP;
  else         z80main_cpu.io_write  = main_io_out;

#else

  if( memory_wait || highspeed_mode ){
    z80main_cpu.fetch   = main_fetch;
  }else{
    z80main_cpu.fetch   = main_mem_read;
  }
  z80main_cpu.mem_read  = main_mem_read;
  z80main_cpu.mem_write = main_mem_write;
  z80main_cpu.io_read   = main_io_in;
  z80main_cpu.io_write  = main_io_out;

#endif
}




/***********************************************************************
 * ステートロード／ステートセーブ
 ************************************************************************/

#define	SID	"MAIN"
#define	SID2	"MAI2"

static	T_SUSPEND_W	suspend_pc88main_work[]=
{
  { TYPE_STR,	&file_tape[0][0],	},
  { TYPE_STR,	&file_tape[1][0],	},

  { TYPE_INT,	&boot_basic,		},
  { TYPE_INT,	&boot_dipsw,		},
  { TYPE_INT,	&boot_from_rom,		},
  { TYPE_INT,	&boot_clock_4mhz,	},
  { TYPE_INT,	&monitor_15k,		},

  { TYPE_PAIR,	&z80main_cpu.AF,	},
  { TYPE_PAIR,	&z80main_cpu.BC,	},
  { TYPE_PAIR,	&z80main_cpu.DE,	},
  { TYPE_PAIR,	&z80main_cpu.HL,	},
  { TYPE_PAIR,	&z80main_cpu.IX,	},
  { TYPE_PAIR,	&z80main_cpu.IY,	},
  { TYPE_PAIR,	&z80main_cpu.PC,	},
  { TYPE_PAIR,	&z80main_cpu.SP,	},
  { TYPE_PAIR,	&z80main_cpu.AF1,	},
  { TYPE_PAIR,	&z80main_cpu.BC1,	},
  { TYPE_PAIR,	&z80main_cpu.DE1,	},
  { TYPE_PAIR,	&z80main_cpu.HL1,	},
  { TYPE_BYTE,	&z80main_cpu.I,		},
  { TYPE_BYTE,	&z80main_cpu.R,		},
  { TYPE_BYTE,	&z80main_cpu.R_saved,	},
  { TYPE_CHAR,	&z80main_cpu.IFF,	},
  { TYPE_CHAR,	&z80main_cpu.IFF2,	},
  { TYPE_CHAR,	&z80main_cpu.IM,	},
  { TYPE_CHAR,	&z80main_cpu.HALT,	},
  { TYPE_INT,	&z80main_cpu.INT_active,},
  { TYPE_INT,	&z80main_cpu.icount,	},
  { TYPE_INT,	&z80main_cpu.state0,	},
  { TYPE_INT,	&z80main_cpu.skip_intr_chk,	},
  { TYPE_CHAR,	&z80main_cpu.log,		},
  { TYPE_CHAR,	&z80main_cpu.break_if_halt,	},

  { TYPE_INT,	&high_mode,		},

  { TYPE_BYTE,	&dipsw_1,		},
  { TYPE_BYTE,	&dipsw_2,		},
  { TYPE_BYTE,	&ctrl_boot,		},
  { TYPE_INT,	&memory_bank,		},
  { TYPE_BYTE,	&cpu_clock,		},

  { TYPE_BYTE,	&common_out_data,	},
  { TYPE_BYTE,	&misc_ctrl,		},
  { TYPE_BYTE,	&ALU1_ctrl,		},
  { TYPE_BYTE,	&ALU2_ctrl,		},
  { TYPE_BYTE,	&ctrl_signal,		},
  { TYPE_BYTE,	&baudrate_sw,		},
  { TYPE_WORD,	&window_offset,		},
  { TYPE_BYTE,	&ext_rom_bank,		},
  { TYPE_BYTE,	&ext_ram_ctrl,		},
  { TYPE_BYTE,	&ext_ram_bank,		},

  { TYPE_PAIR,	&kanji1_addr,		},
  { TYPE_PAIR,	&kanji2_addr,		},

  { TYPE_BYTE,	&jisho_rom_bank,	},
  { TYPE_BYTE,	&jisho_rom_ctrl,	},

  { TYPE_INT,	&calendar_stop,		},
  { TYPE_CHAR,	&calendar_data[0],	},
  { TYPE_CHAR,	&calendar_data[1],	},
  { TYPE_CHAR,	&calendar_data[2],	},
  { TYPE_CHAR,	&calendar_data[3],	},
  { TYPE_CHAR,	&calendar_data[4],	},
  { TYPE_CHAR,	&calendar_data[5],	},
  { TYPE_CHAR,	&calendar_data[6],	},

  { TYPE_INT,	&cmt_speed,		},
  { TYPE_INT,	&cmt_intr,		},
  { TYPE_INT,	&cmt_wait,		},

  { TYPE_INT,	&highspeed_mode,	},
  { TYPE_INT,	&memory_wait,		},

  { TYPE_INT,	&ALU_buf.l,	},	/*  TYPE_CHAR, ALU_buf.c[0]-[3] ?  */
  { TYPE_INT,	&ALU_comp.l,	},	/*  TYPE_CHAR, ALU_comp.c[0]-[3] ? */

  { TYPE_INT,	&pcg_data,		},
  { TYPE_INT,	&pcg_addr,		},

  { TYPE_INT,	&sio_instruction,	},
  { TYPE_BYTE,	&sio_mode,		},
  { TYPE_BYTE,	&sio_command,		},
  { TYPE_INT,	&sio_data_exist,	},
  { TYPE_BYTE,	&sio_data,		},

  { TYPE_INT,	&com_X_flow,		},

  { TYPE_INT,	&cmt_dummy_read_cnt,	},
  { TYPE_INT,	&cmt_skip,		},
  { TYPE_INT,	&cmt_skip_data,		},

  { TYPE_LONG,	&cmt_read_chars,	},

  { TYPE_CHAR,	&shift_reg[0],		},
  { TYPE_CHAR,	&shift_reg[1],		},
  { TYPE_CHAR,	&shift_reg[2],		},
  { TYPE_CHAR,	&shift_reg[3],		},
  { TYPE_CHAR,	&shift_reg[4],		},
  { TYPE_CHAR,	&shift_reg[5],		},
  { TYPE_CHAR,	&shift_reg[6],		},
  { TYPE_CHAR,	&calendar_cdo,		},
  { TYPE_INT,	&calendar_diff,		},

  { TYPE_END,	0			},
};

static	T_SUSPEND_W	suspend_pc88main_work2[]=
{
  { TYPE_INT,	&use_siomouse,		},
  { TYPE_END,	0			},
};


int	statesave_pc88main( void )
{
/*if( fp_ti ) printf("%d\n",osd_ftell(fp_ti));*/

  if( statesave_table( SID, suspend_pc88main_work ) != STATE_OK ) return FALSE;

  if( statesave_table(SID2, suspend_pc88main_work2) != STATE_OK ) return FALSE;

  return TRUE;
}

int	stateload_pc88main( void )
{
  if( stateload_table( SID, suspend_pc88main_work ) != STATE_OK ) return FALSE;

  if( stateload_table(SID2, suspend_pc88main_work2) != STATE_OK ) {

    /* 旧バージョンなら、みのがす */

    printf( "stateload : Statefile is old. (ver 0.6.0, 1, 2 or 3?)\n" );

    use_siomouse = FALSE;
  }


  cmt_stateload_chars = cmt_read_chars;
  cmt_read_chars = 0;

  cmt_stateload_skip = cmt_skip;

  return TRUE;
}
