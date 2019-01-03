/***********************************************************************
 * ウエイト調整処理 (システム依存)
 *
 *	詳細は、 wait.h 参照
 ************************************************************************/
#include <gtk/gtk.h>

#include "quasi88.h"
#include "initval.h"
#include "wait.h"



/*---------------------------------------------------------------------------*/
static	GTimer *timer_id = NULL;

static	int	wait_do_sleep;			/* idle時間 sleep する       */

static	int	wait_counter = 0;		/* 連続何回時間オーバーしたか*/
static	int	wait_count_max = 10;		/* これ以上連続オーバーしたら
						   一旦,時刻調整を初期化する */

/* ウェイトに使用する時間の内部表現は、 us単位とする。 (msだと精度が低いので) 

   GTK の時刻取得関数 g_timer_elapsed() は、浮動小数型を返す。
   これを整数 us に変換して使用することにする。
   long 型だと 71分で桁あふれしてしまうので、内部表現は long long 型にしよう */

#ifdef G_HAVE_GINT64
typedef	gint64		T_WAIT_TICK;
#else
typedef	gint32		T_WAIT_TICK;
#endif

static	T_WAIT_TICK	next_time;		/* 次フレームの時刻 */
static	T_WAIT_TICK	delta_time;		/* 1 フレームの時間 */



/* ---- 現在時刻を取得する (usec単位) ---- */

#define	GET_TICK() ((T_WAIT_TICK)(g_timer_elapsed(timer_id, NULL) * 1000000.0))





/****************************************************************************
 * ウェイト調整処理の初期化／終了
 *****************************************************************************/
int	wait_vsync_init(void)
{
    if (timer_id == NULL) {
	timer_id = g_timer_new();
    }

    return TRUE;
}

void	wait_vsync_exit(void)
{
    if (timer_id) {
	g_timer_destroy(timer_id);
	timer_id = NULL;
    }
}



/****************************************************************************
 * ウェイト調整処理の設定
 *****************************************************************************/
void	wait_vsync_setup(long vsync_cycle_us, int do_sleep)
{
    g_timer_start(timer_id);
    /* stopしなくても、startすると、タイマーはリセット & ゴーされるようだ */

    wait_counter = 0;


    delta_time = (T_WAIT_TICK) vsync_cycle_us;		/* 1フレーム時間 */
    next_time  = GET_TICK() + delta_time;		/* 次フレーム時刻 */

    wait_do_sleep = do_sleep;				/* Sleep 有無 */
}



/****************************************************************************
 * ウェイト調整処理の実行
 *****************************************************************************/
int	wait_vsync_update(void)
{
    int on_time = FALSE;
    T_WAIT_TICK diff_ms;


    diff_ms = (next_time - GET_TICK()) / 1000;

    if (diff_ms > 0) {			/* 遅れてない(時間が余っている)なら */
					/* diff_ms ミリ秒、ウェイトする     */

	if (wait_do_sleep) {		/* 時間が来るまで sleep する場合 */

#if 0	    /* 方法 (1) */
	    g_usleep(diff_ms * 1000);		/* スリープ */
#else	    /* 方法 (2) */
	    while (GET_TICK() <= next_time)	/* ビジーウェイト */
		;
#endif

	} else {			/* 時間が来るまでTickを監視する場合 */

	    while (GET_TICK() <= next_time)
		;				/* ビジーウェイト */
	}

	on_time = TRUE;
    }


    /* 次フレーム時刻を算出 */
    next_time += delta_time;


    if (on_time) {			/* 時間内に処理できた */
	wait_counter = 0;
    } else {				/* 時間内に処理できていない */
	wait_counter ++;
	if (wait_counter >= wait_count_max) {	/* 遅れがひどい場合は */
	    wait_vsync_setup(delta_time,	/* ウェイトを初期化   */
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
