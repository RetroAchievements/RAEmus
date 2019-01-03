/************************************************************************/
/*									*/
/* ローマ字→カナ変換処理						*/
/*									*/
/************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "keyboard.h"
#include "romaji.h"


#define Bt7	0x80
#define Bt6	0x40
#define Bt5	0x20
#define Bt4	0x10
#define Bt3	0x08
#define Bt2	0x04
#define Bt1	0x02
#define Bt0	0x01

#define P0	0
#define P1	1
#define P2	2
#define P3	3
#define P4	4
#define P5	5
#define P6	6
#define P7	7
#define P8	8
#define P9	9
#define Pa	10
#define Pb	11
#define Pc	12
#define Pd	13
#define Pe	14

/* キューに蓄える値。ポート情報がパックしてある */
#define	RJ( port, bit, shift )	((Uchar)( (port<<4) | (shift<<3) | bit ))

/*----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------*/
/*	ローマ字 → カナ 変換テーブル	*/
typedef struct {
  const char *s;
  Uchar      list[4];
} romaji_list;

#include "romaji-table.h"

/* ↑ romaji-table.h 内にて、以下のテーブルが定義してある */
/*
static const romaji_list list_NN;
static const romaji_list list_tu;
static const romaji_list list_mark[];
static const romaji_list list_msime[];
static const romaji_list list_atok[];
static const romaji_list list_egg[];
*/



/*----------------------------------------------------------------------
 * ワーク (サスペンド情報には残す必要なし)
 *----------------------------------------------------------------------*/

/*	ローマ字変換前のバッファ	*/

static	char	input_buf[4];			/* 入力済みの文字のバッファ  */
static	int	input_size;			/* 入力済みの文字の数	     */


/*	ローマ字変換後のバッファ(キュー) */

#define	ROMAJI_QUE_SIZE	(64)
static	int	romaji_set;			/* ローマ字入力されたカナは */
static	int	romaji_ptr;			/* キューに蓄えられ、順次   */
static	Uchar	romaji_que[ ROMAJI_QUE_SIZE ];	/* I/O ポートに送られていく */

static	int	press_timer;			/* キーオン・オフのタイマー */
#define	KEY_ON_OFF_INTERVAL	(4)		/* キーオン・オフの時間	    */





/*----------------------------------------------------------------------
 *
 *	ローマ字カナ変換テーブルの作成 (1回だけ呼び出しておく)
 *
 *----------------------------------------------------------------------*/

static romaji_list list[280];
static int         nr_list;

static int romajicmp( const void *p1, const void *p2 )
{
  return strcmp( ((const romaji_list *)p1)->s, ((const romaji_list *)p2)->s );
}

void	romaji_init( void )
{
  int i,            nr_p;
  const romaji_list *p;

  romaji_clear();
  nr_list = 0;

  if     ( romaji_type==1 ){ p = list_msime; nr_p = COUNTOF(list_msime); }
  else if( romaji_type==2 ){ p = list_atok;  nr_p = COUNTOF(list_atok);  }
  else                     { p = list_egg;   nr_p = COUNTOF(list_egg);   }
  for( i=0; i<nr_p; i++ ){
    list[ nr_list ++ ] = *p ++;
    if( nr_list >= COUNTOF(list) ) break;
  }
  

  qsort( &list, nr_list, sizeof(romaji_list), romajicmp );

  p    = list_mark;
  nr_p = COUNTOF(list_mark);
  for( i=0; i<nr_p; i++ ){
    list[ nr_list ++ ] = *p ++;
    if( nr_list >= COUNTOF(list) ) break;
  }
  
/*
  printf("%d\n",nr_list);
  for( i=0; i<nr_list; i++ ){ printf("%s ", list[i].s ); }
  printf("\n%d %d %d\n",COUNTOF(list_msime),COUNTOF(list_atok),COUNTOF(list_egg));
*/
}



/*----------------------------------------------------------------------
 *
 *	ローマ字変換のワーク初期化 (ローマ字変換モード開始時に呼び出す)
 *
 *----------------------------------------------------------------------*/
void	romaji_clear( void )
{
  /* 変換後のカナをオン出力中だったら、オフ出力する */

  if( 0< press_timer && press_timer <= KEY_ON_OFF_INTERVAL ){
    press_timer = KEY_ON_OFF_INTERVAL;
    romaji_output();
  }

  /* ワークを全てクリア */

  romaji_set  = 0;
  romaji_ptr  = 0;
  press_timer = 0;
  input_size  = 0;
}



