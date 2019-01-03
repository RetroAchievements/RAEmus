#ifndef EMU_H_INCLUDED
#define EMU_H_INCLUDED


extern	int	cpu_timing;			/* SUB-CPU 駆動方式	*/
extern	int	select_main_cpu;		/* -cpu 0 実行するCPU	*/
extern	int	dual_cpu_count;			/* -cpu 1 同時処理STEP数*/
extern	int	CPU_1_COUNT;			/* その、初期値		*/
extern	int	cpu_slice_us;			/* -cpu 2 処理時分割(us)*/

extern	int	trace_counter;			/* TRACE 時のカウンタ	*/


typedef struct{					/* ブレークポイント制御 */
  short	type;
  word	addr;
} break_t;

typedef struct{					/* FDC ブレークポイント制御 */
  short type;
  short drive;
  short track;
  short sector;
} break_drive_t;

enum BPcpu { BP_MAIN, BP_SUB,                                    EndofBPcpu  };
enum BPtype{ BP_NONE, BP_PC,  BP_READ, BP_WRITE, BP_IN, BP_OUT,  BP_DIAG, 
								 EndofBPtype };

#define	NR_BP			(10)		/* ブレークポイントの数   */
#define	BP_NUM_FOR_SYSTEM	(9)		/* システムが使うBPの番号 */
extern	break_t	break_point[2][NR_BP];
extern  break_drive_t break_point_fdc[NR_BP];









	/**** 関数 ****/

void	emu_breakpoint_init( void );
void	emu_reset( void );
void	set_emu_exec_mode( int mode );

void	emu_init(void);
void	emu_main(void);


#endif	/* EMU_H_INCLUDED */
