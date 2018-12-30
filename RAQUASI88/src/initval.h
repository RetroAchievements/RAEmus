#ifndef INITVAL_H_INCLUDED
#define INITVAL_H_INCLUDED


/*----------------------------------------------------------------------*/
/* 定数									*/
/*----------------------------------------------------------------------*/
#define	CONST_4MHZ_CLOCK	(3.9936)
#define	CONST_8MHZ_CLOCK	(CONST_4MHZ_CLOCK*2.0)
#define	CONST_VSYNC_FREQ	(55.4)


/*----------------------------------------------------------------------*/
/* 起動時の初期設定値							*/
/*----------------------------------------------------------------------*/

enum BasicMode { BASIC_AUTO=-1, BASIC_N,    BASIC_V1S,  BASIC_V1H,  BASIC_V2 };
enum CpuClock  {                CLOCK_8MHZ, CLOCK_4MHZ  };
enum BootDevice{ BOOT_AUTO =-1, BOOT_DISK,  BOOT_ROM    };
enum SoundBoard{                SOUND_I,    SOUND_II    };
enum JishoRom  {                NOT_JISHO,  EXIST_JISHO };
enum MenuLang  {                MENU_ENGLISH, MENU_JAPAN };
enum BaudRate  { BAUDRATE_75,   BAUDRATE_150,   BAUDRATE_300,  BAUDRATE_600,
		 BAUDRATE_1200, BAUDRATE_2400,  BAUDRATE_4800, BAUDRATE_9600,
		 BAUDRATE_19200 };


/* DEFAULT_BASIC    BASIC モード       -1:自動 / 0:N / 1:V1S / 2:V1H / 3:V2  */
/* DEFAULT_CLOCK    CPUクロック        -1:自動 / 0:8MHz / 1:4MHz             */
/* DEFAULT_BOOT     起動デバイス       -1:自動 / 0:DISK / 1:ROM              */
/* DEFAULT_EXTRAM   拡張RAM             0:なし / 1〜4:あり(カード数)         */
/* DEFAULT_JISHO    辞書ROM             0:なし / 1:あり                      */
/* DEFAULT_SOUND    サウンドボード      0:搭載 / 1:サウンドボードII搭載      */
/* DEFAULT_DIPSW    ディップスイッチ   16bit値を指定                         */
/* DEFAULT_BAUDRATE ボーレート         BADU_RATE_75 〜 BADU_RATE_19200を指定 */

/* DEFAULT_VERBOSE         -verbose   の初期値 */
/* DEFAULT_FRAMESKIP       -frameskip の初期値 */
/* DEFAULT_CPU             -cpu       の初期値 */


#define	DEFAULT_BASIC		( BASIC_AUTO )
#define	DEFAULT_CLOCK		( CLOCK_4MHZ )
#define	DEFAULT_BOOT		( BOOT_AUTO  )
#define	DEFAULT_SOUND		( SOUND_I    )
#define	DEFAULT_JISHO		( NOT_JISHO  )
#define	DEFAULT_EXTRAM		( 0 )
#define	DEFAULT_DIPSW		( 0x391a )
#define	DEFAULT_BAUDRATE	( BAUDRATE_1200 )

#define	DEFAULT_VERBOSE		(0x00)
#define	DEFAULT_FRAMESKIP	(1)
#define	DEFAULT_CPU		(0)


/*----------------------------------------------------------------------*/
/* DEFAULT_CPU_CLOCK_MHZ	メイン CPUのクロック     double値 [MHz] */
/* DEFAULT_SOUND_CLOCK_MHZ	サウンドチップのクロック double値 [MHz] */
/* DEFAULT_VSYNC_FREQ_HZ	VSYNC 割り込みの周期     int値    [Hz]  */
/* DEFAULT_WAIT_FREQ		ウエイト調整用周波数     int値    [Hz]  */

#define	DEFAULT_CPU_CLOCK_MHZ		CONST_4MHZ_CLOCK
#define	DEFAULT_SOUND_CLOCK_MHZ		CONST_4MHZ_CLOCK
#define	DEFAULT_VSYNC_FREQ_HZ		CONST_VSYNC_FREQ
#define	DEFAULT_WAIT_FREQ_HZ		CONST_VSYNC_FREQ


/*----------------------------------------------------------------------*/
/* ドライブの数 および ファイル内のイメージの最大数			*/
/*	ドライブの数 は 2以外の値は未対応。設定を変えないように！！	*/
/*----------------------------------------------------------------------*/
enum {
  DRIVE_1,
  DRIVE_2,
  NR_DRIVE
};
#define	MAX_NR_IMAGE	(32)

enum {
  CLOAD,
  CSAVE,
  NR_TAPE
};

#endif		/* INITVAL_H_INCLUDED */
