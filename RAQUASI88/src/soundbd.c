/************************************************************************/
/*									*/
/* サウンドの処理							*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "initval.h"
#include "soundbd.h"

#include "pc88cpu.h"
#include "pc88main.h"
#include "intr.h"

#include "snddrv.h"
#include "suspend.h"


/*
 * サウンドボードIIの対応はそうとういい加減です。
 */


int	sound_board     = DEFAULT_SOUND;	/* サウンドボード	*/
int	sound_port;				/* サウンドポートの種別	*/


int	intr_sound_enable = 0x00;	/* I/O[31] 割り込みマスク SOUND	*/ 


	/* サウンドボード I 部 */

int	sound_ENABLE_A = FALSE;		/* TIMER A 割込を起こすかどうか */
int	sound_ENABLE_B = FALSE;		/* TIMER B 割込を起こすかどうか */

int	sound_LOAD_A = FALSE;		/* TIMER A を動かすかどうか	*/
int	sound_LOAD_B = FALSE;		/* TIMER B を動かすかどうか	*/

int	sound_FLAG_A = 0;		/* FLAG A の状態		*/
int	sound_FLAG_B = 0;		/* FLAG B の状態		*/

int	sound_TIMER_A = 0;	/* TIMER A 設定時間 reg[0x24〜0x25]	*/
int	sound_TIMER_B = 0;	/* TIMER B 設定時間 reg[0x26]		*/

int	sound_prescaler = 6;		/* 1/プリスケーラー (2,3,6)	*/
int	sound_prescaler_sel = 2;	/* プリスケーラー設定		*/

byte	sound_reg[0x100];		/* サウンドボード レジスタ	*/
int	sound_reg_select = 0x00;


	/* サウンドボード II 部 */

int	sound2_MSK_TA   = FALSE;	/* TIMER A 割り込みマスク	*/
int	sound2_MSK_TB   = FALSE;	/* TIMER B 割り込みマスク	*/
int	sound2_MSK_EOS  = FALSE;	/* EOS     割り込みマスク	*/ 
int	sound2_MSK_BRDY = FALSE;	/* BRDY    割り込みマスク	*/ 
int	sound2_MSK_ZERO = FALSE;	/* ZERO    割り込みマスク	*/ 

int	sound2_EN_TA   = 0x01;		/* TIMER A 割り込み許可		*/
int	sound2_EN_TB   = 0x02;		/* TIMER B 割り込み許可		*/
int	sound2_EN_EOS  = FALSE;		/* EOS     割り込み許可		*/
int	sound2_EN_BRDY = FALSE;		/* BDRY    割り込み許可		*/
int	sound2_EN_ZERO = FALSE;		/* ZERO    割り込み許可		*/

int	sound2_FLAG_EOS    = 0;		/* FLAG EOS  の状態		*/
int	sound2_FLAG_BRDY   = 0;		/* FLAG BRDY の状態		*/
int	sound2_FLAG_ZERO   = 0;		/* FLAG ZERO の状態		*/
int	sound2_FLAG_PCMBSY = 0;		/* FLAG PCMBSY の状態		*/



byte	sound2_reg[0x100];		/* サウンドボードII レジスタ	*/
int	sound2_reg_select = 0x00;


	/* サウンドボード II  ADPCM 部 */

	byte	*sound2_adpcm = NULL;	/* ADPCM用 DRAM (256KB)		*/

static	int	sound2_mem       = 0;	/* ADPCM メモリ 1:アクセス可能	*/
static	int	sound2_rec       = 0;	/* ADPCM        1:録音 0:再生	*/
	int	sound2_repeat    = 0;	/* ADPCM リピートプレイ		*/
static	int	sound2_mem_size  = 5;	/* ADPCM のメモリ単位 5 or 2	*/
static	int	sound2_record_intr_base = 1;/* ADPCM 録音割り込みレート	*/
static	int	sound2_replay_intr_base = 1;/* ADPCM 再生割り込みレート	*/
static	int	sound2_start_addr = 0;	/* ADPCM スタートアドレス	*/
static	int	sound2_stop_addr  = 0;	/* ADPCM ストップアドレス	*/
static	int	sound2_limit_addr = 0;	/* ADPCM リミットアドレス	*/
static	int	sound2_read_dummy = 2;	/* ADPCM リード時ダミーカウンタ	*/

