/************************************************************************/
/*									*/
/* CRTC と DMAC の処理							*/
/*									*/
/************************************************************************/

#include "quasi88.h"
#include "crtcdmac.h"
#include "memory.h"

#include "screen.h"
#include "suspend.h"


/*======================================================================
 *
 *======================================================================*/

		/* CRTC側から見たアトリビュート	*/

#define MONO_SECRET	0x01
#define MONO_BLINK	0x02
#define MONO_REVERSE	0x04
#define MONO_UPPER	0x10
#define MONO_UNDER	0x20
#define MONO_GRAPH	0x80
#define COLOR_SWITCH	0x08
#define COLOR_GRAPH	0x10
#define COLOR_B		0x20
#define COLOR_R		0x40
#define COLOR_G		0x80

		/* 内部表現で使用するアトリビュート */

#define ATTR_REVERSE	0x01			/* 反転			*/
#define ATTR_SECRET	0x02			/* 表示/非表示		*/
#define ATTR_UPPER	0x04			/* アッパーライン	*/
#define ATTR_LOWER	0x08			/* アンダーライン	*/
#define ATTR_GRAPH	0x10			/* グラフィックモード	*/
#define ATTR_B		0x20			/* 色 Blue		*/
#define ATTR_R		0x40			/* 色 Reg		*/
#define ATTR_G		0x80			/* 色 Green		*/

#define MONO_MASK	0x0f
#define COLOR_MASK	0xe0

/*======================================================================*/

int		text_display = TEXT_ENABLE;	/* テキスト表示フラグ	*/

int		blink_cycle;		/* 点滅の周期	8/16/24/32	*/
int		blink_counter = 0;	/* 点滅制御カウンタ		*/

int		dma_wait_count = 0;	/* DMAで消費するサイクル数	*/


static	int	crtc_command;
static	int	crtc_param_num;

static	byte	crtc_status;
static	byte	crtc_light_pen[2];
static	byte	crtc_load_cursor_position;


	int	crtc_active;		/* CRTCの状態 0:CRTC作動 1:CRTC停止 */
	int	crtc_intr_mask;		/* CRTCの割込マスク ==3 で表示	    */
	int	crtc_cursor[2];		/* カーソル位置 非表示の時は(-1,-1) */
	byte	crtc_format[5];		/* CRTC 期化時のフォーマット	    */


	int	crtc_reverse_display;	/* 真…反転表示 / 偽…通常表示	*/

	int	crtc_skip_line;		/* 真…1行飛ばし表示 / 偽…通常 */
	int	crtc_cursor_style;	/* ブロック / アンダライン	*/
	int	crtc_cursor_blink;	/* 真…点滅する 偽…点滅しない	*/
	int	crtc_attr_non_separate;	/* 真…VRAM、ATTR が交互に並ぶ	*/
	int	crtc_attr_color;	/* 真…カラー 偽…白黒		*/
	int	crtc_attr_non_special;	/* 偽…行の終りに ATTR が並ぶ	*/

	int	CRTC_SZ_LINES	   =20;	/* 表示する桁数 (20/25)		*/
#define		CRTC_SZ_COLUMNS	   (80)	/* 表示する行数 (80固定)	*/

	int	crtc_sz_lines      =20;	/* 桁数 (20〜25)		*/
	int	crtc_sz_columns    =80;	/* 行数 (2〜80)			*/
	int	crtc_sz_attrs      =20;	/* 属性量 (1〜20)		*/
	int	crtc_byte_per_line=120;	/* 1行あたりのメモリ バイト数	*/
	int	crtc_font_height   =10;	/* フォントの高さ ドット数(8/10)*/



/******************************************************************************

			←─────── crtc_byte_per_line  ───────→
			←──   crtc_sz_columns  ──→ ←  crtc_sz_attrs →
			+-------------------------------+-------------------+
		      ↑|				|↑		    |
		      │|	+--+ ↑			|│		    |
		      │|	|  | crtc_font_height	|│		    |
			|	+--+ ↓			|		    |
	   CRTC_SZ_LINES|				|crtc_sz_lines	    |
			|				|		    |
		      │|				|│		    |
		      │|				|│		    |
		      ↓|				|↓		    |
			+-------------------------------+-------------------+
			←──   CRTC_SZ_COLUMNS  ──→ 

	crtc_sz_columns		桁数	2〜80
	crtc_sz_attrs		属性量	1〜20
	crtc_byte_per_line	1行あたりのメモリ量	columns + attrs*2
	crtc_sz_lines		行数	20〜25
	crtc_font_height	フォントの高さドット量	8/10
	CRTC_SZ_COLUMNS		表示する桁数	80
	CRTC_SZ_LINES		表示する行数	20/25

******************************************************************************/









