/***********************************************************************
 *			ファイル名制御／管理
 ************************************************************************/
#include "getconf.h"

char	file_disk[2][QUASI88_MAX_FILENAME];	/*ディスクイメージファイル名*/
int	image_disk[2];	 	  		/*イメージ番号0〜31,-1は自動*/
int	readonly_disk[2];			/*リードオンリーで開くなら真*/

char	file_tape[2][QUASI88_MAX_FILENAME];	/* テープ入出力のファイル名 */
char	file_prn[QUASI88_MAX_FILENAME];		/* パラレル出力のファイル名 */
char	file_sin[QUASI88_MAX_FILENAME];		/* シリアル出力のファイル名 */
char	file_sout[QUASI88_MAX_FILENAME];	/* シリアル入力のファイル名 */

int	file_coding = 0;			/* ファイル名の漢字コード   */
int	filename_synchronize = TRUE;		/* ファイル名を同調させる   */



static char *assemble_filename(const char *imagename,
			       const char *basedir,
			       const char *suffix);

/*----------------------------------------------------------------------
 * 変数 file_XXX[] に設定されているファイルをすべて開く。
 *	通常は、ファイルを開く際に file_XXX[] を設定するのだが、
 *	以下の場合は、先に file_XXX[] が設定されてしまっているので、
 *	この関数を呼び出す必要がある。
 *
 *	・起動時          …オプションで file_XXX[] が設定される
 *	・ステートロード時…変数 file_XXX[] が復元される。
 *			    (起動時のオプションでステートロードした場合も同様)
 *
 *	ステートロード時は、 stateload を真にして、呼び出す。
 *----------------------------------------------------------------------*/
