/************************************************************************/
/*									*/
/* エミュモード								*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "initval.h"
#include "emu.h"

#include "pc88cpu.h"

#include "screen.h"
#include "keyboard.h"
#include "intr.h"
#include "event.h"
#include "menu.h"
#include "monitor.h"
#include "pause.h"
#include "wait.h"
#include "suspend.h"
#include "status.h"
#include "graph.h"
#include "snddrv.h"




break_t		break_point[2][NR_BP];	/* ブレークポイント		*/
break_drive_t	break_point_fdc[NR_BP];	/* FDC ブレークポイント		*/


int	cpu_timing	= DEFAULT_CPU;		/* SUB-CPU 駆動方式	*/

int	select_main_cpu = TRUE;			/* -cpu 0 実行するCPU	*/
						/* 真なら MAIN CPUを実行*/

int	dual_cpu_count	= 0;			/* -cpu 1 同時処理STEP数*/
int	CPU_1_COUNT	= 4000;			/* その、初期値		*/

int	cpu_slice_us    = 5;			/* -cpu 2 処理時分割(us)*/
						/* 10>でSILPHEEDが動かん*/

int	trace_counter	= 1;			/* TRACE 時のカウンタ	*/



static	int	main_state   = 0;
static	int	sub_state    = 0;
#define	JACKUP	(256)


static	int	emu_mode_execute= GO;
static	int	emu_rest_step;

void	set_emu_exec_mode( int mode )
{
  emu_mode_execute = mode;
}

/***********************************************************************
 * エミュレート処理の初期化関連
 ************************************************************************/

void	emu_reset( void )
{
  select_main_cpu = TRUE;
  dual_cpu_count  = 0;

  main_state   = 0;
  sub_state    = 0;
}


void	emu_breakpoint_init( void )
{
  int	i, j;
	/* ブレークポイントのワーク初期化 (モニターモード用) */
  for( j=0; j<2; j++ )
    for( i=0; i<NR_BP; i++ )
      break_point[j][i].type = BP_NONE;

  for( i=0; i<NR_BP; i++ )
    break_point_fdc[i].type = BP_NONE;
}




/***********************************************************************
 * CPU実行処理 (EXEC) の制御
 *	-cpu <n> に応じて、動作を変える。
 *
 *	STEP  時は、1step だけ実行する。
 *	TRACE 時は、指定回数分、1step 実行する。
 *
 *	ブレークポイント指定時は、1step実行の度に PC がブレークポイントに
 *	達したかどうかを確認する。
 *
 ************************************************************************/

#define	INFINITY	(0)
#define	ONLY_1STEP	(1)

/*------------------------------------------------------------------------*/

/*
 * ブレークポイント (タイプ PC) の有無をチェックする
 */

static	int	check_break_point_PC( void )
{
  int	i, j;

  for( i=0; i<NR_BP; i++ ) if( break_point[BP_MAIN][i].type == BP_PC ) break;
  for( j=0; j<NR_BP; j++ ) if( break_point[BP_SUB][j].type  == BP_PC ) break;

  if( i==NR_BP && j==NR_BP ) return FALSE;
  else                       return TRUE;
}

/*------------------------------------------------------------------------*/

/*
 * CPU を 1step 実行して、PCがブレークポイントに達したかチェックする
 *	ブレークポイント(タイプPC)未設定ならこの関数は使わず、z80_emu()を使う
 */

static	int	z80_emu_with_breakpoint( z80arch *z80, int unused )
{
  int i, cpu, states;

  states = z80_emu( z80, 1 );		/* 1step だけ実行 */

  if( z80==&z80main_cpu ) cpu = BP_MAIN;
  else                    cpu = BP_SUB;

  for( i=0; i<NR_BP; i++ ){
    if( break_point[cpu][i].type == BP_PC     &&
	break_point[cpu][i].addr == z80->PC.W ){

      if( i==BP_NUM_FOR_SYSTEM ){
	break_point[cpu][i].type = BP_NONE;
      }

      printf( "*** Break at %04x *** ( %s[#%d] : PC )\n",
	      z80->PC.W, (cpu==BP_MAIN)?"MAIN":"SUB", i+1 );

      quasi88_debug();
    }
  }

  return states;
}

