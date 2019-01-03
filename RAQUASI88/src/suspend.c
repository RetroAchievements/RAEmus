/************************************************************************/
/*									*/
/* サスペンド、レジューム処理						*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "suspend.h"
#include "initval.h"
#include "file-op.h"

int	resume_flag  = FALSE;			/* 起動時のレジューム	*/
int	resume_force = FALSE;			/* 強制レジューム	*/
int	resume_file  = FALSE;			/* ファイル名指定あり	*/

char	file_state[QUASI88_MAX_FILENAME];	/* ステートファイル名   */



/*======================================================================
  ステートファイルの構成

	ヘッダ部	32バイト
	データ部	不定バイト
	データ部	不定バイト
	  ：
	  ：
	終端部


  ヘッダ部	32バイト	とりあえず、内容は以下のとおり。_ は NUL文字
				QUASI88_0.6.0_1_________________
					識別ID		QUASI88
					バージョン	0.6.0
					互換番号	1

  データ部	ID		ASCII4バイト
		データ長	4バイト整数 (リトルエンディアン)
		データ		不定バイト数

		データ長には、 ID と 自身のデータ長 の 8バイトは含まない

  終端部	ID		0x00 4バイト
		データ長	0x00 4バイト


  整数値はすべてリトルエンディアンにでもしておこう。

  データ部の詳細は、その都度考えることにします・・・
  ======================================================================*/

#define	SZ_HEADER	(32)


/*----------------------------------------------------------------------
 * ステートファイルにデータを記録する関数
 * ステートファイルに記録されたデータを取り出す関数
 *		整数データはリトルエンディアンで記録
 *		int 型、 short 型、char 型、pair 型、256バイトブロック、
 *		文字列(1023文字まで)、double型 (1000000倍してintに変換)
 *----------------------------------------------------------------------*/
INLINE	int	statesave_int( OSD_FILE *fp, int *val )
{
  unsigned char c[4];
  c[0] = ( *val       ) & 0xff;
  c[1] = ( *val >>  8 ) & 0xff;
  c[2] = ( *val >> 16 ) & 0xff;
  c[3] = ( *val >> 24 ) & 0xff;
  if( osd_fwrite( c, sizeof(char), 4, fp )==4 ) return TRUE;
  return FALSE;
}
INLINE	int	stateload_int( OSD_FILE *fp, int *val )
{
  unsigned char c[4];
  if( osd_fread( c, sizeof(char), 4, fp )!=4 ) return FALSE;
  *val = ( ((unsigned int)c[3] << 24) | 
	   ((unsigned int)c[2] << 16) |
	   ((unsigned int)c[1] <<  8) |
	    (unsigned int)c[0]       );
  return TRUE;
}
INLINE	int	statesave_short( OSD_FILE *fp, short *val )
{
  unsigned char c[2];
  c[0] = ( *val       ) & 0xff;
  c[1] = ( *val >>  8 ) & 0xff;
  if( osd_fwrite( c, sizeof(char), 2, fp )==2 ) return TRUE;
  return FALSE;
}
INLINE	int	stateload_short( OSD_FILE *fp, short *val )
{
  unsigned char c[2];
  if( osd_fread( c, sizeof(Uchar), 2, fp )!=2 ) return FALSE;
  *val = ( ((unsigned short)c[1] << 8) | 
	    (unsigned short)c[0]       );
  return TRUE;
}
INLINE	int	statesave_char( OSD_FILE *fp, char *val )
{
  if( osd_fwrite( val, sizeof(char), 1, fp )==1 ) return TRUE;
  return FALSE;
}
INLINE	int	stateload_char( OSD_FILE *fp, char *val )
{
  if( osd_fread( val, sizeof(char), 1, fp )!=1 ) return FALSE;
  return TRUE;
}