static	void	imagefile_all_open(int stateload)
{
    int err0 = TRUE;
    int err1 = TRUE;
    int err2 = TRUE;
    int err3 = TRUE;

    if (stateload == FALSE) {
	int i;
	for (i=0; i<NR_DRIVE; i++) {
	    memset(file_disk[i], 0, QUASI88_MAX_FILENAME);
	    if (config_image.d[i]) {
		strcpy(file_disk[i], config_image.d[i]);
	    }
	    image_disk[i]    = config_image.n[i];
	    readonly_disk[i] = config_image.ro[i];
	}

	for (i=0; i<NR_TAPE; i++) {
	    memset(file_tape[i], 0, QUASI88_MAX_FILENAME);
	    if (config_image.t[i]) {
		strcpy(file_tape[i], config_image.t[i]);
	    }
	}
    }
    /* ステートロード時は、 file_XXX は設定済み */
    /* が、以下は現在ステートセーブしていないので、無条件で設定 */
    {
	memset(file_prn, 0, QUASI88_MAX_FILENAME);
	if (config_image.prn) {
	    strcpy(file_prn, config_image.prn);
	}

	memset(file_sin, 0, QUASI88_MAX_FILENAME);
	if (config_image.sin) {
	    strcpy(file_sin, config_image.sin);
	}

	memset(file_sout, 0, QUASI88_MAX_FILENAME);
	if (config_image.sout) {
	    strcpy(file_sout, config_image.sout);
	}
    }


    if (file_disk[0][0] &&	/* ドライブ1,2 ともイメージ指定済みの場合 */
	file_disk[1][0]) {		/*	% quasi88 file file       */
					/*	% quasi88 file m m        */
					/*	% quasi88 file n file     */
					/*	% quasi88 file file m     */
					/*	% quasi88 file n file m   */
	int same = (strcmp(file_disk[0], file_disk[1]) == 0) ? TRUE : FALSE;

	err0 = disk_insert(DRIVE_1,		/* ドライブ 1 をセット */
			   file_disk[0],
			   (image_disk[0] < 0) ? 0 : image_disk[0],
			   readonly_disk[0]);

	if (same) {				/* 同一ファイルの場合は */

	    if (err0 == FALSE) {			/* 1: → 2: 転送 */
		err1 = disk_insert_A_to_B(DRIVE_1, DRIVE_2, 
					  (image_disk[1] < 0) ? 0
							      : image_disk[1]);
	    }

	} else {				/* 別ファイルの場合は */

	    err1 = disk_insert(DRIVE_2,			/* ドライブ2 セット */
			       file_disk[1],
			       (image_disk[1] < 0) ? 0 : image_disk[1],
			       readonly_disk[1]);
	}

	/* 両ドライブで同じファイル かつ イメージ指定自動の場合の処理 */
	if (err0 == FALSE &&
	    err1 == FALSE &&
	    drive[DRIVE_1].fp == drive[DRIVE_2].fp && 
	    image_disk[0] < 0 && image_disk[1] < 0) {
	    disk_change_image(DRIVE_2, 1);		/* 2: は イメージ2へ */
	}

    } else if (file_disk[0][0]) {/* ドライブ1 だけ イメージ指定済みの場合 */
					/*	% quasi88 file		 */
					/*	% quasi88 file num       */
	err0 = disk_insert(DRIVE_1,
			   file_disk[0],
			   (image_disk[0] < 0) ? 0 : image_disk[0],
			   readonly_disk[0]);

	if (err0 == FALSE) {
	    if (image_disk[0] < 0 &&		/* イメージ番号指定なしなら */
		disk_image_num(DRIVE_1) >= 2) {	/* ドライブ2にもセット      */

		err1 = disk_insert_A_to_B(DRIVE_1, DRIVE_2, 1);
		if (err1 == FALSE) {
		    memcpy(file_disk[1], file_disk[0], QUASI88_MAX_FILENAME);
		}
	    }
	}

    } else if (file_disk[1][0]) {/* ドライブ2 だけ イメージ指定済みの場合 */
					/*	% quasi88 noexist file	 */
	err1 = disk_insert(DRIVE_2,
			   file_disk[1],
			   (image_disk[1] < 0) ? 0 : image_disk[1],
			   readonly_disk[1]);
    }



    /* オープンしなかった(出来なかった)場合は、ファイル名をクリア */
    if (err0) memset(file_disk[ 0 ], 0, QUASI88_MAX_FILENAME);
    if (err1) memset(file_disk[ 1 ], 0, QUASI88_MAX_FILENAME);


    /* その他のイメージファイルもセット */
    if (file_tape[CLOAD][0]) { err2 = sio_open_tapeload(file_tape[CLOAD]); }
    if (file_tape[CSAVE][0]) { err3 = sio_open_tapesave(file_tape[CSAVE]); }
    if (file_sin[0])         {        sio_open_serialin(file_sin);         }
    if (file_sout[0])        {        sio_open_serialout(file_sout);       }
    if (file_prn[0])         {        printer_open(file_prn);              }
	    /* これらは、ステートロードでもSEEKしてない。どうしよう？ */


    /* ファイル名にあわせて、スナップショットファイル名も設定 */
    if (filename_synchronize) {
	if (err0 == FALSE || err1 == FALSE /*|| err2 == FALSE*/) {
	    if (stateload == FALSE) {
		filename_init_state(TRUE);
	    }
	    filename_init_snap(TRUE);
	    filename_init_wav(TRUE);
	}
    }


    if (verbose_proc) {
	int i;
	for (i=0; i<2; i++) {
	    if (disk_image_exist(i)) {
		printf("DRIVE %d: <= %s [%d]\n", i+1,
		       file_disk[i], disk_image_selected(i)+1);
	    } else {
		printf("DRIVE %d: <= (empty)\n", i+1);
	    }
	}
    }
}

static	void	imagefile_all_close(void)
{
    disk_eject(0);          memset(file_disk[0],     0, QUASI88_MAX_FILENAME);
    disk_eject(1);          memset(file_disk[1],     0, QUASI88_MAX_FILENAME);

    sio_close_tapeload();   memset(file_tape[CLOAD], 0, QUASI88_MAX_FILENAME);
    sio_close_tapesave();   memset(file_tape[CSAVE], 0, QUASI88_MAX_FILENAME);
    sio_close_serialin();   memset(file_sin,         0, QUASI88_MAX_FILENAME);
    sio_close_serialout();  memset(file_sout,        0, QUASI88_MAX_FILENAME);
    printer_close();        memset(file_prn,         0, QUASI88_MAX_FILENAME);

#if 0	/* 閉じる時は、ファイル名同期は考慮不要 … か? */
    if (filename_synchronize) {
	filename_init_state(TRUE);
	filename_init_snap(TRUE);
	filename_init_wav(TRUE);
    }
#endif
}




