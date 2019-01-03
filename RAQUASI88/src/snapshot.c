/************************************************************************/
/*									*/
/* スクリーン スナップショット						*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "screen.h"
#include "screen-func.h"
#include "graph.h"

#include "crtcdmac.h"
#include "memory.h"
#include "file-op.h"
#include "snapshot.h"
#include "initval.h"



char	file_snap[QUASI88_MAX_FILENAME];/* スナップショットベース部	*/
int	snapshot_format  = 0;		/* スナップショットフォーマット	*/

char	snapshot_cmd[ SNAPSHOT_CMD_SIZE ];/* スナップショット後コマンド	*/
char	snapshot_cmd_do  = FALSE;	/* コマンド実行の有無		*/

#ifdef	USE_SSS_CMD
char	snapshot_cmd_enable = TRUE;	/* コマンド実行の可否		*/
#else
char	snapshot_cmd_enable = FALSE;	/* コマンド実行の可否		*/
#endif


/* スナップショットを作成する一時バッファと、色のインデックス */

char			screen_snapshot[ 640*400 ];
static PC88_PALETTE_T	pal[16 +1];





/***********************************************************************
 * スナップショットファイル名などを初期化
 ************************************************************************/
void	screen_snapshot_init(void)
{
    const char *s;

    if (file_snap[0] == '\0') {
	filename_init_snap(FALSE);
    }


    memset(snapshot_cmd, 0, SNAPSHOT_CMD_SIZE);
    s = getenv("QUASI88_SSS_CMD");			/* 実行コマンド */
    if (s  &&  (strlen(s) < SNAPSHOT_CMD_SIZE)) {
	strcpy(snapshot_cmd, s);
    }

    snapshot_cmd_do = FALSE;	/* 初期値は、コマンド実行『しない』にする */



    if (file_wav[0] == '\0') {
	filename_init_wav(FALSE);
    }
}
void	screen_snapshot_exit(void)
{
    waveout_save_stop();
}




/*----------------------------------------------------------------------*/
/* 画面イメージを生成する						*/
/*----------------------------------------------------------------------*/

typedef	int		( *SNAPSHOT_FUNC )( void );

static	void	make_snapshot( void )
{
  int vram_mode, text_mode;
  SNAPSHOT_FUNC		(*list)[4][2];


  /* skipline の場合は、予め snapshot_clear() を呼び出しておく */

  if     ( use_interlace == 0 ){ list = snapshot_list_normal; }
  else if( use_interlace >  0 ){ list = snapshot_list_itlace; }
  else                         { snapshot_clear();
				 list = snapshot_list_skipln; }



	/* VRAM/TEXT の内容を screen_snapshot[] に転送 */

  if( sys_ctrl & SYS_CTRL_80 ){
    if( CRTC_SZ_LINES == 25 ){ text_mode = V_80x25; }
    else                     { text_mode = V_80x20; }
  }else{
    if( CRTC_SZ_LINES == 25 ){ text_mode = V_40x25; }
    else                     { text_mode = V_40x20; }
  }

  if( grph_ctrl & GRPH_CTRL_VDISP ){
    if( grph_ctrl & GRPH_CTRL_COLOR ){		/* カラー */
        vram_mode = V_COLOR;
    }else{
      if( grph_ctrl & GRPH_CTRL_200 ){		/* 白黒 */
	vram_mode = V_MONO;
      }else{					/* 400ライン */
	vram_mode = V_HIRESO;
      }
    }
  }else{					/* 非表示 */
        vram_mode = V_UNDISP;
  }

  (list[ vram_mode ][ text_mode ][ V_ALL ])();


	/* パレットの内容を pal[] に転送 */

  screen_get_emu_palette( pal );

  pal[16].red   = 0;		/* pal[16] は黒固定で使用する */
  pal[16].green = 0;
  pal[16].blue  = 0;
}






#if 0		/* XPM はサポート対象外 */
/*----------------------------------------------------------------------*/
/* キャプチャした内容を、xpm 形式でファイルに出力			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_xpm( OSD_FILE *fp )
{
  unsigned char buf[80];
  int i, j, c;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  sprintf( buf,
	   "/* XPM */\n"
	   "static char * quasi88_xpm[] = {\n"
	   "\"640 400 16 1\",\n" );
  osd_fwrite( buf, sizeof(char), strlen(buf), fp );

  for( i=0; i<16; i++ ){
    sprintf( buf, "\"%1X      c #%04X%04X%04X\",\n",
	     i,
	     (unsigned short)pal[i].red   << 8,
	     (unsigned short)pal[i].green << 8,
	     (unsigned short)pal[i].blue  << 8 );
    osd_fwrite( buf, sizeof(char), strlen(buf), fp );
  }


  for( i=0; i<400; i++ ){

    osd_fputc( '\"', fp );

    for( j=0; j<640; j++ ){
      c = *p++;
      if( c < 10 ) c += '0';
      else         c += 'A' - 10;
      osd_fputc( c, fp );
    }

    sprintf( buf, "\",\n" );
    osd_fwrite( buf, sizeof(char), strlen(buf), fp );

  }

  sprintf( buf, "};\n" );
  osd_fwrite( buf, sizeof(char), strlen(buf), fp );

  return 1;
}
#endif