INLINE	int	statesave_pair( OSD_FILE *fp, pair *val )
{
  unsigned char c[2];
  c[0] = ( (*val).W      ) & 0xff;
  c[1] = ( (*val).W >> 8 ) & 0xff;
  if( osd_fwrite( c, sizeof(char), 2, fp )==2 ) return TRUE;
  return FALSE;
}
INLINE	int	stateload_pair( OSD_FILE *fp, pair *val )
{
  unsigned char c[2];
  if( osd_fread( c, sizeof(char), 2, fp )!=2 ) return FALSE;
  (*val).W = ( ((unsigned short)c[1] << 8) | 
	        (unsigned short)c[0]       );
  return TRUE;
}

INLINE	int	statesave_256( OSD_FILE *fp, char *array )
{
  if( osd_fwrite( array, sizeof(char), 256, fp )==256 ) return TRUE;
  return FALSE;
}
INLINE	int	stateload_256( OSD_FILE *fp, char *array )
{
  if( osd_fread( array, sizeof(char), 256, fp )!=256 ) return FALSE;
  return TRUE;
}


INLINE	int	statesave_str( OSD_FILE *fp, char *str )
{
  char wk[1024];

  if( strlen(str) >= 1024-1 ) return FALSE;

  memset( wk, 0, 1024 );
  strcpy( wk, str );

  if( osd_fwrite( wk, sizeof(char), 1024, fp )==1024 ) return TRUE;
  return FALSE;
}
INLINE	int	stateload_str( OSD_FILE *fp, char *str )
{
  if( osd_fread( str, sizeof(char), 1024, fp )!=1024 ) return FALSE;
  return TRUE;
}

INLINE	int	statesave_double( OSD_FILE *fp, double *val )
{
  unsigned char c[4];
  int	wk;

  wk = (int) ((*val) * 1000000.0);
  c[0] = ( wk       ) & 0xff;
  c[1] = ( wk >>  8 ) & 0xff;
  c[2] = ( wk >> 16 ) & 0xff;
  c[3] = ( wk >> 24 ) & 0xff;
  if( osd_fwrite( c, sizeof(char), 4, fp )==4 ) return TRUE;
  return FALSE;
}
INLINE	int	stateload_double( OSD_FILE *fp, double *val )
{
  unsigned char c[4];
  int	wk;

  if( osd_fread( c, sizeof(char), 4, fp )!=4 ) return FALSE;

  wk = ( ((unsigned int)c[3] << 24) |
	 ((unsigned int)c[2] << 16) |
	 ((unsigned int)c[1] <<  8) |
	  (unsigned int)c[0]        );
  *val = (double)wk / 1000000.0;
  return TRUE;
}




/*----------------------------------------------------------------------
 * IDを検索する関数	戻り値：データサイズ (-1でエラー、-2でデータなし)
 * IDを書き込む関数	戻り値：データサイズ (-1でエラー)
 *----------------------------------------------------------------------*/

static	int	read_id( OSD_FILE *fp, const char id[4] )
{
  char c[4];
  int  size;

  /* ファイル先頭から検索。まずはヘッダをスキップ */
  if( osd_fseek( fp, SZ_HEADER, SEEK_SET ) != 0 ) return -1;

  /* ID が合致するまで SEEK していく */
  for( ;; ){

    if( osd_fread( c, sizeof(char), 4, fp ) != 4 ) return -1;
    if( stateload_int( fp, &size ) == FALSE )      return -1;

    if( memcmp( c, id, 4 ) == 0 ){			/* ID合致した */
      return size;
    }

    if( memcmp( c, "\0\0\0\0", 4 ) == 0 ) return -2;	/* データ終端 */

    if( osd_fseek( fp, size, SEEK_CUR ) != 0 ) return -1;
  }
}


static	int	write_id( OSD_FILE *fp, const char id[4], int size )
{
  /* ファイル現在位置に、書き込む */

  if( osd_fwrite( id, sizeof(char), 4, fp ) != 4 ) return -1;
  if( statesave_int( fp, &size ) == FALSE )        return -1;

  return size;
}




/*======================================================================
 *
 * ステートファイルにデータを記録
 *
 *======================================================================*/
static	OSD_FILE	*statesave_fp;

