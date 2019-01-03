/***********************************************************************
 * ウエイト調整処理 (システム依存)
 *
 *	詳細は、 wait.h 参照
 ************************************************************************/

#include <stdio.h>
#include <SDL.h>

#include "quasi88.h"
#include "initval.h"
#include "wait.h"



/*---------------------------------------------------------------------------*/
static	int	wait_do_sleep;			/* idle時間 sleep する       */

static	int	wait_counter = 0;		/* 連続何回時間オーバーしたか*/
static	int	wait_count_max = 10;		/* これ以上連続オーバーしたら
						   一旦,時刻調整を初期化する */

/* ウェイトに使用する時間の内部表現は、 us単位とする。 (msだと精度が低いので) 

   SDL の時刻取得関数 SDL_GetTicks() は ms 単位で、 unsigned long 型を返す。
   これを 1000倍して (usに変換して) 使用すると、71分で桁あふれしてしまうので、
   内部表現は long long 型にしよう。

   なお、 SDL_GetTicks は 49日目に戻って(wrap)しまうので、内部表現もこの瞬間は
   おかしなものになる (ウェイト時間が変になる) が、気にしないことにする。 */

#ifdef SDL_HAS_64BIT_TYPE
typedef	Sint64		T_WAIT_TICK;
#else
typedef	long		T_WAIT_TICK;
#endif

static	T_WAIT_TICK	next_time;		/* 次フレームの時刻 */
static	T_WAIT_TICK	delta_time;		/* 1 フレームの時間 */



/* ---- 現在時刻を取得する (usec単位) ---- */

#define	GET_TICK()	((T_WAIT_TICK)SDL_GetTicks() * 1000)





/****************************************************************************
 * ウェイト調整処理の初期化／終了
 *****************************************************************************/
int	wait_vsync_init(void)
{
    if (! SDL_WasInit(SDL_INIT_TIMER)) {
	if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0) {
	    if (verbose_wait) printf("Error Wait (SDL)\n");
	    return FALSE;
	}
    }

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


    delta_time = (T_WAIT_TICK) vsync_cycle_us;		/* 1フレーム時間 */
    next_time  = GET_TICK() + delta_time;		/* 次フレーム時刻 */

    wait_do_sleep = do_sleep;				/* Sleep 有無 */
}



/****************************************************************************
 * ウェイト処理
 *****************************************************************************/
int	wait_vsync_update(void)
{
    int slept   = FALSE;
    int on_time = FALSE;
    T_WAIT_TICK diff_ms;


    diff_ms = (next_time - GET_TICK()) / 1000;

    if (diff_ms > 0) {			/* 遅れてない(時間が余っている)なら */
					/* diff_ms ミリ秒、ウェイトする     */

	if (wait_do_sleep) {		/* 時間が来るまで sleep する場合 */

#if 1	    /* 方法 (1) */
	    SDL_Delay((Uint32) diff_ms);	/* diff_ms ミリ秒、ディレイ */
	    slept = TRUE;

#else	    /* 方法 (2) */
	    if (diff_ms < 10) {			/* 10ms未満ならビジーウェイト*/
		while (GET_TICK() <= next_time)
		    ;
	    } else {				/* 10ms以上ならディレイ      */
		SDL_Delay((Uint32) diff_ms);
		slept = TRUE;
	    }
#endif

	} else {			/* 時間が来るまでTickを監視する場合 */

	    while (GET_TICK() <= next_time)
		;				/* ビジーウェイト */
	}

	on_time = TRUE;
    }

    if (slept == FALSE) {	/* 一度も SDL_Delay しなかった場合 */
	SDL_Delay(1);			/* for AUDIO thread ?? */
    }


    /* 次フレーム時刻を算出 */
    next_time += delta_time;


    if (on_time) {			/* 時間内に処理できた */
	wait_counter = 0;
    } else {				/* 時間内に処理できていない */
	wait_counter ++;
	if (wait_counter >= wait_count_max) {	/* 遅れがひどい場合は */
	    wait_vsync_setup((long) delta_time,	/* ウェイトを初期化   */
			     wait_do_sleep);
	}
    }

#if 0
    {
	static int x = 0, y = 0;
	if (++x == 55) {
	    y++;
	    x = 0;
	    printf("wait %d\n", y);
	    fflush(stdout);
	}
    }
#endif

    if (on_time) return WAIT_JUST;
    else         return WAIT_OVER;
}