/* 参考までに……… 						*/
/*	SORCERIAN          … 1行飛ばし指定			*/
/*	Marchen Veil       … アトリビュートなしモード		*/
/*	Xanadu II (E disk) …             〃			*/
/*	Wizardry V         … ノントランスペアレント白黒モード	*/


enum{
  CRTC_RESET		= 0,
  CRTC_STOP_DISPLAY	= 0,
  CRTC_START_DISPLAY,
  CRTC_SET_INTERRUPT_MASK,
  CRTC_READ_LIGHT_PEN,
  CRTC_LOAD_CURSOR_POSITION,
  CRTC_RESET_INTERRUPT,
  CRTC_RESET_COUNTERS,
  CRTC_READ_STATUS,
  EndofCRTC
};
#define CRTC_STATUS_VE	(0x10)		/* 画面表示有効		*/
#define CRTC_STATUS_U	(0x08)		/* DMAアンダーラン	*/
#define CRTC_STATUS_N	(0x04)		/* 特殊制御文字割込発生 */
#define CRTC_STATUS_E	(0x02)		/* 表示終了割込発生	*/
#define CRTC_STATUS_LP	(0x01)		/* ライトペン入力 	*/


/****************************************************************/
/* CRTCへ同期信号を送る (OUT 40H,A ... bit3)			*/
/*	特にエミュレートの必要なし。。。。。と思う。		*/
/****************************************************************/
#ifdef	SUPPORT_CRTC_SEND_SYNC_SIGNAL
void	crtc_send_sync_signal( int flag )
{
}
#endif




/****************************************************************/
/*    CRTC エミュレーション					*/
/****************************************************************/

/*-------- 初期化 --------*/

void	crtc_init( void )
{
  crtc_out_command( CRTC_RESET << 5 );
  crtc_out_parameter( 0xce );
  crtc_out_parameter( 0x98 );
  crtc_out_parameter( 0x6f );
  crtc_out_parameter( 0x58 );
  crtc_out_parameter( 0x53 );

  crtc_out_command( CRTC_LOAD_CURSOR_POSITION << 5 );
  crtc_out_parameter( 0 );
  crtc_out_parameter( 0 );
}

/*-------- コマンド入力時 --------*/

void	crtc_out_command( byte data )
{
  crtc_command = data >> 5;
  crtc_param_num = 0;

  switch( crtc_command ){

  case CRTC_RESET:					/* リセット */
    crtc_status &= ~( CRTC_STATUS_VE | CRTC_STATUS_N | CRTC_STATUS_E );
    crtc_active = FALSE;
    set_text_display();
    screen_set_dirty_all();
    break;

  case CRTC_START_DISPLAY:				/* 表示開始 */
    crtc_reverse_display = data & 0x01;
    crtc_status |= CRTC_STATUS_VE;
    crtc_status &= ~( CRTC_STATUS_U );
    crtc_active = TRUE;
    set_text_display();
    screen_set_dirty_palette();
    break;

  case CRTC_SET_INTERRUPT_MASK:
    crtc_intr_mask = data & 0x03;
    set_text_display();
    screen_set_dirty_all();
    break;

  case CRTC_READ_LIGHT_PEN:
    crtc_status &= ~( CRTC_STATUS_LP );
    break;

  case CRTC_LOAD_CURSOR_POSITION:			/* カーソル設定 */
    crtc_load_cursor_position = data & 0x01;
    crtc_cursor[ 0 ] = -1;
    crtc_cursor[ 1 ] = -1;
    break;

  case CRTC_RESET_INTERRUPT:
  case CRTC_RESET_COUNTERS:
    crtc_status &= ~( CRTC_STATUS_N | CRTC_STATUS_E );
    break;

  }
}