/* ヘッダ情報を書き込む */
static int statesave_header( void )
{
  size_t off;
  char	header[ SZ_HEADER ];
  OSD_FILE *fp = statesave_fp;

  memset( header, 0, SZ_HEADER );
  off = 0;
  memcpy( &header[off], STATE_ID,  sizeof(STATE_ID)  );
  off += sizeof(STATE_ID);
  memcpy( &header[off], STATE_VER, sizeof(STATE_VER) );
  off += sizeof(STATE_VER);
  memcpy( &header[off], STATE_REV, sizeof(STATE_REV) );

  if( osd_fseek( fp, 0, SEEK_SET ) == 0 &&
      osd_fwrite( header, sizeof(char), SZ_HEADER, fp ) == SZ_HEADER ){

    return STATE_OK;
  }

  return STATE_ERR;
}

/* メモリブロックを書き込む */
int	statesave_block( const char id[4], void *top, int size )
{
  OSD_FILE *fp = statesave_fp;

  if( write_id( fp, id, size ) == size  &&
      osd_fwrite( (char*)top, sizeof(char), size, fp ) == (size_t)size ){

    return STATE_OK;
  }

  return STATE_ERR;
}

/* テーブル情報に従い、書き込む */
int	statesave_table( const char id[4], T_SUSPEND_W *tbl )
{
  OSD_FILE *fp = statesave_fp;
  T_SUSPEND_W *p = tbl;
  int	size = 0;
  int	loop = TRUE;

  while( loop ){		/* 書き込むサイズの総計を計算 */
    switch( p->type ){
    case TYPE_END:	loop = FALSE;	break;
    case TYPE_DOUBLE:
    case TYPE_INT:	
    case TYPE_LONG:	size += 4;	break;
    case TYPE_PAIR:
    case TYPE_SHORT:
    case TYPE_WORD:	size += 2;	break;
    case TYPE_CHAR:
    case TYPE_BYTE:	size += 1;	break;
    case TYPE_STR:	size += 1024;	break;
    case TYPE_256:	size += 256;	break;
    }
    p ++;
  }

  if( write_id( fp, id, size ) != size ) return STATE_ERR;

  for( ;; ){
    switch( tbl->type ){

    case TYPE_END:
      return STATE_OK;

    case TYPE_INT:
    case TYPE_LONG:
      if( statesave_int( fp, (int *)tbl->work )==FALSE ) return STATE_ERR;
      break;

    case TYPE_SHORT:
    case TYPE_WORD:
      if( statesave_short( fp, (short *)tbl->work )==FALSE ) return STATE_ERR;
      break;

    case TYPE_CHAR:
    case TYPE_BYTE:
      if( statesave_char( fp, (char *)tbl->work )==FALSE ) return STATE_ERR;
      break;

    case TYPE_PAIR:
      if( statesave_pair( fp, (pair *)tbl->work )==FALSE ) return STATE_ERR;
      break;

    case TYPE_DOUBLE:
      if( statesave_double( fp, (double *)tbl->work )==FALSE) return STATE_ERR;
      break;

    case TYPE_STR:
      if( statesave_str( fp, (char *)tbl->work )==FALSE ) return STATE_ERR;
      break;

    case TYPE_256:
      if( statesave_256( fp, (char *)tbl->work )==FALSE ) return STATE_ERR;
      break;

    default:	return STATE_ERR;
    }

    tbl ++;
  }
}


/*======================================================================
 *
 * ステートファイルからデータを取り出す
 *
 *======================================================================*/
static	OSD_FILE	*stateload_fp;
static	int		statefile_rev = 0;