enum AdpcmStat{
  ADPCM_HALT,
  ADPCM_RECORD,
  ADPCM_REPLAY,
  ADPCM_WRITE,
  ADPCM_READ,
  EndofAdpcmStat
};
static	int	sound2_stat = 0;	/* ADPCM の状態			*/

static	int	sound2_data_addr   = 0;	/* ADPCM 処理中のアドレス	*/
	int	sound2_intr_base   = 1;	/* ADPCM 割り込みレート		*/
static	int	sound2_counter     = 0;	/* ADPCM 再生録音カウンタ	*/
static	int	sound2_counter_old = 0;

	int	sound2_notice_EOS  = FALSE;	/* EOSチェックの要不要	*/

static	int	sound2_delay = 0;	/* フラグの遅れ			*/


/* BEEP関連。手頃な定義場所がない… */

int	use_cmdsing = TRUE;		/* 真で、CMD SING有効		*/

/********************************************************/
/* SOUND 初期化						*/
/********************************************************/
void	sound_board_init( void )
{
  int	i;
  for( i=0; i<0x100; i++ ) sound_reg [ i ] = 0x00;
  for( i=0; i<0x100; i++ ) sound2_reg[ i ] = 0x00;
  sound_reg[0x0e]=0xff;
  sound_reg[0x0f]=0xff;

  intr_sound_enable = 0x00;
/*
  sound_reg[0x24]=0;
  sound_reg[0x25]=0;
  sound_reg[0x26]=0;
  sound_reg[0x27]=0;
*/
  sound_reg[0x29]=0x03;		/* こいつだけは 0x00 だとまずいのでは… */

  sound_ENABLE_A = FALSE;
  sound_ENABLE_B = FALSE;
  sound_LOAD_A   = FALSE;
  sound_LOAD_B   = FALSE;
  sound_FLAG_A   = 0;
  sound_FLAG_B   = 0;
  sound_TIMER_A  = 0;
  sound_TIMER_B  = 0;

  sound_prescaler = 6;
  sound_prescaler_sel = 2;

  sound2_MSK_TA = 0x00;
  sound2_MSK_TB = 0x00;
  sound2_EN_TA  = 0x01;
  sound2_EN_TB  = 0x02;

  sound2_EN_EOS  = FALSE;
  sound2_EN_BRDY = FALSE;
  sound2_EN_ZERO = FALSE;

  sound2_FLAG_EOS    = 0;
  sound2_FLAG_BRDY   = 0;
  sound2_FLAG_ZERO   = 0;
  sound2_FLAG_PCMBSY = 0;

  sound2_read_dummy = 2;
}






/************************************************************************/
/* サウンドボード部							*/
/************************************************************************/

/********************************************************/
/* データを入出力するレジスタの指定			*/
/********************************************************/
void	sound_out_reg( byte data )
{
  sound_reg_select = data & 0xff;

  xmame_dev_sound_out_reg( data );
}

/********************************************************/
/* データを指定したレジスタに出力			*/
/********************************************************/
void	sound_out_data( byte data )
{
  static const int pres[4] = { 2, 2, 6, 3 };


  sound_reg[ sound_reg_select ] = data;

  xmame_dev_sound_out_data( data );


  switch( sound_reg_select ){

#if 0
  case 0x07:
    /*if( verbose_snd ) if( !(data&0xc0) ) printf("SOUND joy %02X\n",data );*/
    break;
#endif


  case 0x24:
  case 0x25:
    sound_TIMER_A = ((int)sound_reg[ 0x24 ]<<2) | (sound_reg[ 0x25 ] & 0x03);
    interval_work_set_TIMER_A();
    break;

  case 0x26:
    sound_TIMER_B = (int)data;
    interval_work_set_TIMER_B();
    break;

  case 0x27:
    change_sound_flags( 0x27 );
/*printf("%x%x%x%x%x%x%x%x  \n",
       (data>>7)&1,(data>>6)&1,(data>>5)&1,(data>>4)&1,
       (data>>3)&1,(data>>2)&1,(data>>1)&1,(data>>0)&1);*/
    break;

  case 0x29:
    if( sound_board==SOUND_II ){
      change_sound_flags( 0x29 );
    }
    break;

  case 0x2d:
    if( verbose_snd ) printf("SOUND out %X\n",sound_reg_select);
    sound_prescaler_sel |= 2;
    change_sound_prescaler( pres[sound_prescaler_sel] );	/* 6 */
    break;
  case 0x2e:
    if( verbose_snd ) printf("SOUND out %X\n",sound_reg_select);
    sound_prescaler_sel |= 1;
    change_sound_prescaler( pres[sound_prescaler_sel] );	/* 3 */
    break;
  case 0x2f:
    if( verbose_snd ) printf("SOUND out %X\n",sound_reg_select);
    sound_prescaler_sel = 0;
    change_sound_prescaler( pres[sound_prescaler_sel] );	/* 2 */
    break;

  }
}


