/************************************************************************/
/*									*/
/* FDC の制御 と ディスクイメージの読み書き				*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "drive.h"
#include "image.h"
#include "fdc.h"

#include "emu.h"		/* emu_mode			*/
#include "pc88cpu.h"

#include "file-op.h"
#include "suspend.h"
#include "status.h"
#include "event.h"
#include "snddrv.h"



static int fdc_break_flag = FALSE;

int	fdc_debug_mode = FALSE;	/* FDC デバッグモードのフラグ		*/
int	disk_exchange = FALSE;	/* ディスク疑似入れ替えフラグ		*/
int	disk_ex_drv = 0;	/* ディスク疑似入れ替えドライブ		*/
				/*    drive 1 ... bit 0			*/
				/*    drive 2 ... bit 1			*/
/* 上記処理は、peach氏の提供による */

int	FDC_flag = 0;			/* FDC 割り込み信号		*/
int	fdc_wait = 0;			/* FDC の ウエイト 0無 1有	*/

int	fdc_ignore_readonly = FALSE;	/* 読込専用時、ライトを無視する	*/


/* FDCのシーク音処理のワーク
   連続でシークした場合、一定間隔で音を出すようにする。
   本来はドライブ毎のワークなのだが、かまわんだろう。
   このワークは、ステートセーブしない */
static	int	fdc_sound_counter;
static	int	fdc_sound_skipper = 1;	/* SEEK 4回毎に音を出す */

#define	MAX_DRIVE	(4)		/* FDCで扱えるドライブ数	*/
					/* 処理は NR_DRIVE(==2)だけ対応	*/



/*
 * ディスクイメージに関するワーク (ドライブ単位)
 */
PC88_DRIVE_T	drive[ NR_DRIVE ];


/*
 * ディスクイメージからセクタを読んだ時際の、情報を格納
 */
static	struct{
  Uchar   c;			/* セクタ ID の C	*/
  Uchar   h;			/* セクタ ID の H	*/
  Uchar   r;			/* セクタ ID の R	*/
  Uchar   n;			/* セクタ ID の N	*/
  Uchar   density;		/* セクタの記録密度	*/
  Uchar   deleted;		/* DELETED DATA フラグ	*/
  Uchar   status;		/* PC98 BIOSのステータス*/
  Uchar   padding;
  int     sec_nr;		/* トラック内のセクタ数	*/
  int	  size;			/* DATA サイズ		*/

  int	  drv;			/* この情報元のドライブ	*/
} sec_buf;


/*
 * READ / WRITE 時のデータはここにセット
 *	WRITE ID では、4バイトデータ×セクタ数をここにセットする。
 *	READ DIAGNOSTIC では、ベタデータのイメージがここに作成される。
 */

#define DATA_BUF_SIZE 0x4000	/* 最大 2D/2DD=6250byte、2HD=10416byte 位? */

static	Uchar	data_buf[ DATA_BUF_SIZE ];


/*
 * FDC の各種情報ワーク
 *	ホストから受け取ったコマンド、処理後のステータスは、ここに。
 */
static	struct{

  int	command;		/* コマンド (enum値)			*/
  int	phase;			/* PHASE (C/E/R)			*/
  int	step;			/* PAHSE内の処理手順			*/
  int	counter;		/* 各種カウンタ				*/
  int	data_ptr;		/* データ転送のポインタ(data_buf内)	*/

  int	limit;			/* データ転送タイムアウトダウンカウンタ */
  int	wait;			/* 次処理実行開始までのウエイト		*/
  int	carry;			/* 次処理までのウェイト繰り越し分	*/
  int	gap3;			/* 次処理までのGAP3のウェイト分		*/

  enum {			/* シーク状態				*/
    SEEK_STAT_STOP = 0,		/* 		シークなし 		*/
    SEEK_STAT_MOVE,		/* 		シーク中   		*/
    SEEK_STAT_END,		/* 		シーク完了 		*/
    SEEK_STAT_INTR		/* 		シーク完了割込		*/
  }	seek_stat[MAX_DRIVE];
  int	seek_wait[MAX_DRIVE];	/* シーク用ウェイト			*/

  int	srt_clk;		/* SRT (ウェイト換算)			*/
  int	hut_clk;		/* HUT (ウェイト換算)			*/
  int	hlt_clk;		/* HLT (ウェイト換算)			*/
  int	hl_stat[MAX_DRIVE];	/* ヘッドロード状態なら、真		*/
  int	hl_wait[MAX_DRIVE];	/* ヘッドアンロード用ウェイト		*/

  int	ddam_not_skipped;	/* DDAMなのにスキップしなかったら、真	*/

  byte	status;			/* STATUS		*/
  byte	read;			/* DATA  for  FDC->MAIN */
  byte	write;			/* DATA  for  FDC<-MAIN */
  byte	TC;			/* TC (1 or 0 )		*/

  Uchar	sk;			/* SK ビット		*/
  Uchar	mf;			/* MF ビット		*/
  Uchar	mt;			/* MT ビット		*/
  Uchar	us;			/* US 番号		*/
  Uchar	hd;			/* HD 側		*/
  Uchar	c;			/* ID - C		*/
  Uchar	h;			/* ID - H		*/
  Uchar	r;			/* ID - R		*/
  Uchar	n;			/* ID - N		*/
  Uchar	eot;			/* EOT 番号		*/
  Uchar	gpl;			/* GPL 長さ		*/
  Uchar	dtl;			/* DTL 長さ		*/
  Uchar	d;			/* D   データ		*/
  Uchar	sc;			/* SC  セクタ数		*/
  Uchar	stp;			/* STP 間隔		*/
  Uchar	ncn[MAX_DRIVE];		/* NCN 位置 (4台分)	*/
  Uchar	pcn[MAX_DRIVE];		/* PCN 位置 (4台分)	*/
  Uchar	st0;			/* ST0			*/
  Uchar	st1;			/* ST1			*/
  Uchar	st2;			/* ST2			*/
  Uchar	st3;			/* ST3			*/

  Uchar c0;			/* C-Phase 1バイト目	*/
  Uchar c1;			/* C-Phase 2バイト目	*/
  Uchar cn;			/* C-Phase NCN		*/
  Uchar s0;			/* C-Phase SPECIFY	*/
  Uchar s1;			/* C-Phase SPECIFY	*/
  Uchar r0;			/* R-Phase ST0		*/
  Uchar r1;			/* R-Phase PCN		*/
  Uchar	intr_unit;		/* 割込発生元ユニット	*/

} fdc;


/* 各種マクロ */


#define	disk_not_exist(drv)	(drive[drv].fp==NULL  || drive[drv].empty)
#define	disk_unformat(drv)	(drive[drv].sec_nr<=0 || drive[drv].empty)
#define disk_unformatable(drv)	(drive[drv].sec_nr<0  || drive[drv].empty)

#define	sector_density_mismatch()					\
		(((sec_buf.density==DISK_DENSITY_SINGLE)&&( fdc.mf))||	\
		 ((sec_buf.density==DISK_DENSITY_DOUBLE)&&(!fdc.mf)) )

#define	idr_match()							\
		( fdc.c==sec_buf.c && fdc.h==sec_buf.h &&		\
		  fdc.r==sec_buf.r && fdc.n==sec_buf.n )

#define	printf_system_error( code )					      \
    do{									      \
      if     ( code==1 ) printf("FDC Read/Write Error in DRIVE %d:\n", drv+1);\
      else if( code==2 ) printf("FDC Seek Error in DRIVE %d:\n", drv+1);      \
      else if( code==3 ) printf("FDC Over-size Error in DRIVE %d:\n", drv+1); \
      else               printf("Internal error !\n");			      \
    }while(0)


/*	------ ディスクイメージのステータス ------			*/
/*		実際に意味のあるステータスは以下のとおり		*/
/*			STATUS_MA	このセクタの ID は無効		*/
/*			STATUS_DE	ID CRC Error			*/
/*			STATUS_MA_MD	このセクタの DATA は無効	*/
/*			STATUS_DE_DD	DATA CRC Error			*/
/*			STATUS_CM	正常 (DELETED DATA 扱い)	*/
/*			その他		正常				*/

#define	STATUS_NORMAL	(0x00)		/* Normal End			*/
#define	STATUS_CM	(0x10)		/* Control Mark			*/
#define	STATUS_ALIGN	(0x20)		/* Alignment Error		*/
#define	STATUS_EN	(0x30)		/* End of Cylinder		*/
#define STATUS_EC	(0x40)		/* Equipment Check		*/
#define	STATUS_OR	(0x50)		/* Over Run			*/
#define	STATUS_NR	(0x60)		/* Not Ready			*/
#define	STATUS_NW	(0x70)		/* Not Writable			*/
#define	STATUS_UNDEF	(0x80)		/* Another Error		*/
#define	STATUS_TMOUT	(0x90)		/* Time out			*/
#define	STATUS_DE	(0xa0)		/* Data Error (ID)		*/
#define	STATUS_DE_DD	(0xb0)		/* Data Error (DATA)		*/
#define	STATUS_ND	(0xc0)		/* No Data			*/
#define	STATUS_BC	(0xd0)		/* Bad Cylinder			*/
#define	STATUS_MA	(0xe0)		/* Missing Address Mark (ID)	*/
#define	STATUS_MA_MD	(0xf0)		/* Missing Address Mark (DATA)	*/




/* FDC の ステータス */

#define FD0_BUSY	(0x01)
#define	FD1_BUSY	(0x02)
#define	FD2_BUSY	(0x04)
#define	FD3_BUSY	(0x08)
#define FDC_BUSY	(0x10)
#define NON_DMA		(0x20)
#define DATA_IO		(0x40)
#define	REQ_MASTER	(0x80)

/* FDC の リザルトステータス */

#define	ST0_US		(0x03)		/* Unit Select			*/
#define	ST0_HD		(0x04)		/* Head Address			*/
#define	ST0_NR		(0x08)		/* Not Ready			*/
#define	ST0_EC		(0x10)		/* Equipment Check		*/
#define	ST0_SE		(0x20)		/* Seek End			*/
#define	ST0_IC		(0xc0)		/* Interrupt Code		*/
#define	ST0_IC_NT	(0x00)		/* 	Normal Terminate	*/
#define	ST0_IC_AT	(0x40)		/* 	Abnormal Terminate	*/
#define	ST0_IC_IC	(0x80)		/* 	Invalid Command		*/
#define	ST0_IC_AI	(0xc0)		/* 	Attention Interrupt	*/

#define	ST1_MA		(0x01)		/* Missing Address Mark		*/
#define	ST1_NW		(0x02)		/* Not Writable			*/
#define	ST1_ND		(0x04)		/* No Data			*/
#define	ST1_OR		(0x10)		/* Over Run			*/
#define	ST1_DE		(0x20)		/* Data Error			*/
#define	ST1_EN		(0x80)		/* End of Cylinder		*/

#define	ST2_MD		(0x01)		/* Missing Address Mark in Data	*/
#define	ST2_BC		(0x02)		/* Bad Cylinder			*/
#define	ST2_SN		(0x04)		/* Scan Not Satisfied		*/
#define	ST2_SH		(0x08)		/* Scan Equal Hit		*/
#define	ST2_NC		(0x10)		/* No Cylinder			*/
#define	ST2_DD		(0x20)		/* Data Error in Data Files	*/
#define	ST2_CM		(0x40)		/* Control Mark			*/

#define	ST3_US		(0x03)		/* Unit Select			*/
#define	ST3_HD		(0x04)		/* Head Address			*/
#define	ST3_TS		(0x08)		/* Two Side			*/
#define	ST3_T0		(0x10)		/* Track 0			*/
#define	ST3_RY		(0x20)		/* Ready			*/
#define	ST3_WP		(0x40)		/* Write Protect		*/
#define	ST3_FT		(0x80)		/* Fault			*/



/* FDC の コマンド */

enum FdcCommand
{
  WAIT			= 0,		/* 処理待ちの状態 */
  READ_DATA,
  READ_DELETED_DATA,
  READ_DIAGNOSTIC,
  READ_ID,
  WRITE_DATA,		/* = 5 */
  WRITE_DELETED_DATA,
  WRITE_ID,
  SCAN_EQUAL,
  SCAN_LOW_OR_EQUAL,
  SCAN_HIGH_OR_EQUAL,	/* = 10 */
  SEEK,
  RECALIBRATE,
  SENSE_INT_STATUS,
  SENSE_DEVICE_STATUS,
  SPECIFY,		/* = 15 */
  INVALID,
  EndofFdcCmd
};
enum FdcPhase
{
  C_PHASE,
  E_PHASE,
  R_PHASE,
  EndofFdcPhase
};

static const char *cmd_name[] =	/* デバッグ用表示データ */
{
  "WAIT ------------------",
  "READ DATA -------------",
  "READ DELETED DATA -----",
  "READ DIAGNOSTIC -------",
  "READ ID ---------------",
  "WRITE DATA ------------",
  "WRITE DELETED DATA ----",
  "WRITE ID --------------",
  "SCAN EQUAL ------------",
  "SCAN LOW OR EQUAL -----",
  "SCAN HIGH OR EQUAL ----",
  "SEEK ------------------",
  "RECALIBRATE -----------",
  "SENSE INT STATUS ------",
  "SENSE DEVIC STATUS ----",
  "SPECIFY ---------------",
  "INVALID ---------------",
};







/************************************************************************/
/* fdc デバッグ								*/
/************************************************************************/
/*
 * FDC デバッグ処理は、peach氏により提供されました。
 */
void pc88fdc_break_point(void)
{
    int i;
    for(i = 0; i < NR_BP; i++){
	if (break_point_fdc[i].type != BP_NONE) {
	    fdc_break_flag = TRUE;
	    return;
	}
    }
    fdc_break_flag = FALSE;
}


#ifndef	USE_MONITOR

#define	print_fdc_status(nStatus,nDrive,nTrack,nSector)

#else	/* USE_MONITOR */

void print_fdc_status(int nStatus, int nDrive, int nTrack, int nSector)
{
    static int oDrive = -1;
    static int oTrack[2];
    static int oSector[2];
    static int oStatus;
    char c = ' ';
    int i;
    
    if (fdc_debug_mode == TRUE) {
	if (oDrive < 0 || nStatus != oStatus || nDrive != oDrive ||
	    nTrack != oTrack[nDrive])
	{
	    oStatus = nStatus;
	    oDrive = nDrive;
	    oTrack[oDrive] = nTrack;
	    oSector[oDrive] = nSector;
	    switch (nStatus) {
	    case BP_READ:  c = 'R'; break;
	    case BP_WRITE: c = 'W'; break;
	    case BP_DIAG:  c = 'D'; break;
	    }
	    printf("\n%c D:%d T:%d S:%d", c, nDrive+1, nTrack, nSector+1);
	    fflush(stdout);
	} else if (nSector != oSector[nDrive]){
	    oSector[nDrive] = nSector;
	    printf(",%d", nSector+1);
	    fflush(stdout);
	}
    }
    
    if (fdc_break_flag == TRUE) {
	for (i = 0; i < NR_BP; i++) {
	    if (break_point_fdc[i].type == nStatus &&
	        break_point_fdc[i].drive == nDrive + 1 &&
		break_point_fdc[i].track == nTrack) {
		if (break_point_fdc[i].sector == nSector + 1 ||
		    break_point_fdc[i].sector < 0) {
		    printf( "*** Break at D:%d T:%d S:%d *** ",
			    nDrive + 1, nTrack, nSector + 1);
		    switch (nStatus) {
		    case BP_READ:  printf("( Read )\n"); break;
		    case BP_WRITE: printf("( Write )\n"); break;
		    case BP_DIAG:  printf("( Diag )\n"); break;
	 	    }
		    quasi88_debug();
		    break;
		}
	    }
	}
    }
 
}
#endif	/* USE_MONITOR */

/************************************************************************/
/* セクタ間を埋める                                                     */
/************************************************************************/
/*
 * READ DIAG のセクタ間のデータを埋める処理は、peach氏により提供されました。
 */
static int fill_sector_gap(int ptr, int drv, Uchar fdc_mf);








/************************************************************************/
/* ドライブの初期化							*/
/************************************************************************/
static	void	fdc_init( void );
void	drive_init( void )
{
  int	i;

  fdc_init();

  for( i=0; i<NR_DRIVE; i++ ){
    drive[ i ].fp     = NULL;
    drive[ i ].sec_nr = -1;
    drive[ i ].empty  = TRUE;
    /* memset( drive[ i ].filename, 0, QUASI88_MAX_FILENAME ); */
  }
  disk_ex_drv = 0;

  sec_buf.drv = -1;
}