/* ヘッダ情報を取り出す */
static int stateload_header( void )
{
  char	header[ SZ_HEADER + 1 ];
  char	*title, *ver, *rev;
  OSD_FILE *fp = stateload_fp;

  if( osd_fseek( fp, 0, SEEK_SET ) == 0 &&
      osd_fread( header, sizeof(char), SZ_HEADER, fp ) == SZ_HEADER ){

    header[ SZ_HEADER ] = '\0';

    title = header;
    ver   = title + strlen(title) + 1;
    rev   = ver   + strlen(ver)   + 1;
    if( verbose_suspend ){
      printf( "stateload: file header is \"%s\", \"%s\", \"%s\".\n",
	      						title, ver, rev );
    }

    if( memcmp( title, STATE_ID, sizeof(STATE_ID) ) != 0 ){

      printf( "stateload: ID mismatch ('%s' != '%s')\n",
							STATE_ID, title );
    }else{
      if( memcmp( ver, STATE_VER, sizeof(STATE_VER) ) != 0 ){

	printf( "stateload: version mismatch ('%s' != '%s')\n",
							STATE_VER, ver );
	if( resume_force == FALSE ) return STATE_ERR;

      }else{

	if( verbose_suspend ){
	  if( memcmp( rev, STATE_REV, sizeof(STATE_REV) ) != 0 ){
	    printf( "stateload: older revision ('%s' != '%s')\n",
							STATE_REV, rev );
	  }
	}
      }

      if( rev[0] == '1' ) statefile_rev = 1;
      else                statefile_rev = 0;

      return STATE_OK;
    }
  }

  return STATE_ERR;
}

/* メモリブロックを取り出す */
int	stateload_block( const char id[4], void *top, int size )
{
  OSD_FILE *fp = stateload_fp;

  int s = read_id( fp, id );

  if( s == -1 )   return STATE_ERR;
  if( s == -2 )   return STATE_ERR_ID;
  if( s != size ) return STATE_ERR_SIZE;

  if( osd_fread( (char*)top, sizeof(char), size, fp ) == (size_t)size ){

    return STATE_OK;
  }

  return STATE_ERR;
}

/* テーブル情報に従い、取り出す */
int	stateload_table( const char id[4], T_SUSPEND_W *tbl )
{
  OSD_FILE *fp = stateload_fp;
  int	size = 0;
  int	s = read_id( fp, id );

  if( s == -1 )   return STATE_ERR;
  if( s == -2 )   return STATE_ERR_ID;

  for( ;; ){
    switch( tbl->type ){

    case TYPE_END:
      if( s != size ) return STATE_ERR_SIZE;
      else            return STATE_OK;

    case TYPE_INT:
    case TYPE_LONG:
      if( stateload_int( fp, (int *)tbl->work )==FALSE ) return STATE_ERR;
      size += 4;
      break;

    case TYPE_SHORT:
    case TYPE_WORD:
      if( stateload_short( fp, (short *)tbl->work )==FALSE ) return STATE_ERR;
      size += 2;
      break;

    case TYPE_CHAR:
    case TYPE_BYTE:
      if( stateload_char( fp, (char *)tbl->work )==FALSE ) return STATE_ERR;
      size += 1;
      break;

    case TYPE_PAIR:
      if( stateload_pair( fp, (pair *)tbl->work )==FALSE ) return STATE_ERR;
      size += 2;
      break;

    case TYPE_DOUBLE:
      if( stateload_double( fp, (double *)tbl->work )==FALSE) return STATE_ERR;
      size += 4;
      break;

    case TYPE_STR:
      if( stateload_str( fp, (char *)tbl->work )==FALSE ) return STATE_ERR;
      size += 1024;
      break;

    case TYPE_256:
      if( stateload_256( fp, (char *)tbl->work )==FALSE ) return STATE_ERR;
      size += 256;
      break;

    default:	return STATE_ERR;
    }

    tbl ++;
  }
}

/* リビジョン取得 */
int	statefile_revision( void )
{
  return statefile_rev;
}



/***********************************************************************
 *
 *
 *
 ************************************************************************/