/********************************************************/
/* サウンドのステータスを入力				*/
/********************************************************/
byte	sound_in_status( void )
{
  xmame_dev_sound_in_status();

  return ( sound_FLAG_B << 1 ) | sound_FLAG_A;	/* 常に ready */

}

/********************************************************/
/* 指定したレジスタの内容を入力				*/
/*	flag が 真なら、つねに SOUND BORD II 扱い	*/
/*	flag が 偽なら、判別				*/
/********************************************************/
byte	sound_in_data( int always_sound_II )
{
  xmame_dev_sound_in_data();

  if      ( sound_reg_select < 0x10 ){		/* 0x00〜0x0f はリード可 */

    return sound_reg[ sound_reg_select ];

  }else if( sound_reg_select == 0xff ){		/* 0xff は、SD/SD II の判定 */

    if( always_sound_II ) return 0x01;
    else{
      if( sound_port & SD_PORT_46_47 ) return 0x01;
      else                             return 0x00;
    }

  }else{					/* その他は読めない */

    return 0x00;

  }
}






/************************************************************************/
/* サウンドボードII部	(かなりいい加減)				*/
/************************************************************************/

/********************************************************/
/* データを入出力するレジスタの指定			*/
/********************************************************/
void	sound2_out_reg( byte data )
{
  sound2_reg_select = data & 0xff;

  xmame_dev_sound2_out_reg( data );
}