/*----------------------------------------------------------------------*/
/* キャプチャした内容を、ppm 形式(raw)でファイルに出力			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_ppm( OSD_FILE *fp )
{
  unsigned char buf[32];
  int i, j;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  strcpy( (char *)buf, 
	  "P6\n"
	  "# QUASI88\n"
	  "640 400\n"
	  "255\n" );
  osd_fwrite( buf, sizeof(char), strlen((char *)buf), fp );


  for( i=0; i<400; i++ ){
    for( j=0; j<640; j++ ){
      buf[0] = pal[ (int)*p ].red;
      buf[1] = pal[ (int)*p ].green;
      buf[2] = pal[ (int)*p ].blue;
      osd_fwrite( buf, sizeof(char), 3, fp );
      p++;
    }
  }

  return 1;
}



#if 0		/* PPM(ascii)  はサポート対象外 */
/*----------------------------------------------------------------------*/
/* キャプチャした内容を、ppm 形式(ascii)でファイルに出力		*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_ppm_ascii( OSD_FILE *fp )
{
  unsigned char buf[32];
  int i, j, k;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  strcpy( buf, 
	  "P3\n"
	  "# QUASI88\n"
	  "640 400\n"
	  "255\n" );
  osd_fwrite( buf, sizeof(char), strlen(buf), fp );

  
  for( i=0; i<400; i++ ){
    for( j=0; j<640; j+=5 ){
      for( k=0; k<5; k++ ){
	sprintf( buf, "%3d %3d %3d ",
		 pal[ (int)*p ].red,
		 pal[ (int)*p ].green,
		 pal[ (int)*p ].blue );
	osd_fwrite( buf, sizeof(char), strlen(buf), fp );
	p++;
      }
      osd_fputc( '\n', fp );
    }

  }

  return 1;
}
#endif



/*----------------------------------------------------------------------*/
/* キャプチャした内容を、bmp 形式(win)でファイルに出力			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_bmp( OSD_FILE *fp )
{
  static const unsigned char header[] =
  {
    'B', 'M',			/* BM */
    0x36, 0xb8, 0x0b, 0x00,	/* ファイルサイズ 0xbb836 */
    0x00, 0x00,
    0x00, 0x00,
    0x36, 0x00, 0x00, 0x00,	/* 画像データオフセット 0x36 */

    0x28, 0x00, 0x00, 0x00,	/* 情報サイズ 0x28 */
    0x80, 0x02, 0x00, 0x00,	/* 幅	0x280 */
    0x90, 0x01, 0x00, 0x00,	/* 高さ	0x190 */
    0x01, 0x00,
    0x18, 0x00,			/* 色深度 */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	/* 画像サイズ?	0xbb800 */
    0x00, 0x00, 0x00, 0x00,	/* 横方向解像度?	*/
    0x00, 0x00, 0x00, 0x00,	/* 縦方向解像度?	*/
    0x00, 0x00, 0x00, 0x00,	/* 使用パレット数	*/
    0x00, 0x00, 0x00, 0x00,	/* 重要?		*/
  };

  unsigned char buf[4];
  int i, j;
  char *p;

  if( fp==NULL ) return 0;

  osd_fwrite( header, sizeof(char), sizeof(header), fp );


  for( i=0; i<400; i++ ){
    p = screen_snapshot + (399-i)*640;
    for( j=0; j<640; j++ ){
      buf[0] = pal[ (int)*p ].blue;
      buf[1] = pal[ (int)*p ].green;
      buf[2] = pal[ (int)*p ].red;
      osd_fwrite( buf, sizeof(char), 3, fp );
      p++;
    }
  }

  return 1;
}



/*----------------------------------------------------------------------*/
/* キャプチャした内容を、raw形式でファイルに出力			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_raw( OSD_FILE *fp )
{
  unsigned char buf[4];
  int i, j;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  for( i=0; i<400; i++ ){
    for( j=0; j<640; j++ ){
      buf[0] = pal[ (int)*p ].red;
      buf[1] = pal[ (int)*p ].green;
      buf[2] = pal[ (int)*p ].blue;
      osd_fwrite( buf, sizeof(char), 3, fp );
      p++;
    }
  }

  return 1;
}



/***********************************************************************
 * 画面のスナップショットをセーブする
 *	現時点のVRAMをもとにスナップショットを作成する。
 *	現在表示されている画面をセーブするわけではない。
 *
 *	環境変数 ${QUASI88_SSS_CMD} が定義されている場合、セーブ後に
 *	この内容をコマンドとして実行する。この際、%a がファイル名に、
 *	%b がファイル名からサフィックスを削除したものに、置き換わる。
 *
 *	例) setenv QUASI88_SSS_CMD 'ppmtopng %a > %b.png'
 *
 ************************************************************************/

