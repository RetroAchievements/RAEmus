/***********************************************************************
 * ウエイト調整処理 (システム依存)
 *
 *	詳細は、 wait.h 参照
 ************************************************************************/

/* select, usleep, nanosleep のいずれかのシステムコールを使用するので、
   以下のどれか一つを残して、他はコメントアウトする */

#define USE_SELECT
/* #define USE_USLEEP */
/* #define USE_NANOSLEEP */


#include <stdio.h>

#include <sys/types.h>		/* select                        */
#include <sys/time.h>		/* select           gettimeofday */
#include <unistd.h>		/* select usleep                 */
#include <time.h>		/*        nanosleep clock        */

#include "quasi88.h"
#include "initval.h"
#include "wait.h"
#include "suspend.h"
#include "event.h"		/* quasi88_is_exec		*/

#include "intr.h"		/* test */
#include "screen.h"		/* test */
#include "graph.h"		/* test */


/*---------------------------------------------------------------------------*/
static	int	wait_do_sleep;			/* idle時間 sleep する       */
	int	wait_sleep_min_us = 100;	/* 残り idle時間がこの us以下の
						   場合は、 sleep せずに待つ。
						   (MAX 1秒 = 1,000,000us) */

static	int	wait_counter = 0;		/* 連続何回時間オーバーしたか*/
static	int	wait_count_max = 10;		/* これ以上連続オーバーしたら
						   一旦,時刻調整を初期化する */

	int	show_fps;			/* test */
static	void	display_fps(void);		/* test */

/* ウェイトに使用する時間の内部表現は、 us単位とする。 (msだと精度が低いので) 

   時刻取得関数 (gettimeofday() など) で取得した時刻を us に変換して、
   long long 型で保持することにしよう。 */

#ifdef	HAVE_LONG_LONG
typedef	long long	T_WAIT_TICK;
#else
typedef	long		T_WAIT_TICK;
#endif

static	T_WAIT_TICK	next_time;		/* 次フレームの時刻 */
static	T_WAIT_TICK	delta_time;		/* 1 フレームの時間 */

static	T_WAIT_TICK	sleep_min_time = 100;	/* sleep 可能な最小時間 */



/* ---- 指定された時間 (usec単位) sleep する ---- */

INLINE	void	delay_usec(unsigned int usec)
{
#if	defined(USE_SELECT)		/* select を使う */

    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = usec;
    select(0, NULL, NULL, NULL, &tv);

#elif	defined(USE_USLEEP)		/* usleep を使う */

    usleep(usec);

#elif	defined(USE_NANOSLEEP)		/* nanosleep を使う */

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = usec * 1000;
    nanosleep(&ts, NULL);

#else					/* どれも使えない ! */
    wait_do_sleep = FALSE; /* X_X; */
#endif
}



/* ---- 現在時刻を取得する (usec単位) ---- */
static int tick_error = FALSE;


#ifdef  HAVE_GETTIMEOFDAY		/* gettimeofday() を使う */

static struct timeval start_tv;

INLINE	void		set_tick(void)
{
    if (gettimeofday(&start_tv, 0)) {
	if (verbose_wait) printf("Clock Error\n");
	tick_error = TRUE;
	start_tv.tv_sec  = 0;
	start_tv.tv_usec = 0;
    }
}

INLINE	T_WAIT_TICK	get_tick(void)
{
    struct timeval tv;

    if (gettimeofday(&tv, 0)) {
	if (verbose_wait) { if (tick_error == FALSE) printf("Clock Error\n"); }
	tick_error = TRUE;
	tv.tv_sec  = 1;
	tv.tv_usec = 1;
    }

 #if 1
    return ((T_WAIT_TICK) (tv.tv_sec  - start_tv.tv_sec) * 1000000 +
	    (T_WAIT_TICK) (tv.tv_usec - start_tv.tv_usec));
 #else
    tv.tv_sec  -= start_tv.tv_sec;
    tv.tv_usec -= start_tv.tv_usec;
    if (tv.tv_usec < 0) {
	--tv.tv_sec;
	tv.tv_usec += 1000000;
    }
    return ((T_WAIT_TICK) tv.tv_sec * 1000000 + (T_WAIT_TICK) tv.tv_usec);
 #endif
}

#else					/* clock() を使う */

/* #define CLOCK_SLICE	CLK_TCK */		/* これじゃ駄目？ */
#define	CLOCK_SLICE	CLOCKS_PER_SEC		/* こっちが正解？ */

INLINE	void		set_tick(void)
{
}

INLINE	T_WAIT_TICK	get_tick(void)
{
    clock_t t = clock();
    if (t == (clock_t)-1) {
	if (verbose_wait) { if (tick_error == FALSE) printf("Clock Error\n"); }
	tick_error = TRUE;
	t = CLOCK_SLICE;
    }

    return ((T_WAIT_TICK) (t / CLOCK_SLICE) * 1000000 +
	    (T_WAIT_TICK)((t % CLOCK_SLICE) * 1000000.0 / CLOCK_SLICE));
}

#endif





/****************************************************************************
 * ウェイト調整処理の初期化／終了
 *****************************************************************************/