/*-------- パラメータ入力時 --------*/

void	crtc_out_parameter( byte data )
{
  switch( crtc_command ){
  case CRTC_RESET:
    if( crtc_param_num < 5 ){
      crtc_format[ crtc_param_num++ ] = data;
    }

    crtc_skip_line         = crtc_format[2] & 0x80;		/* bool  */

    crtc_attr_non_separate = crtc_format[4] & 0x80;		/* bool */
    crtc_attr_color        = crtc_format[4] & 0x40;		/* bool */
    crtc_attr_non_special  = crtc_format[4] & 0x20;		/* bool */

    crtc_cursor_style      =(crtc_format[2] & 0x40) ?ATTR_REVERSE :ATTR_LOWER;
    crtc_cursor_blink      = crtc_format[2] & 0x20;		/* bool */
    blink_cycle            =(crtc_format[1]>>6) * 8 +8;		/* 8,16,24,48*/

    crtc_sz_lines          =(crtc_format[1] & 0x3f) +1;		/* 1〜25 */
    if     ( crtc_sz_lines <= 20 ) crtc_sz_lines = 20;
    else if( crtc_sz_lines >= 25 ) crtc_sz_lines = 25;
    else                           crtc_sz_lines = 24;

    crtc_sz_columns        =(crtc_format[0] & 0x7f) +2;		/* 2〜80 */
    if( crtc_sz_columns > 80 ) crtc_sz_columns = 80;

    crtc_sz_attrs          =(crtc_format[4] & 0x1f) +1;		/* 1〜20 */
    if     ( crtc_attr_non_special ) crtc_sz_attrs = 0;
    else if( crtc_sz_attrs > 20 )    crtc_sz_attrs = 20;

    crtc_byte_per_line  = crtc_sz_columns + crtc_sz_attrs * 2;	/*column+attr*/

    crtc_font_height    = (crtc_sz_lines>20) ?  8 : 10;
    CRTC_SZ_LINES	= (crtc_sz_lines>20) ? 25 : 20;

    frameskip_blink_reset();
    break;

  case CRTC_LOAD_CURSOR_POSITION:
    if( crtc_param_num < 2 ){
      if( crtc_load_cursor_position ){
	crtc_cursor[ crtc_param_num++ ] = data;
      }else{
	crtc_cursor[ crtc_param_num++ ] = -1;
      }
    }
    break;

  }
}

/*-------- ステータス出力時 --------*/

byte	crtc_in_status( void )
{
  return crtc_status;
}

/*-------- パラメータ出力時 --------*/

byte	crtc_in_parameter( void )
{
  byte data = 0xff;

  switch( crtc_command ){
  case CRTC_READ_LIGHT_PEN:
    if( crtc_param_num < 2 ){
      data = crtc_light_pen[ crtc_param_num++ ];
    }
    return data;
  }

  return 0xff;
}





/****************************************************************/
/*    DMAC エミュレーション					*/
/****************************************************************/

static	int	dmac_flipflop;
	pair	dmac_address[4];
	pair	dmac_counter[4];
	int	dmac_mode;


void	dmac_init( void )
{
  dmac_flipflop = 0;
  dmac_address[0].W = 0;
  dmac_address[1].W = 0;
  dmac_address[2].W = 0xf3c8;
  dmac_address[3].W = 0;
  dmac_counter[0].W = 0;
  dmac_counter[1].W = 0;
  dmac_counter[2].W = 0;
  dmac_counter[3].W = 0;
}


void	dmac_out_mode( byte data )
{
  dmac_flipflop = 0;
  dmac_mode = data;

  set_text_display();
  screen_set_dirty_all();
}
byte	dmac_in_status( void )
{
  return 0x1f;
}


void	dmac_out_address( byte addr, byte data )
{
  if( dmac_flipflop==0 ) dmac_address[ addr ].B.l=data;
  else                   dmac_address[ addr ].B.h=data;

  dmac_flipflop ^= 0x1;
  screen_set_dirty_all();	/* 本当は、addr==2の時のみ……… */
}
void	dmac_out_counter( byte addr, byte data )
{
  if( dmac_flipflop==0 ) dmac_counter[ addr ].B.l=data;
  else                   dmac_counter[ addr ].B.h=data;

  dmac_flipflop ^= 0x1;
}


