/************************************************************************/
/*									*/
/* PC8801 サブシステム(FDD側)						*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "pc88sub.h"

#include "pc88cpu.h"
#include "fdc.h"
#include "screen.h"	/* state_of_vsync */
#include "intr.h"	/* state_of_vsync */
#include "event.h"
#include "memory.h"
#include "pio.h"

#include "emu.h"
#include "suspend.h"



z80arch	z80sub_cpu;			/* Z80 CPU ( sub system )	*/

int	sub_load_rate = 6;		/*				*/

/************************************************************************/
/* メモリアクセス							*/
/*			メモリアクセス処理の方法は、笠松健一さんの	*/
/*			助言により、改良。				*/
/*				Copyright (c) kenichi kasamatsu		*/
/************************************************************************/
/*----------------------*/
/*    メモリ・フェッチ	*/
/*----------------------*/
byte	sub_fetch( word addr )
{
  if( memory_wait ){
    if( addr < 0x4000 )
      z80sub_cpu.state0 += 1;		/* M1サイクルウェイト */
  }

#if 0
  if( verbose_io ){
    if( ( 0x2000 <= addr && addr < 0x4000 ) || ( addr & 0x8000 ) ){
      printf("SUB Memory Read BAD %04x\n",addr);
    }
  }
#endif
  return sub_romram[ addr & 0x7fff ];
}


/*----------------------*/
/*    メモリ・リード	*/
/*----------------------*/
byte	sub_mem_read( word addr )
{
#if 0
  if( verbose_io ){
    if( ( 0x2000 <= addr && addr < 0x4000 ) || ( addr & 0x8000 ) ){
      printf("SUB Memory Read BAD %04x\n",addr);
    }
  }
#endif
  return sub_romram[ addr & 0x7fff ];
}


/*----------------------*/
/*     メモリライト	*/
/*----------------------*/
void	sub_mem_write( word addr, byte data )
{
  if( (addr & 0xc000) == 0x4000 ){

    sub_romram[ addr & 0x7fff ] = data;

  }else{

    if( verbose_io ) printf("SUB Memory Write BAD %04x\n",addr);
    if( (addr & 0x4000 ) == 0x4000 ){
      sub_romram[ addr & 0x7fff ] = data;
    }

  }
}




/************************************************************************/
/* Ｉ／Ｏポートアクセス							*/
/************************************************************************/

/*----------------------*/
/*    ポート・ライト	*/
/*----------------------*/

void	sub_io_out( byte port, byte data )
{
  switch( port ){

  case 0xf4:				/* ドライブモード？ 2D/2DD/2HD ? */
    return;
  case 0xf7:				/* プリンタポート出力		*/
    return;
  case 0xf8:				/* ドライブモータ制御出力	*/
    return;

  case 0xfb:				/* FDC データ WRITE */
    fdc_write( data );
    CPU_REFRESH_INTERRUPT();
    return;

	/* ＰＩＯ */

  case 0xfc:
    logpio("   <==%02x\n",data);
    pio_write_AB( PIO_SIDE_S, PIO_PORT_A, data );
    return;
  case 0xfd:
    logpio("   <--%02x\n",data);
    pio_write_AB( PIO_SIDE_S, PIO_PORT_B, data );
    return;
  case 0xfe:
    pio_write_C_direct( PIO_SIDE_S, data );
    return;
  case 0xff:
    if( data & 0x80 ) pio_set_mode( PIO_SIDE_S, data );
    else              pio_write_C( PIO_SIDE_S, data );
    return;

  }



  if( verbose_io ) printf("SUB OUT data %02X to undecoeded port %02XH\n",data,port);

}

/*----------------------*/
/*    ポート・リード	*/
/*----------------------*/

byte	sub_io_in( byte port )
{
  switch( port ){

  case 0xf8:				/* FDC に TC を出力	*/
    CPU_REFRESH_INTERRUPT();
    fdc_TC();
    return 0xff;

  case 0xfa:				/* FDC ステータス 入力 */
    CPU_REFRESH_INTERRUPT();
    return	fdc_status();
  case 0xfb:				/* FDC データ READ */
    CPU_REFRESH_INTERRUPT();
    return	fdc_read();

	/* ＰＩＯ */

  case 0xfc:
    {
      byte data = pio_read_AB( PIO_SIDE_S, PIO_PORT_A );
      logpio("   ==>%02x\n",data);
      return data;
    }
  case 0xfd:
    {
      byte data = pio_read_AB( PIO_SIDE_S, PIO_PORT_B );
      logpio("   -->%02x\n",data);
      return data;
    }
  case 0xfe:
    return pio_read_C( PIO_SIDE_S );

  }

  if( verbose_io ) printf("SUB IN        from undecoeded port %02XH\n",port);

  return 0xff;
}






/************************************************************************/
/* Peripheral エミュレーション						*/
/************************************************************************/

