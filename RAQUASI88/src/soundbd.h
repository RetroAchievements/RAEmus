#ifndef SOUND_H_INCLUDED
#define SOUND_H_INCLUDED


extern	int	sound_board;			/* サウンドボード	*/

#define	SD_PORT_44_45	(0x01)			/* ポート 44H〜45H 使用	*/
#define	SD_PORT_46_47	(0x02)			/* ポート 46H〜47H 使用	*/
#define	SD_PORT_A8_AD	(0x04)			/* ポート A8H〜ADH 使用	*/
extern	int	sound_port;			/* サウンドポートの種別	*/

extern	int	intr_sound_enable;		/*         割込マスク音源 */




extern	int	sound_ENABLE_A;			/* サウンドタイマー許可状況 */
extern	int	sound_ENABLE_B;
extern	int	sound_LOAD_A;			/* サウンドタイマー作動状況 */
extern	int	sound_LOAD_B;
extern	int	sound_FLAG_A;			/* FLAG の状態		*/
extern	int	sound_FLAG_B;

extern	int	sound_TIMER_A;			/* サウンドタイマー割込間隔 */
extern	int	sound_TIMER_B;

extern	int	sound_prescaler;		/* 1/プリスケーラー (2,3,6) */

extern	byte	sound_reg[0x100];
extern	int	sound_reg_select;


extern	int	sound2_MSK_TA;		/* TIMER A 割り込みマスク	*/
extern	int	sound2_MSK_TB;		/* TIMER B 割り込みマスク	*/
extern	int	sound2_MSK_EOS;		/* EOS     割り込みマスク	*/ 
extern	int	sound2_MSK_BRDY;	/* BRDY    割り込みマスク	*/ 
extern	int	sound2_MSK_ZERO;	/* ZERO    割り込みマスク	*/ 

extern	int	sound2_EN_TA;		/* TIMER A 割り込み許可		*/
extern	int	sound2_EN_TB;		/* TIMER B 割り込み許可		*/
extern	int	sound2_EN_EOS;		/* EOS     割り込み許可		*/
extern	int	sound2_EN_BRDY;		/* BDRY    割り込み許可		*/
extern	int	sound2_EN_ZERO;		/* ZERO    割り込み許可		*/

extern	int	sound2_FLAG_EOS;	/* FLAG EOS  の状態		*/
extern	int	sound2_FLAG_BRDY;	/* FLAG BRDY の状態		*/
extern	int	sound2_FLAG_ZERO;	/* FLAG ZERO の状態		*/
extern	int	sound2_FLAG_PCMBSY;	/* FLAG PCMBSY の状態		*/

extern	byte	sound2_reg[0x100];
extern	int	sound2_reg_select;
extern	byte	*sound2_adpcm;			/* ADPCM用 DRAM (256KB)	*/

extern	int	sound2_repeat;			/* ADPCM リピートプレイ	*/
extern	int	sound2_intr_base;		/* ADPCM 割り込みレート	*/
extern	int	sound2_notice_EOS;		/* EOSチェックの要不要	*/



extern	int	use_cmdsing;			/* 真で、CMD SING有効	*/



void	sound_board_init( void );

void	sound_out_reg( byte data );
void	sound_out_data( byte data );
byte	sound_in_status( void );
byte	sound_in_data( int always_sound_II );
void	sound2_out_reg( byte data );
void	sound2_out_data( byte data );
byte	sound2_in_status( void );
byte	sound2_in_data( void );


void	sound_output_after_stateload( void );

#endif	/* SOUND_H_INCLUDED */