/************************************************************************/
/* ドライブのリセット。(現在設定されているイメージにワークを再設定)	*/
/************************************************************************/
void	drive_reset( void )
{
  int	i;

  fdc_init();

  for( i=0; i<NR_DRIVE; i++ ){
    if( drive[ i ].fp ){
      disk_change_image( i, drive[ i ].selected_image );
    }
  }
  disk_ex_drv = 0;
}



/************************************************************************/
/* ドライブを一時的に空にする／もとに戻す／切替える／どっちの状態か知る	*/
/*	drv…ドライブ(0/1)						*/
/************************************************************************/
void	drive_set_empty( int drv )
{
  drive[ drv ].empty  = TRUE;
}
void	drive_unset_empty( int drv )
{
  drive[ drv ].empty  = FALSE;
}
void	drive_change_empty( int drv )
{
  drive[ drv ].empty  ^= 1;
}
int	drive_check_empty( int drv )
{
  return drive[ drv ].empty;
}



/***********************************************************************
 * ドライブ にディスクを挿入する
 *	drv		ドライブ 0 / 1
 *	filename	ファイル名
 *	img		イメージ番号 0〜31
 *			範囲外ないし存在しないイメージ番号の場合は 0 とする
 *	readonly	真なら、リードオンリーでファイルを開く
 *			偽なら、リードライトでファイルを開く
 *
 *	エラー時は、ディスクをセットせずに 1 を返す。
 ************************************************************************/

/* エラー表示( s はメッセージ。\n はダメ)。 ディスクはイジェクトされる */

#define		DISK_ERROR( s, drv )					\
  do {									\
    if (quasi88_is_menu() == FALSE) {					\
      printf( "\n" );				 			\
      printf( "[[[ %-26s ]]]\n", s );				  	\
      printf( "[[[   Eject Disk from drive %d: ]]]\n" "\n", drv+1 ); 	\
    }									\
    disk_eject( drv );							\
  } while(0)

/* 警告表示( fmt は、"%s %d \n" を含む)。 ディスクはそのまま */

#define		DISK_WARNING( fmt, s, n )				\
  do {									\
    if( verbose_proc ){							\
      printf( fmt, s, n );						\
    }									\
  } while(0)




int	disk_insert( int drv, const char *filename, int img, int readonly )
{
  int	exit_flag;
  Uchar c[32];
  long	offset;
  int	num;

  int	open_as_readonly = readonly;

  		/* 現在のディスクはイジェクト */

  disk_eject( drv );


		/* ファイル名を覚えておく */
  /*
  if( strlen(filename) >= QUASI88_MAX_FILENAME ){
    DISK_ERROR( "filename too long", drv );
    return 1;
  }
  strcpy( drive[ drv ].filename, filename );
  */

		/* "r+b" でファイルを開く。だめなら "rb" でファイルを開く */

  if( open_as_readonly == FALSE ){
    drive[ drv ].fp = osd_fopen( FTYPE_DISK, filename, "r+b" );
  }
  if( drive[ drv ].fp == NULL ){
    drive[ drv ].fp = osd_fopen( FTYPE_DISK, filename, "rb" );
    open_as_readonly = TRUE;
  }

  if( drive[ drv ].fp == NULL ){
    DISK_ERROR( "Open failed", drv );
    return 1;
  }



	/* 反対側のドライブのファイルと同じファイルだった場合は	*/
	/*			反対側ドライブのワークをコピー	*/
	/* そうでない場合は	ドライブのワークを初期化	*/

  if( drive[ drv ].fp == drive[ drv^1 ].fp ){	/* 反対ドライブと同じ場合 */

/*  drive[ drv ].file_size           = drive[ drv^1 ].file_size;*/
    drive[ drv ].read_only           = drive[ drv^1 ].read_only;
    drive[ drv ].over_image          = drive[ drv^1 ].over_image;
    drive[ drv ].detect_broken_image = drive[ drv^1 ].detect_broken_image;
    drive[ drv ].image_nr            = drive[ drv^1 ].image_nr;
    memcpy( &drive[ drv   ].image,
	    &drive[ drv^1 ].image, sizeof(drive[ drv ].image) );

    if( drv==0 ){
      DISK_WARNING( " (( %s : Set in drive %d: <- 2: ))\n", filename, drv+1 );
    }else{
      DISK_WARNING( " (( %s : Set in drive 1: -> %d: ))\n", filename, drv+1 );
    }


  }else{					/* 反対ドライブと違う場合 */


    if( open_as_readonly ){
      drive[ drv ].read_only = TRUE;

      DISK_WARNING( " (( %s : Set in drive %d: as read only ))\n", 
		    filename, drv+1 );
    }else{
      drive[ drv ].read_only = FALSE;

      DISK_WARNING( " (( %s : Set in drive %d: ))\n", filename, drv+1 );
    }

    /*
    drive[ drv ].file_size = osd_file_size( drive[ drv ].filename );
    if( drive[ drv ].file_size == -1 ){
      DISK_ERROR( "Access Error(size)", drv );
      return 1;
    }
    */

    drive[ drv ].over_image          = FALSE;
    drive[ drv ].detect_broken_image = FALSE;

    num = 0;	offset = 0;
    exit_flag = FALSE;


			/* 各イメージのヘッダ情報を全て取得 */
    while( !exit_flag ){

      switch( d88_read_header( drive[ drv ].fp, offset, c ) ){

      case D88_SUCCESS:			/* イメージ情報取得成功 */

	memcpy( &drive[ drv ].image[ num ].name[0], &c[0], 16 );
	drive[ drv ].image[ num ].name[16] = '\0';
	drive[ drv ].image[ num ].protect  = c[DISK_PROTECT];
	drive[ drv ].image[ num ].type     = c[DISK_TYPE];
	drive[ drv ].image[ num ].size     = READ_SIZE_IN_HEADER( c );

						/* 次のイメージの情報取得へ */
	offset += drive[ drv ].image[ num ].size;
	num ++;

	if( num >= MAX_NR_IMAGE ){		/* イメージ数が多い時は中断 */
	  DISK_WARNING( " (( %s : Too many images [>=%d] ))\n", 
			filename, MAX_NR_IMAGE );
	  drive[ drv ].over_image = TRUE;
	  exit_flag = TRUE;
	}
	else if( offset < 0 ){			/* イメージサイズが大きすぎ? */
	  DISK_WARNING( " (( %s : Too big image? [%d] ))\n", filename, num+1 );
	  drive[ drv ].detect_broken_image = TRUE;
	  exit_flag = TRUE;
	}
	break;

      case D88_NO_IMAGE:		/* これ以上イメージがない */
	exit_flag = TRUE;
	break;

      case D88_BAD_IMAGE:		/* このイメージは壊れている */
	DISK_WARNING( " (( %s : Image No. %d Broken? ))\n", filename, num+1 );
	drive[ drv ].detect_broken_image = TRUE;
	exit_flag = TRUE;
	break;

      default:				/* ??? */
	DISK_WARNING( " (( %s : Image No. %d Error? ))\n", filename, num+1 );
	drive[ drv ].detect_broken_image = TRUE;
	exit_flag = TRUE;
	break;

      }
    }

    if( num==0 ){
      DISK_ERROR( "Image not found", drv );
      return 1;
    }

    drive[ drv ].image_nr = num;

  }



	/* disk_top をimg 枚目のディスクイメージの先頭に設定	*/

  if( img < 0 || img >= drive[ drv ].image_nr ){
    DISK_WARNING( " (( %s : Image No. %d Not exist ))\n", filename, img+1 );
    drive_set_empty( drv );
  }else{
    disk_change_image( drv, img );
  }



  return 0;
}



/***********************************************************************
 * 反対側のドライブのイメージを、こっちのドライブにもセットする
 *	src	反対側のドライブ 0 or 1
 *	dst	こっちのドライブ 1 or 0
 *	img	イメージ番号 -1, 0〜31
 *		範囲外ないし存在しないイメージ番号の場合は、空とする
 *
 *	エラー時は、ディスクをセットせずに 1 を返す。
 ************************************************************************/
int	disk_insert_A_to_B( int src, int dst, int img )
{

  disk_eject( dst );


  if( drive[ src ].fp == NULL ){
    return 0;
  }


  /* strcpy( drive[ dst ].filename, drive[ src ].filename ); */


  drive[ dst ].fp = drive[ src ].fp;

/*drive[ dst ].file_size           = drive[ src ].file_size;*/
  drive[ dst ].read_only           = drive[ src ].read_only;
  drive[ dst ].over_image          = drive[ src ].over_image;
  drive[ dst ].detect_broken_image = drive[ src ].detect_broken_image;
  drive[ dst ].image_nr            = drive[ src ].image_nr;

  memcpy( &drive[ dst ].image, &drive[ src ].image, sizeof(drive[0].image) );


  if( img < 0 || img >= drive[ dst ].image_nr ){
    drive_set_empty( dst );
  }else{
    disk_change_image( dst, img );
  }

  return 0;
}



/************************************************************************/
/* イメージを変更する。							*/
/*	disk_top をimg 枚目のディスクイメージの先頭に設定し、		*/
/*	pcn*2 トラックの先頭に移動する。				*/
/*	drv…ドライブ(0/1)						*/
/************************************************************************/
static void disk_now_track( int drv, int trk );

int	disk_change_image( int drv, int img )
{
  int	i;

  if( drive[ drv ].fp==NULL ){				/* ドライブ未セット */
    return 1;
  }
  if( img < 0 || img >= drive[ drv ].image_nr ){	/* 指定イメージ無し */
    return -1;
  }

		/* disk_top を計算 */

  drive[ drv ].selected_image = img;
  drive[ drv ].empty          = FALSE;

  drive[ drv ].disk_top = 0;
  for( i=0; i<img; i++ ){
    drive[ drv ].disk_top += drive[ drv ].image[ i ].size;
  }
  drive[ drv ].disk_end = drive[ drv ].disk_top + drive[ drv ].image[img].size;
  drive[ drv ].protect  = drive[ drv ].image[ img ].protect;
  drive[ drv ].type     = drive[ drv ].image[ img ].type;


  if( fdc_ignore_readonly == FALSE ){

    /* ReadOnly でファイルを開いた場合、無条件でライトプロテクト状態とする */

    if( drive[ drv ].read_only ) drive[ drv ].protect = DISK_PROTECT_TRUE;

  }else{

    /* ReadOnly でファイルを開いた場合も、ライトプロテクト状態はイメージの
       属性にしたがう。このイメージに対して書き込みを行なった場合、
       実際には書き込みは行なわれないが、常に正常終了を返す。
       ( エメドラ、ミスティブルー、天使たちの午後2 など、 ReadOnly だと
         起動すらしてくれないゲームを強引に起動させる。いいのか？ ) */
    ;
  }

		/* pcn*2 トラックの先頭に移動する */

  disk_now_track( drv, fdc.pcn[drv]*2 );

  if (disk_exchange) disk_ex_drv |= 1 << drv;	/* ディスク入れ替えたよん */

  return 0;
}



/************************************************************************/
/* ディスクをイジェクトする						*/
/*	2 ドライブであると限定し、反対のドライブと比較する。		*/
/*	同じであれば、ワークを初期化。違えば、ファイルを閉じる		*/
/*	drv…ドライブ(0/1)						*/
/************************************************************************/
void	disk_eject( int drv )
{
  if( drive[ drv ].fp ){
    if( drive[ drv ].fp != drive[ drv^1 ].fp ){
      osd_fclose( drive[ drv ].fp );
    }
  }
  drive[ drv ].fp = NULL;
  drive[ drv ].sec_nr = -1;
  drive[ drv ].empty  = TRUE;
  /* memset( drive[ drv ].filename, 0, QUASI88_MAX_FILENAME ); */

  sec_buf.drv = -1;
}




/*======================================================================*/
/* ヘッドをトラックの先頭に移動する					*/
/*	drv…ドライブ(0/1)  trk…トラック番号(0〜)			*/
/*									*/
/*	エラーが出た場合は、そのトラックはアンフォーマットになる。	*/
/*======================================================================*/
static int disk_now_sec( int drv );

static	void	disk_now_track( int drv, int trk )
{
  int	error = 0;
  Uchar c[4];
  long	track_top;



	/* シーク可能シリンダのチェック */

  if     ( drive[ drv ].type==DISK_TYPE_2D  && trk>=84  ) trk =  83;
  else if( drive[ drv ].type==DISK_TYPE_2DD && trk>=164 ) trk = 163;
  else if( drive[ drv ].type==DISK_TYPE_2HD && trk>=158 ) trk = 157;
  else if( trk>=164 ) trk = 163; /* ここまでのばすと 2DD/2HD に対応できる */
						        /* thanks peach ! */

	/* ワーク設定 & 初期化 */

  drive[ drv ].track     = trk;
  drive[ drv ].sec       = 0;


	/* トラックのインデックスで指定されたファイル位置を取得 */

  if( osd_fseek( drive[ drv ].fp,
		 drive[ drv ].disk_top + DISK_TRACK + trk*4,  SEEK_SET )==0 ){
    if( osd_fread( c, sizeof(Uchar), 4, drive[ drv ].fp )==4 ){

	/* トラックおよび、先頭セクタの位置を設定   */
	/* そのセクタのセクタ情報および、セクタ数を得る */

      track_top = (long)c[0]+((long)c[1]<<8)+((long)c[2]<<16)+((long)c[3]<<24);
      if( track_top!=0 ){
	drive[ drv ].track_top =
	drive[ drv ].sec_pos   = drive[ drv ].disk_top + track_top;
	drive[ drv ].sec_nr    = disk_now_sec( drv );
      }else{
	drive[ drv ].track_top =
	drive[ drv ].sec_pos   = drive[ drv ].disk_top;
	drive[ drv ].sec_nr    = -1;
      }
    }
    else error = 1;
  } else error = 2;

  if( error ){					/* SEEK / READ Error */
    printf_system_error( error );
  }

	/* エラー時は、アンフォーマット(再フォーマット不能)にして、戻る */

  if( error ){
    drive[ drv ].track_top =
    drive[ drv ].sec_pos   = drive[ drv ].disk_top;
    drive[ drv ].sec_nr    = -1;
  }

  sec_buf.drv = drv;			/* 処理対象のドライブを覚えておく */
  return;
}



/*======================================================================*/
/* 指定されたディスクの現在のセクタの情報を読みとる			*/
/*	drv…ドライブ(0/1)						*/
/*									*/
/*	エラー時は、そのセクタのは ID CRC Error エラーに設定する。	*/
/*	返り値は、そのセクタの、「セクタ数(DISK_SEC_NR)」の値		*/
/*======================================================================*/
static	int	disk_now_sec( int drv )
{
  int	error = 0;
  Uchar	c[16];

	/* ファイル位置 sec_pos の ID情報 を読み、セクタ数を返す */

  if( osd_fseek( drive[ drv ].fp,  drive[ drv ].sec_pos,  SEEK_SET )==0 ){
    if( osd_fread( c, sizeof(Uchar), 16, drive[ drv ].fp )==16 ){
      sec_buf.c       = c[DISK_C];
      sec_buf.h       = c[DISK_H];
      sec_buf.r       = c[DISK_R];
      sec_buf.n       = c[DISK_N];
      sec_buf.density = c[DISK_DENSITY];
      sec_buf.deleted = c[DISK_DELETED];
      sec_buf.status  = c[DISK_STATUS];
      sec_buf.sec_nr  = c[DISK_SEC_NR] + (int)c[DISK_SEC_NR+1]*256;
      sec_buf.size    = c[DISK_SEC_SZ] + (int)c[DISK_SEC_SZ+1]*256;
      if( sec_buf.status==STATUS_CM ){
	sec_buf.deleted = DISK_DELETED_TRUE;
	sec_buf.status  = STATUS_NORMAL;
      }
    }
    else error = 1;
  } else error = 2;

  if( error ){					/* SEEK / READ Error */
    printf_system_error( error );
    status_message( 1, STATUS_WARN_TIME, "DiskI/O Read Error" );
  }



	/* 失敗したら、ID CRC Error にし、0 (==unformat) を返す */

  if( error ){
    sec_buf.sec_nr  = 0;
    sec_buf.status  = STATUS_DE;
  }

  return ( sec_buf.sec_nr );
}