/*
  statesave() / stateload() でセーブ/ロードされるファイル名は、
  自動的に設定されるので、セーブ/ロード時に指定する必要はない。
  (ディスクイメージの名前などに基づき、設定される)

  でも、これだと1種類しかロード/セーブできずに不便なので、
  filename_set_state_serial(int serial) で連番を指定できる。
	
			ステートファイル名
	引数 '5'	/my/state/dir/file-5.sta
	引数 'z'	/my/state/dir/file-z.sta
	引数 0		/my/state/dir/file.sta

  よって、連番指定でステートセーブする場合は、
	  filename_set_state_serial('1');
	  statesave();
  のように呼び出す。

  ----------------------------------------------------------------------
  ファイル名を変更したい場合は、以下の関数を使う。

  ファイル名の取得 … filename_get_state()
	現在設定されているステートファイル名が取得できる。
	/my/state/dir/file-a.sta のような文字列が返る。

  ファイル連番の取得 … filename_get_state_serial()
	現在設定されているステートファイル名の連番が取得できる。
	/my/state/dir/file-Z.sta ならば、 'Z' が返る。
	/my/state/dir/file.sta ならば、   0 が返る。
	拡張子が .sta でないなら、        -1 が返る。

  ファイル名の設定 … filename_set_state(name)
	ステートファイル名を name に設定する。
	連番つきのファイル名でも、連番なしでもよい。
	なお、NULL を指定すると、初期値がセットされる。

  ファイル連番の設定 … filename_set_state_serial(num)
	連番を num に設定する。  ファイル名の拡張子が .sta でないなら付加する。
	num が 0 なら、連番無し。ファイル名の拡張子が .sta でないなら付加する。
	num が負 なら、連番無し。ファイル名の拡張子はそのままとする。
*/




const char	*filename_get_state(void)
{
    return file_state;
}

int		filename_get_state_serial(void)
{
    const char  *str_sfx = STATE_SUFFIX;		/* ".sta" */
    const size_t len_sfx = strlen(STATE_SUFFIX);	/* 4      */
    size_t len = strlen(file_state);

    if (len > len_sfx &&
	my_strcmp(&file_state[ len - len_sfx ], str_sfx) == 0) {

	if (len > len_sfx + 2 &&	/* ファイル名が xxx-N.sta */
	    '-' ==  file_state[ len - len_sfx -2 ]   &&
	    isalnum(file_state[ len - len_sfx -1 ])) {
						/* '0'-'9','a'-'z' を返す */
	    return file_state[ len - len_sfx -1 ];    

	} else {			/* ファイル名が xxx.sta */
	    return 0;
	}
    } else {				/* ファイル名が その他 */
	return -1;
    }
}

void		filename_set_state(const char *filename)
{
    if (filename) {
	strncpy(file_state, filename, QUASI88_MAX_FILENAME - 1);
	file_state[ QUASI88_MAX_FILENAME - 1 ] = '\0';
    } else {
	filename_init_state(FALSE);
    }
}

void		filename_set_state_serial(int serial)
{
    const char  *str_sfx = STATE_SUFFIX;		/* ".sta"   */
    const size_t len_sfx = strlen(STATE_SUFFIX);	/* 4        */
    char         add_sfx[] = "-N" STATE_SUFFIX;		/* "-N.sta" */
    size_t len;
    int now_serial;

    add_sfx[1] = serial;

    len = strlen(file_state);

    now_serial = filename_get_state_serial();

    if (now_serial > 0) {		/* 元のファイル名が xxx-N.sta */

	file_state[ len - len_sfx -2 ] = '\0';	/* -N.sta を削除 */

	if (serial <= 0) {			/* xxx → xxx.sta */
	    strcat(file_state, str_sfx);
	} else {				/* xxx → xxx-M.sta */
	    strcat(file_state, add_sfx);
	}

    } else if (now_serial == 0) {	/* 元のファイル名が xxx.sta */

	if (serial <= 0) {			/* xxx.sta のまま */
	    ;
	} else {
	    if (len + 2 < QUASI88_MAX_FILENAME) {
		file_state[ len - len_sfx ] = '\0';  /* .sta を削除 */
		strcat(file_state, add_sfx);	/* xxx → xxx-M.sta */
	    }
	}

    } else {				/* 元のファイル名が その他 xxx */

	if (serial < 0) {			/* xxx のまま */
	    ;
	} else if (serial == 0) {		/* xxx → xxx.sta */
	    if (len + len_sfx < QUASI88_MAX_FILENAME) {
		strcat(file_state, str_sfx);
	    }
	} else {				/* xxx → xxx-M.sta */
	    if (len + len_sfx + 2 < QUASI88_MAX_FILENAME) {
		strcat(file_state, add_sfx);
	    }
	}
    }
}