byte	dmac_in_address( byte addr )
{
  byte data;

  if( dmac_flipflop==0 ) data = dmac_address[ addr ].B.l;
  else                   data = dmac_address[ addr ].B.h;

  dmac_flipflop ^= 0x1;
  return data;
}
byte	dmac_in_counter( byte addr )
{
  byte data;

  if( dmac_flipflop==0 ) data = dmac_counter[ addr ].B.l;
  else                   data = dmac_counter[ addr ].B.h;

  dmac_flipflop ^= 0x1;
  return data;
}


/***********************************************************************
 * CRTC,DMAC設定時および、I/O 31H / 53H 出力時に呼ぶ
 ************************************************************************/
void	set_text_display(void)
{
    if( (dmac_mode & 0x4) && (crtc_active) && crtc_intr_mask==3){
	if( !(grph_pile & GRPH_PILE_TEXT) ){
	    text_display = TEXT_ENABLE;
	}else{
	    if( grph_ctrl & GRPH_CTRL_COLOR )
		text_display = TEXT_DISABLE;
	    else
		text_display = TEXT_ATTR_ONLY;
	}
    }else{
	text_display = TEXT_DISABLE;
    }
}


/***********************************************************************
 * 画面表示のための関数
 ************************************************************************/


/*======================================================================
 * テキストVRAMのアトリビュートを専用ワークに設定する
 *
 *	バッファは2個あり、交互に切替えて使用する。
 *	画面書き換えの際は、この2個のバッファを比較し、変化の
 *	あった部分だけを更新する。
 *
 *	ワークは、16bitで、上位8bitが文字コード、下位は属性。
 *		色、グラフィックモード、アンダーライン、
 *		アッパーライン、シークレット、リバース
 *		+---------------------+--+--+--+--+--+--+--+--+
 *		|    ASCII 8bit       |Ｇ|Ｒ|Ｂ|GR|LO|UP|SC|RV|
 *		+---------------------+--+--+--+--+--+--+--+--+
 *	BLINK属性は、点灯時は無視、消灯時はシークレット。
 *
 *	さらに、シークレット属性の場合は 文字コードを 0 に置換する。
 *	(文字コード==0は無条件で空白としているので)
 *		+---------------------+--+--+--+--+--+--+--+--+
 *	     →	|    ASCII == 0       |Ｇ|Ｒ|Ｂ|０|LO|UP|０|RV|
 *		+---------------------+--+--+--+--+--+--+--+--+
 *	        グラフィックモードとシークレット属性も消してもOKだが、
 *		アンダー、アッパーライン、リバースは有効なので残す。
 *
 *======================================================================*/

int	text_attr_flipflop = 0;
Ushort	text_attr_buf[2][2048];		/* アトリビュート情報	*/
			/* ↑ 80文字x25行=2000で足りるのだが、	*/
			/* 余分に使うので、多めに確保する。	*/
				   

