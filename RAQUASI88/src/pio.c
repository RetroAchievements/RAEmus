/************************************************************************/
/*									*/
/* PIO の 処理								*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "pio.h"

#include "pc88cpu.h"

#include "emu.h"
#include "suspend.h"


/*
  PIOアクセスと、サブCPUの駆動
	  PC88 は、本体側に メインCPU、ディスクドライブ側に サブCPU と、
	2個のCPUを持っている。ディスクアクセスをする場合は、メインCPU が
	サブCPU に PIO経由でコマンドを送信し、それを受信した サブCPU が
	実際に処理を行って、結果をメインCPU に送信するようになっている。
	つまり、サブCPUはディスク処理中以外の時間は全て、メインCPU 
	からのコマンドの受信待ちということになる。
	  以上のことより、サブCPUを常時エミュレートするのは無駄なので、
	必要な時のみエミュレートすることにする。

	  通常、メイン・サブCPUとも通信時には、PIO の Cポートをリードする。
	そこで、メインCPU が Cポートをリードした時以降は、サブCPU のみを
	駆動させ、サブCPU が Cポートをリードした時以降は、メインCPU のみを
	駆動させるようにする。
		(-cpu 0 オプションを指定した時は上記の動作をする)

	  しかし、中には Cポートを介さずに、通信を行うアプリケーションも
	存在する。(2個のCPUの処理速度の同期に依存している)。この対策として、
	A/B/C ポートのいずれかにメインCPUがアクセスした時点から一定期間
	(標準では 4000ステップ)、サブCPUも同時に駆動させる。
		(-cpu 1 オプションを指定した時は上記の動作をする)

	  それでもまだ動かないアプリケーションがあるので、この対策として、
	メインCPU とサブCPU を常時、同時に駆動させる。
		(-cpu 2 オプションを指定した時は上記の動作をする)
*/

/*---------------------------------------------------------------------------*/
/* 処理									     */
/*					A  ######## --\/-- ######## A	     */
/*	・ライトは自分のワークに	B  ######## --/\-- ######## B	     */
/*	  対して行なう。		CH ####     --\/-- ####	    CH	     */
/*	・リードは相手のワーク		CL     #### --/\--     #### CL	     */
/*	  から行なう。							     */
/*	  自分のワークがREAD設定なら、					     */
/*	  自分のワークを読む。						     */
/*	○ポートCのリードライト時は例外処理を行なう。			     */
/*---------------------------------------------------------------------------*/


pio_work	pio_AB[2][2], pio_C[2][2];


static	z80arch	*z80[2] = { &z80main_cpu, &z80sub_cpu };




/*----------------------------------------------------------------------*/
/* PIO 初期化								*/
/*	PA / PCL 受信 ／ PB / PCH 送信					*/
/*----------------------------------------------------------------------*/
void	pio_init( void )
{
  int	side;

  for( side=0; side<2; side++ ){
    pio_AB[ side ][ PIO_PORT_A ].type    = PIO_READ;
    pio_AB[ side ][ PIO_PORT_A ].exist   = PIO_EMPTY;
/*  pio_AB[ side ][ PIO_PORT_A ].cont_f  = not used ! */
    pio_AB[ side ][ PIO_PORT_A ].data    = 0x00;

    pio_AB[ side ][ PIO_PORT_B ].type    = PIO_WRITE;
    pio_AB[ side ][ PIO_PORT_B ].exist   = PIO_EMPTY;
/*  pio_AB[ side ][ PIO_PORT_B ].cont_f  = not used ! */
    pio_AB[ side ][ PIO_PORT_B ].data    = 0x00;

    pio_C[ side ][ PIO_PORT_CH ].type    = PIO_WRITE;
    pio_C[ side ][ PIO_PORT_CH ].cont_f  = 1;
    pio_C[ side ][ PIO_PORT_CH ].data    = 0x00;

    pio_C[ side ][ PIO_PORT_CL ].type    = PIO_READ;
    pio_C[ side ][ PIO_PORT_CL ].cont_f  = 1;
    pio_C[ side ][ PIO_PORT_CL ].data    = 0x00;
  }

}


/* verbose 指定時のメッセージ表示マクロ					*/

#define	pio_mesAB( s )							\
	if( verbose_pio )						\
	  printf( s " : side = %s : port = %s\n",			\
		 (side==PIO_SIDE_M)?"M":"S", (port==PIO_PORT_A)?"A":"B" )

#define	pio_mesC( s )							\
	if( verbose_pio )						\
	  printf( s " : side = %s\n",					\
		 (side==PIO_SIDE_M)?"M":"S" )