int	wait_vsync_init(void)
{
    if (verbose_proc) {
#ifdef  HAVE_GETTIMEOFDAY
	printf("Timer start (gettimeofday(2) - based)\n");
#else
	printf("Timer start (clock(3) - based)\n");
#endif
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
    set_tick();

    sleep_min_time = wait_sleep_min_us;
    wait_counter = 0;


    delta_time = (T_WAIT_TICK) vsync_cycle_us;		/* 1フレーム時間 */
    next_time  = get_tick() + delta_time;		/* 次フレーム時刻 */

    wait_do_sleep = do_sleep;				/* Sleep 有無 */
}



/****************************************************************************
 * ウェイト調整処理の実行
 *****************************************************************************/
int	wait_vsync_update(void)
{
    int	on_time = FALSE;
    T_WAIT_TICK	diff_us;

    /* 実行中は、sleep するかどうかは、オプションによるけど、 */
    /* メニューでは必ず sleep させることにする。              */
    /* (旧いFreeBSD で、なぜかハングアップすることがある……) */
    /* (PAUSE中は問題ないのだが……なぜに?                  ) */
    int need_sleep = (quasi88_is_exec() ? wait_do_sleep : TRUE);

    diff_us = next_time - get_tick();

    if (tick_error == FALSE) {

	if (diff_us > 0) {	/* まだ時間が余っているなら */
			 	/* diff_us μミリ秒ウェイト */
	    if (need_sleep) {		/* 時間が来るまで sleep する場合 */

		/* FreeBSD ？の場合、以下のうち２番目の方法以外は、
		   10ms単位でスリープすることが判明。
		   つまり、55.4hzの場合、18msではなく、20msのスリープ
		   になる。*/

#if 1
		/* ver 0.6.3 までの方法
		   時間丁度までウェイトをしているため若干遅れが発生するはず。
		   サウンドノイズの要因か？ */
		if (diff_us < sleep_min_time) {	/* 残り僅かならビジーウェイト*/
		    while (tick_error == FALSE) {
			if (next_time <= get_tick())
			    break;
		    }
		} else {			/* 残り多ければディレイ      */
		    delay_usec(diff_us);
		}
#elif 0
		/* 1ms 以上時間がある場合は 100us の sleep を、
		   1ms 未満の場合はビジーウェイトをしてみた。XMAME風？
		   ビジーウェイトも時間丁度ではなく、100us早めに終えよう。
		   ところが、 100us の sleep は CPU負荷が 100% 近くになる
		   ことが判明。これはまずいか。*/
		while (tick_error == FALSE) {
		    diff_us = next_time - get_tick();
		    if (diff_us >= 1000) {
			delay_usec(100);
		    } else if (diff_us <= 100) {
			break;
		    }
		}
#elif 0
		/* SDL は 1ms 単位の sleep で、CPU負荷もなく、サウンドも
		   ノイズがでないので、そのまま真似してみよう。*/
		if (diff_us >= 1000) {
		    delay_usec((diff_us/1000) * 1000);
		}
#elif 0
		/* 上の折衷案 */
		while (tick_error == FALSE) {
		    diff_us = next_time - get_tick();
		    if (diff_us >= 1100) {
			delay_usec(diff_us - 100);
		    } else if (diff_us <= 100) {
			break;
		    }
		}
#endif
	    } else {			/* 時間が来るまでTick を監視する場合 */

		while (tick_error == FALSE) {
		    if (next_time <= get_tick())
			break;
		}
	    }

	    on_time = TRUE;
	}
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
    if (show_fps) {
	display_fps();
    }

    if (on_time) return WAIT_JUST;
    else         return WAIT_OVER;
}



/****************************************************************************
 *
 *****************************************************************************/
/* 適当に fps 計算 */
static	void	display_fps(void)
{
#ifdef  HAVE_GETTIMEOFDAY
    static struct timeval tv0;
    static int frame_count;
    static int prev_drawn_count;
    static int prev_vsync_count;

    /* 初回は tv0 は初期化未なので、現在時刻をセット */
    if (tv0.tv_sec == 0 &&
	tv0.tv_usec == 0) {
	gettimeofday(&tv0, 0);
    }

    if (quasi88_is_exec()) {

	/* この関数は、ウェイト毎 ＝ フレーム毎に呼び出される。
	   60フレーム回(約1秒)呼び出されたら、FPSを計算し、表示する */
	if (++ frame_count >= 60) {

	    char buf[32];
	    struct timeval tv1;
	    double dt, fps, hz;
	    int now_drawn_count = quasi88_info_draw_count();
	    int now_vsync_count = quasi88_info_vsync_count();

	    gettimeofday(&tv1, 0);
	    dt  = (double)(tv1.tv_sec  - tv0.tv_sec);
	    dt += (double)(tv1.tv_usec - tv0.tv_usec) / 1000000.0;

	    hz  = (double)(now_vsync_count - prev_vsync_count) / dt;
	    fps = (double)(now_drawn_count - prev_drawn_count) / dt;
	    sprintf(buf, "FPS: %4.1f (VSYNC %4.1f)", fps, hz);
	    graph_set_window_title(buf);


	    tv0 = tv1;
	    frame_count = 0;
	    prev_drawn_count = now_drawn_count;
	    prev_vsync_count = now_vsync_count;
	}

    } else {
	tv0.tv_sec  = 0;
	tv0.tv_usec = 0;
	frame_count = 0;
	prev_drawn_count = quasi88_info_draw_count();
	prev_vsync_count = quasi88_info_vsync_count();
    }
#endif
}
