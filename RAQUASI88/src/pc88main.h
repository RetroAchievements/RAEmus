#ifndef PC88MAIN_H_INCLUDED
#define PC88MAIN_H_INCLUDED

#include <stdio.h>
#include "file-op.h"


#if 0	/* → file-op.h */
extern char file_tape[2][QUASI88_MAX_FILENAME];	/* テープ入出力のファイル名 */
extern char file_prn[QUASI88_MAX_FILENAME];	/* パラレル出力のファイル名 */
extern char file_sin[QUASI88_MAX_FILENAME];	/* シリアル出力のファイル名 */
extern char file_sout[QUASI88_MAX_FILENAME];	/* シリアル入力のファイル名 */
#endif

	/**** ブート状態 (I/Oに反映) ****/

extern	int	boot_basic;			/* 起動時の BASICモード	*/
extern	int	boot_dipsw;			/* 起動時のディップ設定	*/
extern	int	boot_from_rom;			/* 起動デバイスの設定	*/
extern	int	boot_clock_4mhz;		/* 起動時の CPUクロック	*/

extern	int	monitor_15k;			/* 15k モニター 1:Yes 0:No  */

extern	int	high_mode;			/* 高速モード 1:Yes 0:No     */

	/**** ディップスイッチ ****/

#define SW_1_MASK	(0x3e)
#define	SW_2_MASK	(0x3f)
#define	SW_N88		(0x01)			/* 1: N88  / 0: N	*/
#define	SW_V1		(0x80)			/* 1: V1   / 0: V2	*/
#define	SW_H		(0x40)			/* 1: H    / 0: S	*/
#define	SW_ROMBOOT	(0x08)			/* 1: ROM  / 0: DISK	*/
#define	SW_4MHZ		(0x80)			/* 1: 4MHz / 0: 8MHz	*/

	/**** Ｉ／Ｏポート ****/

/*extern byte	dipsw_1;			 * IN[30] ディップスイッチ 1 */
/*extern byte	dipsw_2;			 * IN[31] ディップスイッチ 2 */
/*extern byte	ctrl_boot;			 * IN[40] ディスクブート情報 */
/*extern byte	cpu_clock;			 * IN[6E] CPU クロック       */
extern	int	memory_bank;			/* OUT[5C-5F] IN[5C] バンク  */

extern	byte	misc_ctrl;			/* I/O[32] 各種Ctrl       */
extern	byte	ALU1_ctrl;			/* OUT[34] ALU Ctrl 1     */
extern	byte	ALU2_ctrl;			/* OUT[35] ALU Ctrl 2     */
extern	byte	ctrl_signal;			/* OUT[40] コントロール信号*/
extern	byte	baudrate_sw;			/* I/O[6F] ボーレート     */
extern	word	window_offset;			/* I/O[70] WINDOW オフセット*/
extern	byte	ext_rom_bank;			/* I/O[71] 拡張ROM BANK   */
extern	byte	ext_ram_ctrl;			/* I/O[E2] 拡張RAM制御	  */
extern	byte	ext_ram_bank;			/* I/O[E3] 拡張RAMセレクト*/

extern	byte	jisho_rom_bank;			/* OUT[F0] 辞書ROMセレクト*/
extern	byte	jisho_rom_ctrl;			/* OUT[F1] 辞書ROMバンク  */


#define	MISC_CTRL_EBANK		(0x03)		/* EROM BANK 00..11        */
#define	MISC_CTRL_AVC		(0x0c)		/* AVC                     */
#define	MISC_CTRL_TEXT_MAIN	(0x10)		/* TEXT   MAIN-RAM/HIGH-RAM*/
#define	MISC_CTRL_ANALOG	(0x20)		/* PALETTE  ANALOG/DEGITAL */
#define	MISC_CTRL_EVRAM		(0x40)		/* VRAM     EXTEND/STANDARD*/
#define INTERRUPT_MASK_SOUND	(0x80)		/* SND INT  Disable/Enable */

#define	ALU1_CTRL_BLUE		(0x11)		/* ALU処理対象プレーン B   */
#define	ALU1_CTRL_RED		(0x22)		/*                     R   */
#define	ALU1_CTRL_GREEN		(0x44)		/*                     G   */

#define	ALU2_CTRL_DATA		(0x07)		/* ALU色比較データ    0..7 */
#define	ALU2_CTRL_MODE		(0x30)		/* ALU処理モード  00B..11B */
#define	ALU2_CTRL_VACCESS	(0x80)		/* メモリアクセス VRAM/MAIN*/

#define	MEMORY_BANK_MAIN	(3)		/* バンク指定        MAIN  */
#define	MEMORY_BANK_GRAM0	(0)		/*		       B   */
#define	MEMORY_BANK_GRAM1	(1)		/*		       R   */
#define	MEMORY_BANK_GRAM2	(2)		/*		       G   */

#define	CPU_CLOCK_4HMZ		(0x80)		/* CPU CLOCK 4MHz / 8MHz   */

#define	EXT_ROM_NOT		(0x01)		/* 拡張 ROM 非セレクト	   */

#define INTERRUPT_MASK_RTC	(0x01)		/* 1/600 割り込み 許可     */
#define INTERRUPT_MASK_VSYNC	(0x02)		/* VSYNC 割り込み 許可     */
#define INTERRUPT_MASK_SIO	(0x04)		/* COM   割り込み 許可     */

#define	JISHO_NOT_SELECT	(0x01)		/* 辞書ROMセレクト	   */
#define	JISHO_BANK		(0x1f)		/* 辞書ROMバンク	   */


	/**** カレンダー ****/

extern	int	calendar_stop;			/* 時計停止フラグ	*/


	/**** シリアル、パラレル ****/

extern	int	cmt_speed;	/* テープ速度 0で自動   */
extern	int	cmt_intr;	/* 割込でテープ処理する */
extern	int	cmt_wait;	/* 真で、テープ読込ウェイトあり(T88のみ) */


	/**** 高速 BASIC モード ****/

#define EndofBasicAddr 0xffff
#define HS_BASIC_COUNT 50000000	/* 割り込みなしで回すステート数 */

extern word highspeed_routine[];

extern int highspeed_flag;
extern int highspeed_mode;	/* 高速 BASIC 処理 するなら 真      */


	/**** シリアルマウス ****/

extern int use_siomouse;	/* 真で、シリアルマウスあり	*/





	/**** 関数 ****/

void	pc88main_init( int init );
void	pc88main_term( void );
void	pc88main_bus_setup( void );
void	power_on_ram_init( void );

byte	main_mem_read( word addr );
void	main_mem_write( word addr, byte data );
byte	main_io_in( byte port );
void	main_io_out( byte port, byte data );


int	sio_open_tapeload( const char *filename );
void	sio_close_tapeload( void );
int	sio_open_tapesave( const char *filename );
void	sio_close_tapesave( void );
int	sio_open_serialin( const char *filename );
void	sio_close_serialin( void );
int	sio_open_serialout( const char *filename );
void	sio_close_serialout( void );
int	printer_open( const char *filename );
void	printer_close( void );
void	sio_mouse_init(int initial);
int	sio_tape_rewind( void );

int	sio_tape_pos( long *cur, long *end );
int	sio_com_pos( long *cur, long *end );
int	sio_intr( void );
void	sio_data_clear(void);

int	tape_exist( void );
int	tape_readable(void);
int	tape_writable(void);
int	tape_reading( void );
int	tape_writing( void );

#endif	/* PC88MAIN_H_INCLUDED */
