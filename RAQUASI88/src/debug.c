/************************************************************************/
/*	デバッグ用							*/
/************************************************************************/


/*----------------------------------------------------------------------
 *	デバッグ用 printf
 *----------------------------------------------------------------------*/
#ifdef	DEBUGPRINTF
#include <stdarg.h>
void	debugprintf(const char *format, ...)
{
    va_list  list;

    va_start(list, format);
    vfprintf(stdout, format, list);
    va_end(list);

    fflush(stdout);
}
#endif

/*----------------------------------------------------------------------
 *	デバッグ用 ログ
 *----------------------------------------------------------------------*/
int	pio_debug = 0;			/* 真なら、ファイル出力		*/
int	fdc_debug = 0;			/* 真なら、ファイル出力		*/
int	main_debug = 0;			/* 真なら、ファイル出力		*/
int	sub_debug = 0;			/* 真なら、ファイル出力		*/

#ifdef	DEBUGLOG
#include <stdarg.h>
static	FILE	*LOG = NULL;

void	debuglog_init(void)
{
    LOG = fopen("quasi88.log","w");

    if (verbose_proc) {
	printf("+ Support debug logging.\n");
    }
}

void	debuglog_sync(void)
{
    fflush(LOG);
}

void	debuglog_exit(void)
{
    if (LOG) fclose(LOG);
}

void	logpio(const char *format, ...)
{
    va_list  list;
    if (pio_debug) {
	va_start(list, format);
	vfprintf(LOG, format, list);
	va_end(list);
    }
}

void	logfdc(const char *format, ...)
{
    va_list  list;
    if (fdc_debug) {
	va_start(list, format);
	vfprintf(LOG, format, list);
	va_end(list);
    }
}

static	int	z80_debug_wk;
void	logz80_target(int debug_flag)
{
    z80_debug_wk = debug_flag;
}
void	logz80( const char *format, ... )
{
    va_list  list;
    if (z80_debug_wk) {
	va_start(list, format);
	vfprintf(LOG, format, list);
	va_end(list);
    }
}
#endif

/*----------------------------------------------------------------------
 *	処理時間 区間計測
 *----------------------------------------------------------------------*/
int	debug_profiler;			/*
					  bit0: 区間ラップをファイル出力
					  bit1: 終了時に区間ラップ平均表示
					  bit2: 1秒毎に描画状況を表示
					*/

#ifdef	PROFILER
#ifdef  HAVE_GETTIMEOFDAY
#include <sys/time.h>		/* gettimeofday */

/* 使用方法
   ※ 起動時に、 -profiler オプションをつけると、このログが生成される

   profiler_init();			最初に1回だけ呼び出す
   profiler_exit();			最後に1回だけ呼び出す


   profiler_lapse(PROF_LAPSE_RESET);	最初のRESETは内部初期化のみ
	…
   profiler_lapse(PROF_LAPSE_CPU);	RESET直後はなにもしない
	…
   profiler_lapse(PROF_LAPSE_SND);	CPU〜ここまでの時間を計測
	…
   profiler_lapse(PROF_LAPSE_AUDIO);	SND〜ここまでの時間を計測
	…
   profiler_lapse(PROF_LAPSE_INPUT);	AUDIO〜ここまでの時間を計測
	…
   profiler_lapse(PROF_LAPSE_CPU2);	INPUT〜ここまでの時間を計測
	…
   profiler_lapse(PROF_LAPSE_BLIT);	CPU2〜ここまでの時間を計測
	…
   profiler_lapse(PROF_LAPSE_VIDEO);	BLIT〜ここまでの時間を計測
	…
   profiler_lapse(PROF_LAPSE_IDLE);	VIDEO〜ここまでの時間を計測
	…
   profiler_lapse(PROF_LAPSE_RESET);	IDLE〜ここまでの時間を計測
					前回のRESET〜ここまでの時間を計測
	…
   profiler_lapse(PROF_LAPSE_CPU);	RESET直後はなにもしない
	…
	…
   profiler_lapse(PROF_LAPSE_RESET);	
	…
   profiler_lapse(PROF_LAPSE_RESET);	RESETが連続した場合は内部初期化のみ
	…
*/

static const char *prof_label[PROF_LAPSE_END] = {
    "----total----",	/* PROF_LAPSE_RESET	*/
    "CPU",		/* PROF_LAPSE_CPU	*/
    "INPUT",		/* PROF_LAPSE_INPUT	*/
    "SND",		/* PROF_LAPSE_SND	*/
    "AUDIO",		/* PROF_LAPSE_AUDIO	*/
    "CPU2",		/* PROF_LAPSE_CPU2	*/
    "BLIT",		/* PROF_LAPSE_BLIT	*/
    "VIDEO",		/* PROF_LAPSE_VIDEO	*/
    "IDLE",		/* PROF_LAPSE_IDLE	*/
};
static FILE		*prof_lap_fp;
static struct timeval	prof_lap_reset_t0;	/* 前回 RESET 呼び出し時刻 */
static struct timeval	prof_lap_t0;		/* 前回呼び出し時刻 */
static int              prof_lap_type;		/* 前回呼び出し種類 */
static struct {					/* 累計情報 */
    struct timeval all;				/* 累計時間 */
    int            count;			/* 累計回数 */
}	prof_lap[PROF_LAPSE_END];

void	profiler_init(void)
{
    prof_lap_fp = fopen("quasi88.lap", "w");

    if (verbose_proc) {
	printf("+ Support profiler logging.\n");
    }

    if (prof_lap_fp) {
	fprintf(prof_lap_fp, "%-16s%8ld[us]\n",
		"(vsync)",
		(long)(1000000.0/(CONST_VSYNC_FREQ * wait_rate / 100)));
    }
}