void	crtc_make_text_attr( void )
{
  int		global_attr  = (ATTR_G|ATTR_R|ATTR_B);
  int		global_blink = FALSE;
  int		i, j, tmp;
  int		column, attr, attr_rest;
  word		char_start_addr, attr_start_addr;
  word		c_addr, a_addr;
  Ushort	*text_attr = &text_attr_buf[ text_attr_flipflop ][0];


	/* CRTC も DMAC も止まっている場合 */
	/*  (文字もアトリビュートも無効)   */

  if( text_display==TEXT_DISABLE ){		/* ASCII=0、白色、装飾なし */
    for( i=0; i<CRTC_SZ_LINES; i++ ){		/* で初期化する。	   */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ =  (ATTR_G|ATTR_R|ATTR_B);
      }
    }
    return;			/* 全画面反転やカーソルもなし。すぐに戻る  */
  }



	/* ノン・トランスペアレント型の場合 */
	/* (1文字置きに、VRAM、ATTR がある) */

			/* ……… ？詳細不明 				*/
			/*	CRTCの設定パターンからして、さらに行の	*/
			/*	最後に属性がある場合もありえそうだが…?	*/

  if( crtc_attr_non_separate ){

    char_start_addr = text_dma_addr.W;
    attr_start_addr = text_dma_addr.W + 1;

    for( i=0; i<crtc_sz_lines; i++ ){

      c_addr	= char_start_addr;
      a_addr	= attr_start_addr;

      char_start_addr += crtc_byte_per_line;
      attr_start_addr += crtc_byte_per_line;

      for( j=0; j<CRTC_SZ_COLUMNS; j+=2 ){		/* 属性を内部コードに*/
	attr = main_ram[ a_addr ];			/* 変換し、属性ワーク*/
	a_addr += 2;					/* を全て埋める。    */
	global_attr =( global_attr & COLOR_MASK ) |
		     ((attr &  MONO_GRAPH) >> 3 ) |
		     ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
		     ((attr &  MONO_SECRET) << 1 );

					/* BLINKのOFF時はSECRET扱い    */
	if( (attr & MONO_BLINK) && ((blink_counter&0x03)==0) ){
	  global_attr |= ATTR_SECRET;
	}

	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;
	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;

      }

      if( crtc_skip_line ){
	if( ++i < crtc_sz_lines ){
	  for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	    *text_attr ++ =  global_attr | ATTR_SECRET;
	  }
	}
      }

    }
    for( ; i<CRTC_SZ_LINES; i++ ){		/* 残りの行は、SECRET */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){	/*  (24行設定対策)    */
	*text_attr ++ =  global_attr | ATTR_SECRET;
      }
    }

  }else{

	/* トランスペアレント型の場合 */
	/* (行の最後に、ATTRがある)   */

    char_start_addr = text_dma_addr.W;
    attr_start_addr = text_dma_addr.W + crtc_sz_columns;

    for( i=0; i<crtc_sz_lines; i++ ){			/* 行単位で属性作成 */

      c_addr	= char_start_addr;
      a_addr	= attr_start_addr;

      char_start_addr += crtc_byte_per_line;
      attr_start_addr += crtc_byte_per_line;


      attr_rest = 0;						/*属性初期化 */
      for( j=0; j<=CRTC_SZ_COLUMNS; j++ ) text_attr[j] = 0;	/* [0]〜[80] */


      for( j=0; j<crtc_sz_attrs; j++ ){			/* 属性を指定番目の */
	column = main_ram[ a_addr++ ];			/* 配列に格納       */
	attr   = main_ram[ a_addr++ ];

	if( j!=0 && column==0    ) column = 0x80;		/* 特殊処理?*/
	if( j==0 && column==0x80 ){column = 0;
/*				   global_attr = (ATTR_G|ATTR_R|ATTR_B);
				   global_blink= FALSE;  }*/}

	if( column==0x80  &&  !attr_rest ){			/* 8bit目は */
	  attr_rest = attr | 0x100;				/* 使用済の */
	}							/* フラグ   */
	else if( column <= CRTC_SZ_COLUMNS  &&  !text_attr[ column ] ){
	  text_attr[ column ] = attr | 0x100;
	}
      }


      if( !text_attr[0] && attr_rest ){			/* 指定桁-1まで属性が*/
	for( j=CRTC_SZ_COLUMNS; j; j-- ){		/* 有効、という場合の*/
	  if( text_attr[j] ){				/* 処理。(指定桁以降 */
	    tmp          = text_attr[j];		/* 属性が有効、という*/
	    text_attr[j] = attr_rest;			/* ふうに並べ替える) */
	    attr_rest    = tmp;
	  }
	}
	text_attr[0] = attr_rest;
      }


      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){		/* 属性を内部コードに*/
							/* 変換し、属性ワーク*/
	if( ( attr = *text_attr ) ){			/* を全て埋める。    */
	  if( crtc_attr_color ){
	    if( attr & COLOR_SWITCH ){
	      global_attr =( global_attr & MONO_MASK ) |
			   ( attr & (COLOR_G|COLOR_R|COLOR_B|COLOR_GRAPH));
	    }else{
	      global_attr =( global_attr & (COLOR_MASK|ATTR_GRAPH) ) |
			   ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
			   ((attr &  MONO_SECRET) << 1 );
	      global_blink= (attr & MONO_BLINK);
	    }
	  }else{
	    global_attr =( global_attr & COLOR_MASK ) |
			 ((attr &  MONO_GRAPH) >> 3 ) |
			 ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
			 ((attr &  MONO_SECRET) << 1 );
	    global_blink= (attr & MONO_BLINK);
	  }
					/* BLINKのOFF時はSECRET扱い    */
	  if( global_blink && ((blink_counter&0x03)==0) ){
	    global_attr =  global_attr | ATTR_SECRET;
	  }
	}

	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;

      }

      if( crtc_skip_line ){				/* 1行飛ばし指定時は*/
	if( ++i < crtc_sz_lines ){			/* 次の行をSECRETで */
	  for( j=0; j<CRTC_SZ_COLUMNS; j++ ){		/* 埋める。         */
	    *text_attr ++ =  global_attr | ATTR_SECRET;
	  }
	}
      }

    }

    for( ; i<CRTC_SZ_LINES; i++ ){		/* 残りの行は、SECRET */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){	/*  (24行設定対策)    */
	*text_attr ++ =  global_attr | ATTR_SECRET;
      }
    }

  }



	/* CRTC や DMAC は動いているけど、 テキストが非表示 */
	/* でVRAM白黒の場合 (アトリビュートの色だけが有効)  */

  if( text_display==TEXT_ATTR_ONLY ){

    text_attr = &text_attr_buf[ text_attr_flipflop ][0];

    for( i=0; i<CRTC_SZ_LINES; i++ ){
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ &=  (ATTR_G|ATTR_R|ATTR_B);
      }
    }
    return;			/* 全画面反転やカーソルは不要。ここでに戻る  */
  }




		/* 全体反転処理 */

  if( crtc_reverse_display && (grph_ctrl & GRPH_CTRL_COLOR)){
    text_attr = &text_attr_buf[ text_attr_flipflop ][0];
    for( i=0; i<CRTC_SZ_LINES; i++ ){
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ ^= ATTR_REVERSE;
      }
    }
  }

		/* カーソル表示処理 */

  if( 0 <= crtc_cursor[0] && crtc_cursor[0] < crtc_sz_columns &&
      0 <= crtc_cursor[1] && crtc_cursor[1] < crtc_sz_lines   ){
    if( !crtc_cursor_blink || (blink_counter&0x01) ){
      text_attr_buf[ text_attr_flipflop ][ crtc_cursor[1]*80 + crtc_cursor[0] ]
							^= crtc_cursor_style;
    }
  }


	/* シークレット属性処理 (文字コード 0x00 に置換) */

  text_attr = &text_attr_buf[ text_attr_flipflop ][0];
  for( i=0; i<CRTC_SZ_LINES; i++ ){
    for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
      if( *text_attr & ATTR_SECRET ){		/* SECRET 属性は、コード00に */
	*text_attr &= (COLOR_MASK|ATTR_UPPER|ATTR_LOWER|ATTR_REVERSE);
      }
      text_attr ++;
    }
  }

}