/*----------------------------------------------------------------------*/
/* PIO A or B からリード						*/
/*	リードの際のデータは、相手の側／逆のポートから読み出す。	*/
/*		相手のポートの設定が  READ ならエラー表示		*/
/*		自分のポートの設定が WRITE ならエラー表示		*/
/*		連続リードの際は、カウンタをカウントダウンする。	*/
/*			カウンタが 0 なら諦めて、連続リードする。	*/
/*			カウンタが 1 以上なら、CPU を切替える。		*/
/*----------------------------------------------------------------------*/
byte	pio_read_AB( int side, int port )
{
		/* ポート属性不一致 */

  if( pio_AB[ side^1 ][ port^1 ].type == PIO_READ  ){	/* 相手ポートが READ */
    pio_mesAB( "PIO AB READ PORT Mismatch" );
  }
  if( pio_AB[ side   ][ port   ].type == PIO_WRITE ){	/* 自分ポートが WRITE*/
    pio_mesAB( "PIO Read from WRITE-PORT" );
    return (pio_AB[ side ][ port ].data);
  }
		/* 読みだし */

  if( pio_AB[ side^1 ][ port^1 ].exist == PIO_EXIST ){	/* -- 最初の読みだし */

    pio_AB[ side^1 ][ port^1 ].exist   = PIO_EMPTY;

  }else{						/* -- 連続の読みだし */

    switch( cpu_timing ){
    case 1:						/*     1:サブCPU起動 */
      if( side==PIO_SIDE_M ){
	dual_cpu_count = CPU_1_COUNT;
	CPU_BREAKOFF();
      } /*No Break*/
    case 0:						/*     0:そのまま読む*/
    case 2:						/*     2:そのまま読む*/
      pio_mesAB( "PIO Read continuously" );
      break;
    }

  }
  return (pio_AB[ side^1 ][ port^1 ].data);
}


/*----------------------------------------------------------------------*/
/* PIO A or B にライト							*/
/*	ライトは、自分の側／自分のポートに対して行なう。		*/
/*		相手のポートの設定が WRITE ならエラー表示		*/
/*		自分のポートの設定が  READ ならエラー表示		*/
/*		連続ライトの際は、カウンタをカウントダウンする。	*/
/*			カウンタが 0 なら諦めて、連続ライトする。	*/
/*			カウンタが 1 以上なら、CPU を切替える。		*/
/*----------------------------------------------------------------------*/
void	pio_write_AB( int side, int port, byte data )
{
		/* ポート属性不一致 */

  if( pio_AB[ side^1 ][ port^1 ].type == PIO_WRITE ){	/* 相手のポート WRITE*/
    pio_mesAB( "PIO AB Write PORT Mismatch" );
  }
  if( pio_AB[ side   ][ port   ].type == PIO_READ ){	/* 自分のポート READ */
    pio_mesAB( "PIO Write to READ-PORT" );
  }
		/* 書き込み */

  if( pio_AB[ side ][ port ].exist == PIO_EMPTY ){	/* -- 最初の書き込み */

    pio_AB[ side ][ port ].exist   = PIO_EXIST;
    pio_AB[ side ][ port ].data    = data;

  }else{						/* -- 連続の書き込み */

    switch( cpu_timing ){
    case 1:						/*     1:サブCPU起動 */
      if( side==PIO_SIDE_M ){
	dual_cpu_count = CPU_1_COUNT;
	CPU_BREAKOFF();
      } /*No Break*/
    case 0:						/*     0:そのまま書く*/
    case 2:						/*     2:そのまま書く*/
      pio_mesAB( "PIO Write continuously" );
      pio_AB[ side ][ port ].data    = data;
      break;
    }

  }
  return;
}