void	profiler_lapse(int type)
{
    struct timeval t1, dt;

    if (debug_profiler & 3) {

	gettimeofday(&t1, 0);

	if (prof_lap_type == PROF_LAPSE_RESET) {
	    if (type      == PROF_LAPSE_RESET) {
		/* RESET が連続で呼び出された場合 (ないし初回) は、初期化 */
		prof_lap_reset_t0 = t1;

	    } else {   /* != PROF_LAPSE_RESET */
		/* RESET の次に RESET 以外が呼び出されたら、なにもしない */
		/* DO NOTHING */
	    }
	} else {
	    {
		/* 直前の profiler_lapse 呼び出しからの経過時間 */
		dt.tv_sec  = t1.tv_sec  - prof_lap_t0.tv_sec;
		dt.tv_usec = t1.tv_usec - prof_lap_t0.tv_usec;
		if (dt.tv_usec < 0) {
		    dt.tv_sec --;
		    dt.tv_usec += 1000000;
		}

		/* 経過時間累計に、加算していく */
		prof_lap[ prof_lap_type ].all.tv_sec  += dt.tv_sec;
		prof_lap[ prof_lap_type ].all.tv_usec += dt.tv_usec;
		if (prof_lap[ prof_lap_type ].all.tv_usec >= 1000000) {
		    prof_lap[ prof_lap_type ].all.tv_sec ++;
		    prof_lap[ prof_lap_type ].all.tv_usec -= 1000000;
		}
		prof_lap[ prof_lap_type ].count ++;

		/* dt 表示 */
		if ((debug_profiler & 1) && prof_lap_fp)
		    fprintf(prof_lap_fp, "%-13s%6ld\n",
			    prof_label[ prof_lap_type ], dt.tv_usec);
	    }

	    if (type == PROF_LAPSE_RESET) {
		/* 前回の PROF_LAPSE_RESET 呼び出しからの経過時間 */
		dt.tv_sec  = t1.tv_sec  - prof_lap_reset_t0.tv_sec;
		dt.tv_usec = t1.tv_usec - prof_lap_reset_t0.tv_usec;
		if (dt.tv_usec < 0) {
		    dt.tv_sec --;
		    dt.tv_usec += 1000000;
		}
		prof_lap_reset_t0 = t1;

		/* 経過時間累計に、加算していく */
		prof_lap[ type ].all.tv_sec  += dt.tv_sec;
		prof_lap[ type ].all.tv_usec += dt.tv_usec;
		if (prof_lap[ type ].all.tv_usec >= 1000000) {
		    prof_lap[ type ].all.tv_sec ++;
		    prof_lap[ type ].all.tv_usec -= 1000000;
		}
		prof_lap[ type ].count ++;

		/* dt 表示 */
		if ((debug_profiler & 1) && prof_lap_fp)
		    fprintf(prof_lap_fp, "%-13s%6ld\n",
			    prof_label[ type ], dt.tv_usec);
	    }
	}
	prof_lap_t0 = t1;
	prof_lap_type = type;
    }
}
void	profiler_exit(void)
{
    int i;
    double d;

    if (debug_profiler & 2) {

	printf("\n*** profiler ***\n");

	for (i = 0; i < PROF_LAPSE_END; i++) {

	    d = prof_lap[i].all.tv_sec + (prof_lap[i].all.tv_usec / 1000000.0);

	    printf("%-16s%5d[times], %4d.%06ld[sec] (ave = %f[sec])\n",
		   prof_label[i], prof_lap[i].count,
		   (int) prof_lap[i].all.tv_sec, prof_lap[i].all.tv_usec,
		   d / prof_lap[i].count);
	}
	printf("\n");
    }

    if (prof_lap_fp) {
	fclose(prof_lap_fp);
	prof_lap_fp = NULL;
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* その他の雑多な、時間計測デバッグ関数 */

/* 1970/01/01 からの秒数を usec で表示 */
void	profiler_current_time(void)
{
    struct timeval tv;
    if (gettimeofday(&tv, 0) == 0) {
	printf("%d.%06ld\n", (int) tv.tv_sec, tv.tv_usec);
    }
}


/* profiler_watch_start 〜 profiler_watch_stop までの時間を usec で表示  */
static struct timeval watch_t0;
void	profiler_watch_start(void)
{
    gettimeofday(&watch_t0, 0);
}

void	profiler_watch_stop(void)
{
    static struct timeval watch_t1, dt;

    gettimeofday(&watch_t1, 0);

    dt.tv_sec  = watch_t1.tv_sec  - watch_t0.tv_sec;
    dt.tv_usec = watch_t1.tv_usec - watch_t0.tv_usec;
    if (dt.tv_usec < 0) {
	dt.tv_sec --;
	dt.tv_usec += 1000000; 
    }

    printf("%d.%06ld\n", (int) dt.tv_sec, dt.tv_usec);
}


#else
void	profiler_init(void) {}
void	profiler_lapse(int type) {}
void	profiler_exit(void) {}
void	profiler_current_time(void) {}
void	profiler_watch_start(void) {}
void	profiler_watch_stop(void) {}
#endif

void	profiler_video_output(int timing, int skip, int drawn)
{
    static int n;

    if (debug_profiler & 4) {
	if (timing) {
	    if (skip == FALSE) {
		if (drawn) printf("@"); /* 画像処理の結果、更新が必要だった */
		else       printf("o"); /* 画像処理の結果、更新は不要だった */
	    } else         printf("-"); /* 時間がないので、スキップした     */
	} else             printf(" "); /* 今回は、画像処理しなかった       */

	if (++n > 56){
	    n=0;
	    printf("\n");
	    fflush(stdout);
	}
    }
}

#endif