/*======================================================================*/
/* 指定されたディスクの次のセクタの情報を読みとる			*/
/*	drv…ドライブ(0/1)						*/
/*======================================================================*/
static	void	disk_next_sec( int drv )
{
  int	overwrite_id;

	/* アンフォーマット時は、なにもしない */

  if( disk_unformat( drv ) ) return;


	/* sec_top を次のセクタに。最終セクタの時はトラック先頭に */

			/* ミックスセクタ作成時に上書きされた ID の数 */
			/* この辺が正確にチェックできない。どうしよう */

  if( sec_buf.size == 0x80 ||		/* sec_buf.sizeが 0x80,0x100,0x200 */
     (sec_buf.size & 0xff) == 0 ){	/* 0x400,0x800,0x1000 の場合(正常) */
    overwrite_id = 0;
  }else{				/* それ以外は、ミックスセクタかも  */
    overwrite_id = ( sec_buf.size - ( 128 << (sec_buf.n & 7)) ) / SZ_DISK_ID;
    if( overwrite_id < 0 ) overwrite_id = 0;
  }


  drive[ drv ].sec += ( 1 + overwrite_id );
  if( drive[ drv ].sec < drive[ drv ].sec_nr ){

    drive[ drv ].sec_pos += (sec_buf.size + SZ_DISK_ID);

  }else{

    drive[ drv ].sec = 0;
    drive[ drv ].sec_pos = drive[ drv ].track_top;

  }

	/* sec_pos の セクタID情報 を読む */

  disk_now_sec( drv );
}



/************************************************************************/
/* FDC の初期化								*/
/************************************************************************/
static	void	fdc_init( void )
{
  int	i;

  fdc.status  = 0 | REQ_MASTER;
  fdc.read    = 0xff;
  fdc.write   = 0xff;
  fdc.TC      = FALSE;

  fdc.command = WAIT;

  for( i=0; i<MAX_DRIVE; i++ ){
    fdc.seek_stat[ i ] = SEEK_STAT_STOP;
    fdc.seek_wait[ i ] = 0;
    fdc.ncn[ i ]  = 0;
    fdc.pcn[ i ]  = 0;
  }
  fdc.intr_unit = 4;
}



/************************************************************************/
/* CPUが FDC にアクセスした時に呼ぶ関数郡				*/
/*	void	fdc_write( byte data )	……… OUT A,(0FBH)		*/
/*	byte	fdc_read( void )	……… IN  A,(0FBH)		*/
/*	byte	fdc_status( void )	……… IN  A,(0FAH)		*/
/*	void	fdc_TC( void )		……… IN  A,(0F8H)		*/
/************************************************************************/
void	fdc_write( byte data )
{
  if( (fdc.status & DATA_IO)==0 ){
    fdc.status &= ~REQ_MASTER;
    fdc.write   = data;
  }
}

byte	fdc_read( void )
{
  if( (fdc.status & DATA_IO) ){
    fdc.status &= ~REQ_MASTER;
    return fdc.read;
  }else{
    return 0xff;
  }
}

byte	fdc_status( void )
{
  return fdc.status;
}

void	fdc_TC( void )
{
  fdc.TC = TRUE;
}



/* FDC からCPUへの割り込み通知  */

#define	fdc_occur_interrupt()	FDC_flag = TRUE
#define	fdc_cancel_interrupt()	FDC_flag = FALSE


/* FDC処理ウェイトの制御 */

#define	ICOUNT(x)	do{  fdc.wait = (x);			      }while(0)
#define	REPEAT()	do{  if( fdc_wait==FALSE ){ fdc.wait = 0; }   }while(0)


/*#define	logfdc	printf*/
/*======================================================================
 * E-PHASE にて、各種処理に要するクロック数を算出する関数・マクロ
 *======================================================================*/

#if 0	/* ウェイトの精度をあげようとしたけど、なんかおかしい。遅過ぎる…… */

/* GAP3/GAP4のバイト数。かなり適当。セクタ数もかなり適当に、		*/
/* N = 0/1/2/3/4/5/6/7 の時、 SC = 26/16/ 9/ 5/ 2/ 1/ 1/ 1 とした。	*/
static const int gap3_tbl[] = { 26, 54, 84, 116, 150, 186, 224, 264 };
static const int gap4_tbl[] = { 488, 152, 182, 94, 1584, 1760, 2242, 4144 };

#define	CLOCK_GAP0()      (128*( 80+12+(3+1)+50 ))

#define	CLOCK_ID()        (128*( 12+(3+1)+4+2+22 ))
#define	CLOCK_DATA(n)     (128*( 12+(3+1)+(128<<(n))+2+gap3_tbl[(n)&7] ))
#define	CLOCK_SECTOR(n)   ( CLOCK_ID() + CLOCK_DATA(n) )

#define	CLOCK_RID_ID()    (128*( 12+(3+1)+4+2 ))
#define	CLOCK_RID_DATA(n) (128*( 22 + 12+(3+1)+(128<<(n))+2+gap3_tbl[(n)&7] ))

#define	CLOCK_RDT_ID()    (128*( 12+(3+1)+4+2+22 + 12+(3+1) ))
#define	CLOCK_RDT_DATA(n) (128*( (128<<(n))+2+gap3_tbl[(n)&7] ))

#define	CLOCK_GAP3(n)     (128*( gap3_tbl[(n)&7] ))
#define	CLOCK_GAP4(n)     (128*( gap4_tbl[(n)&7] ))
#define	CLOCK_TRACK()     (128*( 6250 ))

/* 1バイトの転送時間 32us */
#define	CLOCK_BYTE()      (128)
#define	CLOCK_2MS()       (8000)

#else	/* とりあえず、この程度でいいか */

#define	CLOCK_GAP0()      (0)
#define	CLOCK_ID()        (0)
#define	CLOCK_DATA(n)     (0)
#define	CLOCK_SECTOR(n)   ( CLOCK_ID() + CLOCK_DATA(n) )
#define	CLOCK_RID_ID()    (0)
#define	CLOCK_RID_DATA(n) (0)
#define	CLOCK_RDT_ID()    (128*( 12+(3+1)+4+2+22 + 12+(3+1) ))
#define	CLOCK_RDT_DATA(n) (0)
#define	CLOCK_GAP3(n)     (0)
#define	CLOCK_GAP4(n)     (0)
#define	CLOCK_TRACK()     (128*( 6250 ))
#define	CLOCK_BYTE()      (128)
#define	CLOCK_2MS()       (8000)
#endif


/* ヘッドロード・シークのウェイトを計測するかどうか */
#define WAIT_FOR_HEADLOAD
#define WAIT_FOR_SEEK




/*===========================================================================
 * READ/WRITE系コマンドで、ID検索前にユニットなどのチェックを行う
 *	戻り値が 0 の場合、ディスクがないので、ディスクセットを無限に待つ
 *	戻り値が 1 の場合、結果が ST0〜ST2 にセットされる
 *			   (一部のエラーはここではセットしない)
 *===========================================================================*/
static	int	fdc_check_unit( void )
{
  int	drv = (fdc.us);

	/* 未接続ドライブや、シーク中ドライブありなら、異常終了 */

  if( fdc.us >= NR_DRIVE || fdc.status & (0x0f) ){
    fdc.st0 = ST0_IC_AT | ST0_NR | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    if( fdc.command == READ_ID ){ fdc.c = fdc.h = fdc.r = fdc.n = 0xff; }
    fdc.carry = 0;
    return 1;
  }

	/* ディスクが無い時は、いつまでたっても終らない */

  if( disk_not_exist( drv ) ){
    fdc.wait  = CLOCK_2MS();			   /* 2ms後にやりなおし	*/
    fdc.carry = 0;
    return 0;
  }

	/* ライト系コマンドで、ライトプロテクト時は、異常終了 */

  if( fdc.command == WRITE_ID   ||
      fdc.command == WRITE_DATA ||
      fdc.command == WRITE_DELETED_DATA ){
    if( drive[ drv ].protect == DISK_PROTECT_TRUE ){
      fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
      fdc.st1 = ST1_NW;
      fdc.st2 = 0;
      fdc.carry = 0;
      status_message( 1, STATUS_WARN_TIME, "Disk Write Protected" );
      return 1;
    }
  }

	/* ヘッドがアンロード時は、ヘッドロード時間を加算 */

#ifdef	WAIT_FOR_HEADLOAD
  if( fdc.hl_stat[drv] == FALSE ){
    /* ロード音 ? */
    if ((cpu_timing > 0) && (fdc_wait)) {
      /*logfdc("### Head Down ###\n");*/
      xmame_dev_sample_headdown();
    }
    fdc.hl_stat[drv] = TRUE;
    fdc.wait += fdc.hlt_clk;
  }
  fdc.hl_wait[drv] = 0;
#endif


	/* 指定のIDR が現在のトラック位置と違う場合は、トラック変更 */

  if( fdc.command == READ_DIAGNOSTIC ||	    /* READ DIAGNOSTIC または        */
      fdc.command == WRITE_ID        ){	    /* WRITE ID        の場合        */
					    /*    トラック先頭から処理する為 */
					    /*    必ずトラック変更 (頭出し)  */
    if( ! disk_unformat(drv)   &&
	( drive[drv].track & 1 ) == fdc.hd ){
      fdc.wait += CLOCK_TRACK()
		    * (drive[drv].sec_nr - drive[drv].sec) / drive[drv].sec_nr;
    }
    disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
    fdc.carry = 0;

  }else{				    /* それ以外の場合                */
					    /*    現在セクタ位置から読む     */

    if( sec_buf.drv != drv ){			/* ドライブ情報の食い違いあり*/

      disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
      fdc.carry = 0;
      logfdc("\n<< sector reload >>\t\t\t");
      if( verbose_fdc )
	printf("FDC log : sec_buf reload $$$$\n" );

    }else

    if( ( drive[drv].track & 1 ) != fdc.hd ){	/* ヘッド位置が異なる時のみ  */
      int i, s = drive[drv].sec;		/* トラック変更する          */

      disk_now_track( drv, ((drive[drv].track & ~1)|fdc.hd) );
      fdc.carry = 0;

      if( fdc_wait ){
	if( ! disk_unformat(drv) ){
	  for( i=0; i<s; i++ ){				/* セクタ位置を移動  */
	    if( drive[drv].sec >= s ) break;		/* ヘッド移動前と    */
	    disk_next_sec( drv );			/* だいたい同じ位置  */
	  }						/* に移動させよう    */
	}
      }
    }
  }

	/* アンフォーマットの時は、直ちに異常終了 (WRITE_ID 除く) */

  if( fdc.command != WRITE_ID ){
    if( disk_unformat( drv ) || disk_unformatable( drv ) ){
      fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
      fdc.st1 = ST1_MA;
      fdc.st2 = 0;
      if( fdc.command == READ_ID ){ fdc.c = fdc.h = fdc.r = fdc.n = 0xff; }
      fdc.wait += CLOCK_TRACK() * 2;
      fdc.carry = CLOCK_GAP0();
      return 1;
    }
  }

	/* 繰り越し分のウェイトがある場合は、加算 */

  if( fdc.carry > 0 ){
    fdc.wait += fdc.carry;
  }
  fdc.carry = 0;


	/* 一旦終了。ST0 は NT(Normal Terminate:正常終了) をセット */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;

  return 1;
}



/*===========================================================================
 * READ/WRITE系コマンドで、IDを探す
 *	戻り値が 0 の場合、ディスクがないので、ディスクセットを無限に待つ
 *	戻り値が 1 の場合、結果が ST0〜ST2 にセットされる
 *			   (一部のエラーはここではセットしない)
 *===========================================================================*/
static	int	fdc_search_id( void )
{
  int	drv = (fdc.us);
  int	index_cnt = 0;			/* インデックスホール検出回数 */
  int	exist_iam = FALSE;		/* IAMが1度でも見つかったら真 */
  int	n;

	/* 最初に、ユニットなどのチェックを行う */

  if( fdc_check_unit() == 0 ){		/* ディスクが無いときは戻る */
    return 0;
  }else{
    if( fdc.st0 & ST0_IC ){		/* なにか異常があれば戻る   */
      return 1;				/* (Busy,Protect,Un-format) */
    }
  }

	/* セクタ検索 (セクタが見つかるかインデックスホール2回検出で終了) */

  if     ( sec_buf.sec_nr > 19 ) n = 0;		/* トラックフォーマット時の */
  else if( sec_buf.sec_nr > 10 ) n = 1;		/* N を適当に推測する       */
  else if( sec_buf.sec_nr >  5 ) n = 2;		/* (GAP3、GAP4長の算出用)   */
  else if( sec_buf.sec_nr >  2 ) n = 3;
  else if( sec_buf.sec_nr >  1 ) n = 4;
  else                           n = 5;

  while( 1 ){

    if( sector_density_mismatch()   ||	/* このセクタには IAM がない         */
        sec_buf.status == STATUS_MA ){
      ;							/* このセクタは無視  */

    }else{				/* このセクタには IAM がある         */
      exist_iam = TRUE;

      if( fdc.command == READ_DIAGNOSTIC ){	/* READ DIAG の場合          */
	if( sec_buf.status == STATUS_MA_MD ){		/* DATA mark なし    */
	  return 0;					/*	ハングする？ */
	}else{						/* DATA mark あり    */
	  break;					/*	該セクタ処理 */
	}

      }else if( fdc.command == READ_ID ){	/* READ IDの場合             */
	if( sec_buf.status == STATUS_DE ){		/* ID CRC 異常時     */
	  ;						/*	次セクタ検索 */
	}else{						/* ID CRC 正常時     */
	  break;					/*	該セクタ処理 */
	}

      }else{					/* READ / WRITE の場合       */
	if( idr_match() ){				/* IDR 一致時        */
	  if( sec_buf.status == STATUS_DE ){		    /* ID CRC 異常   */
	    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	    fdc.st1 = ST1_DE;
	    fdc.st2 = 0;

	    fdc.wait += CLOCK_RID_ID();
	    fdc.carry = CLOCK_RID_DATA( n );
	    disk_next_sec( drv );
	    if( drive[drv].sec==0 ) fdc.carry += CLOCK_GAP4(n) + CLOCK_GAP0();
	    return 1;
	  }
	  else
	  if( sec_buf.status == STATUS_MA_MD     &&	    /* (D)DAM なし   */
	      (fdc.command == READ_DATA ||		    /*   (READ時のみ)*/
	       fdc.command == READ_DELETED_DATA) ){
	      
	    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	    fdc.st1 = ST1_MA;
	    fdc.st2 = ST2_MD;

	    fdc.wait += CLOCK_RID_ID() + CLOCK_2MS();
	    disk_next_sec( drv );
	    if( drive[drv].sec==0 ) fdc.carry += CLOCK_GAP4(n) + CLOCK_GAP0();
	    return 1;
	  }
	  else{
	    break;					    /* 該セクタ処理  */
	  }
	}else{						/*  IDR 一致しない時 */
	  ;						    /* 次セクタ検索  */
	}
      }
    }

					/* 合致しなかったので次セクタを検索 */
    fdc.wait += CLOCK_SECTOR( n );
    disk_next_sec( drv );

    if( drive[drv].sec == 0 ){		/* インデックスホールを検出         */
      index_cnt ++;
      fdc.wait += CLOCK_GAP4( n );

      if( index_cnt >= 2 ){			/* 合計で、2回検出した      */

	fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
	fdc.st1 = (exist_iam) ? ST1_ND   : ST1_MA;	    /* IAMが1度でも */
	fdc.st2 = (exist_iam) ? 0 /*↓*/ : 0;		    /* 見つかったら */
	if( exist_iam ){				    /* ステータスが */
	  if( fdc.c != fdc.pcn[drv] ){ fdc.st2 |= ST2_NC;   /* 若干異なる   */
	  if( fdc.c == 0xff )          fdc.st2 |= ST2_BC; }
	}
	if( fdc.command == READ_ID ){ fdc.c = fdc.h = fdc.r = fdc.n = 0xff; }
	fdc.carry = CLOCK_GAP0();
	return 1;
      }

      fdc.wait += CLOCK_GAP0();
    }

  }


	/* ID無事みつかったので、 ST0 に NT をセットして戻る */

  fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
  fdc.st1 = 0;
  fdc.st2 = 0;

  if( fdc.command == READ_ID ){		/* READ ID の場合 */

    fdc.c = sec_buf.c;				/* 見つけたセクタのCHRNを設定*/
    fdc.h = sec_buf.h;
    fdc.r = sec_buf.r;
    fdc.n = sec_buf.n;

    fdc.wait += CLOCK_RID_ID();
    fdc.carry = CLOCK_RID_DATA( n );

    disk_next_sec( drv );
    if( drive[drv].sec==0 ) fdc.carry += CLOCK_GAP4(n) + CLOCK_GAP0();

  }else{				/* それ以外 の場合 */

    fdc.wait += CLOCK_RDT_ID();
    fdc.gap3  = CLOCK_GAP3( n );

    if( fdc.command == READ_DATA ||
	fdc.command == READ_DELETED_DATA ){
      if( sec_buf.deleted == DISK_DELETED_TRUE ) fdc.st2 |= ST2_CM;
    }
  }

  return 1;
}