/************************************************************************/
/* マスカブル割り込みエミュレート					*/
/************************************************************************/

/*------------------------------*/
/* 初期化(Z80リセット時に呼ぶ)	*/
/*------------------------------*/
void	sub_INT_init( void )
{
  FDC_flag = FALSE;
}

/*----------------------------------------------------------------------*/
/* 割り込みを生成する。と同時に、次の割り込みまでの、最小 state も計算	*/
/*	帰り値は、Z80処理強制終了のフラグ(TRUE/FALSE)			*/
/*----------------------------------------------------------------------*/
void	sub_INT_update( void )
{
  static int sub_total_state = 0;	/* サブCPUが処理した命令数      */
  int icount;

  icount = fdc_ctrl( z80sub_cpu.state0 );

  if( FDC_flag ){ z80sub_cpu.INT_active = TRUE;  }
  else          { z80sub_cpu.INT_active = FALSE; }


	/* キースキャンや画面表示の処理は、メインCPU処理で	*/
	/* 行なっているため、-cpu 0 指定時に、サブCPUに制御が	*/
	/* 移ったまま戻ってこなくなると、メニュー画面への移行	*/
	/* などができなくなる。					*/
	/* そこで、サブCPUでも一定時間処理を行なうたびに、	*/
	/* キースキャンと画面表示を行なうことにする。		*/
	/* しかし、今度はディスクアクセスなどのサブCPUの処理が	*/
	/* 増えた時に、サウンドが止まるなどの弊害が出てきた。	*/
	/* なので、この「サブCPUでもキースキャン」処理を行なう	*/
	/* 頻度を変更できるようにしておこう。			*/

	/* D.C.コネクションとか… */

  if( sub_load_rate && cpu_timing < 2 ){
    sub_total_state += z80sub_cpu.state0;
    if( sub_total_state/sub_load_rate >= state_of_vsync ){

      CPU_BREAKOFF();
      quasi88_event_flags |= (EVENT_FRAME_UPDATE | EVENT_AUDIO_UPDATE);
      sub_total_state = 0;
    }
  }

  z80sub_cpu.icount = (icount<0) ? 999999 : icount;


	/* メニューへの遷移などは、ここで確認 */

  if (quasi88_event_flags & EVENT_MODE_CHANGED) {
    CPU_BREAKOFF();
  }
}


/*----------------------------------------------*/
/* チェック (割込許可時 1ステップ毎に呼ばれる)	*/
/*----------------------------------------------*/
int	sub_INT_chk( void )
{
  z80sub_cpu.INT_active = FALSE;

  if( FDC_flag ) return 0;
  else           return -1;
}










/************************************************************************/
/* PC88 サブシステム 初期化						*/
/************************************************************************/
void	pc88sub_init( int init )
{

	/* Z80 エミュレータワーク初期化 */

  if( init == INIT_POWERON  ||  init == INIT_RESET ){

    z80_reset( &z80sub_cpu );
  }

  pc88sub_bus_setup();

  z80sub_cpu.intr_update = sub_INT_update;
  z80sub_cpu.intr_ack    = sub_INT_chk;

  z80sub_cpu.break_if_halt = TRUE;
  z80sub_cpu.PC_prev   = z80sub_cpu.PC;		/* dummy for monitor */

#ifdef	DEBUGLOG
  z80sub_cpu.log	= TRUE;
#else
  z80sub_cpu.log	= FALSE;
#endif


  if( init == INIT_POWERON  ||  init == INIT_RESET ){

    sub_INT_init();

    /* fdc_init(); は drive_init()    で処理済み */
    /* pio_init(); は pc88main_init() で処理済み */
  }
}


/************************************************************************/
/* PC88 サブシステム 終了						*/
/************************************************************************/
void	pc88sub_term( void )
{
}















/************************************************************************/
/* ブレークポイント関連							*/
/************************************************************************/
INLINE	void	check_break_point( int type, word addr, char *str )
{
  int	i;

  if (quasi88_is_monitor()) return; /* モニターモード時はスルー */
  for( i=0; i<NR_BP; i++ ){
    if( break_point[BP_SUB][i].type == type &&
        break_point[BP_SUB][i].addr == addr ){
      printf( "*** Break at %04x *** ( SUB[#%d] : %s %04x )\n",
	      z80sub_cpu.PC.W, i+1, str, addr );
      quasi88_debug();
      break;
    }
  }
}

byte	sub_fetch_with_BP( word addr )
{
  check_break_point( BP_READ, addr, "FETCH from" );
  return sub_fetch( addr );
}

byte	sub_mem_read_with_BP( word addr )
{
  check_break_point( BP_READ, addr, "READ from" );
  return sub_mem_read( addr );
}

void	sub_mem_write_with_BP( word addr, byte data )
{
  check_break_point( BP_WRITE, addr, "WRITE to" );
  sub_mem_write( addr, data );
}