/*----------------------------------------------------------------------*/
/* PIO C からリード							*/
/*	リードの際のデータは、相手の側／逆のポートから読み出す。	*/
/*		相手のポートの設定が  READ ならエラー表示		*/
/*		自分のポートの設定が WRITE ならエラー表示		*/
/*		リードの際に、CPUを切替え判定を入れる			*/
/*----------------------------------------------------------------------*/
byte	pio_read_C( int side )
{
  byte	data;

		/* ポート属性不一致 */
  if( pio_C[ side^1 ][ PIO_PORT_CH ].type == PIO_READ  &&
      pio_C[ side^1 ][ PIO_PORT_CL ].type == PIO_READ  ){
    pio_mesC( "PIO C READ PORT Mismatch" );
  }
  if( pio_C[ side   ][ PIO_PORT_CH ].type == PIO_WRITE &&
      pio_C[ side   ][ PIO_PORT_CL ].type == PIO_WRITE ){
    pio_mesC( "PIO C Read from WRITE-PORT" );
  }
		/* リード */

  if( pio_C[ side ][ PIO_PORT_CH ].type == PIO_READ ){
    data  = pio_C[ side^1 ][ PIO_PORT_CL ].data << 4;
  }else{
    data  = pio_C[ side   ][ PIO_PORT_CH ].data << 4;
  }

  if( pio_C[ side ][ PIO_PORT_CL ].type == PIO_READ ){
    data |= pio_C[ side^1 ][ PIO_PORT_CH ].data;
  }else{
    data |= pio_C[ side   ][ PIO_PORT_CL ].data;
  }

  pio_C[ side ][ PIO_PORT_CL ].cont_f ^= 1;
  if( pio_C[ side ][ PIO_PORT_CL ].cont_f == 0 ){	/* -- 連続の読みだし */

    switch( cpu_timing ){
    case 0:						/*     0:CPUを切替え*/
      select_main_cpu ^= 1;
      CPU_BREAKOFF();        /* PC-=2 */
      break;
    case 1:						/*     1:サブCPU起動 */
      if( side==PIO_SIDE_M ){
	dual_cpu_count = CPU_1_COUNT;
	CPU_BREAKOFF();
      }
      break;
    case 2:						/*     2:なにもしない*/
      break;
    }

  }

  return data;
}


/*----------------------------------------------------------------------*/
/* PIO C にライト							*/
/*	ライトは、自分の側／自分のポートに対して行なう。		*/
/*		相手のポートの設定が WRITE ならエラー表示		*/
/*		自分のポートの設定が  READ ならエラー表示		*/
/*		ライトの際に、CPUを切替え判定を入れる			*/
/*----------------------------------------------------------------------*/
void	pio_write_C( int side, byte data )
{
  int port;

  if( data & 0x08 ) port = PIO_PORT_CH;
  else              port = PIO_PORT_CL;
  data &= 0x07;

		/* ポート属性不一致 */

  if( pio_C[ side^1 ][ port^1 ].type == PIO_WRITE ){	/* 相手のポート WRITE*/
    pio_mesC( "PIO C Write PORT Mismatch" );
  }
  if( pio_C[ side   ][ port   ].type == PIO_READ ){	/* 自分のポート READ */
    pio_mesC( "PIO C Write to READ-PORT" );
  }
		/* ライト */

  if( data & 0x01 ) pio_C[ side ][ port ].data |=  ( 1 << (data>>1) );
  else              pio_C[ side ][ port ].data &= ~( 1 << (data>>1) );

  switch( cpu_timing ){
  case 0:						/*     0:そのまま書く*/
  case 2:						/*     2:そのまま書く*/
    break;
  case 1:						/*     1:サブCPU起動 */
    if( side==PIO_SIDE_M ){
      dual_cpu_count = CPU_1_COUNT;
      CPU_BREAKOFF();
    }
    break;
  }
  return;
}


/*--------------------------------------------------------------*/
/* 直接 Port C に書き込む					*/
/*--------------------------------------------------------------*/
void	pio_write_C_direct( int side, byte data )
{
		/* ポート属性不一致 */
  if( pio_C[ side^1 ][ PIO_PORT_CH ].type == PIO_WRITE &&
      pio_C[ side^1 ][ PIO_PORT_CL ].type == PIO_WRITE ){
    pio_mesC( "PIO C WRITE PORT Mismatch" );
  }
  if( pio_C[ side   ][ PIO_PORT_CH ].type == PIO_READ  &&
      pio_C[ side   ][ PIO_PORT_CL ].type == PIO_READ  ){
    pio_mesC( "PIO C Write to READ-PORT" );
  }
		/* ライト */

  pio_C[ side ][ PIO_PORT_CH ].data = data >> 4;
  pio_C[ side ][ PIO_PORT_CL ].data = data & 0x0f;

  switch( cpu_timing ){
  case 0:						/*     0:そのまま書く*/
  case 2:						/*     2:そのまま書く*/
    break;
  case 1:						/*     1:サブCPU起動 */
    if( side==PIO_SIDE_M ){
      dual_cpu_count = CPU_1_COUNT;
      CPU_BREAKOFF();
    }
    break;
  }
  return;
}