/*===========================================================================
 * READ系コマンドで、DATAをバッファに読み込む
 *	結果は ST0〜ST2 とバッファセットされる
 *	(READ ID の場合、この関数は呼ばないこと)
 *===========================================================================*/
static	int	fdc_read_data( void )
{
  int	drv = (fdc.us);
  int	read_size, size, ptr, error;


  print_fdc_status(( (fdc.command==READ_DIAGNOSTIC) ? BP_DIAG : BP_READ ),
		   drv, drive[drv].track, drive[drv].sec);

	/* STATUS を再設定 (READ(DELETED)DATAの場合は DATA CRCエラーのみ) */

  if( sec_buf.status == STATUS_DE ){		/* ID CRC err */
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_DE;
    fdc.st2 = ( (sec_buf.deleted==DISK_DELETED_TRUE)? ST2_CM: 0 );
  }
  else
  if( sec_buf.status == STATUS_DE_DD ){		/* DATA CRC err */
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = ST1_DE;
    fdc.st2 = ST2_DD | ( (sec_buf.deleted==DISK_DELETED_TRUE)? ST2_CM: 0 );
  }
  else{						/* CRC OK */
    fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = ( (sec_buf.deleted==DISK_DELETED_TRUE)? ST2_CM: 0 );
  }
  if( ! idr_match() ){				/* IDR不一致 */
    fdc.st0 |= ST0_IC_AT;
    fdc.st1 |= ST1_ND;
  }


	/* DATA 部分を読む */

  read_size = 128 << (fdc.n & 7);		/* 読み込みサイズ       */
  ptr       = 0;				/* 書き込み位置		*/

  if( fdc.command == READ_DIAGNOSTIC ){
    if((sec_buf.size==0x80 && read_size!=0x80) ||	/* セクタの Nと      */
       (sec_buf.size & 0xff00) != read_size    ){	/* IDRの Nが違う時は */
      fdc.st0 |= ST0_IC_AT;				/* DATA CRC err      */
      fdc.st1 |= ST1_DE;
      fdc.st2 |= ST2_DD;
    }
  }

  while( read_size > 0 ){	/* -------------------指定サイズ分読み続ける */

    size = MIN( read_size, sec_buf.size );
    if( osd_fseek( drive[ drv ].fp,
		   drive[ drv ].sec_pos + SZ_DISK_ID,  SEEK_SET )==0 ){
      if( osd_fread( &data_buf[ptr], sizeof(Uchar),
		     size, drive[ drv ].fp )==(size_t)size ){
	error = 0;
      }
      else error = 1;
    } else error = 2;
    if( error ){			/* OSレベルのエラー発生 */
      printf_system_error( error );		/* DATA CRC err にする*/
      status_message( 1, STATUS_WARN_TIME, "DiskI/O Read Error" );
      fdc.st0 |= ST0_IC_AT;
      fdc.st1 |= ST1_DE;
      fdc.st2 |= ST2_DD;
      break;
    }

    ptr       += size;
    read_size -= size;
    if( read_size <= 0 ) break;


    fdc.st0 |= ST0_IC_AT;		/* 次のセクタに跨った */
    fdc.st1 |= ST1_DE;				/* DATA CRC err にする */
    fdc.st2 |= ST2_DD;


	/* セクタ間を埋める (DATA-CRC,GAP3,ID-SYNC,IAM,ID,ID-CRC,GAP2など) */

#if 0		/* セクタ間のデータ作成なし */
		    /* CRC  GAP3  SYNC   AM    ID  CRC GAP2 */
    if( fdc.mf ) size = 2 + 0x36 + 12 + (3+1) + 4 + 2 + 22;
    else         size = 2 + 0x2a +  6 + (1+1) + 4 + 2 + 11;

    ptr       += size;
    read_size -= size;

    disk_next_sec( drv );

#else		/* peach氏より、セクタ間データ生成処理が提供されました */

    size = fill_sector_gap(ptr, drv, fdc.mf);
    if (size < -1) goto FDC_READ_DATA_RETURN;

    ptr       += size;
    read_size -= size;
#endif
  }				/* ----------------------------------------- */

		/* 読み込み終わったら、次セクタへ進めておく */

  disk_next_sec( drv );


 FDC_READ_DATA_RETURN:

	/* READ DIAGNOSTIC の場合、CRC err と IDR不一致は正常としてみる	*/
	/* (ST1, ST2のビットは、そのまま残す)				*/

  if( fdc.command == READ_DIAGNOSTIC ){
    fdc.st0 &= ~ST0_IC;		/* == ST0_IC_NT     */
  }

  return 1;
}



/*===========================================================================
 * WRITE系コマンドで、バッファのDATAをファイルに書き出す
 *	結果は ST0〜ST2 とバッファセットされる
 *
 *	以下のようなイメージがあった場合、前のセクタにデータをライトしたら
 *	        +----------+----------------+----------+----------------+
 *	        |C       00|                |C       00|                |
 *	        |H       00|                |H       00|                |
 *	        |R       01|    データ部    |R       01|    データ部    |
 *	        |N       02|    256バイト   |N       01|    256バイト   |
 *	        |セクタ数16|                |セクタ数16|                |
 *	        |サイズ 256|                |サイズ 256|                |
 *	        +----------+----------------+----------+----------------+
 *	N=2 なので、ライトするの 512 バイト。つまり後ろのセクタを破壊する。
 *	        +----------+---------------------------------+----------+
 *	        |C       00|                                 |ここは    |
 *	        |H       00|                                 |意味の無い|
 *	        |R       01|            データ部             |ゴミデータ|
 *	        |N       02|            512バイト            |となる    |
 *	        |セクタ数16|                                 |          |
 *	        |サイズ 528|                                 | 16バイト |
 *	        +----------+---------------------------------+----------+
 *	サイズは「後のセクタのサイズ+16バイト」増やす (セクタ数は変えない)。
 *===========================================================================*/
static	int	fdc_write_data( void )
{
  long	id_pos, write_pos;
  int	write_size, total_size, size, ptr, error=0, sys_err=0;
  int	drv = fdc.us;
  unsigned char	c[2];
  int	gap4_size  = sec_buf.size;
  int	gap4_wrote = FALSE;

  print_fdc_status(BP_WRITE, drv, drive[drv].track, drive[drv].sec);


	/* 処理の最中にディスクが交換されたりした時は、処理せず異常終了 */

  if( disk_not_exist( drv ) ||				/* ディスク無し */
      drive[ drv ].read_only ||				/* 書き込み禁止 */
      drive[ drv ].protect == DISK_PROTECT_TRUE ||	/* ライトプロテクト */
      disk_unformat(drv) || disk_unformatable(drv) ){	/* アンフォーマット */

    int write_protected = FALSE;	/* 書込不可が原因の場合、真 */
    if( ! disk_not_exist( drv ) &&
	( drive[ drv ].read_only ||
	  drive[ drv ].protect == DISK_PROTECT_TRUE ) ){
      write_protected = TRUE;
    }

    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    if( write_protected ) fdc.st1 |= ST1_NW;
    else                  fdc.st0 |= ST0_NR;


    if( fdc_ignore_readonly &&		/* ReadOnly を無視する場合は正常終了 */
	write_protected     &&
	drive[ drv ].protect != DISK_PROTECT_TRUE ){

      fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
      fdc.st1 = 0;
      fdc.st2 = ( (fdc.command==WRITE_DELETED_DATA)? ST2_CM: 0 );

      if( verbose_fdc ){
	printf("FDC %s : Drive %d Write Skipped (write protected)\n",
	       cmd_name[fdc.command],drv+1);
      }
      status_message( 1, STATUS_WARN_TIME, "Disk Write Skipped" );

    }else{

      if( verbose_fdc ){
	if( write_protected )
	  printf("FDC %s : Drive %d Write Failed (write protected)\n",
		 cmd_name[fdc.command],drv+1);
	else
	  printf("FDC %s : Drive %d Write Failed (file changed ?)\n",
		 cmd_name[fdc.command],drv+1);
      }
      status_message( 1, STATUS_WARN_TIME, "Disk Write Failed" );
    }
    return 1;
  }


	/* DATA 部分を書く */

  id_pos     = drive[ drv ].sec_pos;		/* ID更新時のファイル位置 */

  write_size = 128 << (fdc.n & 7);		/* 書き込みサイズ         */
  ptr        = 0;				/* データのポインタ       */
  write_pos  = drive[drv].sec_pos + SZ_DISK_ID;	/* 書き込みのファイル位置 */
  total_size = sec_buf.size;			/* セクタのデータ部サイズ */

  while( write_size > 0 ){	/* -------------------指定サイズ分書き続ける */

    size = MIN( write_size, sec_buf.size );
    if( write_pos + size <= drive[drv].disk_end ){
    if( osd_fseek( drive[ drv ].fp,
		   write_pos,  SEEK_SET )==0 ){
      if( osd_fwrite( &data_buf[ptr], sizeof(Uchar),
		      size, drive[ drv ].fp )==(size_t)size ){
	error = 0;
      }
      else error = 1;
    } else error = 2;
    } else error = 3;
    if( error ){
      printf_system_error( error );
      break;
    }

    ptr        += size;
    write_pos  += size;
    write_size -= size;

    if( write_size<=0 ) break;			/* 普通は、ここで抜ける     */

    if( gap4_wrote == FALSE ){
      disk_next_sec( drv );			/* 次のセクタに跨った場合   */
    }

    if( drive[drv].sec != 0 ){			/* 後続するセクタがあれば、 */
						/*	そのセクタに上書き  */
      total_size += (sec_buf.size + SZ_DISK_ID);/*	(その分、サイズ増)  */

    }else{ /* drive[drv].sec == 0 */		/* 無し(先頭に戻った)ならば */
      gap4_wrote = TRUE;			/*	GAP4 に上書とみなす */
      total_size += (gap4_size + SZ_DISK_ID);	/*	(適当に、サイズ増)  */
      /* 次のトラックのデータを破壊するおそれあり。うーん。*/
    }

    if( verbose_fdc ){
      if( gap4_wrote == FALSE )
	printf("FDC %s : Sector Overlap in track %d (DRIVE %d:)\n",
	       cmd_name[fdc.command], drive[drv].track, drv+1);
      else
	printf("FDC %s : GAP4 Overlap in track %d (DRIVE %d:)\n",
	       cmd_name[fdc.command], drive[drv].track, drv+1);
    }

  }				/* ----------------------------------------- */


	/* ID 部を更新する。*/

  if( fdc.mf && fdc.n==0 ){
    c[0] = DISK_DELETED_FALSE;
    c[1] = STATUS_MA_MD;
  }else{
    if( fdc.command==WRITE_DATA ){
      c[0] = DISK_DELETED_FALSE;
      c[1] = (error) ? STATUS_DE_DD : STATUS_NORMAL;
    }else{
      c[0] = DISK_DELETED_TRUE;
      c[1] = (error) ? STATUS_DE_DD : STATUS_CM;
    }
  }
  if( error > 0 ) sys_err = 1;
  if( osd_fseek( drive[ drv ].fp,		/* ID の、DAM/DDAMを更新 */
		 id_pos + DISK_DELETED,  SEEK_SET )==0 ){
    if( osd_fwrite( &c[0], sizeof(Uchar), 2, drive[ drv ].fp )==2 ){
      error = 0;
    }
    else error = 1;
  } else error = 2;
  if( error ){
    printf_system_error( error );
    sys_err = 1;
  }


  c[0] = ( total_size >>  0  ) & 0xff;
  c[1] = ( total_size >>  8  ) & 0xff;
  if( osd_fseek( drive[ drv ].fp,		/* ID の、セクタサイズを更新 */
		 id_pos + DISK_SEC_SZ,  SEEK_SET )==0 ){
    if( osd_fwrite( &c[0], sizeof(Uchar), 2, drive[ drv ].fp )==2 ){
      error = 0;
    }
    else error = 1;
  } else error = 2;
  if( error ){
    printf_system_error( error );
    sys_err = 1;
  }


  osd_fflush( drive[ drv ].fp );


	/* 途中、システムのエラーが起こったら異常終了する */

  if( sys_err ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us | ST0_NR;
    fdc.st1 = 0;
    fdc.st2 = 0;
    status_message( 1, STATUS_WARN_TIME, "DiskI/O Write Error" );
  }else{
    fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = ( (fdc.command==WRITE_DELETED_DATA)? ST2_CM: 0 );
  }

  if( gap4_wrote == FALSE ){
    disk_next_sec( drv );
  }
  return 1;
}



/*===========================================================================
 * WRITE ID コマンドで、バッファ の IDR に基づきファイルに書き出す
 *	結果は ST0〜ST2 
 *===========================================================================*/
static	int	fdc_write_id( void )
{
  int	size, error, i, j, id_ptr;
  long	format_pos;
  char	id[SZ_DISK_ID],	data[128];
  int	drv = fdc.us;


	/* 処理の最中にディスクが交換されたりした時は、処理せず異常終了 */

  if( disk_not_exist( drv ) ||				/* ディスク無し */
      drive[ drv ].read_only ||				/* 書き込み禁止 */
      drive[ drv ].protect == DISK_PROTECT_TRUE ||	/* ライトプロテクト */
      disk_unformatable(drv) ){				/* フォーマット不能 */

    int write_protected = FALSE;	/* 書込不可が原因の場合、真 */
    if( ! disk_not_exist( drv ) &&
	( drive[ drv ].read_only ||
	  drive[ drv ].protect == DISK_PROTECT_TRUE ) ){
      write_protected = TRUE;
    }

    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
    if( write_protected ) fdc.st1 |= ST1_NW;
    else                  fdc.st0 |= ST0_NR;


    if( fdc_ignore_readonly &&		/* ReadOnly を無視する場合は正常終了 */
	write_protected     &&
	drive[ drv ].protect != DISK_PROTECT_TRUE ){

      fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
      fdc.st1 = 0;
      fdc.st2 = 0;

      if( verbose_fdc ){
	printf("FDC %s : Drive %d Write Skipped (write protected)\n",
	       cmd_name[fdc.command],drv+1);
      }
      status_message( 1, STATUS_WARN_TIME, "Disk Write Skipped" );

    }else{

      if( verbose_fdc ){
	if( write_protected )
	  printf("FDC %s : Drive %d Format Failed (write protected)\n",
		 cmd_name[fdc.command],drv+1);
	else if( ! disk_not_exist( drv ) && disk_unformatable(drv) )
	  printf("FDC %s : Drive %d Format Failed (no file space)\n",
		 cmd_name[fdc.command],drv+1);
	else
	  printf("FDC %s : Drive %d Format Failed (file changed ?)\n",
		 cmd_name[fdc.command],drv+1);
      }
      status_message( 1, STATUS_WARN_TIME, "Disk Write Failed" );
    }
    return 1;
  }


	/* フォーマットデータの準備 */

  for( size=0; size<128; size++ ) data[size] = fdc.d;


	/*        フォーマットパラメータの解析       */
	/* トラック1周を越えないか判定。越えた時は、 */
	/* 最後の1周に収まる分のみを WRITE ID する。 */

  id_ptr = 0;
  {
    int	SZ_BYTE = (drive[ drv ].type==DISK_TYPE_2HD) ? 10400 : 6250;
    int	SZ_GAP0 = (fdc.mf) ? (80+12+3+1    +50) : (40+6+1+1+    50);
    int	SZ_AM   = (fdc.mf) ? (   12+3+1+4+2+22) : (   6+1+1+4+2+11);
    int	SZ_DAM  = (fdc.mf) ? (   12+3+1+0+2+ 0) : (   6+1+1+0+2+ 0);
    int	max_sec;
    SZ_GAP0 = 0; /* とりあえず、GAP0 の分は越えても支障ナシ */
    max_sec = (SZ_BYTE-SZ_GAP0)/( SZ_AM+SZ_DAM + 128*(1<<(fdc.n&7))+ fdc.gpl );
    if( fdc.sc > max_sec ){

      if( verbose_fdc )
        printf("FDC %s : Over Sector %d -> fixed %d (DRIVE %d:)\n",
	       cmd_name[fdc.command],fdc.sc,max_sec,drv+1);

      id_ptr = (fdc.sc - max_sec) * 4;
      fdc.sc = max_sec;

    }
  }



	/* ID / DATA をファイルに書き込む */

  format_pos = drive[ drv ].track_top;
  error = 0;

  if( fdc.sc==0 ){				/* アンフォーマットを作成 */

    for( i=0; i<SZ_DISK_ID; i++ ) id[ i ] = 0;

    if( format_pos + 16 <= drive[drv].disk_end ){
    if( osd_fseek( drive[ drv ].fp,  format_pos,  SEEK_SET )==0 ){
      if( osd_fwrite( id, sizeof(Uchar), 16, drive[ drv ].fp )==16 ){
	error = 0;
      }
      else error = 1;
    } else error = 2;
    } else error = 3;

  }else for( i=0; i<fdc.sc; i++ ){		/* フォーマットを作成 */

    id[ DISK_C ] = data_buf[ id_ptr++ ];
    id[ DISK_H ] = data_buf[ id_ptr++ ];
    id[ DISK_R ] = data_buf[ id_ptr++ ];
    id[ DISK_N ] = data_buf[ id_ptr++ ];
    id[ DISK_SEC_NR   ] = (fdc.sc    ) & 0xff;
    id[ DISK_SEC_NR+1 ] = (fdc.sc>>8 ) & 0xff;
    id[ DISK_DENSITY ] = (fdc.mf) ? DISK_DENSITY_DOUBLE : DISK_DENSITY_SINGLE;
    id[ DISK_DELETED ] = DISK_DELETED_FALSE;
    id[ DISK_STATUS ] = STATUS_NORMAL;
    id[ DISK_RESERVED+0 ] = 0;
    id[ DISK_RESERVED+1 ] = 0;
    id[ DISK_RESERVED+2 ] = 0;
    id[ DISK_RESERVED+3 ] = 0;
    id[ DISK_RESERVED+4 ] = 0;
    id[ DISK_SEC_SZ   ] = ( 128*(1<<(fdc.n&7))      ) & 0xff;
    id[ DISK_SEC_SZ+1 ] = ( 128*(1<<(fdc.n&7)) >> 8 ) & 0xff;

    if( verbose_fdc )
      if( id[ DISK_N ] != fdc.n )
        printf("FDC %s : Mix Sector in track %d (DRIVE %d:)\n",
	       cmd_name[fdc.command],drive[drv].track,drv+1);

    if( format_pos + 16 <= drive[drv].disk_end ){
    if( osd_fseek( drive[ drv ].fp,  format_pos,  SEEK_SET )==0 ){
      if( osd_fwrite( id, sizeof(Uchar), 16, drive[ drv ].fp )==16 ){
	format_pos += 16;

	for( j=0; j<(1<<(fdc.n&7)); j++ ){

	  if( format_pos + 128 <= drive[drv].disk_end ){
	  if( osd_fseek( drive[ drv ].fp,  format_pos,  SEEK_SET )==0 ){
	    if( osd_fwrite( data, sizeof(Uchar), 128, drive[ drv ].fp )==128 ){
	      format_pos += 128;
	    }
	    else{ error = 1; break; }
	  } else{ error = 2; break; }
	  } else{ error = 3; break; }

	}

      }
      else error = 1;
    } else error = 2;
    } else error = 3;

  }

  if( error ){					/* SEEK / READ Error */
    printf_system_error( error );
  }

  osd_fflush( drive[ drv ].fp );


	/* 途中、システムのエラーが起こったら異常終了する */

  if( error ){
    fdc.st0 = ST0_IC_AT | (fdc.hd<<2) | fdc.us | ST0_NR;
    fdc.st1 = 0;
    fdc.st2 = 0;
    status_message( 1, STATUS_WARN_TIME, "DiskI/O Write Error" );
  }else{
    fdc.st0 = ST0_IC_NT | (fdc.hd<<2) | fdc.us;
    fdc.st1 = 0;
    fdc.st2 = 0;
  }

  disk_now_track( drv,   drive[drv].track );
  return 1;
}