/********************************************************/
/* データを指定したレジスタに出力			*/
/********************************************************/
void	sound2_out_data( byte data )
{
  int	wk, l;

  sound2_reg[ sound2_reg_select ] = data;

  xmame_dev_sound2_out_data( data );

/*
if( sound2_reg_select==0x08 )
printf("[%05x = %04x : %02x] %02x\n",(int)sound2_data_addr,(int)sound2_data_addr>>sound2_mem_size,(int)sound2_data_addr&((1<<sound2_mem_size)-1),(int)data);
else
if( sound2_reg_select<=0x10 )
printf("%02x %02x\n",(int)sound2_reg_select,(int)data);
*/

  switch( sound2_reg_select ){
  case 0x00:					/* コントロール 1 */
    sound2_repeat = data & 0x10;
    sound2_mem    = data & 0x20;
    sound2_rec    = data & 0x40;

    if( sound2_FLAG_PCMBSY ){			/* ADPCM動作中 */
      if( !sound2_mem ) sound2_FLAG_PCMBSY = 0;
    }else{					/* 非動作中    */

      if( sound2_mem ){

	if( data & 0x80 ){				/* 録音再生 */

	  if( sound2_rec ){
	    sound2_stat = ADPCM_RECORD;
	    sound2_intr_base = sound2_record_intr_base;
	  }else{
	    sound2_stat = ADPCM_REPLAY;
	    sound2_intr_base = sound2_replay_intr_base;
	  }
	  interval_work_set_BDRY();

#define S sound2_start_addr
#define E sound2_stop_addr
#define L sound2_limit_addr
	  if( E == L ){
	    if( S < E ) l = E - S;
	    else        l = 0x3ffff - S;
	  }else{
	    if      ( S<E && S<L ){ if( E<L ) l = E - S;	 /* S< E< L */
	    			    else      l = 0;		 /* S< L< E */
	    }else if( E<S && E<L ){ if( S<L ) l = L - S + E;     /* E< S< L */
	    			    else      l = 0x3ffff - S;   /* E< L< S */
	    }else  /* L<S && L<E*/{ if( S<E ) l = E - S;         /* L< S< E */
				    else      l = 0x3ffff - S;   /* L< E< S */
	    }
	  }
	  if( l ){
	    sound2_notice_EOS = TRUE;
	    interval_work_set_EOS( l+1 );
	  }else{
	    sound2_notice_EOS = FALSE;
	  }
#undef S
#undef E
#undef L
	  sound2_FLAG_PCMBSY = 1;
	}else{						/* 読み書き */
	  if( sound2_rec ){
	    sound2_stat = ADPCM_WRITE;
	    /* if( !sound2_MSK_BRDY ) sound2_FLAG_BRDY = 1; */
	    sound2_delay |= 0x08;
	  }else{
	    sound2_stat = ADPCM_READ;
	    sound2_read_dummy = 2;
	  }
	  sound2_data_addr = sound2_start_addr;
	}
      }
    }
    if( data & 0x01 ){					/* リセット */
      sound2_repeat = 0;
      sound2_FLAG_PCMBSY = 0;
    }
    if( !sound2_mem ) sound2_stat = ADPCM_HALT;
    break;

  case 0x01:					/* コントロール 2 */
    sound2_mem_size = (data & 0x02) ? 5 : 2;
    break;

  case 0x02:					/* スタートアドレス */
  case 0x03:
    sound2_start_addr  = ((int)sound2_reg[ 0x03 ] << 8) | sound2_reg[ 0x02 ];
    sound2_start_addr  = (sound2_start_addr << sound2_mem_size);
    sound2_start_addr &= 0x3ffff;
    sound2_data_addr = sound2_start_addr;
    break;

  case 0x04:					/* ストップアドレス */
  case 0x05:
    sound2_stop_addr  = ((int)sound2_reg[ 0x05 ] << 8) | sound2_reg[ 0x04 ];
    sound2_stop_addr  = ((sound2_stop_addr+1) << sound2_mem_size ) -1;
    sound2_stop_addr &= 0x3ffff;
    break;

  case 0x08:					/* データ書き込み */
    if( sound2_stat==ADPCM_WRITE ){
#if 0
      if( sound2_data_addr != sound2_stop_addr ){
	sound2_adpcm[ sound2_data_addr ] = data;
/*
	if( sound2_data_addr == sound2_limit_addr )
	  sound2_data_addr = 0;
	else
*/
	  sound2_data_addr = (sound2_data_addr+1) & 0x3ffff;
	/* sound2_FLAG_BRDY = sound2_MSK_BRDY ? 0 : 1; */
	sound2_delay |= 0x08;
      }
      if( sound2_data_addr == sound2_stop_addr ||
	  sound2_data_addr == 0x3ffff ){
	/* sound2_FLAG_EOS  = sound2_MSK_EOS  ? 0 : 1; */
	sound2_delay |= 0x04;
      }
#else
      if( sound2_data_addr == sound2_stop_addr ||
	  sound2_data_addr == 0x3ffff ){
	/* sound2_FLAG_EOS  = sound2_MSK_EOS  ? 0 : 1; */
	sound2_delay |= 0x04;
      }
      sound2_adpcm[ sound2_data_addr ] = data;
/*
      if( sound2_data_addr == sound2_limit_addr )
	sound2_data_addr = 0;
      else
*/
	sound2_data_addr = (sound2_data_addr+1) & 0x3ffff;
      /* sound2_FLAG_BRDY = sound2_MSK_BRDY ? 0 : 1; */
      sound2_delay |= 0x08;
#endif
    }
    break;

  case 0x0c:					/* リミットアドレス */
  case 0x0d:
    sound2_limit_addr  = ((int)sound2_reg[ 0x0d ] << 8) | sound2_reg[ 0x0c ];
    sound2_limit_addr  = ((sound2_limit_addr+1)<< sound2_mem_size ) -1;
    sound2_limit_addr &= 0x3ffff;
    break;

  case 0x06:					/* プリスケール */
  case 0x07:
    wk = (((int)sound2_reg[ 0x07 ]&0x07) << 8) | sound2_reg[ 0x06 ];
    wk &= 0x7ff;
    if( wk==0 ) wk = 0x800;
    sound2_record_intr_base = wk;
    break;

  case 0x09:					/* デルタ-N */
  case 0x0a:
    wk = ((int)sound2_reg[ 0x0a ] << 8) | sound2_reg[ 0x09 ];
    if( wk==0 ) wk = 0x10000;
    sound2_replay_intr_base = 72 * 65536 / wk;
    break;

  case 0x10:					/* 割り込みフラグ制御 */
    change_sound_flags( 0x10 );
    break;
  }
}