/***********************************************************************
 * 指定された文字コード(属性・文字)より、フォント字形データを生成する
 *
 *	int attr	… 文字コード。 text_attr_buf[]の値である。
 *	T_GRYPH *gryph	… gryph->b[0]〜[7] に フォントのビットマップが
 *			   格納される。(20行時は、b[0]〜[9]に格納)
 *	int *color	… フォントの色が格納される。値は、 8〜15
 *
 *	字形データは、char 8〜10個なのだが、姑息な高速化のために long で
 *	アクセスしている。そのため、 T_GRYPH という妙な型を使っている。
 *	(大丈夫・・・だよね？)
 *
 *	注)	アトリビュート情報が必要なので、
 *		予め make_text_attr_table( ) を呼んでおくこと
 ************************************************************************/

void	get_font_gryph( int attr, T_GRYPH *gryph, int *color )
{
  int	chara;
  bit32	*src;
  bit32	*dst = (bit32 *)gryph;

  *color = ((attr & COLOR_MASK) >> 5) | 8;


  if( ( attr & ~(COLOR_MASK|ATTR_REVERSE) )==0 ){

    if( ( attr & ATTR_REVERSE ) == 0 ){		/* 空白フォント時 */

      *dst++ = 0;
      *dst++ = 0;
      *dst   = 0;

    }else{					/* ベタフォント時 */

      *dst++ = 0xffffffff;
      *dst++ = 0xffffffff;
      *dst   = 0xffffffff;
    }

  }else{					/* 通常フォント時 */

    chara = attr >> 8;

    if( attr & ATTR_GRAPH )
      src = (bit32 *)&font_rom[ (chara | 0x100)*8 ];
    else
      src = (bit32 *)&font_rom[ (chara        )*8 ];

					/* フォントをまず内部ワークにコピー */
    *dst++ = *src++;
    *dst++ = *src;
    *dst   = 0;

					/* 属性により内部ワークフォントを加工*/
    if( attr & ATTR_UPPER ) gryph->b[ 0 ] |= 0xff;
    if( attr & ATTR_LOWER ) gryph->b[ crtc_font_height-1 ] |= 0xff;
    if( attr & ATTR_REVERSE ){
      dst -= 2;
      *dst++ ^= 0xffffffff;
      *dst++ ^= 0xffffffff;
      *dst   ^= 0xffffffff;
    }
  }
}