/*
  screen_snapshot_save() で、セーブされるファイル名は、
  自動的に設定されるので、セーブ時に指定する必要はない。
  (ディスクイメージの名前などに基づき、設定される)

	ファイル名は、 /my/snap/dir/save0001.bmp のように、連番 + 拡張子 が
	付加される。連番は、 0000 〜 9999 で、既存のファイルと重複しない
	ように適宜決定される。

  ----------------------------------------------------------------------
  ファイル名を変更したい場合は、以下の関数を使う。

  ファイル名の取得 … filename_get_snap_base()
	取得できるファイル名は、/my/snap/dir/save のように、連番と拡張子は
	削除されている。

  ファイル名の設定 … filename_set_snap_base()
	例えば、/my/snap/dir/save0001.bmp と設定しても、末尾の 連番と 拡張子は
	削除される。そのため、filename_set_snap_base() で設定したファイル名と
	filename_get_snap_base() で取得したファイル名は一致しないことがある。
	なお、NULL を指定すると、初期値がセットされる。

*/


/* file_snap[] の末尾が NNNN.suffix となっている場合、末尾を削除する。
   NNNN は連番で 0000〜9999。
   suffix は拡張子で、以下のものを対象とする。*/
static const char *snap_suffix[] = { ".ppm",  ".PPM", 
				     ".xpm",  ".XPM", 
				     ".png",  ".PNG", 
				     ".bmp",  ".BMP", 
				     ".rgb",  ".RGB", 
				     ".raw",  ".RAW", 
				     ".gif",  ".GIF", 
				     ".xwd",  ".XWD", 
				     ".pict", ".PICT",
				     ".tiff", ".TIFF",
				     ".tif",  ".TIF", 
				     ".jpeg", ".JPEG",
				     ".jpg",  ".JPG", 
				     NULL
};
static void truncate_filename(char filename[], const char *suffix[])
{
    int i;
    char *p;

    for (i=0; suffix[i]; i++) {

	if (strlen(filename) > strlen(suffix[i]) + 4) {

	    p = &filename[ strlen(filename) - strlen(suffix[i]) - 4 ];
	    if (isdigit(*(p+0)) &&
		isdigit(*(p+1)) &&
		isdigit(*(p+2)) &&
		isdigit(*(p+3)) &&
		strcmp(p+4, suffix[i]) == 0) {

		*p = '\0';
/*	printf("screen-snapshot : filename truncated (%s)\n", filename);*/
		break;
	    }
	}
    }
}



void		filename_set_snap_base(const char *filename)
{
    if (filename) {
	strncpy(file_snap, filename, QUASI88_MAX_FILENAME - 1);
	file_snap[ QUASI88_MAX_FILENAME - 1 ] = '\0';

	truncate_filename(file_snap, snap_suffix);
    } else {
	filename_init_snap(FALSE);
    }
}
const char	*filename_get_snap_base(void)
{
    return file_snap;
}