int	statesave_check_file_exist(void)
{
    OSD_FILE *fp;

    if (file_state[0] &&
	(fp = osd_fopen(FTYPE_STATE_LOAD, file_state, "rb"))) {
	osd_fclose(fp);
	return TRUE;
    }
    return FALSE;
}


int	statesave( void )
{
  int success = FALSE;

  if( file_state[0] == '\0' ){
    printf( "state-file name not defined\n" );
    return FALSE;
  }

  if( verbose_suspend )
    printf( "statesave : %s\n", file_state );

  if( (statesave_fp = osd_fopen( FTYPE_STATE_SAVE, file_state, "wb" )) ){

    if( statesave_header() == STATE_OK ){

      do{
	if( statesave_emu()      == FALSE ) break;
	if( statesave_memory()   == FALSE ) break;
	if( statesave_pc88main() == FALSE ) break;
	if( statesave_crtcdmac() == FALSE ) break;
	if( statesave_sound()    == FALSE ) break;
	if( statesave_pio()      == FALSE ) break;
	if( statesave_screen()   == FALSE ) break;
	if( statesave_intr()     == FALSE ) break;
	if( statesave_keyboard() == FALSE ) break;
	if( statesave_pc88sub()  == FALSE ) break;
	if( statesave_fdc()      == FALSE ) break;
	if( statesave_system()   == FALSE ) break;

	success = TRUE;
      }while(0);

    }

    osd_fclose( statesave_fp );
  }

  return success;
}




int	stateload_check_file_exist(void)
{
    int success = FALSE;

    if (file_state[0] &&
	(stateload_fp = osd_fopen(FTYPE_STATE_LOAD, file_state, "rb"))) {

	if (stateload_header() == STATE_OK) {	/* ヘッダだけチェック */
	    success = TRUE;
	}
	osd_fclose(stateload_fp);
    }

    if (verbose_suspend) {
	printf("stateload: file check ... %s\n", (success) ? "OK" : "FAILED");
    }
    return success;
}


int	stateload( void )
{
  int success = FALSE;

  if( file_state[0] == '\0' ){
    printf( "state-file name not defined\n" );
    return FALSE;
  }

  if( verbose_suspend )
    printf( "stateload: %s\n", file_state );

  if( (stateload_fp = osd_fopen( FTYPE_STATE_LOAD, file_state, "rb" )) ){

    if( stateload_header() == STATE_OK ){

      do{
	if( stateload_emu()      == FALSE ) break;
	if( stateload_sound()    == FALSE ) break;
	if( stateload_memory()   == FALSE ) break;
	if( stateload_pc88main() == FALSE ) break;
	if( stateload_crtcdmac() == FALSE ) break;
      /*if( stateload_sound()    == FALSE ) break; memoryの前に！ */
	if( stateload_pio()      == FALSE ) break;
	if( stateload_screen()   == FALSE ) break;
	if( stateload_intr()     == FALSE ) break;
	if( stateload_keyboard() == FALSE ) break;
	if( stateload_pc88sub()  == FALSE ) break;
	if( stateload_fdc()      == FALSE ) break;
	if( stateload_system()   == FALSE ) break;

	success = TRUE;
      }while(0);

    }

    osd_fclose( stateload_fp );
  }

  return success;
}



/***********************************************************************
 * ステートファイル名を初期化
 ************************************************************************/
void	stateload_init(void)
{
    if (file_state[0] == '\0') {
	filename_init_state(FALSE);
    }

    /* 起動時のオプションでステートロードが指示されている場合、
       なんらかのファイル名がすでにセットされているはず */
}