/********************************************************/
/* サウンドボードIIのステータス入力			*/
/********************************************************/
byte	sound2_in_status( void )
{
  xmame_dev_sound2_in_status();

  if( sound2_delay & 0x08 ){
    sound2_delay &= ~0x08;
    if( !sound2_MSK_BRDY ) sound2_FLAG_BRDY = 1;	      
  }
  if( sound2_delay & 0x04 ){
    sound2_delay &= ~0x04;
    if( !sound2_MSK_EOS ) sound2_FLAG_EOS = 1;	      
  }

  /* sound2_FLAG_ZERO の生成方法がわからん */

  return ( sound2_FLAG_PCMBSY << 5 ) | ( sound2_FLAG_ZERO << 4 ) |
         ( sound2_FLAG_BRDY << 3 ) | ( sound2_FLAG_EOS << 2 ) |
	 ( sound_FLAG_B << 1 ) | sound_FLAG_A;
}

/********************************************************/
/* 指定したレジスタの内容を入力				*/
/********************************************************/
byte	sound2_in_data( void )
{
  byte data = 0x00;

  xmame_dev_sound2_in_data();

  if( sound2_reg_select==0x08 ){		/* データ読み出し */

    if( sound2_stat==ADPCM_READ ){
      if( sound2_read_dummy ){
	sound2_read_dummy --;
      }else{
	if( sound2_data_addr != sound2_stop_addr ){
	  data = sound2_adpcm[ sound2_data_addr ];
/*
	  if( sound2_data_addr == sound2_limit_addr )
	    sound2_data_addr = 0;
	  else
*/
	    sound2_data_addr = (sound2_data_addr+1) & 0x3ffff;
	}
      }
      if( sound2_data_addr != sound2_stop_addr ){
	sound2_FLAG_BRDY = sound2_MSK_BRDY ? 0 : 1;
      }
      return data;
    }

  }

  return 0x00;
}







/***********************************************************************
 * ステートロード／ステートセーブ
 ************************************************************************/

#define	SID	"SND "
#define	SID2	"SND2"
#define	SID3	"SND3"

static	T_SUSPEND_W	suspend_sound_work[] =
{
  { TYPE_256,	sound_reg,	},
  { TYPE_256,	sound2_reg,	},

  { TYPE_INT,	&sound_board,		},
  { TYPE_INT,	&sound_port,		},	/* 再初期化するので不要だが */

  { TYPE_INT,	&intr_sound_enable,	},
  { TYPE_INT,	&sound_ENABLE_A,	},
  { TYPE_INT,	&sound_ENABLE_B,	},
  { TYPE_INT,	&sound_LOAD_A,		},
  { TYPE_INT,	&sound_LOAD_B,		},
  { TYPE_INT,	&sound_FLAG_A,		},
  { TYPE_INT,	&sound_FLAG_B,		},
  { TYPE_INT,	&sound_TIMER_A,		},
  { TYPE_INT,	&sound_TIMER_B,		},
  { TYPE_INT,	&sound_prescaler,	},
  { TYPE_INT,	&sound_reg_select,	},

  { TYPE_INT,	&sound2_MSK_TA,		},
  { TYPE_INT,	&sound2_MSK_TB,		},
  { TYPE_INT,	&sound2_MSK_EOS,	},
  { TYPE_INT,	&sound2_MSK_BRDY,	},
  { TYPE_INT,	&sound2_MSK_ZERO,	},

  { TYPE_INT,	&sound2_EN_TA,		},
  { TYPE_INT,	&sound2_EN_TB,		},
  { TYPE_INT,	&sound2_EN_EOS,		},
  { TYPE_INT,	&sound2_EN_BRDY,	},
  { TYPE_INT,	&sound2_EN_ZERO,	},

  { TYPE_INT,	&sound2_FLAG_EOS,	},
  { TYPE_INT,	&sound2_FLAG_BRDY,	},
  { TYPE_INT,	&sound2_FLAG_ZERO,	},
  { TYPE_INT,	&sound2_FLAG_PCMBSY,	},

  { TYPE_INT,	&sound2_reg_select,	},

  { TYPE_INT,	&sound2_mem,		},
  { TYPE_INT,	&sound2_rec,		},
  { TYPE_INT,	&sound2_repeat,		},
  { TYPE_INT,	&sound2_mem_size,	},
  { TYPE_INT,	&sound2_record_intr_base,},
  { TYPE_INT,	&sound2_replay_intr_base,},
  { TYPE_INT,	&sound2_start_addr,	},
  { TYPE_INT,	&sound2_stop_addr,	},
  { TYPE_INT,	&sound2_limit_addr,	},
  { TYPE_INT,	&sound2_read_dummy,	},
  { TYPE_INT,	&sound2_stat,		},
  { TYPE_INT,	&sound2_data_addr,	},
  { TYPE_INT,	&sound2_intr_base,	},
  { TYPE_INT,	&sound2_counter,	},
  { TYPE_INT,	&sound2_counter_old,	},

  { TYPE_INT,	&sound2_notice_EOS,	},
  { TYPE_INT,	&sound2_delay,		},

  { TYPE_END,	0			},
};