/***********************************************************************
 *
 *
 ************************************************************************/
const char	*filename_get_disk(int drv)
{
    if (file_disk[drv][0] != '\0') return file_disk[drv];
    else                           return NULL;
}
const char	*filename_get_tape(int mode)
{
    if (file_tape[mode][0] != '\0') return file_tape[mode];
    else                            return NULL;
}
const char	*filename_get_prn(void)
{
    if (file_prn[0] != '\0') return file_prn;
    else                     return NULL;
}
const char	*filename_get_sin(void)
{
    if (file_sin[0] != '\0') return file_sin;
    else                     return NULL;
}
const char	*filename_get_sout(void)
{
    if (file_sout[0] != '\0') return file_sout;
    else                      return NULL;
}

/* 指定ドライブないし反対ドライブにディスクがあれば、そのファイル名を、
   なければ、ディスク用ディレクトリを返す */
const char	*filename_get_disk_or_dir(int drv)
{
    const char *p;

    if      (file_disk[drv  ][0] != '\0') p = file_disk[drv  ];
    else if (file_disk[drv^1][0] != '\0') p = file_disk[drv^1];
    else {
	p = osd_dir_disk();
	if (p == NULL) p = osd_dir_cwd();
    }

    return p;
}
/* 指定された区分のテープがセットされていれば、そのファイル名を、
   なければ、テープ用ディレクトリを返す */
const char	*filename_get_tape_or_dir(int mode)
{
    const char *p;

    if (file_tape[ mode ][0] != '\0') p = file_tape[ mode ];
    else {
	p = osd_dir_tape();
	if (p == NULL) p = osd_dir_cwd();
    }

    return p;
}

const char	*filename_get_disk_name(int drv)
{
           char  dir[ OSD_MAX_FILENAME ];
    static char file[ OSD_MAX_FILENAME ];

    if (file_disk[drv][0]) {
	if (osd_path_split(file_disk[drv], dir, file, OSD_MAX_FILENAME)) {
	    return file;
	}
    }
    return NULL;
}
const char	*filename_get_tape_name(int mode)
{
           char  dir[ OSD_MAX_FILENAME ];
    static char file[ OSD_MAX_FILENAME ];

    if (file_tape[mode][0]) {
	if (osd_path_split(file_tape[mode], dir, file, OSD_MAX_FILENAME)) {
	    return file;
	}
    }
    return NULL;
}













/***********************************************************************
 * ステートファイル、スナップショットファイルのファイル名に、
 * 初期文字列をセットする。
 *
 *	引数 set_default
 *		真なら、初期値をセットする。
 *		偽なら、イメージファイル名に応じた値をセットする。
 ************************************************************************/

void	filename_init_state(int synchronize)
{
    char *s, *buf;
    const char *dir;

    dir = osd_dir_state();
    if (dir == NULL) dir = osd_dir_cwd();

    memset(file_state, 0, QUASI88_MAX_FILENAME);

    if (synchronize) {
	if      (file_disk[0][0]     != '\0') s = file_disk[0];
	else if (file_disk[1][0]     != '\0') s = file_disk[1];
/*	else if (file_tape[CLOAD][0] != '\0') s = file_tape[CLOAD];*/
/*	else if (file_tape[CSAVE][0] != '\0') s = file_tape[CSAVE];*/
	else                                  s = STATE_FILENAME;
    } else {
	s = STATE_FILENAME;
    }

    buf = assemble_filename(s, dir, STATE_SUFFIX);

    if (buf) {
	if (strlen(buf) < QUASI88_MAX_FILENAME) {
	    strcpy(file_state, buf);
	    return;
	}
    }

    /* 途中でエラーになったら、適当な値をセット */
    strcpy(file_state, STATE_FILENAME STATE_SUFFIX);
}