/*----------------------------------------------------------------------*/
/* PIO 設定								*/
/*	PA / PB / PCH / PCL の送受信を指定。				*/
/*	モードを設定 (モードは 0 に限定。詳細不明)			*/
/*----------------------------------------------------------------------*/
void	pio_set_mode( int side, byte data )
{
  if( data & 0x60 ){
    if( verbose_pio )
      printf("PIO mode A & CH not 0 : side = %s : mode = %d\n",
	     (side!=PIO_SIDE_M)?"M":"S", (data>>5)&0x3 );
  }
	/* PIO A */

  if( data & 0x10 ){
    pio_AB[ side ][ PIO_PORT_A ].type  = PIO_READ;
  }else{
    pio_AB[ side ][ PIO_PORT_A ].type   = PIO_WRITE;
  }
  pio_AB[ side ][ PIO_PORT_A ].data    = 0;
  pio_AB[ side ][ PIO_PORT_A ].exist   = PIO_EMPTY;

	/* PIO C-H */

  if( data & 0x08 ){
    pio_C[ side ][ PIO_PORT_CH ].type  = PIO_READ;
  }else{
    pio_C[ side ][ PIO_PORT_CH ].type  = PIO_WRITE;
  }
  pio_C[ side ][ PIO_PORT_CH ].data    = 0;
  pio_C[ side ][ PIO_PORT_CH ].cont_f  = 1;

  if( data & 0x04 ){
    if( verbose_pio )
      printf("PIO mode B & CL not 0 : side = %s : mode = %d\n",
	     (side!=PIO_SIDE_M)?"M":"S", (data>>2)&0x1 );
  }
	/* PIO B */

  if( data & 0x02 ){
    pio_AB[ side ][ PIO_PORT_B ].type  = PIO_READ;
  }else{
    pio_AB[ side ][ PIO_PORT_B ].type  = PIO_WRITE;
  }
  pio_AB[ side ][ PIO_PORT_B ].data    = 0;
  pio_AB[ side ][ PIO_PORT_B ].exist   = PIO_EMPTY;

	/* PIO C-L */

  if( data & 0x01 ){
    pio_C[ side ][ PIO_PORT_CL ].type  = PIO_READ;
  }else{
    pio_C[ side ][ PIO_PORT_CL ].type  = PIO_WRITE;
  }
  pio_C[ side ][ PIO_PORT_CL ].data    = 0;
  pio_C[ side ][ PIO_PORT_CL ].cont_f  = 1;

}






/***********************************************************************
 * ステートロード／ステートセーブ
 ************************************************************************/

#define	SID	"PIO "

static	T_SUSPEND_W	suspend_pio_work[]=
{
  { TYPE_INT,	&pio_AB[0][0].type,	},
  { TYPE_INT,	&pio_AB[0][0].exist,	},
  { TYPE_INT,	&pio_AB[0][0].cont_f,	},
  { TYPE_BYTE,	&pio_AB[0][0].data,	},

  { TYPE_INT,	&pio_AB[0][1].type,	},
  { TYPE_INT,	&pio_AB[0][1].exist,	},
  { TYPE_INT,	&pio_AB[0][1].cont_f,	},
  { TYPE_BYTE,	&pio_AB[0][1].data,	},

  { TYPE_INT,	&pio_AB[1][0].type,	},
  { TYPE_INT,	&pio_AB[1][0].exist,	},
  { TYPE_INT,	&pio_AB[1][0].cont_f,	},
  { TYPE_BYTE,	&pio_AB[1][0].data,	},

  { TYPE_INT,	&pio_AB[1][1].type,	},
  { TYPE_INT,	&pio_AB[1][1].exist,	},
  { TYPE_INT,	&pio_AB[1][1].cont_f,	},
  { TYPE_BYTE,	&pio_AB[1][1].data,	},

  { TYPE_INT,	&pio_C[0][0].type,	},
  { TYPE_INT,	&pio_C[0][0].exist,	},
  { TYPE_INT,	&pio_C[0][0].cont_f,	},
  { TYPE_BYTE,	&pio_C[0][0].data,	},

  { TYPE_INT,	&pio_C[0][1].type,	},
  { TYPE_INT,	&pio_C[0][1].exist,	},
  { TYPE_INT,	&pio_C[0][1].cont_f,	},
  { TYPE_BYTE,	&pio_C[0][1].data,	},

  { TYPE_INT,	&pio_C[1][0].type,	},
  { TYPE_INT,	&pio_C[1][0].exist,	},
  { TYPE_INT,	&pio_C[1][0].cont_f,	},
  { TYPE_BYTE,	&pio_C[1][0].data,	},

  { TYPE_INT,	&pio_C[1][1].type,	},
  { TYPE_INT,	&pio_C[1][1].exist,	},
  { TYPE_INT,	&pio_C[1][1].cont_f,	},
  { TYPE_BYTE,	&pio_C[1][1].data,	},

  { TYPE_END,	0			},
};


int	statesave_pio( void )
{
  if( statesave_table( SID, suspend_pio_work ) == STATE_OK ) return TRUE;
  else                                                       return FALSE;
}

int	stateload_pio( void )
{
  if( stateload_table( SID, suspend_pio_work ) == STATE_OK ) return TRUE;
  else                                                       return FALSE;
}