/*===========================================================================
 * E-PHASE 正常終了時に、次の CHRN を指す
 *	with_TC が真 (TCにより正常終了) の場合、最終セクタなら 次 CHRN を指す
 *	with_TC が偽                    の場合、最終セクタでも 現 CHRN のまま
 *	なお、最終セクタに達したら真を返す
 *
 *	2DD/2HD でも同じ処理でいいんだろうか・・・(資料がない ;_;)
 *===========================================================================*/
static	int	fdc_next_chrn( int with_TC )
{
  if( fdc.mt==0 ){		/* マルチトラック処理をしない時 */

    if( fdc.r == fdc.eot ){		/* 最終セクタなら、	*/
      if( with_TC ){
	fdc.c ++;			/* C+=1, R=1 にする。	*/
	fdc.r = 1;			/* 			*/
      }
      return 1;				/* 返り値は 真 (最終)	*/
    }else{
      fdc.r ++;				/* 最終でなければ、R+=1	*/
      return 0;				/* 返り値は 偽 (継続)	*/
    }

  }else{			/* マルチトラック処理の時 */

    if( fdc.hd==0 ){		/*   表面処理時 */

      if( fdc.r == fdc.eot ){		/* 最終セクタなら、	*/
	fdc.hd = 1;			/* 裏面に切替えて、	*/
	fdc.h ^= 1;			/* H 反転、R = 1	*/
	fdc.r  = 1;			/* 返り値は 偽 (継続)	*/
	return 0;
      }else{
	fdc.r ++;			/* 最終でなければ、R+=1	*/
	return 0;			/* 返り値は 偽 (継続)	*/
      }

    }else{			/*   裏面処理時 */

      if( fdc.r == fdc.eot ){		/* 最終セクタなら、	*/
	if( with_TC ){
	  fdc.h ^= 1;			/* H 反転、		*/
	  fdc.c ++;			/* C+=1, R=1 にする。	*/
	  fdc.r = 1;			/* 			*/
	}
	return 1;			/* 返り値は 真 (最終)	*/
      }else{
	fdc.r ++;			/* 最終でなければ、R+=1	*/
	return 0;			/* 返り値は 偽 (継続)	*/
      }
    }
  }
}



/*===========================================================================
 * コマンドフェーズ共通
 *	ホスト(サブCPU)とのデータのやり取りは 4clock 以内に終わると
 *	勝手に想定 (つまり、データのやり取りにウェイトは無し)
 *===========================================================================*/
static	void	c_phase( void )
{
  int cmd, nd, i;
  unsigned char * const table[][10] =
  {
/*WAIT */{ 0 },
/*R_DAT*/{ &fdc.c0,&fdc.c1, &fdc.c,&fdc.h,&fdc.r,&fdc.n, &fdc.eot,&fdc.gpl,&fdc.dtl,0},
/*R_DEL*/{ &fdc.c0,&fdc.c1, &fdc.c,&fdc.h,&fdc.r,&fdc.n, &fdc.eot,&fdc.gpl,&fdc.dtl,0},
/*R_DIA*/{ &fdc.c0,&fdc.c1, &fdc.c,&fdc.h,&fdc.r,&fdc.n, &fdc.eot,&fdc.gpl,&fdc.dtl,0},
/*R_ID */{ &fdc.c0,&fdc.c1, 0 },
/*W_DAT*/{ &fdc.c0,&fdc.c1, &fdc.c,&fdc.h,&fdc.r,&fdc.n, &fdc.eot,&fdc.gpl,&fdc.dtl,0},
/*W_DEL*/{ &fdc.c0,&fdc.c1, &fdc.c,&fdc.h,&fdc.r,&fdc.n, &fdc.eot,&fdc.gpl,&fdc.dtl,0},
/*W_ID */{ &fdc.c0,&fdc.c1, &fdc.n, &fdc.sc, &fdc.gpl, &fdc.d, 0 },
/*S_EQU*/{ &fdc.c0,&fdc.c1, &fdc.c,&fdc.h,&fdc.r,&fdc.n, &fdc.eot,&fdc.gpl,&fdc.stp,0},
/*S_LOW*/{ &fdc.c0,&fdc.c1, &fdc.c,&fdc.h,&fdc.r,&fdc.n, &fdc.eot,&fdc.gpl,&fdc.stp,0},
/*S_HIG*/{ &fdc.c0,&fdc.c1, &fdc.c,&fdc.h,&fdc.r,&fdc.n, &fdc.eot,&fdc.gpl,&fdc.stp,0},
/*SEEK */{ &fdc.c0,&fdc.c1, &fdc.cn, 0 },
/*RECAL*/{ &fdc.c0,&fdc.c1, 0 },
/*SNS_I*/{ &fdc.c0,         0 },
/*SNS_D*/{ &fdc.c0,&fdc.c1, 0 },
/*SPECI*/{ &fdc.c0,         &fdc.s0, &fdc.s1, 0 },
/*INVAL*/{ &fdc.c0,         0 },
  };

  /* 「ホストがデータ書込→FDCがそれを検知」のウェイトが必要なら、 */
  /* ここで時間経過を待つ。(待っている間は処理せずに関数を終える)  */
  ;


  if( fdc.status & REQ_MASTER ){	/* ホストからのデータ無し */

    ICOUNT( -1 );

  }else{				/* ホストからデータが来た */

    *table[ fdc.command ][ fdc.step ] = fdc.write;		/*データ引取*/
    fdc.step ++;

    if( table[ fdc.command ][ fdc.step ] ){	/* まだデータが必要な時 */

      fdc.status = (fdc.status&0x0f) | FDC_BUSY | REQ_MASTER;
      ICOUNT( -1 );

    }else{					/* 全データ引き取り完了 */

      if( FDC_flag ){					/* 割込発生中ならば  */
	for( i=0; i<MAX_DRIVE; i++ ){			/* シーク完了した    */
	  if( fdc.seek_stat[i]==SEEK_STAT_INTR ){	/* ドライブ (割込の  */
	    fdc.seek_stat[i] = SEEK_STAT_STOP;		/* 発生元) を検索    */
	    fdc.status &= ~(1<<i);
	    break;
	  }
	}
	fdc.intr_unit = i;				/* このドライブは    */
	fdc_cancel_interrupt();				/* SENSE INT で通知  */

	if( verbose_fdc )
	  if( fdc.command != SENSE_INT_STATUS )
	    printf("FDC log : Missing SENSE INT STATUS $$$$\n" );
      }


      cmd     = (fdc.c0 & 0x1f);
      fdc.sk  = (fdc.c0 & 0x20) >> 5;
      fdc.mf  = (fdc.c0 & 0x40) >> 6;
      fdc.mt  = (fdc.c0 & 0x80) >> 7;

      fdc.us  = (fdc.c1 & 0x03);			/* SENSE INTとINVALID*/
      fdc.hd  = (fdc.c1 & 0x04) >> 2;			/* では不要なんだけど*/


      switch( fdc.command ){				/* コマンド別の処理  */

      case READ_DIAGNOSTIC:					/* READ系 */
	if( verbose_fdc ){
	  printf("FDC %s : Drive %d Track %d\n",
		 cmd_name[fdc.command],fdc.us+1,fdc.ncn[fdc.us]*2+fdc.hd);
	}
	fdc.sk = 0;
	fdc.mt = 0;	/* FALLTHROUGH */
      case READ_DATA:
      case READ_DELETED_DATA:
      case READ_ID:
	fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	fdc.phase  = E_PHASE;

	if( fdc.command==READ_ID ){
	  if( verbose_fdc ){
	    printf("FDC %s : Drive %d Track %d\n",
		   cmd_name[fdc.command],fdc.us+1,fdc.ncn[fdc.us]*2+fdc.hd);
	  }
	  logfdc("%s mf%d us%d hd%d\n",
		 cmd_name[fdc.command],
		 fdc.mf,fdc.us,fdc.hd);
	}else{
	  logfdc("%s sk%d mf%d mt%d us%d hd%d eot=%d gpl=%d dtl=%d\n",
		 cmd_name[fdc.command],
		 fdc.sk,fdc.mf,fdc.mt,fdc.us,fdc.hd,fdc.eot,fdc.gpl,fdc.dtl);
	}
	break;

      case WRITE_DATA:						/* WRITE系 */
      case WRITE_DELETED_DATA:
	fdc.sk = 0;	/* FALLTHROUGH */
      case WRITE_ID:
	fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	fdc.phase  = E_PHASE;

	if( fdc.command==WRITE_ID ){
	  logfdc("%s mf%d us%d hd%d n%d sc%d gpl%d d%02x\n",
		 cmd_name[fdc.command],
		 fdc.mf,fdc.us,fdc.hd,fdc.n,fdc.sc,fdc.gpl,fdc.d);
	}else{
	  logfdc("%s sk%d mf%d mt%d us%d hd%d eot=%d gpl=%d dtl=%d\n",
		 cmd_name[fdc.command],
		 fdc.sk,fdc.mf,fdc.mt,fdc.us,fdc.hd,fdc.eot,fdc.gpl,fdc.dtl);
	}
	break;

      case SEEK:						/* SEEK系 */
      case RECALIBRATE:
	fdc.ncn[ fdc.us ] = (fdc.command==SEEK) ? fdc.cn : 0 ;
	fdc.status = (fdc.status&0x0f) | FDC_BUSY;
	fdc.phase  = E_PHASE;

	logfdc("%s us%d %02x (Tr.%02d,%02d=>%02d,%02d)\n", 
	       cmd_name[fdc.command],
	       fdc.us,fdc.ncn[fdc.us],fdc.pcn[fdc.us]*2,fdc.pcn[fdc.us]*2+1,
				      fdc.ncn[fdc.us]*2,fdc.ncn[fdc.us]*2+1);
	break;

      case SENSE_INT_STATUS:					/* SENSE系 */
      case SENSE_DEVICE_STATUS:
      case INVALID:
	fdc.status  = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
	fdc.phase   = R_PHASE;	

	if( fdc.command==SENSE_DEVICE_STATUS ){
	  logfdc("%s us%d hd%d\n",   cmd_name[fdc.command], fdc.us, fdc.hd );
	}else{
	  logfdc("%s \n",            cmd_name[fdc.command] );
	}
	break;

      case SPECIFY:						/* SPECIFY系 */
	fdc.srt_clk = (16-((fdc.s0>>4)&0x0f)) *  (2*4000);  /* (16-srt)x 2ms */
	fdc.hut_clk =      (fdc.s0    &0x0f)  * (32*4000);  /*     hut x32ms */
	fdc.hlt_clk =     ((fdc.s1>>1)&0x7f)  *  (4*4000);  /*     hlt x 4ms */
	nd          =       fdc.s1    &   1;
	if( fdc.hut_clk==0 ) fdc.hut_clk = 16 * (32*4000);		/* ? */

	logfdc("%s srt%dms hut%dms hlt%dms (%02X %02X)\n",
	       cmd_name[fdc.command],
	     fdc.srt_clk/4000,fdc.hut_clk/4000,fdc.hlt_clk/4000,fdc.s0,fdc.s1);

	fdc.status  = (fdc.status&0x0f) | REQ_MASTER;
	fdc.command = WAIT;
	break;

      default:							/* 他はNG */
	fprintf( stderr, "FDC unsupported %02x(%s)\n",
		 cmd, cmd_name[ fdc.command ] );
	ICOUNT( -1 );
	return;
      }

      fdc.step = 0;
      ICOUNT( 0 );
      REPEAT();
    }
  }
}



/*===========================================================================
 * リザルトフェーズ共通
 *	ホスト(サブCPU)とのデータのやり取りは 4clock 以内に終わると
 *	勝手に想定 (つまり、データのやり取りにウェイトは無し)
 *===========================================================================*/
