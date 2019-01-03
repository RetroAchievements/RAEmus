/***********************************************************************
 * ウエイト調整処理 (システム依存)
 *
 *      詳細は、 wait.h 参照
 ************************************************************************/

#include "quasi88.h"

#include "device.h"
#include <OSUtils.h>

#include "wait.h"



/*---------------------------------------------------------------------------*/
static	int	wait_counter = 0;		/* 連続何回時間オーバーしたか*/
static	int	wait_count_max = 10;		/* これ以上連続オーバーしたら
						   一旦,時刻調整を初期化する */

/* ウェイトに使用する時間の内部表現は、 us単位とする。 (msだと精度が低いので) 

   ToolBox の時刻取得関数 TickCount() は 1/60s 単位の値を返す。
   これを整数 us に変換して使用することにする。
   内部表現を long 型にすると、71分で桁あふれ(wrap)を起こしてしまい、この瞬間は
   おかしなものになる (ウェイト時間が変になる) 。
   できれば 64bit型(long long)にしたいけど、どこか定義されてない ? */

#if	0
typedef	long long	T_WAIT_TICK;
#else
typedef	long		T_WAIT_TICK;
#endif

static	T_WAIT_TICK	next_time;		/* 次フレームの時刻 */
static	T_WAIT_TICK	delta_time;		/* 1 フレームの時間 */



/* ---- 現在時刻を取得する (usec単位) ---- */

#define	GET_TICK()	( (T_WAIT_TICK)TickCount() * 1000000/60 )





/****************************************************************************
 * ウェイト調整処理の初期化／終了
 *****************************************************************************/
int	wait_vsync_init(void)
{
    return TRUE;
}

void	wait_vsync_exit(void)
{
}



/****************************************************************************
 * ウェイト調整処理の設定
 *****************************************************************************/
void	wait_vsync_setup(long vsync_cycle_us, int do_sleep)
{
    wait_counter = 0;


    delta_time = (T_WAIT_TICK)(1000000.0 / 60.0);	/* 1フレーム時間 */
    next_time  = GET_TICK() + delta_time;		/* 次フレーム時刻 */


    /* 設定 vsync_cycle_us, do_sleep は無視する */
}



/****************************************************************************
 * ウェイト調整処理の実行
 *****************************************************************************/
int	wait_vsync_update(void)
{
    int on_time = FALSE;
    T_WAIT_TICK diff_us;


    diff_us = next_time - GET_TICK();

    if (diff_us > 0) {			/* 遅れてない(時間が余っている)なら */

#if 0					/* ビジーウェイトするとこける？  */
	while (GET_TICK() <= next_time)
	    ;

#else					/* Delay してみる・・・ */
	UInt32 unused;
	diff_us = diff_us * 60 / 1000000;
	if (diff_us) {
	    Delay(diff_us,&unused);
	}
#endif

	on_time = TRUE;
    }


    /* 次フレーム時刻を算出 */
    next_time += delta_time;


    if (on_time) {			/* 時間内に処理できた */
	wait_counter = 0;
    } else {				/* 時間内に処理できていない */
	wait_counter ++;
	if (wait_counter >= wait_count_max) {	/* 遅れがひどい場合は */
	    wait_vsync_setup(0,0);		/* ウェイトを初期化   */
	}
    }

    if (on_time) return WAIT_JUST;
    else         return WAIT_OVER;
}