/*----------------------------------------------------------------------
 *
 *	入力をローマ字に変換し、キューに蓄える処理関数
 *
 *----------------------------------------------------------------------*/

static	void	set_romaji_que( const Uchar *p )
{
  Uchar c;
  while( (c = *p++ ) ){
    romaji_que[ romaji_set++ ] = c;
    romaji_set &= (ROMAJI_QUE_SIZE-1);
  }
}


int	romaji_input( int key )
{
  int i, j;

  if( key == ' ' ||		/* これらのキーは特殊変換に使う */
      key == '@' ||
      key == '[' ||
      key == '/' ||
      key == '-' ||
      key == '{' ||
      key == '}' ||
      key == '.' ||
      key == ',' ||
      key == '\'' ){
    ;
  }
  else if( islower(key) ){	/* 小文字は大文字に変換して使う */
    key = toupper( key );
  }
  else if( isupper(key) ){	/* 大文字はそのまま使う */
    ;
  }
  else {			/* それ以外は使わない */

    if( key == KEY88_ESC && input_size != 0 ){
      romaji_clear();
      return 0;
    }
    if( key == KEY88_SHIFT  ||
	key == KEY88_SHIFTL ||
	key == KEY88_SHIFTR ||
	key == KEY88_CAPS   ||
	key == KEY88_KANA   ||
	key == KEY88_GRAPH  ){
      ;
    }else{
      romaji_clear();
    }
    return key;
  }



  input_buf[ input_size ] = key;
  input_size ++;

  while( input_size ){
    
    int          list_size = nr_list;
    romaji_list *list_p   = list;

    int same   = FALSE;
    int nearly = FALSE;

    for( i=0; i<list_size; i++, list_p++ ){

      const char *s1 = input_buf;
      const char *s2 = list_p->s;

      for( j=0; j<input_size; j++, s1++, s2++ ){
	if( *s1 != *s2 ){ j=0; break; }
      }

      if( j==0 ){			/* 不一致 */
	if( nearly ) break;			/* 途中まで一致してたのに */
	/* list->s のソート済みが前提 */	/* 一致しなくなったら中断 */
      }else{
	if( *s2 == '\0' ){		/* 完全に一致 */
	  same   = TRUE;	break;
	}else{				/* 途中まで一致 */
	  nearly = TRUE;
	}
      }
    }

    if( same ){					/*** 完全一致の場合 */
      set_romaji_que( list_p->list );			/* キューにセット */
      input_size = 0;					/* 入力を捨てる   */
      break;

    }else if( nearly ){				/*** 途中まで一致の場合 */
      break;						/* 入力はそのまま */

    }else{					/*** 不一致の場合 */

      if( input_buf[0] == 'N' ){
	set_romaji_que( list_NN.list );			/* んをキューにセット*/

      }else if( input_size >= 2 &&
		input_buf[0]==input_buf[1] ){
	set_romaji_que( list_tu.list );			/* っをキューにセット*/
      }

      input_size --;					/* 入力をずらす */
      memmove( &input_buf[0], &input_buf[1],input_size );

      /* 一致するまでチェックしなおす */
    }
  }


  if( input_size >= (int)sizeof(input_buf) )	/* バッファオーバー防止 */
    input_size = 0;

  return 0;
}



/*----------------------------------------------------------------------
 *
 *	変換後のカナをポートに出力していく関数 (1/60s毎に呼び出す)
 *
 *----------------------------------------------------------------------*/
/*
 *	キューからポートに出力していく関数
 */
void	romaji_output( void )
{
  Uint c;

  switch( press_timer ){
  case 0:
    if( romaji_ptr != romaji_set ){
      c = romaji_que[ romaji_ptr ];
      if( c & 0x08 ) key_scan[P8] &= ~Bt6;
      key_scan[ c>>4 ] &= ~(1<<(c&7));
      press_timer ++;
    }
    break;

  case KEY_ON_OFF_INTERVAL:
    c = romaji_que[ romaji_ptr++ ];
    romaji_ptr &= (ROMAJI_QUE_SIZE-1);
    if( c & 0x08 ) key_scan[P8] |=  Bt6;
    key_scan[ c>>4 ] |=  (1<<(c&7));
    press_timer ++;
    break;

  case KEY_ON_OFF_INTERVAL*2:
    press_timer = 0;
    break;

  default:
    press_timer ++;
  }
}