static	void	r_phase( void )
{
  int i;
  unsigned char * const table[][8] =
  {
/*WAIT */{ 0 },
/*R_DAT*/{ &fdc.st0, &fdc.st1, &fdc.st2, &fdc.c, &fdc.h, &fdc.r, &fdc.n, 0 },
/*R_DEL*/{ &fdc.st0, &fdc.st1, &fdc.st2, &fdc.c, &fdc.h, &fdc.r, &fdc.n, 0 },
/*R_DIA*/{ &fdc.st0, &fdc.st1, &fdc.st2, &fdc.c, &fdc.h, &fdc.r, &fdc.n, 0 },
/*R_ID */{ &fdc.st0, &fdc.st1, &fdc.st2, &fdc.c, &fdc.h, &fdc.r, &fdc.n, 0 },
/*W_DAT*/{ &fdc.st0, &fdc.st1, &fdc.st2, &fdc.c, &fdc.h, &fdc.r, &fdc.n, 0 },
/*W_DEL*/{ &fdc.st0, &fdc.st1, &fdc.st2, &fdc.c, &fdc.h, &fdc.r, &fdc.n, 0 },
/*W_ID */{ &fdc.st0, &fdc.st1, &fdc.st2, &fdc.c, &fdc.h, &fdc.r, &fdc.n, 0 },
/*S_EQU*/{ &fdc.st0, &fdc.st1, &fdc.st2, &fdc.c, &fdc.h, &fdc.r, &fdc.n, 0 },
/*S_LOW*/{ &fdc.st0, &fdc.st1, &fdc.st2, &fdc.c, &fdc.h, &fdc.r, &fdc.n, 0 },
/*S_HIG*/{ &fdc.st0, &fdc.st1, &fdc.st2, &fdc.c, &fdc.h, &fdc.r, &fdc.n, 0 },
/*SEEK */{ 0 },
/*RECAL*/{ 0 },
/*SNS_I*/{ &fdc.r0, &fdc.r1, 0 },
/*SNS_D*/{ &fdc.st3, 0 },
/*SPECI*/{ 0 },
/*INVAL*/{ &fdc.r0, 0 },
  };

	/* R-PHASE の一番最初に、通知するデータの一覧を生成する */

  if( fdc.step == 0 ){

    fdc.status &= ~REQ_MASTER;		/* 前準備 */
/*  ICOUNT( 0 );*/

    switch( fdc.command ){		/* コマンド別の処理  */

    case SENSE_INT_STATUS:			/* SENSE INT STATUS          */
      i = fdc.intr_unit;				/* シーク完(割込発生)*/
      fdc.intr_unit = 4;
      if( i < MAX_DRIVE ){				/* のドライブあり    */
	if( i<NR_DRIVE ) fdc.r0 = ST0_IC_NT | ST0_SE | i;
	else             fdc.r0 = ST0_IC_AT | ST0_SE | ST0_NR | i;
	fdc.r1 = fdc.pcn[ i ];
	logfdc("\t\t\t\t\t\t<st0:%02x pcn:%02x>\n",
	       fdc.r0,fdc.r1);
      }else{						/* 見つからない時は  */
	fdc.command = INVALID;				/* INVALID扱いとする */
	fdc.r0 = ST0_IC_IC;
	logfdc("\t\t\t\t\t\tinvalid\n");
      }
      break;

    case SENSE_DEVICE_STATUS:			/* SENSE DEVICE STATUS    */
      if( fdc.us >= NR_DRIVE ){			/* ST3 を生成する         */
	fdc.st3 = ST3_FT | ((fdc.hd<<2) & ST3_HD) | ( fdc.us & ST3_US );
      }else{
	if( disk_not_exist( fdc.us ) ||
	    (disk_ex_drv & (1 << fdc.us)) ){ /* ディスク入れ替えチェック */
							    /* thanks peach! */
	  disk_ex_drv ^= (1 << fdc.us); /* ドライブ番号のビット反転 */
	  fdc.st3 =((fdc.status & (1<<fdc.us)) ? 0 : ST3_RY ) |
			((fdc.pcn[fdc.us]==0) ? ST3_T0 : 0 ) |
			((fdc.hd<<2) & ST3_HD) | ( fdc.us & ST3_US );
	}else{
	  fdc.st3 =((fdc.status & (1<<fdc.us)) ? 0 : ST3_RY ) |
	      		((drive[fdc.us].protect==DISK_PROTECT_TRUE)?ST3_WP:0) |
			((fdc.pcn[fdc.us]==0) ? ST3_T0 : 0 ) | ST3_TS |
			((fdc.hd<<2) & ST3_HD) | ( fdc.us & ST3_US );
	}
      }
      logfdc("\t\t\t\t\t\t<st3:%02x>\n",
	     fdc.st3);
      break;

    case INVALID:				/* INVALID                */
      fdc.r0 = ST0_IC_IC;			/* ST0 を生成する         */
      logfdc("\t\t\t\t\t\t<st0:%02x>\n",
	     fdc.r0);
      break;

    default:					/* それ以外は             */
      fdc_occur_interrupt();			/* 割り込み発生させる     */
      logfdc("\t\t\t\t\t\t<ST:%02x %02x %02x  CHRN:%02x %02x %02x %02x>\n",
	     fdc.st0,fdc.st1,fdc.st2,fdc.c,fdc.h,fdc.r,fdc.n);
      break;
    }
  }



  if( fdc.status & REQ_MASTER ){	/* ホストまだデータ読まず */

    /* 前準備により、 fdc.step == 0 の時はここにはこない */
    ICOUNT( -1 );

  }else{				/* ホストがデータが読んだ */

    /* 「ホストがデータ読込→FDCがそれを検知」のウェイトが必要なら、 */
    /* ここで時間経過を待つ。(待っている間は処理せずに関数を終える)  */
    ;

    if( table[ fdc.command ][ fdc.step ] ){		/* まだデータあり */

      if( fdc.step != 0 ) fdc_cancel_interrupt();

      fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO | REQ_MASTER;
      fdc.read   = *table[ fdc.command ][ fdc.step ];
      fdc.step ++;
      ICOUNT( -1 );

    }else{						/* もうデータない */

      fdc.read = 0xff;
      fdc.status = (fdc.status&0x0f) | REQ_MASTER;
      fdc.command = WAIT;
      fdc.step = 0;
      ICOUNT( -1 );
    }
  }
}



/*===========================================================================
 * エグゼキューションフェーズ個別
 *===========================================================================*/

/*---------------------------------------------------------------------------
 *	SEEK系
 *---------------------------------------------------------------------------*/
static	void	e_phase_seek( void )
{

  fdc.status = (fdc.status&0x0f) | REQ_MASTER | (1<<fdc.us);

  if( fdc.us >= NR_DRIVE ||				/* 未接続ドライブ or */
      fdc.pcn[ fdc.us ] == fdc.ncn[ fdc.us ] ){		/* シーク不要の場合  */

    if( sec_buf.drv != fdc.us ) sec_buf.drv = -1;

    fdc.seek_stat[ fdc.us ] = SEEK_STAT_END;
    fdc.seek_wait[ fdc.us ] = 0;
    ICOUNT( 0 );
    REPEAT();

  }else{		/* 動作中のドライブを再度シークしたらどうしよう ? */

    sec_buf.drv = -1;

#ifdef	WAIT_FOR_SEEK
    if( fdc_wait == FALSE ){
#endif
      fdc.pcn[ fdc.us ] = fdc.ncn[ fdc.us ];
      fdc.seek_stat[ fdc.us ] = SEEK_STAT_END;
      fdc.seek_wait[ fdc.us ] = 0;
      ICOUNT( 0 );
      REPEAT();

#ifdef	WAIT_FOR_SEEK
    }else{
      /* シーク音 ? */
      if ((cpu_timing > 0) && (fdc_wait)) {
	fdc_sound_counter = 0;
	/*logfdc("### Seek ###\n");*/
	xmame_dev_sample_seek();
      }
      fdc.seek_stat[ fdc.us ] = SEEK_STAT_MOVE;
      if( fdc.pcn[ fdc.us ] < fdc.ncn[ fdc.us ] ) fdc.pcn[ fdc.us ] ++;
      else                                        fdc.pcn[ fdc.us ] --;
      fdc.seek_wait[ fdc.us ] = fdc.srt_clk;
      ICOUNT( -1 );
      REPEAT();
    }
#endif
  }

  fdc.command = WAIT;
  fdc.step = 0;
}



/*---------------------------------------------------------------------------
 *	READ系
 *---------------------------------------------------------------------------*/

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * 指定IDを検索し、みつかったらバッファに読み込む
 * 戻り値 -1:終了(E-PHASE完)  0:今のまま  1:次へ
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	*/ 	
static	int	e_phase_read_search( void )
{
  int search, ret, skip_this;

  if( fdc.command != READ_ID ){
    logfdc("\t\t\tC:%02X H:%02X R:%02X N:%02X  ",fdc.c,fdc.h,fdc.r,fdc.n);
  }


  fdc.ddam_not_skipped = 0;


  search = fdc_search_id();		/* ID検索する			*/
/*printf("<%d><%d>",fdc.wait,fdc.carry);*/
  if( search == FALSE ){		/* 検索不能 (ディスク未挿入)	*/

    ret = 0;						/* 何度もやりなおし  */

  }else{				/* 検索可能 (ディスクあり)	*/

    if( fdc.command == READ_ID ){	    /* READ ID の場合		     */

      ret = -1;

    }else if( fdc.st0 & ST0_IC ){	    /* 検索したけど IDが見つかんない */

      logfdc("-Miss\n");
      ret = -1;

    }else{				    /* 検索したら IDが見つかった     */

      if( ( fdc.command==READ_DATA &&		/* (D)DAM 一致しないなら    */
	    ( fdc.st2 & ST2_CM )   )          ||
	  ( fdc.command==READ_DELETED_DATA &&
	    !( fdc.st2 & ST2_CM )          )  ){
							/* スキップチェック */
	if( fdc.sk ){ skip_this = TRUE;  }
	else        { skip_this = FALSE; fdc.ddam_not_skipped = TRUE; }

      }else{					/* (D)DAM が一致したなら    */
						/* (READ DIAGNOSTIC の時も) */
	skip_this = FALSE;				/* スキップはしない */
      }

      if( skip_this ){				/* セクタをスキップする場合 */
	if( fdc_next_chrn(FALSE)==0 ){			/* 次セクタに進む   */
	  logfdc("Skip\n");
	  ret = 0;
	}else{						/* 既に EOT だった  */
	  fdc.st0 |= ST0_IC_AT;
	  fdc.st1 |= ST1_EN;
	  logfdc("Skip-EOT\n");
	  ret = -1;
	}
      }else{					/* スキップせずに読む場合    */
	fdc_read_data();
	fdc.data_ptr = 0;
	if( fdc.n==0 ) fdc.counter = (128>fdc.dtl) ? 128 : fdc.dtl;
	else           fdc.counter = 128 << (fdc.n & 7);
	ret = 1;
      }
    }
    REPEAT();
  }
  return ret;
}



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * バッファから1バイトとりだし、ホストへ転送 (割り込み発生)
 * 戻り値 -1:終了(セクタ終了処理へ)  1:次へ
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	*/ 	
static	int	e_phase_read_send( void )
{
  int ret;

  if( fdc.TC && fdc.data_ptr ){		/* TC信号あり (1回以上、割込発生済) */

    fdc.limit = 0;
    ICOUNT( (fdc.counter + 2) * CLOCK_BYTE() );
    REPEAT();
    ret = -1;

  }else{				/* TC信号無しなら、割込発生 */
    fdc_occur_interrupt();
    fdc.status = (fdc.status&0x0f) | FDC_BUSY | NON_DMA | DATA_IO | REQ_MASTER;
    fdc.read   = data_buf[ fdc.data_ptr ++ ];
    fdc.limit  = CLOCK_BYTE();
    ICOUNT( -1 );
    ret = 1;
  }

  return ret;
}



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * ホストの引き取り確認 (割り込みクリア)
 * 戻り値 -1:終了(セクタ終了処理へ) 0:今のまま  1:次へ(転送orセクタ終了処理)
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	*/ 	
static	int	e_phase_read_recv( int interval )
{
  int ret;

  fdc.limit -= interval;
  if( fdc.limit < 0 ){			/* 一定時間経過でオーバーラン */
    /* fdc.st1 |= ST1_OR; */
    fdc.limit = 0;
    if( verbose_fdc )
      if( fdc_wait ) printf("FDC %s : Over Run\n", cmd_name[fdc.command] );
  }

  if( fdc.TC ){				/* TC信号ありの場合 */

    fdc_cancel_interrupt();
    fdc.limit = 0;
    ICOUNT( (fdc.counter + 2) * CLOCK_BYTE() );
    REPEAT();
    ret = -1;

  }else{				/* データが読みだされた場合 */

    if( !( fdc.status & REQ_MASTER ) ){
      fdc_cancel_interrupt();
      -- fdc.counter;
      ICOUNT( fdc.limit );
      REPEAT();
      if( fdc.counter==0 ){ fdc.limit = 2 * CLOCK_BYTE(); }
      else                { fdc.limit = 0;                }
      ret = 1;

    }else{				/* まだ、読まれていない場合 */
      ICOUNT( -1 );
      ret = 0;
    }
  }
  return ret;
}



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * 1セクタの終了(全転送完了ないしTC受信)。CRCエラー、TCを確認する
 * 戻り値 -1:終了(E-PHASE完) 0:今のまま  1:次へ(マルチセクタ)
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	*/ 	
static	int	e_phase_read_end( int interval )
{
  int ret;

  fdc.limit -= interval;
  if( fdc.limit < 0 ){
    fdc.limit = 0;
  }

  if( fdc.command != READ_DIAGNOSTIC &&
      fdc.st0 & ST0_IC ){		/* ID/DATA CRCエラーありの場合	*/

    logfdc("-CRC\n");
    ICOUNT( fdc.limit );
    REPEAT();
    fdc.limit = 0;
    ret = -1;							/* E-PHASE完 */

  }else{				/* CRCは正常だった場合	*/

    if( fdc.TC ){			    /* TC信号があった場合 */

      if( fdc.ddam_not_skipped ){		/* (D)DAM不一致なのにスキップ*/
	logfdc("TC(Noskip)\n");			/* ぜずなら、今のセクタのまま*/

      }else{					/* それ以外 (かREAD DIAG)なら*/
	fdc_next_chrn(TRUE);			/* 次セクタを指す            */
	logfdc("TC\n");
      }
      ICOUNT( fdc.limit );
      REPEAT();
      fdc.limit = 0;
      ret = -1;							/* E-PHASE完 */

    }else{				    /* TC信号がない場合 */

      if( fdc.limit <= 0 ){		    /* 2バイトタイム経過 */

	if( fdc.ddam_not_skipped ){		/* (D)DAM不一致なのにスキップ*/
	  fdc.st0 |= ST0_IC_AT;			/* せずなら、異常終了        */
	  fdc.st1 |= ST1_EN;
	  logfdc("MT-Noskip\n");
	  ret = -1;						/* E-PHASE完 */

	}else{					/* それ以外 (かREAD DIAG)なら*/

	  if( fdc_next_chrn(FALSE)==0 ){		/* 次セクタに進む   */
	    logfdc("MT\n");
	    ret = 1;
	  }else{					/* 既に EOT だった  */
	    fdc.st0 |= ST0_IC_AT;
	    fdc.st1 |= ST1_EN;
	    logfdc("MT-EOT\n");
	    ret = -1;
	  }
	}
	ICOUNT( 0 );
	REPEAT();

      }else{				    /* 2バイトタイム経過を待つ */
	ICOUNT( 10 );					/* 適当な小さな時間 */
	ret = 0;
      }
    }
  }

  if( ret != 0 ){
    fdc.carry = fdc.gap3;
/*  printf("<%d><%d>",fdc.wait,fdc.carry);*/
  }
  return ret;
}


/*---------------------------------------------------------------------------
 *	READ/WRITE共通
 *---------------------------------------------------------------------------*/

static	void	e_phase_finish( void )
{
  fdc.TC = FALSE;
  fdc.status = (fdc.status&0x0f) | FDC_BUSY | DATA_IO;
  fdc.phase  = R_PHASE;
  fdc.step   = 0;
  ICOUNT( 0 );
  REPEAT();
}