static	T_SUSPEND_W	suspend_sound_work2[] =
{
  { TYPE_INT,	&sound_prescaler_sel,	},
  { TYPE_END,	0			},
};

static	T_SUSPEND_W	suspend_sound_work3[] =
{
  { TYPE_INT,	&use_cmdsing,		},
  { TYPE_END,	0			},
};



int	statesave_sound( void )
{
  if( statesave_table( SID, suspend_sound_work ) != STATE_OK ) return FALSE;

  if( statesave_table( SID2, suspend_sound_work2 ) != STATE_OK ) return FALSE;

  if( statesave_table( SID3, suspend_sound_work3 ) != STATE_OK ) return FALSE;

  return TRUE;
}

int	stateload_sound( void )
{
  if( stateload_table( SID, suspend_sound_work ) != STATE_OK ) return FALSE;

  if( stateload_table( SID2, suspend_sound_work2 ) != STATE_OK ){

    /* 旧バージョンなら、みのがす */

    printf( "stateload : Statefile is old. (ver 0.6.0 or 1?)\n" );

    if      ( sound_prescaler == 6 ) sound_prescaler_sel = 2;
    else if ( sound_prescaler == 3 ) sound_prescaler_sel = 3;
    else                      /* 2*/ sound_prescaler_sel = 0;	/* or 1 */
  }

  if( stateload_table( SID3, suspend_sound_work3 ) != STATE_OK ){

    /* 旧バージョンなら、みのがす */

    printf( "stateload : Statefile is old. (ver 0.6.0, 1, 2 or 3?)\n" );
  }

  return TRUE;
}
















#ifdef	USE_SOUND