/*---------------------------------------------------------------------------*/

static	int	passed_step;		/* 実行した step数 */
static	int	target_step;		/* この step数に達するまで実行する */

static	int	infinity, only_1step;
static	int	(*z80_exec)( z80arch *, int );


void	emu_init(void)
{
/*xmame_sound_update();*/
  xmame_update_video_and_audio();
  event_update();
/*keyboard_update();*/



/*screen_set_dirty_all();*/
/*screen_set_dirty_palette();*/

  /* ステータス部クリア */
  status_message_default(0, NULL);
  status_message_default(1, NULL);
  status_message_default(2, NULL);



	/* ブレークポイント設定の有無で、呼び出す関数を変える */
  if( check_break_point_PC() ) z80_exec = z80_emu_with_breakpoint;
  else                         z80_exec = z80_emu;


	/* GO/TRACE/STEP/CHANGE に応じて処理の繰り返し回数を決定 */

  passed_step = 0;

  switch( emu_mode_execute ){
  default:
  case GO:
    target_step = 0;			/* 無限に実行 */
    infinity    = INFINITY;
    only_1step  = ONLY_1STEP;
    break;

  case TRACE:
    target_step = trace_counter;	/* 指定ステップ数実行 */
    infinity    = ONLY_1STEP;
    only_1step  = ONLY_1STEP;
    break;

  case STEP:
    target_step = 1;			/* 1ステップ実行 */
    infinity    = ONLY_1STEP;
    only_1step  = ONLY_1STEP;
    break;

  case TRACE_CHANGE:
    target_step = 0;			/* 無限に実行 */
    infinity    = ONLY_1STEP;
    only_1step  = ONLY_1STEP;
    break;
  }


  /* 実行する残りステップ数。
	TRACE / STEP の時は、指定されたステップ数。
	GO / TRACE_CHANGE なら 無限なので、 0。
		なお、途中でメニューに遷移した場合、強制的に 1 がセットされる。
		これにより無限に処理する場合でも、ループを抜けるようになる。 */
  emu_rest_step = target_step;
}