/*---------------------------------------------------------------------------
 *	WRITE系
 *---------------------------------------------------------------------------*/

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * 書き込み可能かチェック
 * 戻り値 -1:終了(E-PHASE完)  0:今のまま  1:次へ
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	*/ 	
static	int	e_phase_writeid_search( void )
{
  int search, ret;

  search = fdc_check_unit();		/* 書込判定する			*/

  if( search == FALSE ){		/* 判定不能 (ディスク未挿入)	*/

    ret = 0;						/* 何度もやりなおし  */

  }else{				/* 判定結果			*/

    if( fdc.st0 & ST0_IC ){		    /* 書き込み不能 */

      logfdc("-WrPro\n");
      ret = -1;

    }else{				    /* 書き込み可能 */
      fdc.data_ptr = 0;
      if( fdc.sc==0 ){
	fdc.counter = 256 * 4;
	if( verbose_fdc )
	  printf("FDC %s : no sector\n",cmd_name[fdc.command]);
      }else{
	fdc.counter = fdc.sc * 4;
      }
      ret = 1;
    }
    REPEAT();
  }
  return ret;
}



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * 書き込みIDを検索する
 * 戻り値 -1:終了(E-PHASE完)  0:今のまま  1:次へ
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	*/ 	
static	int	e_phase_write_search( void )
{
  int search, ret;

  logfdc("\t\t\tC:%02X H:%02X R:%02X N:%02X  ",fdc.c,fdc.h,fdc.r,fdc.n);


  search = fdc_search_id();		/* ID検索する			*/

  if( search == FALSE ){		/* 検索不能 (ディスク未挿入)	*/

    ret = 0;						/* 何度もやりなおし  */

  }else{				/* 検索可能 (ディスクあり)	*/

    if( fdc.st0 & ST0_IC ){	    	    /* 検索したけど 異常発生         */

      if( fdc.st1 & ST1_NW ){			/* 書き込み不能     */
	logfdc("-WrPro\n");
      }else{					/* IDが見つからない */
	logfdc("-Miss\n");
      }
      ret = -1;

    }else{				    /* 検索して、無事IDを見つけた    */
      fdc.data_ptr = 0;
      if( fdc.n==0 ) fdc.counter = (128>fdc.dtl) ? 128 : fdc.dtl;
      else           fdc.counter = 128 << (fdc.n & 7);
      ret = 1;
    }
    REPEAT();
  }
  return ret;
}



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * ホストへ転送を要求 (割り込み発生)
 * 戻り値 -1:終了(セクタ終了処理へ)  1:次へ
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	*/ 	
static	int	e_phase_write_request( void )
{
  int ret;

  if( fdc.TC && fdc.data_ptr ){		/* TC信号あり (1回以上、割込発生済) */

    fdc.limit = 0;
    if( fdc.command == WRITE_ID ) ICOUNT( 0 ); /* 計算が面倒なので 0 */
    else                          ICOUNT( (fdc.counter + 2) * CLOCK_BYTE() );
    REPEAT();
    ret = -1;

  }else{				/* TC信号無しなら、割込発生 */
    fdc_occur_interrupt();
    fdc.status = (fdc.status&0x0f) | FDC_BUSY | NON_DMA | REQ_MASTER;
    fdc.limit  = CLOCK_BYTE();
    ICOUNT( -1 );
    ret = 1;
  }
  return ret;
}



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * ホストから1バイト引き取り、バッファへ入れる (割り込みクリア)
 * 戻り値 -1:終了(セクタ終了処理へ) 0:今のまま  1:次へ(転送orセクタ終了処理)
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	*/ 	
static	int	e_phase_write_respond( int interval )
{
  int ret;

  fdc.limit -= interval;

  if( fdc.limit<0 ){			/* 一定時間経過でオーバーラン */
    /* fdc.st1 |= ST1_OR; */
    fdc.limit = 0;
    if( verbose_fdc )
      if( fdc_wait ) printf("FDC %s : Over Run\n", cmd_name[fdc.command] );
  }

  if( fdc.TC ){				/* TC信号ありの場合 */

    fdc_cancel_interrupt();
    fdc.limit = 0;
    if( fdc.command == WRITE_ID ) ICOUNT( 0 ); /* 計算が面倒なので 0 */
    else                          ICOUNT( (fdc.counter + 2) * CLOCK_BYTE() );
    REPEAT();
    ret = -1;

  }else{				/* データが書き込まれた場合 */

    if( !( fdc.status & REQ_MASTER ) ){
      fdc_cancel_interrupt();
      data_buf[ fdc.data_ptr ++ ] = fdc.write;
      -- fdc.counter;

      if( fdc.command == WRITE_ID ){
	logfdc("%02X %s",fdc.write,((fdc.counter%4)==0)?"\n":"");

	ICOUNT( fdc.limit + ( ((fdc.counter%4)==0)? CLOCK_SECTOR(fdc.n) :0 ) );
	REPEAT();
	if( fdc.counter==0 ){ fdc.limit = CLOCK_GAP4(fdc.n); }
	else                { fdc.limit = 0; }
      }else{
	ICOUNT( fdc.limit );
	REPEAT();
	if( fdc.counter==0 ){ fdc.limit = 2 * CLOCK_BYTE(); }
	else                { fdc.limit = 0;                }
      }
      ret = 1;

    }else{				/* まだ、書かれていない場合 */
      ICOUNT( -1 );
      ret = 0;
    }
  }
  return ret;
}



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * 1トラック分、イメージファイルに書き出す
 * 戻り値 1:終了(セクタ終了処理へ)
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	*/ 	
static	int	e_phase_writeid_track( void )
{
  int ret;

  fdc.TC = FALSE;			/* TC信号があれば消す */
  if( fdc.counter!=0 ){
    if( verbose_fdc ) printf("FDC %s : CHRN missing\n",cmd_name[fdc.command]);

    while( (fdc.counter%4)!=0 ){		/* 4バイトに満たない */
      data_buf[ fdc.data_ptr ++ ] = 0x00;	/* 部分は00Hで埋める */
      -- fdc.counter;
    }
    fdc.sc = (4*fdc.sc - fdc.counter) /4;	/* その後 SC を更新 */
  }

  fdc_write_id();			/* ライトする			*/
  {
    ICOUNT( fdc.limit );
    REPEAT();
    fdc.limit = 0;
    fdc.carry = 0;
    ret = 1;				
  }
  return ret;
}



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * 1セクタ分、イメージファイルに書き出す
 * 戻り値 1:終了(セクタ終了処理へ)
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	*/ 	
static	int	e_phase_write_sector( void )
{
  int i, ret;

  while( fdc.counter ){			/* セクタサイズに満たなければ	*/
    data_buf[ fdc.data_ptr ++ ] = 0x00;	/*	00Hで埋める		*/
    -- fdc.counter;
  }
  if( fdc.n==0 && fdc.dtl<128 ){
    for( i=0; i<128-fdc.dtl; i++ ){
      data_buf[ fdc.data_ptr ++ ] = 0x00;
    }
  }

  fdc_write_data();			/* ライトする			*/
  {
    ICOUNT( 0 );
    REPEAT();
    ret = 1;
  }
  return ret;
}



/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * 1セクタの終了(書き込み済)。TCを確認する (エラーはありえない)
 * 戻り値 -1:終了(E-PHASE完) 0:今のまま  1:次へ(マルチセクタ)
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -	*/ 	
static	int	e_phase_write_end( int interval )
{
  int ret;

  fdc.limit -= interval;
  if( fdc.limit < 0 ){
    fdc.limit = 0;
  }

  if( fdc.TC ){				    /* TC信号があった場合       */

    fdc_next_chrn(TRUE);			    /* 次セクタを指す   */
    logfdc("TC\n");
    ICOUNT( fdc.limit + CLOCK_BYTE() );
    REPEAT();
    fdc.limit = 0;
    ret = -1;							/* E-PHASE完 */

  }else{				    /* TC信号がない場合         */

    if( fdc.limit <= 0 ){			/* 2バイトタイム経過    */

      if( fdc_next_chrn(FALSE)==0 ){		    /* 次セクタに進む   */
	logfdc("MT\n");
	ret = 1;
      }else{					    /* 既に EOT だった  */
	fdc.st0 |= ST0_IC_AT;
	fdc.st1 |= ST1_EN;
	logfdc("MT-EOT\n");
	ret = -1;
      }
      ICOUNT( 0 );
      REPEAT();

    }else{					/* 2バイトタイム経過待つ*/
      ICOUNT( 10 );				    /* 適当な小さな時間 */
      ret = 0;
    }
  }

  if( ret != 0 ){
    fdc.carry = fdc.gap3;
  }
  return ret;
}







/************************************************************************/
/* FDC の 処理メイン							*/
/*	引数は 前回にこの関数を呼んだ時からの経過時間( 4MHzステートで )	*/
/************************************************************************/

int	fdc_ctrl( int interval )
{
  int	i, w, rest;


  if( fdc.wait < 0 ){			/* 無限待ちだった場合 */
    rest = 0;
    fdc.wait = 0;
    fdc.carry -= interval;
    if( fdc.carry < 0 ) fdc.carry = 0;
  }else{				/* 有限待ちだった場合 */
    if( fdc.wait >= interval ){			/* ウェイト減算 */
      rest = 0;
      fdc.wait -= interval;
    }else{
      rest = interval - fdc.wait;		/* あまり分は rest にセット */
      fdc.wait = 0;
    }
  }

  /********************************** シーク動作 *****************************/

  for( i=0; i<NR_DRIVE; i++ ){				/* ドライブ 0〜1のみ */
    if( fdc.seek_stat[i] == SEEK_STAT_MOVE ){		/*  シーク中なら処理 */
      fdc.seek_wait[i] -= interval;
      if( fdc.seek_wait[i] <= 0 ){
	if( fdc.ncn[i] == fdc.pcn[i] ){				/* シーク完 */
	  fdc.seek_stat[i] = SEEK_STAT_END;
	  fdc.seek_wait[i] = 0;
	}else{							/* まだまだ */
	  /* シーク音 ? */
	  if ((cpu_timing > 0) && (fdc_wait)) {
	    fdc_sound_counter ++;
	    if (fdc_sound_counter >= fdc_sound_skipper) {
	      fdc_sound_counter = 0;
	      /*logfdc("### Seek ###\n");*/
	      xmame_dev_sample_seek();
	    }
	  }
	  if( fdc.pcn[i] < fdc.ncn[i] ) fdc.pcn[i] ++;
	  else                          fdc.pcn[i] --;
	  fdc.seek_wait[i] = fdc.srt_clk;
	}
      }
    }
  }

  /*************************** ヘッドアンロード動作 **************************/
#ifdef	WAIT_FOR_HEADLOAD
  if( fdc.command == WAIT ||			/* リード/ライト以外         */
      fdc.command >= SEEK ){
    for( i=0; i<MAX_DRIVE; i++ ){
      if( fdc.hl_stat[i] ){				/* ヘッドロード中なら*/
	fdc.hl_wait[i] += interval;				/* 時間を加算*/
	if( fdc.hl_wait[i] >= fdc.hut_clk ){
	  fdc.hl_stat[i] = FALSE;
	  /* アンロード音 ? */
	  if ((cpu_timing > 0) && (fdc_wait)) {
	    /*logfdc("### Head Up ###\n");*/
	    xmame_dev_sample_headup();
	  }
	}
      }
    }
  }
#endif

  /******* FDC 処理。ウェイトなしなら(条件に次第で)処理を無限に繰り返す ******/

  do {

    if( fdc.command == WAIT ){	/*========================== コマンド待ち ===*/

      if( ( fdc.status & REQ_MASTER )==0 ){	/* コマンド受信した場合	*/
	switch( fdc.write & 0x1f ){
	case 0x06:	fdc.command = READ_DATA;		break;
	case 0x0c:	fdc.command = READ_DELETED_DATA;	break;
	case 0x0a:	fdc.command = READ_ID;			break;
	case 0x0d:	fdc.command = WRITE_ID;			break;
	case 0x05:	fdc.command = WRITE_DATA;		break;
	case 0x09:	fdc.command = WRITE_DELETED_DATA;	break;
	case 0x02:	fdc.command = READ_DIAGNOSTIC;		break;
	case 0x11:	fdc.command = SCAN_EQUAL;		break;
	case 0x19:	fdc.command = SCAN_LOW_OR_EQUAL;	break;
	case 0x1d:	fdc.command = SCAN_HIGH_OR_EQUAL;	break;
	case 0x0f:	fdc.command = SEEK;			break;
	case 0x07:	fdc.command = RECALIBRATE;		break;
	case 0x08:	fdc.command = SENSE_INT_STATUS;		break;
	case 0x04:	fdc.command = SENSE_DEVICE_STATUS;	break;
	case 0x03:	fdc.command = SPECIFY;			break;
	default:	fdc.command = INVALID;			break;
	}
	fdc.phase = C_PHASE;
	fdc.step  = 0;
	fdc.TC    = FALSE;
	ICOUNT( 0 );					/* 処理繰り返し */

      }else {					/* コマンドない場合	*/

	for( i=0; i<MAX_DRIVE; i++ ){			/* シーク完了判定 */
	  if( fdc.seek_stat[i] == SEEK_STAT_END ){
	    fdc_occur_interrupt();
	    fdc.seek_stat[i] = SEEK_STAT_INTR;
	    fdc.seek_wait[i] = 0;
	    if( i<NR_DRIVE && disk_not_exist(i)==FALSE &&
		sec_buf.drv == -1 ){
	      disk_now_track( i, fdc.ncn[ i ]*2 + fdc.hd );
	      fdc.carry = 0;
	    }
	    break;
	  }
	}
	ICOUNT( -1 );
      }
      break;

    }else{			/*========================== コマンド処理 ===*/

      if( fdc.wait ) break;

      switch( fdc.phase ){
      case C_PHASE:		    /* = = = = = = = C-PHASE = = = = = = = = */
	c_phase();
	break;

      case E_PHASE:		    /* = = = = = = = E-PHASE = = = = = = = = */
	switch( fdc.command ){

	case READ_DATA:			/* ---------------------- READ DATA -*/
	case READ_DELETED_DATA:		/* -------------- READ DELETED DATA -*/
	case READ_DIAGNOSTIC:		/* ---------------- READ DIAGNOSTIC -*/
	case READ_ID:			/* ------------------------ READ ID -*/
	  switch( fdc.step ){
	  case 0:				/* ID検索・イメージリード - -*/
	    switch( e_phase_read_search() ){
	    case -1:	fdc.step = 4;	break;		/* エラー発生   */
	    case  1:	fdc.step = 1;	break;		/* リード処理へ */
	    default:			break;		/* 待ち         */
	    }	/* READ ID は case 1: はありえない */
	    break;

	  case 1:				/* 1バイト送信  - - - - - - -*/
	    switch( e_phase_read_send() ){
	    case -1:	fdc.step = 3;	break;		/* TC あり    */
	    case  1:	fdc.step = 2;	break;		/* 転送処理へ */
	    default:			break;		/* 待ち       */
	    }
	    break;

	  case 2:				/* ホスト引取り待ち - - - - -*/
	    switch( e_phase_read_recv( interval ) ){
	    case -1:	fdc.step = 3;	break;		/* TC あり    */
	    case  1:
	      if( fdc.counter==0 ) fdc.step = 3;	/* TC 待ちへ  */
	      else                 fdc.step = 1;	/* 次の転送へ */
	      break;
	    default:			break;		/* 待ち       */
	    }
	    break;

	  case 3:				/* 1セクタ終了  - - - - - - -*/
	    switch( e_phase_read_end( interval ) ){
	    case -1:	fdc.step = 4;	break;		/* 終了       */
	    case  1:	fdc.step = 0;	break;		/* 次セクタへ */
	    default:			break;		/* 待ち       */
	    }
	    break;

	  case 4:				/* E-PHASE終了  - - - - - - -*/
	    e_phase_finish();
	    break;
	  }
	  break;


	case WRITE_ID:			/* ----------------------- WRITE ID -*/
	  switch( fdc.step ){
	  case 0:				/* 書込確認 - - - - - - - - -*/
	    switch( e_phase_writeid_search() ){
	    case -1:	fdc.step = 4;	break;		/* エラー発生   */
	    case  1:	fdc.step = 1;	break;		/* ライト処理へ */
	    default:			break;		/* 待ち         */
	    }
	    break;

	  case 1:				/* ホストへ要求 - - - - - - -*/
	    switch( e_phase_write_request() ){
	    case -1:	fdc.step = 3;	break;		/* TC あり    */
	    case  1:	fdc.step = 2;	break;		/* 転送処理へ */
	    default:			break;		/* 待ち       */
	    }
	    break;

	  case 2:				/* 1バイト受信  - - - - - - -*/
	    switch( e_phase_write_respond( interval ) ){
	    case -1:	fdc.step = 3;	break;		/* TC あり    */
	    case  1:
	      if( fdc.counter==0 ) fdc.step = 3;	/* 書き込みへ */
	      else                 fdc.step = 1;	/* 次の転送へ */
	      break;
	    default:			break;		/* 待ち       */
	    }
	    break;

	  case 3:				/* イメージ書き込み - - - - -*/
	    e_phase_writeid_track();
	    fdc.step = 4;	/* FALLTHROUGH */
	  case 4:				/* E-PHASE終了  - - - - - - -*/
	    e_phase_finish();
	    break;
	  }
	  break;


	case WRITE_DATA:		/* --------------------- WRITE DATA -*/
	case WRITE_DELETED_DATA:	/* ------------- WRITE DELETED DATA -*/
	  switch( fdc.step ){
	  case 0:				/* ID検索 - - - - - - - - - -*/
	    switch( e_phase_write_search() ){
	    case -1:	fdc.step = 5;	break;		/* エラー発生   */
	    case  1:	fdc.step = 1;	break;		/* ライト処理へ */
	    default:			break;		/* 待ち         */
	    }
	    break;

	  case 1:				/* ホストへ要求 - - - - - - -*/
	    switch( e_phase_write_request() ){
	    case -1:	fdc.step = 3;	break;		/* TC あり    */
	    case  1:	fdc.step = 2;	break;		/* 転送処理へ */
	    default:			break;		/* 待ち       */
	    }
	    break;

	  case 2:				/* 1バイト受信  - - - - - - -*/
	    switch( e_phase_write_respond( interval ) ){
	    case -1:	fdc.step = 3;	break;		/* TC あり    */
	    case  1:
	      if( fdc.counter==0 ) fdc.step = 3;	/* TC 待ちへ  */
	      else                 fdc.step = 1;	/* 次の転送へ */
	      break;
	    default:			break;		/* 待ち       */
	    }
	    break;

	  case 3:				/* イメージ書き込み - - - - -*/
	    e_phase_write_sector();
	    fdc.step = 4;	/* FALLTHROUGH */
	  case 4:				/* 1セクタ終了  - - - - - - -*/
	    switch( e_phase_write_end( interval ) ){
	    case -1:	fdc.step = 5;	break;		/* 終了       */
	    case  1:	fdc.step = 0;	break;		/* 次セクタへ */
	    default:			break;		/* 待ち       */
	    }
	    break;

	  case 5:				/* E-PHASE終了  - - - - - - -*/
	    e_phase_finish();
	    break;
	  }
	  break;


	case SEEK:			/* --------------------------- SEEK -*/
	case RECALIBRATE:		/* -------------------- RECALIBRATE -*/
	  e_phase_seek();
	  break;


	case SENSE_INT_STATUS:		/* --------------- SENSE_INT_STATUS -*/
	case SENSE_DEVICE_STATUS:	/* ------------ SENSE_DEVICE_STATUS -*/
	case SPECIFY:			/* ------------------------ SPECIFY -*/
	case INVALID:			/* ------------------------ Invalid -*/
	  /* E-PHASEは無い */
	  break;


	default:			/* ----------------------------------*/
	  /* 未対応コマンド……… */
	  break;
	}
	break;


      case R_PHASE:		    /* = = = = = = = R-PHASE = = = = = = = = */
	r_phase();
	break;
      }
    }


    if( rest > 0 ){			/* ウェイト減算の際のあまり分を消費 */
      if( fdc.wait < 0 ){
	fdc.carry -= rest;
	if( fdc.carry < 0 ) fdc.carry = 0;
      }else{
	fdc.wait -= rest;
	if( fdc.wait < 0 ) fdc.wait = 0;
      }
      rest   = 0;
    }
    interval = 0;

  } while( fdc.wait==0 );


  /************ 次回 FDC処理までの待ち時間を算出 (-1で無限に待つ) ************/
  w = -1;

  if( fdc.command == WAIT ){		/* 待機中のみ、シーク完了時間を確認 */
    for( i=0; i<MAX_DRIVE; i++ ){
      if( fdc.seek_stat[i] == SEEK_STAT_MOVE &&
	  fdc.seek_wait[i] >  0 ){
	if( w == -1 ) w = fdc.seek_wait[i];
	else if( w > fdc.seek_wait[i] ) w = fdc.seek_wait[i];
      }
    }
  }
  /* w はシーク完了までの最短クロック数 または -1 がセットされている */

  if( fdc_wait == FALSE ||		/* ウエイトなし または          */
      fdc.wait < 0 ){			/* ウェイトありで無限待ちの場合 */

    ;/* w (シーク完了ないし無限) まで待つ */

  }else{				/* 待ち時間が設定済みの場合     */
    if( w < 0 ) w = fdc.wait;
    else        w = MIN( w, fdc.wait );
  }

  return w;					
}