void	sound_output_after_stateload( void )
{
  /* レジューム後の再初期化 */
  /* こんなんでうまくいくのか？？？ */
  
  const int (*addr)[2];
  int size;

  static const int addr_2203[][2] = {
    { 0x00, 0x0f },
    { 0x24, 0x28 },
    { 0x30, 0x3e },
    { 0x40, 0x4e },
    { 0x50, 0x5e },
    { 0x60, 0x6e },
    { 0x70, 0x7e },
    { 0x80, 0x8e },
    { 0x90, 0x9e },
    { 0xa0, 0xa2 },
    { 0xa4, 0xa6 },
    { 0xa8, 0xaa },
    { 0xac, 0xae },
    { 0xb0, 0xb2 },
  };
  static const int addr_2608[][2] = {
    { 0x00, 0x0f },
    { 0x11, 0x12 },
    { 0x18, 0x1d },
    { 0x22, 0x22 },
    { 0x24, 0x29 },
    { 0x30, 0x3e },
    { 0x40, 0x4e },
    { 0x50, 0x5e },
    { 0x60, 0x6e },
    { 0x70, 0x7e },
    { 0x80, 0x8e },
    { 0x90, 0x9e },
    { 0xa0, 0xa2 },
    { 0xa4, 0xa6 },
    { 0xa8, 0xaa },
    { 0xac, 0xae },
    { 0xb0, 0xb2 },
  /*{ 0xb4, 0xb6 },*/		/* このレジスタを resume させると音が変 */
  };
  static const int addr_2608_2[][2] = {
    { 0x01, 0x10 },
    { 0x30, 0x3e },
    { 0x40, 0x4e },
    { 0x50, 0x5e },
    { 0x60, 0x6e },
    { 0x70, 0x7e },
    { 0x80, 0x8e },
    { 0x90, 0x9e },
    { 0xa0, 0xa2 },
    { 0xa4, 0xa6 },
    { 0xa8, 0xaa },
    { 0xac, 0xae },
    { 0xb0, 0xb2 },
    { 0xb4, 0xb6 },
  };

  if( xmame_has_sound() ){
    int	i, j;

#if 0
    {
      int  vol = xmame_cfg_get_mastervolume();
      int pvol = xmame_cfg_get_mixer_volume( XMAME_MIXER_PSG );
      int fvol = xmame_cfg_get_mixer_volume( XMAME_MIXER_FM );
      int bvol = xmame_cfg_get_mixer_volume( XMAME_MIXER_BEEP );

	/* ミュート */

      xmame_cfg_set_mastervolume( 0 );
      xmame_cfg_set_mixer_volume( XMAME_MIXER_PSG,  0 );
      xmame_cfg_set_mixer_volume( XMAME_MIXER_FM,   0 );
      xmame_cfg_set_mixer_volume( XMAME_MIXER_BEEP, 0 );
    }
#endif

	/* サウンド レジスタ出力 (必要なポートのみ) */

    switch( sound_prescaler_sel ){
    case 0:
      xmame_dev_sound_out_reg( 0x2f );      xmame_dev_sound_out_data( 0 );
      break;
    case 1:
      xmame_dev_sound_out_reg( 0x2f );      xmame_dev_sound_out_data( 0 );
      xmame_dev_sound_out_reg( 0x2e );      xmame_dev_sound_out_data( 0 );
      break;
    case 3:
      xmame_dev_sound_out_reg( 0x2e );      xmame_dev_sound_out_data( 0 );
      break;
    default:
      break;
    }


    if( sound_board==SOUND_I ){ addr = addr_2203; size = COUNTOF(addr_2203); }
    else                      { addr = addr_2608; size = COUNTOF(addr_2608); }

    for( i=0; i<256; i++ ){

      for( j=0; j<size; j++ ){
	if( addr[j][0] <= i  &&  i <= addr[j][1] ){
	  xmame_dev_sound_out_reg( i );
	  xmame_dev_sound_out_data( sound_reg[ i ] );
	  break;
	}
      }

    }

    if( sound_board==SOUND_II ){
      addr = addr_2608_2;
      size = COUNTOF(addr_2608_2);

      for( i=0; i<256; i++ ){

	for( j=0; j<size; j++ ){
	  if( addr[j][0] <= i  &&  i <= addr[j][1] ){
	    xmame_dev_sound2_out_reg( i );
	    xmame_dev_sound2_out_data( sound2_reg[ i ] );
	    break;
	  }
	}
	
      }

      i = sound2_reg[ 0 ];
      i &= ~0x80;
      xmame_dev_sound2_out_reg( 0 );
      xmame_dev_sound2_out_data( i );
    }

#if 0
	/* 音量復帰 */
    {
      xmame_cfg_set_mastervolume( vol );
      xmame_cfg_set_mixer_volume( XMAME_MIXER_PSG,  pvol );
      xmame_cfg_set_mixer_volume( XMAME_MIXER_FM,   fvol );
      xmame_cfg_set_mixer_volume( XMAME_MIXER_BEEP, bvol );
    }
#endif

    xmame_dev_beep_cmd_sing(use_cmdsing);

    if( ctrl_signal & (0x80|0x20) ){
      xmame_dev_beep_out_data( ctrl_signal );
    }

  }

}



#else

void	sound_output_after_stateload( void )
{
}

#endif
