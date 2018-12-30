#ifndef INTR_H_INCLUDED
#define INTR_H_INCLUDED


extern	int	intr_level;			/* OUT[E4] 割り込みレベル */
extern	int	intr_priority;			/* OUT[E4] 割り込み優先度 */
extern	int	intr_sio_enable;		/* OUT[E6] 割込マスク SIO */ 
extern	int	intr_vsync_enable;		/* OUT[E6] 割込マスクVSYNC*/ 
extern	int	intr_rtc_enable;		/* OUT[E6] 割込マスク RTC */ 



extern	double	cpu_clock_mhz;		/* メイン CPUのクロック     [MHz] */
extern	double	sound_clock_mhz;	/* サウンドチップのクロック [MHz] */
extern	double	vsync_freq_hz;		/* VSYNC 割り込みの周期	    [Hz]  */


extern	int	state_of_cpu;			/*メインCPUが処理した命令数 */
extern	int	state_of_vsync;			/* VSYNC周期のステート数   */

extern	int	wait_rate;			/* ウエイト調整 比率    [%]  */
extern	int	wait_by_sleep;			/* ウエイト調整時 sleep する */

extern	int	no_wait;			/* ウエイトなし		*/

extern	int	boost;				/* ブースト		*/
extern	int	boost_cnt;			/* 			*/




extern	int	ctrl_vrtc;			/* 1:垂直帰線中  0: 表示中 */

extern	int	VSYNC_flag;			/* 各種割り込み信号フラグ */
extern	int	RTC_flag;
extern	int	SOUND_flag;
extern	int	RS232C_flag;





void	interval_work_init_all( void );

void	interval_work_set_RS232C( int bps, int framesize );

void	interval_work_set_TIMER_A( void );
void	interval_work_set_TIMER_B( void );

void	interval_work_set_BDRY( void );
void	interval_work_set_EOS( int length );

void	change_sound_flags( int port );
void	change_sound_prescaler( int new_prescaler );

void	boost_change( int new_val );


void	main_INT_init( void );
void	main_INT_update( void );
int	main_INT_chk( void );

int	quasi88_info_vsync_count(void);

#endif	/* INTR_H_INCLUDED */