/************************************************************************/
/* ドライブの状態を返す関数						*/
/************************************************************************/
int	get_drive_ready( int drv )
{
  if( fdc.command==WAIT ) return 1;
  else{
    if( fdc.us==drv ) return 0;
    else              return 1;
  }
}



/************************************************************************/
/* セクタ間を埋める                                                     */
/************************************************************************/
/*
 * READ DIAG のセクタ間のデータを埋める処理は、peach氏により提供されました。
 */
#define IF_WITHIN_DATA_BUF_SIZE(s)	\
if (s >= DATA_BUF_SIZE) {		\
    printf("FDC : Buffer over flow\n");	\
    fflush(stdout);			\
    return(-1);				\
} else

#define RET_GAP_ERR(cond) if (cond != TRUE) return(-1);

/*
 * GAP3 の計算
 */
INLINE
int calc_gap3_size(int n, Uchar fdc_mf)
{
    int gap3_size;

    /* GAP3 のサイズは決まっているわけではないのであくまで標準的な値 */
    switch (n) {
    case 1:	gap3_size = 27;	break;
    case 2:	gap3_size = 42;	break;
    case 3:	gap3_size = 58;	break;
    default:	gap3_size = 58; /* その他は分かんね */
    }
    if (fdc_mf) gap3_size *= 2;	/* 倍密度 */
    return(gap3_size);
}

INLINE
int input_safely_data(int ptr, int *ad, int data, int size)
{
    if (size == 0) return(TRUE);

    if (ptr + *ad + size < DATA_BUF_SIZE) {
	memset(&data_buf[ptr + *ad], data, size);
	*ad += size;
	return(TRUE);
    } else {
	printf("FDC : Buffer overflow\n");
	fflush(stdout);
	return(FALSE);
    }
}

static int fill_sector_gap(int ptr, int drv, Uchar fdc_mf)
{
    int   sync_size, am_size;
    int   gap0_size, gap1_size, gap2_size, gap3_size, gap4_size;
    int   track_size;
    Uchar gap;
    Uchar undel;
    int   size;
    int   tmp_size;
    
    if (fdc_mf) {
	/* 倍密度 */
	track_size = 6250;
	gap0_size = 80;
	sync_size = 12;
	am_size   = 3 + 1;
	gap1_size = 50;
	gap2_size = 22;
	gap = 0x4e;
	/*if      (sec_buf.sec_nr <= 8)  gap4_size = 654;
	  else if (sec_buf.sec_nr <= 15) gap4_size = 400;
	  else                           gap4_size = 598;*/
    } else {
	/* 単密度 */
	track_size = 3100;
	gap0_size = 40;
	sync_size = 6;
	am_size = 1;
	gap1_size = 26;
	gap2_size = 11;
	gap = 0xff;
	/*if      (sec_buf.sec_nr <= 8)  gap4_size = 311;
	  else if (sec_buf.sec_nr <= 15) gap4_size = 170;
	  else                           gap4_size = 247;*/
    }
    gap3_size = calc_gap3_size(sec_buf.n, fdc_mf);

    size = 0;
    /* DATA フィールドの最後尾 */
    size += 2;			/* DATA CRC */

    /* GAP3 */
    RET_GAP_ERR(input_safely_data(ptr, &size, gap, gap3_size));

    /* 最終セクタならプリアンブル + ポストアンブル */
    if (drive[drv].sec + 1 >= drive[drv].sec_nr) {
	/* ポストアンブル (GAP4) のサイズを調べる */
	tmp_size = gap0_size + sync_size + am_size + gap1_size;
	/* 全てのセクタで GAP をカウント */
	do {
	    disk_next_sec(drv);
	    tmp_size += sync_size + am_size + 4 + 2; /* ID フィールド */
	    tmp_size += gap2_size;
	    tmp_size += sync_size + am_size + sec_buf.size + 2;
	    tmp_size += calc_gap3_size(sec_buf.n, fdc_mf);
	} while (drive[drv].sec + 1 < drive[drv].sec_nr);
	gap4_size = track_size - tmp_size;
	if (gap4_size < 0) {
	    printf("Abnormal sector\n");
	    return(-1);
	}

	
	RET_GAP_ERR(input_safely_data(ptr, &size, gap, gap4_size));
	RET_GAP_ERR(input_safely_data(ptr, &size, 0, sync_size));
	RET_GAP_ERR(input_safely_data(ptr, &size, 0xc2, am_size - 1));
	RET_GAP_ERR(input_safely_data(ptr, &size, 0xfc, 1));
	RET_GAP_ERR(input_safely_data(ptr, &size, gap, gap1_size));
    }
  
    /* 次のセクタ */
    disk_next_sec(drv);

    /* ID フィールド */
    RET_GAP_ERR(input_safely_data(ptr, &size, 0, sync_size));
    RET_GAP_ERR(input_safely_data(ptr, &size, 0xa1, am_size - 1));
    RET_GAP_ERR(input_safely_data(ptr, &size, 0xfe, 1));
  
    RET_GAP_ERR(input_safely_data(ptr, &size, sec_buf.c, 1));
    RET_GAP_ERR(input_safely_data(ptr, &size, sec_buf.h, 1));
    RET_GAP_ERR(input_safely_data(ptr, &size, sec_buf.r, 1));
    RET_GAP_ERR(input_safely_data(ptr, &size, sec_buf.n, 1));
    size += 2;			/* ID CRC */

    /* GAP2 */
    RET_GAP_ERR(input_safely_data(ptr, &size, gap, gap2_size));

    /* DATA フィールド */
    RET_GAP_ERR(input_safely_data(ptr, &size, 0, sync_size));
    RET_GAP_ERR(input_safely_data(ptr, &size, 0xa1, am_size - 1));

    if (sec_buf.deleted == DISK_DELETED_TRUE) undel = 0xf8;
    else undel = 0xfb;
    RET_GAP_ERR(input_safely_data(ptr, &size, undel, 1));
  
    return(size);
}



/***********************************************************************
 * ステートロード／ステートセーブ
 ************************************************************************/

#define	SID		"FDC "
#define	SID_DATA	"FDC0"
#define	SID2		"FDC2"

static	T_SUSPEND_W	suspend_fdc_work[]=
{
  { TYPE_STR,	&file_disk[0][0],	},
  { TYPE_STR,	&file_disk[1][0],	},
  { TYPE_INT,	&image_disk[0],		},
  { TYPE_INT,	&image_disk[1],		},
  { TYPE_INT,	&readonly_disk[0],	},
  { TYPE_INT,	&readonly_disk[1],	},

  { TYPE_INT,	&disk_exchange,		},
  { TYPE_INT,	&disk_ex_drv,		},

  { TYPE_INT,	&FDC_flag,		},
  { TYPE_INT,	&fdc_wait,		},

  { TYPE_INT,	&fdc.command,		},
  { TYPE_INT,	&fdc.phase,		},
  { TYPE_INT,	&fdc.step,		},
  { TYPE_INT,	&fdc.counter,		},
  { TYPE_INT,	&fdc.data_ptr,		},

  { TYPE_INT,	&fdc.limit,		},
  { TYPE_INT,	&fdc.wait,		},
  { TYPE_INT,	&fdc.carry,		},
  { TYPE_INT,	&fdc.gap3,		},

  { TYPE_INT,	&fdc.seek_stat[0],	},
  { TYPE_INT,	&fdc.seek_stat[1],	},
  { TYPE_INT,	&fdc.seek_stat[2],	},
  { TYPE_INT,	&fdc.seek_stat[3],	},
  { TYPE_INT,	&fdc.seek_wait[0],	},
  { TYPE_INT,	&fdc.seek_wait[1],	},
  { TYPE_INT,	&fdc.seek_wait[2],	},
  { TYPE_INT,	&fdc.seek_wait[3],	},

  { TYPE_INT,	&fdc.srt_clk,		},
  { TYPE_INT,	&fdc.hut_clk,		},
  { TYPE_INT,	&fdc.hlt_clk,		},
  { TYPE_INT,	&fdc.hl_stat[0],	},
  { TYPE_INT,	&fdc.hl_stat[1],	},
  { TYPE_INT,	&fdc.hl_stat[2],	},
  { TYPE_INT,	&fdc.hl_stat[3],	},
  { TYPE_INT,	&fdc.hl_wait[0],	},
  { TYPE_INT,	&fdc.hl_wait[1],	},
  { TYPE_INT,	&fdc.hl_wait[2],	},
  { TYPE_INT,	&fdc.hl_wait[3],	},

  { TYPE_INT,	&fdc.ddam_not_skipped,	},

  { TYPE_BYTE,	&fdc.status,		},
  { TYPE_BYTE,	&fdc.read,		},
  { TYPE_BYTE,	&fdc.write,		},
  { TYPE_BYTE,	&fdc.TC,		},

  { TYPE_CHAR,	&fdc.sk,		},
  { TYPE_CHAR,	&fdc.mf,		},
  { TYPE_CHAR,	&fdc.mt,		},
  { TYPE_CHAR,	&fdc.us,		},
  { TYPE_CHAR,	&fdc.hd,		},
  { TYPE_CHAR,	&fdc.c,			},
  { TYPE_CHAR,	&fdc.h,			},
  { TYPE_CHAR,	&fdc.r,			},
  { TYPE_CHAR,	&fdc.n,			},
  { TYPE_CHAR,	&fdc.eot,		},
  { TYPE_CHAR,	&fdc.gpl,		},
  { TYPE_CHAR,	&fdc.dtl,		},
  { TYPE_CHAR,	&fdc.d,			},
  { TYPE_CHAR,	&fdc.sc,		},
  { TYPE_CHAR,	&fdc.stp,		},
  { TYPE_CHAR,	&fdc.ncn[0],		},
  { TYPE_CHAR,	&fdc.ncn[1],		},
  { TYPE_CHAR,	&fdc.ncn[2],		},
  { TYPE_CHAR,	&fdc.ncn[3],		},
  { TYPE_CHAR,	&fdc.pcn[0],		},
  { TYPE_CHAR,	&fdc.pcn[1],		},
  { TYPE_CHAR,	&fdc.pcn[2],		},
  { TYPE_CHAR,	&fdc.pcn[3],		},
  { TYPE_CHAR,	&fdc.st0,		},
  { TYPE_CHAR,	&fdc.st1,		},
  { TYPE_CHAR,	&fdc.st2,		},
  { TYPE_CHAR,	&fdc.st3,		},

  { TYPE_END,	0			},
};

static	T_SUSPEND_W	suspend_fdc_work2[]=
{
  { TYPE_CHAR,	&fdc.c0			},
  { TYPE_CHAR,	&fdc.c1			},
  { TYPE_CHAR,	&fdc.cn			},
  { TYPE_CHAR,	&fdc.s0			},
  { TYPE_CHAR,	&fdc.s1			},
  { TYPE_CHAR,	&fdc.r0			},
  { TYPE_CHAR,	&fdc.r1			},
  { TYPE_CHAR,	&fdc.intr_unit		},

  { TYPE_END,	0			},
};


int	statesave_fdc( void )
{
  image_disk[0] = drive[0].selected_image;
  image_disk[1] = drive[1].selected_image;

  if( statesave_table( SID, suspend_fdc_work ) != STATE_OK ) return FALSE;

  /* データバッファ */
  if( statesave_block( SID_DATA, data_buf, DATA_BUF_SIZE ) != STATE_OK )
								return FALSE;

  if( statesave_table( SID2, suspend_fdc_work2 ) != STATE_OK ) return FALSE;

  return TRUE;
}

int	stateload_fdc( void )
{
  if( stateload_table( SID, suspend_fdc_work ) != STATE_OK ) return FALSE;

  /* データバッファ */
  if( stateload_block( SID_DATA, data_buf, DATA_BUF_SIZE ) != STATE_OK )
								return FALSE;

  if( stateload_table( SID2, suspend_fdc_work2 ) != STATE_OK ){

    /* 旧バージョンなら、みのがす */

    printf( "stateload : Statefile is old. (ver 0.6.0 or 1?)\n" );

    /* といいたいのだが、等価になるような置き換えは不可能だ。困った */
    /* 仕方ないので、必ず異常終了となるようなパラメータに置き換え   */

    fdc.c0 = 0xff;	/* invalid  */
    fdc.c1 = 0xff;	/* 3rd unit */
    fdc.cn = 0xff;	/* 255 Cyl. */
    fdc.s0 = 0xda;	/* default  */
    fdc.s1 = 0x19;	/* default  */
    fdc.r0 = ST0_IC_IC;	/* invalid  */
    fdc.r1 = 0xff;	/* 255 Cyl. */
    fdc.intr_unit = 4;	/* non exist unit */

    return TRUE;

  }

  return TRUE;
}






/* デバッグ用の関数 */
void monitor_fdc(void)
{
  printf("com = %d phs = %d  step = %d\n",fdc.command,fdc.phase,fdc.step);
  printf("FDC flag = %d\n",FDC_flag);
#if 0
  printf("\n");
  printf("fp       = %d , %d\n", drive[0].fp,       drive[1].fp      );
  printf("track    = %d , %d\n", drive[0].track,    drive[1].track   );
  printf("sec_nr   = %d , %d\n", drive[0].sec_nr,   drive[1].sec_nr  );
  printf("sec      = %d , %d\n", drive[0].sec,      drive[1].sec       );
  printf("sec_pos  = %d , %d\n", drive[0].sec_pos,  drive[1].sec_pos   );
  printf("track_top= %d , %d\n", drive[0].track_top,drive[1].track_top );
  printf("disk_top = %d , %d\n", drive[0].disk_top, drive[1].sec     );
  printf("type     = %d , %d\n", drive[0].type,     drive[1].type );
  printf("protect  = %d , %d\n", drive[0].protect,  drive[1].protect     );

  printf("\n");
  {
    int	i,j,k;
    for(k=0;k<(128<<(fdc.n&7))/256;k++){
      for(i=0;i<16;i++){
	for(j=0;j<16;j++){
	  printf("%02X ",data_buf[k*256+i*16+j]);
	}
	printf("\n");
      }
    }
  }
  printf("\n");
#endif
}
