#ifndef Z80_H_INCLUDED
#define Z80_H_INCLUDED



/* --- Z80 CPU のエミュレート構造体 --- */

typedef struct{

  pair	AF, BC, DE, HL;			/* 汎用レジスタ */
  pair	IX, IY, PC, SP;			/* 専用レジスタ */
  pair	AF1,BC1,DE1,HL1;		/*  裏 レジスタ	*/
  byte	I, R;				/* 特殊レジスタ */
  byte	R_saved;			/* R reg - bit7 保存用 */
  Uchar	IFF,IFF2;			/* IFF1 、IFF2	*/
  Uchar	IM;				/* 割込モード	*/
  Uchar	HALT;				/* HALT フラグ	*/

  int	INT_active;
  int	icount;				/* 次の割込発生までのステート数	*/
  int	state0;				/* 処理した総ステート数	*/
					/* (z80->intr_update)()呼出時に初期化*/

  int	skip_intr_chk;

  Uchar	log;				/* 真ならデバッグ用のログを記録	*/
  Uchar	break_if_halt;			/* HALT時に処理ループから強制脱出*/

  byte	(*fetch)(word);
  byte	(*mem_read)(word);		/* メモリリード関数	*/
  void	(*mem_write)(word,byte);	/* メモリライト関数	*/
  byte	(*io_read)(byte);		/* I/O 入力関数		*/
  void	(*io_write)(byte,byte);		/* I/O 出力関数		*/

  void	(*intr_update)(void);		/* 割込情報更新関数	*/
  int	(*intr_ack)(void);		/* 割込応答関数		*/

  pair  PC_prev;			/* 直前の PC (モニタ用)	*/

} z80arch;


/* IFF の中身 */
#define INT_DISABLE	(0)
#define INT_ENABLE	(1)



/*------------------------------------------------------------------------
 * 現在処理中の CPU の動作を強制的に止めるためのマクロ
 *------------------------------------------------------------------------*/

extern	int z80_state_goal;	/* このstate数分、処理を繰り返す(0で無限) */
extern	int z80_state_intchk;	/* このstate数実行後、割込判定する	  */


/* PIO制御によるCPU切替時や、メニュー遷移時などに、CPU処理を強制的に停止 */
   
#define	CPU_BREAKOFF()	do{ z80_state_intchk = 0; z80_state_goal = 1; }while(0)

/* 割込発生条件変更時に、CPU処理を強制的に 割込更新処理に分岐 */

#define	CPU_REFRESH_INTERRUPT()		do{ z80_state_intchk = 0; }while(0)




extern	void	z80_reset( z80arch *z80 );
extern	int	z80_emu( z80arch *z80, int state_of_exec );
extern	void	z80_debug( z80arch *z80, char *mes );
extern	int	z80_line_disasm( z80arch *z80, word addr );

extern	void	z80_logging( z80arch *z80 );



#endif		/* Z80_H_INCLUDED */