void	emu_main(void)
{
  int	wk;

  profiler_lapse( PROF_LAPSE_CPU );

  switch( emu_mode_execute ){

  /*------------------------------------------------------------------------*/
  case GO:				/* ひたすら実行する           */
  case TRACE:				/* 指定したステップ、実行する */
  case STEP:				/* 1ステップだけ、実行する    */

    for(;;){

      switch( cpu_timing ){

      case 0:		/* select_main_cpu で指定されたほうのCPUを無限実行 */
	if( select_main_cpu ) (z80_exec)( &z80main_cpu, infinity );
	else                  (z80_exec)( &z80sub_cpu,  infinity );
	break;

      case 1:		/* dual_cpu_count==0 ならメインCPUを無限実行、*/
			/*               !=0 ならメインサブを交互実行 */
	if( dual_cpu_count==0 ) (z80_exec)( &z80main_cpu, infinity   );
	else{
	  (z80_exec)( &z80main_cpu, only_1step );
	  (z80_exec)( &z80sub_cpu,  only_1step );
	  dual_cpu_count --;
	}
	break;

      case 2:		/* メインCPU、サブCPUを交互に 5us ずつ実行 */
	if( main_state < 1*JACKUP  &&  sub_state < 1*JACKUP ){
	  main_state += (int) ((cpu_clock_mhz * cpu_slice_us) * JACKUP);
	  sub_state  += (int) ((3.9936        * cpu_slice_us) * JACKUP);
	}
	if( main_state >= 1*JACKUP ){
	  wk = (infinity==INFINITY) ? main_state/JACKUP : ONLY_1STEP;
	  main_state -= (z80_exec( &z80main_cpu, wk ) ) * JACKUP;
	}
	if( sub_state >= 1*JACKUP ){
	  wk = (infinity==INFINITY) ? sub_state/JACKUP : ONLY_1STEP;
	  sub_state  -= (z80_exec( &z80sub_cpu, wk ) ) * JACKUP;
	}
	break;
      }

      /* TRACE/STEP実行時、規定ステップ実行完了したら、モニターに遷移する */
      if( emu_rest_step ){
	passed_step ++;
	if( -- emu_rest_step <= 0 ) {
	  quasi88_debug();
	}
      }

      /* サウンド出力タイミングであれば、処理 */
      if (quasi88_event_flags & EVENT_AUDIO_UPDATE) {
	quasi88_event_flags &= ~EVENT_AUDIO_UPDATE;

	profiler_lapse( PROF_LAPSE_SND );

	xmame_sound_update();			/* サウンド出力 */

	profiler_lapse( PROF_LAPSE_AUDIO );

	xmame_update_video_and_audio();		/* サウンド出力 その2 */

	profiler_lapse( PROF_LAPSE_INPUT );

	event_update();				/* イベント処理		*/
	keyboard_update();

	profiler_lapse( PROF_LAPSE_CPU2 );
      }

      /* ビデオ出力タイミングであれば、CPU処理は一旦中止。上位に抜ける */
      if (quasi88_event_flags & EVENT_FRAME_UPDATE) {
	return;
      }

      /* モニター遷移時や終了時は、 CPU処理は一旦中止。上位に抜ける */
      if (quasi88_event_flags & (EVENT_DEBUG | EVENT_QUIT)) {
	return;
      }

      /* モード切替が発生しても、上位には抜けない。ビデオ出力まで待つ */
      /* (抜けると、 エミュ → 描画 → ウェイト の流れが崩れるので…) */
    }
    break;

  /*------------------------------------------------------------------------*/

	/* こっちのブレーク処理はうまく動かないかも・・・ (未検証) */

  case TRACE_CHANGE:			/* CPUが切り替わるまで処理をする */
    if( cpu_timing >= 1 ){
      printf( "command 'trace change' can use when -cpu 0\n");
      quasi88_monitor();
      break;
    }

    wk = select_main_cpu;
    while( wk==select_main_cpu ){
      if( select_main_cpu ) (z80_exec)( &z80main_cpu, infinity );
      else                  (z80_exec)( &z80sub_cpu,  infinity );
      if( emu_rest_step ){
	passed_step ++;
	if( -- emu_rest_step <= 0 ) {
	  quasi88_debug();
	}
      }

      if (quasi88_event_flags & EVENT_AUDIO_UPDATE) {
	quasi88_event_flags &= ~EVENT_AUDIO_UPDATE;
	profiler_lapse( PROF_LAPSE_SND );
	xmame_sound_update();			/* サウンド出力 */
	profiler_lapse( PROF_LAPSE_AUDIO );
	xmame_update_video_and_audio();		/* サウンド出力 その2 */
	profiler_lapse( PROF_LAPSE_INPUT );
	event_update();				/* イベント処理		*/
	keyboard_update();
      }

      if (quasi88_event_flags & EVENT_FRAME_UPDATE) {
	return;
      }
      if (quasi88_event_flags & (EVENT_DEBUG | EVENT_QUIT)) {
	return;
      }
    }
    quasi88_debug();
    break;
  }
  return;
}
















/***********************************************************************
 * ステートロード／ステートセーブ
 ************************************************************************/

#define	SID	"EMU "

static	T_SUSPEND_W	suspend_emu_work[] =
{
  { TYPE_INT,	&cpu_timing,		},
  { TYPE_INT,	&select_main_cpu,	},
  { TYPE_INT,	&dual_cpu_count,	},
  { TYPE_INT,	&CPU_1_COUNT,		},
  { TYPE_INT,	&cpu_slice_us,		},
  { TYPE_INT,	&main_state,		},
  { TYPE_INT,	&sub_state,		},

  { TYPE_END,	0			},
};


int	statesave_emu( void )
{
  if( statesave_table( SID, suspend_emu_work ) == STATE_OK ) return TRUE;
  else                                                       return FALSE;
}

int	stateload_emu( void )
{
  if( stateload_table( SID, suspend_emu_work ) == STATE_OK ) return TRUE;
  else                                                       return FALSE;
}