void	filename_init_snap(int synchronize)
{
    char *s, *buf;
    const char *dir;

    dir = osd_dir_snap();
    if (dir == NULL) dir = osd_dir_cwd();

    memset(file_snap, 0, QUASI88_MAX_FILENAME);

    if (synchronize) {
	if      (file_disk[0][0]     != '\0') s = file_disk[0];
	else if (file_disk[1][0]     != '\0') s = file_disk[1];
/*	else if (file_tape[CLOAD][0] != '\0') s = file_tape[CLOAD];*/
/*	else if (file_tape[CSAVE][0] != '\0') s = file_tape[CSAVE];*/
	else                                  s = SNAPSHOT_FILENAME;
    } else {
	s = SNAPSHOT_FILENAME;
    }

    buf = assemble_filename(s, dir, "");

    if (buf) {
	if (strlen(buf) < QUASI88_MAX_FILENAME) {
	    strcpy(file_snap, buf);
	    return;
	}
    }

    /* 途中でエラーになったら、適当な値をセット */
    strcpy(file_snap, SNAPSHOT_FILENAME);
}



void	filename_init_wav(int synchronize)
{
    char *s, *buf;
    const char *dir;

    dir = osd_dir_snap();
    if (dir == NULL) dir = osd_dir_cwd();

    memset(file_wav, 0, QUASI88_MAX_FILENAME);

    if (synchronize) {
	if      (file_disk[0][0]     != '\0') s = file_disk[0];
	else if (file_disk[1][0]     != '\0') s = file_disk[1];
/*	else if (file_tape[CLOAD][0] != '\0') s = file_tape[CLOAD];*/
/*	else if (file_tape[CSAVE][0] != '\0') s = file_tape[CSAVE];*/
	else                                  s = WAVEOUT_FILENAME;
    } else {
	s = WAVEOUT_FILENAME;
    }

    buf = assemble_filename(s, dir, "");

    if (buf) {
	if (strlen(buf) < QUASI88_MAX_FILENAME) {
	    strcpy(file_wav, buf);
	    return;
	}
    }

    /* 途中でエラーになったら、適当な値をセット */
    strcpy(file_wav, WAVEOUT_FILENAME);
}




/***********************************************************************
 * 各種ファイルのフルパスを取得
 *	引数の処理（機種非依存＋依存）および、
 *	各種設定の処理 (機種依存部) から呼び出される・・・ハズ
 *
 *		指定の ディスクイメージ名の、フルパスを取得
 *		指定の ROMイメージ名の、     フルパスを取得
 *		指定の 共通設定ファイル名の、フルパスを取得
 *		指定の 個別設定ファイル名の、フルパスを取得
 *
 *	成功時は、 char * (mallocされた領域)、失敗時は NULL
 ************************************************************************/

/*
 * あるイメージのファイル名 imagename のディレクトリ部と拡張子部を
 * 取り除いたベース名を取り出し、
 * basedir と ベース名 と suffix を結合したファイル名を返す。
 *
 *	例)
 *	imagename … /my/disk/dir/GAMEDISK.d88
 *                                ^^^^^^^^	これ取り出して、
 *	basedir   …     /new/dir		これと
 *	suffix    …                      .dat	これをくっつけて
 *	返り値    …     /new/dir/GAMEDISK.dat	これと返す
 *
 * この返ってくる領域は、静的な領域なので注意 !
 */
