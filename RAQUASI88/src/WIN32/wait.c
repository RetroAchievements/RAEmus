/***********************************************************************
 * ウエイト調整処理 (システム依存)
 *
 *	詳細は、 wait.h 参照
 ************************************************************************/
#include <windows.h>
#include <mmsystem.h>

#include "quasi88.h"
#include "device.h"
#include "initval.h"
#include "wait.h"



#if	1			/* 最小タイマ分解能は、設定しない */
#define	START_APP_PRECISION()
#define	STOP_APP_PRECISION()
#define	BEGIN_PRECISION()
#define	END_PRECISION()
#elif	0			/* 最小タイマ分解能は、アプリを通じて 1ms */
#define	START_APP_PRECISION()	timeBeginPeriod(1)
#define	STOP_APP_PRECISION()	timeEndPeriod(1)
#define	BEGIN_PRECISION()
#define	END_PRECISION()
#elif	0			/* 最小タイマ分解能は、時間計測の時のみ 1ms */
#define	START_APP_PRECISION()
#define	STOP_APP_PRECISION()
#define	BEGIN_PRECISION()	timeBeginPeriod(1)
#define	END_PRECISION()		timeEndPeriod(1)
#endif


/*---------------------------------------------------------------------------*/
static	int	wait_do_sleep;			/* idle時間 sleep する       */

static	int	wait_counter = 0;		/* 連続何回時間オーバーしたか*/
static	int	wait_count_max = 10;		/* これ以上連続オーバーしたら
						   一旦,時刻調整を初期化する */

/* ウェイトに使用する時間の内部表現は、 us単位とする。 (msだと精度が低いので) 

   WIN32 の時刻取得関数 timeGetTime() は ms 単位で、 DWORD型を返す。
   これを 1000倍して (usに変換して) 使用すると、71分で桁あふれしてしまうので、
   内部表現は __int64 型にしよう。

   なお、 timeGetTime() は 49日目に戻って(wrap)しまうので、内部表現もこの瞬間は
   おかしなものになる (ウェイト時間が変になる) が、気にしないことにする。 */

typedef	signed __int64	T_WAIT_TICK;

static	T_WAIT_TICK	next_time;		/* 次フレームの時刻 */
static	T_WAIT_TICK	delta_time;		/* 1 フレームの時間 */



/* ---- 現在時刻を取得する (usec単位) ---- */

#define	GET_TICK()	((T_WAIT_TICK)timeGetTime() * 1000)





/****************************************************************************
 * ウェイト調整処理の初期化／終了
 *****************************************************************************/
int	wait_vsync_init(void)
{
    START_APP_PRECISION();
    return TRUE;
}

void	wait_vsync_exit(void)
{
    STOP_APP_PRECISION();
}



/****************************************************************************
 * ウェイト調整処理の設定
 *****************************************************************************/
void	wait_vsync_setup(long vsync_cycle_us, int do_sleep)
{
    wait_counter = 0;


    delta_time = (T_WAIT_TICK) vsync_cycle_us;		/* 1フレーム時間 */

    BEGIN_PRECISION();		/* ▽精度を1msにする(timeGetTime, Sleepなど) */

    next_time  = GET_TICK() + delta_time;		/* 次フレーム時刻 */

    END_PRECISION();		/* △精度を戻す */

    wait_do_sleep = do_sleep;				/* Sleep 有無 */
}



/****************************************************************************
 * ウェイト調整処理の実行
 *****************************************************************************/
int	wait_vsync_update(void)
{
    int on_time = FALSE;
    T_WAIT_TICK diff_ms;


    BEGIN_PRECISION();		/* ▽精度を1msにする(timeGetTime, Sleepなど) */

    diff_ms = (next_time - GET_TICK()) / 1000;

    if (diff_ms > 0) {			/* 遅れてない(時間が余っている)なら */
					/* diff_ms ミリ秒、ウェイトする     */

	if (wait_do_sleep) {		/* 時間が来るまで sleep する場合 */

	    Sleep((DWORD)diff_ms);		/* diff_ms ミリ秒、ディレイ */

	} else {			/* 時間が来るまでTickを監視する場合 */

	    while (GET_TICK() <= next_time)
		;				/* ビジーウェイト */
	}

	on_time = TRUE;
    }

    END_PRECISION();		/* △精度を戻す */


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
	    fprintf(debugfp, "wait %d\n", y);
	}
    }
#endif

    if (on_time) return WAIT_JUST;
    else         return WAIT_OVER;
}