/***********************************************************************
 * ステートロード／ステートセーブ
 ************************************************************************/

#define	SID	"CRTC"

static	T_SUSPEND_W	suspend_crtcdmac_work[]=
{
  { TYPE_INT,	&text_display,		},
  { TYPE_INT,	&blink_cycle,		},
  { TYPE_INT,	&blink_counter,		},

  { TYPE_INT,	&dma_wait_count,	},

  { TYPE_INT,	&crtc_command,		},
  { TYPE_INT,	&crtc_param_num,	},
  { TYPE_BYTE,	&crtc_status,		},
  { TYPE_BYTE,	&crtc_light_pen[0],	},
  { TYPE_BYTE,	&crtc_light_pen[1],	},
  { TYPE_BYTE,	&crtc_load_cursor_position,	},
  { TYPE_INT,	&crtc_active,		},
  { TYPE_INT,	&crtc_intr_mask,	},
  { TYPE_INT,	&crtc_cursor[0],	},
  { TYPE_INT,	&crtc_cursor[1],	},
  { TYPE_BYTE,	&crtc_format[0],	},
  { TYPE_BYTE,	&crtc_format[1],	},
  { TYPE_BYTE,	&crtc_format[2],	},
  { TYPE_BYTE,	&crtc_format[3],	},
  { TYPE_BYTE,	&crtc_format[4],	},
  { TYPE_INT,	&crtc_reverse_display,	},
  { TYPE_INT,	&crtc_skip_line,	},
  { TYPE_INT,	&crtc_cursor_style,	},
  { TYPE_INT,	&crtc_cursor_blink,	},
  { TYPE_INT,	&crtc_attr_non_separate,},
  { TYPE_INT,	&crtc_attr_color,	},
  { TYPE_INT,	&crtc_attr_non_special,	},
  { TYPE_INT,	&CRTC_SZ_LINES,		},
  { TYPE_INT,	&crtc_sz_lines,		},
  { TYPE_INT,	&crtc_sz_columns,	},
  { TYPE_INT,	&crtc_sz_attrs,		},
  { TYPE_INT,	&crtc_byte_per_line,	},
  { TYPE_INT,	&crtc_font_height,	},

  { TYPE_INT,	&dmac_flipflop,		},
  { TYPE_PAIR,	&dmac_address[0],	},
  { TYPE_PAIR,	&dmac_address[1],	},
  { TYPE_PAIR,	&dmac_address[2],	},
  { TYPE_PAIR,	&dmac_address[3],	},
  { TYPE_PAIR,	&dmac_counter[0],	},
  { TYPE_PAIR,	&dmac_counter[1],	},
  { TYPE_PAIR,	&dmac_counter[2],	},
  { TYPE_PAIR,	&dmac_counter[3],	},
  { TYPE_INT,	&dmac_mode,		},

  { TYPE_END,	0			},
};


int	statesave_crtcdmac( void )
{
  if( statesave_table( SID, suspend_crtcdmac_work ) == STATE_OK ) return TRUE;
  else                                                            return FALSE;
}

int	stateload_crtcdmac( void )
{
  if( stateload_table( SID, suspend_crtcdmac_work ) == STATE_OK ) return TRUE;
  else                                                            return FALSE;
}