static char *assemble_filename(const char *imagename,
			       const char *basedir,
			       const char *suffix)
{
    static char buf[ OSD_MAX_FILENAME ];
           char file[ OSD_MAX_FILENAME ];

    if (osd_path_split(imagename, buf, file, OSD_MAX_FILENAME)) {

	size_t len = strlen(file);

	if (len >= 4) {
	    if (strcmp(&file[ len-4 ], ".d88") == 0 ||
		strcmp(&file[ len-4 ], ".D88") == 0 ||
		strcmp(&file[ len-4 ], ".t88") == 0 ||
		strcmp(&file[ len-4 ], ".T88") == 0 ||
		strcmp(&file[ len-4 ], ".cmt") == 0 ||
		strcmp(&file[ len-4 ], ".CMT") == 0) {

		file[ len-4 ] = '\0';
	    }
	}

	if (strlen(file) + strlen(suffix) + 1 < OSD_MAX_FILENAME) {

	    strcat(file, suffix);

	    if (osd_path_join(basedir, file, buf, OSD_MAX_FILENAME)) {
		return buf;
	    }
	}
    }

    return NULL;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*
 * ディスクイメージファイルのファイル名を取得する
 *	引数 filename で与えられたディスクイメージファイル名を
 *	補完して、最終的なファイル名返す。このファイル名は malloc された
 *	領域なので、必要に応じて呼び出し元は free すること。
 */
char	*filename_alloc_diskname(const char *filename)
{
    char *p;
    char dir [ OSD_MAX_FILENAME ];
    char file[ OSD_MAX_FILENAME ];
    const char *base;
    OSD_FILE *fp;
    int step;

		/* filename を dir と file に分ける */

    if (osd_path_split(filename, dir, file, OSD_MAX_FILENAME)) {

	if (dir[0] == '\0') {
		/* dir が空、つまり filename にパスの区切りが含まれない */

	    step = 0;	/* dir_disk + filename で ファイル有無を判定	*/

	} else {
		/* filename にパス区切りが含まれる			*/

	    step = 1;	/* dir_cwd + filename で ファイル有無をチェック	*/
			/*	(filenameが絶対パスなら、 filename	*/
			/*	 そのものでファイル有無チェックとなる)	*/
	}

    } else {
	return NULL;
    }


		/* step 0 → step 1 の順に、ファイル有無チェック */

    for ( ; step < 2; step ++) {

	if (step == 0) base = osd_dir_disk();
	else           base = osd_dir_cwd();

	if (base == NULL) continue;

	if (osd_path_join(base, filename, file, OSD_MAX_FILENAME) == FALSE) {
	    return NULL;
	}

			/* 実際に open できるかをチェックする */
	fp = osd_fopen(FTYPE_DISK, file, "rb");
	if (fp) {
	    osd_fclose(fp);

	    p = (char *)malloc(strlen(file) + 1);
	    if (p) {
		strcpy(p, file);
		return p;
	    }
	    break;
	}
    }

    return NULL;
}





char	*filename_alloc_romname(const char *filename)
{
  char *p;
  char buf[ OSD_MAX_FILENAME ];
  OSD_FILE *fp;
  int step;
  const char *dir = osd_dir_rom(); 

	/* step 0 … filenameがあるかチェック			*/
	/* step 1 … dir_rom に、 filename があるかチェック	*/

  for( step=0; step<2; step++ ){

    if( step==0 ){

      if( OSD_MAX_FILENAME <= strlen(filename) ) return NULL;
      strcpy( buf, filename );

    }else{

      if( dir == NULL ||
	  osd_path_join( dir, filename, buf, OSD_MAX_FILENAME ) == FALSE ){

	return NULL;
      }
    }

		/* 実際に open できるかをチェックする */
    fp = osd_fopen( FTYPE_ROM, buf, "rb" );
    if( fp ){
      osd_fclose( fp );

      p = (char *)malloc( strlen(buf) + 1 );
      if( p ){
	strcpy( p, buf );
	return p;
      }
      break;
    }
  }
  return NULL;
}





char	*filename_alloc_global_cfgname(void)
{
    const char *dir  = osd_dir_gcfg();
    const char *file = CONFIG_FILENAME  CONFIG_SUFFIX;
    char *p;
    char buf[ OSD_MAX_FILENAME ];

    if (dir == NULL ||
	osd_path_join(dir, file, buf, OSD_MAX_FILENAME) == FALSE) {

	return NULL;
    }

    p = (char *)malloc(strlen(buf) + 1);
    if (p) {
	strcpy(p, buf);
    }
    return p;
}

char	*filename_alloc_local_cfgname(const char *imagename)
{
    char *p   = NULL;
    char *buf;
    const char *dir = osd_dir_lcfg();

    if (dir == NULL) return NULL;

    buf = assemble_filename(imagename, dir, CONFIG_SUFFIX);

    if (buf) {
	p = (char *)malloc(strlen(buf) + 1);
	if (p) {
	    strcpy(p, buf);
	}
    }
    return p;
}




char	*filename_alloc_keyboard_cfgname(void)
{
  const char *dir  = osd_dir_gcfg();
  const char *file = KEYCONF_FILENAME  CONFIG_SUFFIX;
  char *p;
  char buf[ OSD_MAX_FILENAME ];


  if( dir == NULL ||
      osd_path_join( dir, file, buf, OSD_MAX_FILENAME ) == FALSE )

    return NULL;

  p = (char *)malloc( strlen(buf) + 1 );
  if( p ){
    strcpy( p, buf );
    return p;
  }

  return NULL;
}