byte	sub_io_in_with_BP( byte port )
{
  check_break_point( BP_IN, port, "IN from" );
  return sub_io_in( port );
}

void	sub_io_out_with_BP( byte port, byte data )
{
  check_break_point( BP_OUT, port, "OUT to" );
  sub_io_out( port, data );
}






/************************************************************************/
/*									*/
/************************************************************************/
void	pc88sub_bus_setup( void )
{
#ifdef	USE_MONITOR

  int	i, buf[4];
  for( i=0; i<4; i++ ) buf[i]=0;
  for( i=0; i<NR_BP; i++ ){
    switch( break_point[BP_SUB][i].type ){
    case BP_READ:	buf[0]++;	break;
    case BP_WRITE:	buf[1]++;	break;
    case BP_IN:		buf[2]++;	break;
    case BP_OUT:	buf[3]++;	break;
    }
  }
   
  if( memory_wait ){
    if( buf[0] ) z80sub_cpu.fetch   = sub_fetch_with_BP;
    else         z80sub_cpu.fetch   = sub_fetch;
  }else{
    if( buf[0] ) z80sub_cpu.fetch   = sub_mem_read_with_BP;
    else         z80sub_cpu.fetch   = sub_mem_read;
  }

  if( buf[0] ) z80sub_cpu.mem_read  = sub_mem_read_with_BP;
  else         z80sub_cpu.mem_read  = sub_mem_read;

  if( buf[1] ) z80sub_cpu.mem_write = sub_mem_write_with_BP;
  else         z80sub_cpu.mem_write = sub_mem_write;

  if( buf[2] ) z80sub_cpu.io_read   = sub_io_in_with_BP;
  else         z80sub_cpu.io_read   = sub_io_in;

  if( buf[3] ) z80sub_cpu.io_write  = sub_io_out_with_BP;
  else         z80sub_cpu.io_write  = sub_io_out;

#else

  if( memory_wait ){
    z80sub_cpu.fetch   = sub_fetch;
  }else{
    z80sub_cpu.fetch   = sub_mem_read;
  }
  z80sub_cpu.mem_read  = sub_mem_read;
  z80sub_cpu.mem_write = sub_mem_write;
  z80sub_cpu.io_read   = sub_io_in;
  z80sub_cpu.io_write  = sub_io_out;

#endif
}




/***********************************************************************
 * ステートロード／ステートセーブ
 ************************************************************************/

#define	SID	"SUB "

static	T_SUSPEND_W	suspend_pc88sub_work[]=
{
  { TYPE_PAIR,	&z80sub_cpu.AF,		},
  { TYPE_PAIR,	&z80sub_cpu.BC,		},
  { TYPE_PAIR,	&z80sub_cpu.DE,		},
  { TYPE_PAIR,	&z80sub_cpu.HL,		},
  { TYPE_PAIR,	&z80sub_cpu.IX,		},
  { TYPE_PAIR,	&z80sub_cpu.IY,		},
  { TYPE_PAIR,	&z80sub_cpu.PC,		},
  { TYPE_PAIR,	&z80sub_cpu.SP,		},
  { TYPE_PAIR,	&z80sub_cpu.AF1,	},
  { TYPE_PAIR,	&z80sub_cpu.BC1,	},
  { TYPE_PAIR,	&z80sub_cpu.DE1,	},
  { TYPE_PAIR,	&z80sub_cpu.HL1,	},
  { TYPE_BYTE,	&z80sub_cpu.I,		},
  { TYPE_BYTE,	&z80sub_cpu.R,		},
  { TYPE_BYTE,	&z80sub_cpu.R_saved,	},
  { TYPE_CHAR,	&z80sub_cpu.IFF,	},
  { TYPE_CHAR,	&z80sub_cpu.IFF2,	},
  { TYPE_CHAR,	&z80sub_cpu.IM,		},
  { TYPE_CHAR,	&z80sub_cpu.HALT,	},
  { TYPE_INT,	&z80sub_cpu.INT_active,	},
  { TYPE_INT,	&z80sub_cpu.icount,	},
  { TYPE_INT,	&z80sub_cpu.state0,	},
  { TYPE_INT,	&z80sub_cpu.skip_intr_chk,	},
  { TYPE_CHAR,	&z80sub_cpu.log,		},
  { TYPE_CHAR,	&z80sub_cpu.break_if_halt,	},

  { TYPE_INT,	&sub_load_rate,		},

  { TYPE_END,	0			},
};


int	statesave_pc88sub( void )
{
  if( statesave_table( SID, suspend_pc88sub_work ) == STATE_OK ) return TRUE;
  else                                                           return FALSE;
}

int	stateload_pc88sub( void )
{
  if( stateload_table( SID, suspend_pc88sub_work ) == STATE_OK ) return TRUE;
  else                                                           return FALSE;
}