int	screen_snapshot_save(void)
{
    static char filename[ QUASI88_MAX_FILENAME + sizeof("NNNN.suffix") ];
    static int snapshot_no = 0;		/* 連番 */

    static const char *suffix[] = { ".bmp", ".ppm", ".raw", };

    OSD_FILE *fp;
    int i, j, len, success;

    if (snapshot_format >= COUNTOF(suffix)) return FALSE;


	/* ファイル名が未指定の場合、初期ファイル名にする */

    if (file_snap[0] == '\0') {
	filename_init_snap(FALSE);
    }


	/* file_snap[] の末端が NNNN.suffix なら削除 */

    truncate_filename(file_snap, snap_suffix);


	/* 存在しないファイル名を探しだす (0000.suffix〜 9999.suffix) */

    success = FALSE;
    for (j=0; j<10000; j++) {

	len = sprintf(filename, "%s%04d", file_snap, snapshot_no);
	if (++ snapshot_no > 9999) snapshot_no = 0;

	for (i=0; snap_suffix[i]; i++) {
	    filename[ len ] = '\0';
	    strcat(filename, snap_suffix[ i ]);
	    if (osd_file_stat(filename) != FILE_STAT_NOEXIST) break;
	}
	if (snap_suffix[i] == NULL) {	    /* 見つかった */
	    filename[ len ] = '\0';
	    strcat(filename, suffix[ snapshot_format ]);
	    success = TRUE;
	    break;
	}
    }


	/* ファイルを開いて、スナップショットデータを書き込む */

    if (success) {

	success = FALSE;
	if ((fp = osd_fopen(FTYPE_SNAPSHOT_PPM, filename, "wb"))) {

	    make_snapshot();

	    switch (snapshot_format) {
	    case 0:	success = save_snapshot_bmp(fp);	break;
	    case 1:	success = save_snapshot_ppm(fp);	break;
	    case 2:	success = save_snapshot_raw(fp);	break;
	    }

	    osd_fclose(fp);
	}
/*
	printf("screen-snapshot : %s ... %s\n", 
				filename, (success) ? "OK" : "FAILED");
*/
    }


	/* 書き込み成功後、コマンドを実行する */

#ifdef	USE_SSS_CMD

    if (success &&

	snapshot_cmd_enable &&
	snapshot_cmd_do     &&
	snapshot_cmd[0]) {

	int  a_len, b_len;
	char *cmd, *s, *d;

	a_len = strlen(filename);
	b_len = a_len - 4;	/* サフィックス ".???" の4文字分減算 */

	len = 0;
	s = snapshot_cmd;	/* コマンドの %a, %b は置換するので   */
	while (*s) {		/* コマンドの文字長がどうなるか数える */
	    if (*s == '%') {
		switch (*(s+1)) {
		case '%': len ++;	s++;	break; 
		case 'a': len += a_len;	s++;	break; 
		case 'b': len += b_len;	s++;	break; 
		default:  len ++;		break; 
		}
	    } else {      len ++; }

	    s++;
	}
				/* 数えた文字数分、malloc する */
	cmd = (char *)malloc(len + 1);
	if (cmd) {

	    s = snapshot_cmd;
	    d = cmd;
	    while (*s) {	/* コマンドの %a, %b を置換して格納していく */
		if (*s == '%') {
		    switch (*(s+1)) {
		    case '%': *d++ = *s;			    s++; break;
		    case 'a': memcpy(d, filename, a_len); d+=a_len; s++; break;
		    case 'b': memcpy(d, filename, b_len); d+=b_len; s++; break;
		    default:  *d++ = *s;
		    }
		} else {      *d++ = *s; }
		s++;
	    }
	    *d = '\0';
				/* 出来上がったコマンドを実行 */
	    printf("[SNAPSHOT command]%% %s\n", cmd);
	    system(cmd);

	    free(cmd);
	}
    }
#endif	/* USE_SSS_CMD */

    return success;
}





/***********************************************************************
 * サウンド出力をセーブする
 *
 ************************************************************************/
#include "snddrv.h"

char	file_wav[QUASI88_MAX_FILENAME];		/* サウンド出力ベース部	*/

static const char *wav_suffix[] = { ".wav",  ".WAV", 
				     NULL
};


void		filename_set_wav_base(const char *filename)
{
    if (filename) {
	strncpy(file_wav, filename, QUASI88_MAX_FILENAME - 1);
	file_wav[ QUASI88_MAX_FILENAME - 1 ] = '\0';

	truncate_filename(file_wav, wav_suffix);
    } else {
	filename_init_wav(FALSE);
    }
}
const char	*filename_get_wav_base(void)
{
    return file_wav;
}



int	waveout_save_start(void)
{
    static char filename[ QUASI88_MAX_FILENAME + sizeof("NNNN.suffix") ];
    static int waveout_no = 0;		/* 連番 */

    static const char *suffix = ".wav";

    int i, j, len, success;


	/* ファイル名が未指定の場合、初期ファイル名にする */

    if (file_wav[0] == '\0') {
	filename_init_wav(FALSE);
    }


	/* file_wav[] の末端が NNNN.suffix なら削除 */

    truncate_filename(file_wav, wav_suffix);


	/* 存在しないファイル名を探しだす (0000.suffix〜 9999.suffix) */

    success = FALSE;
    for (j=0; j<10000; j++) {

	len = sprintf(filename, "%s%04d", file_wav, waveout_no);
	if (++ waveout_no > 9999) waveout_no = 0;

	for (i=0; wav_suffix[i]; i++) {
	    filename[ len ] = '\0';
	    strcat(filename, wav_suffix[ i ]);
	    if (osd_file_stat(filename) != FILE_STAT_NOEXIST) break;
	}
	if (wav_suffix[i] == NULL) {	    /* 見つかった */
	    filename[ len ] = '\0';
	    strcat(filename, suffix);
	    success = TRUE;
	    break;
	}
    }


	/* ファイル名が決まったので、ファイルをを開く */

    if (success) {
	success = xmame_wavout_open(filename);
    }

    return success;
}

void	waveout_save_stop(void)
{
/*  if (xmame_wavout_opened())*/
	xmame_wavout_close();
}
