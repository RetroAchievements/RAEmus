/*****************************************************************************/
/* ファイル操作に関する処理						     */
/*									     */
/*	仕様の詳細は、ヘッダファイル file-op.h 参照			     */
/*									     */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"
#include "menu.h"


/*****************************************************************************/

/* 以下のディレクトリ名は、予め OSD_MAX_FILENAME バイトの
   固定長バッファを確保して、それにわりあてる。
	(malloc/free で動的に管理する方がスマートかもしれないけど…)	*/

static char *dir_cwd;	/* デフォルトのディレクトリ (カレント)		*/
static char *dir_rom;	/* ROMイメージファイルの検索ディレクトリ	*/
static char *dir_disk;	/* DISKイメージファイルの検索ディレクトリ	*/
static char *dir_tape;	/* TAPEイメージファイルの基準ディレクトリ	*/
static char *dir_snap;	/* 画面スナップショットファイルの保存先		*/
static char *dir_state;	/* サスペンドファイルの保存先			*/
static char *dir_g_cfg;	/* 共通設定ファイルのディレクトリ		*/
static char *dir_l_cfg;	/* 個別設定ファイルのディレクトリ		*/



/****************************************************************************
 * 各種ディレクトリの取得	(osd_dir_cwd は NULLを返してはだめ !)
 *****************************************************************************/
const char *osd_dir_cwd  (void) { return dir_cwd;   }
const char *osd_dir_rom  (void) { return dir_rom;   }
const char *osd_dir_disk (void) { return dir_disk;  }
const char *osd_dir_tape (void) { return dir_tape;  }
const char *osd_dir_snap (void) { return dir_snap;  }
const char *osd_dir_state(void) { return dir_state; }
const char *osd_dir_gcfg (void) { return dir_g_cfg[0] ? dir_g_cfg : NULL; }
const char *osd_dir_lcfg (void) { return dir_l_cfg[0] ? dir_l_cfg : NULL; }

static int set_new_dir(const char *newdir, char *dir)
{
    if (strlen(newdir) < OSD_MAX_FILENAME) {
	strcpy(dir, newdir);
	return TRUE;
    }
    return FALSE;
}

int osd_set_dir_cwd  (const char *d) { return set_new_dir(d, dir_cwd);   }
int osd_set_dir_rom  (const char *d) { return set_new_dir(d, dir_rom);   }
int osd_set_dir_disk (const char *d) { return set_new_dir(d, dir_disk);  }
int osd_set_dir_tape (const char *d) { return set_new_dir(d, dir_tape);  }
int osd_set_dir_snap (const char *d) { return set_new_dir(d, dir_snap);  }
int osd_set_dir_state(const char *d) { return set_new_dir(d, dir_state); }
int osd_set_dir_gcfg (const char *d) { return set_new_dir(d, dir_g_cfg); }
int osd_set_dir_lcfg (const char *d) { return set_new_dir(d, dir_l_cfg); }







/****************************************************************************
 * ファイル名に使用されている漢字コードを取得
 *		0 … ASCII のみ
 *		1 … 日本語EUC
 *		2 … シフトJIS
 *		3 … UTF-8
 *****************************************************************************/
int	osd_kanji_code(void)
{
    if      (file_coding == 2) return 3;
    else if (file_coding == 1) return 2;
    else                       return 1;
}



/****************************************************************************
 * ファイル操作
 *
 * OSD_FILE *osd_fopen(int type, const char *path, const char *mode)
 * int	osd_fclose(OSD_FILE *stream)
 * int	osd_fflush(OSD_FILE *stream)
 * int	osd_fseek(OSD_FILE *stream, long offset, int whence)
 * long	osd_ftell(OSD_FILE *stream)
 * void	osd_rewind(OSD_FILE *stream)
 * size_t osd_fread(void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
 * size_t osd_fwrite(const void *ptr,size_t size,size_t nobj,OSD_FILE *stream)
 * int	osd_fputc(int c, OSD_FILE *stream)
 * int	osd_fgetc(OSD_FILE *stream)
 * char	*osd_fgets(char *str, int size, OSD_FILE *stream)
 * int	osd_fputs(const char *str, OSD_FILE *stream)
 *****************************************************************************/


/*
 * 全てのファイルに対して排他制御したほうがいいと思うけど、面倒なので、
 * ディスク・テープのイメージに関してのみ、多重にオープンしないようにする。
 *
 * osd_fopen が呼び出されたときに、ファイルの情報を stat にて取得し、
 * すでに開いているファイルの stat と一致しないかをチェックする。
 * ここで、ディスクイメージファイルの場合は、すでに開いているファイルの
 * ファイルポインタを返し、他の場合はオープン失敗として NULL を返す。
 */

struct OSD_FILE_STRUCT {

    FILE	*fp;			/* !=NULL なら使用中	*/
    struct stat	sb;			/* 開いたファイルの状態	*/
    int		type;			/* ファイル種別		*/
    char	mode[4];		/* 開いた際の、モード	*/

};

#define	MAX_STREAM	8
static	OSD_FILE	osd_stream[ MAX_STREAM ];



OSD_FILE *osd_fopen(int type, const char *path, const char *mode)
{
    int i;
    struct stat	sb;
    OSD_FILE	*st;
    int		stat_ok;

    st = NULL;
    for (i=0; i<MAX_STREAM; i++) {	/* 空きバッファを探す */
	if (osd_stream[i].fp == NULL) {		/* fp が NULL なら空き */
	    st = &osd_stream[i];
	    break;
	}
    }
    if (st == NULL) return NULL;		/* 空きがなければ NG */


    if (stat(path, &sb) != 0) {		/* ファイルの状態を取得する */
	if (mode[0] == 'r') return NULL;
	stat_ok = FALSE;
    } else {
	stat_ok = TRUE;
    }



    switch (type) {

    case FTYPE_DISK:		/* "r+b" , "rb"	*/
    case FTYPE_TAPE_LOAD:	/* "rb" 	*/
    case FTYPE_TAPE_SAVE:	/* "ab"		*/
    case FTYPE_PRN:		/* "ab"		*/
    case FTYPE_COM_LOAD:	/* "rb"		*/
    case FTYPE_COM_SAVE:	/* "ab"		*/

	if (stat_ok) {
	    /* すでに開いているファイルかどうかをチェックする */
	    for (i=0; i<MAX_STREAM; i++) {
		if (osd_stream[i].fp) {
		    if (osd_stream[i].sb.st_dev == sb.st_dev &&
			osd_stream[i].sb.st_ino == sb.st_ino) {

			/* DISKの場合かつ同じモードならばそれを返す */
			if (type == FTYPE_DISK                   &&
			    osd_stream[i].type == type           &&
			    strcmp(osd_stream[i].mode, mode) == 0) {

			    return &osd_stream[i];

			} else {
			    /* DISK以外、ないしモードが違うならばNG */
			    return NULL;
			}
		    }
		}
	    }
	}
	/* FALLTHROUGH */


    default:
	st->fp = fopen(path, mode);	/* ファイルを開く */

	if (st->fp) {

	    if (stat_ok == FALSE) {		/* もう一度ファイル状態を */
		fflush(st->fp);			/* 取得してみよう。       */
		if (stat(path, &sb) != 0) {	/* 必ず成功するはず……？ */
		    sb.st_dev = 0;
		    sb.st_ino = 0;
		}
	    }

	    st->type = type;
	    st->sb   = sb;
	    strncpy(st->mode, mode, sizeof(st->mode));
	    return st;

	} else {

	    return NULL;
	}
    }
}



int	osd_fclose(OSD_FILE *stream)
{
    FILE *fp = stream->fp;

    stream->fp = NULL;
    return fclose(fp);
}



int	osd_fflush(OSD_FILE *stream)
{
    if (stream == NULL) return fflush(NULL);
    else                return fflush(stream->fp);
}



int	osd_fseek(OSD_FILE *stream, long offset, int whence)
{
    return fseek(stream->fp, offset, whence);
}



long	osd_ftell(OSD_FILE *stream)
{
    return ftell(stream->fp);
}



void	osd_rewind(OSD_FILE *stream)
{
    (void)osd_fseek(stream, 0L, SEEK_SET);
    osd_fflush(stream);
}



size_t	osd_fread(void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
{
    return fread(ptr, size, nobj, stream->fp);
}



size_t	osd_fwrite(const void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
{
    return fwrite(ptr, size, nobj, stream->fp);
}



int	osd_fputc(int c, OSD_FILE *stream)
{
    return fputc(c, stream->fp);
}


int	osd_fgetc(OSD_FILE *stream)
{
    return fgetc(stream->fp);
}


char	*osd_fgets(char *str, int size, OSD_FILE *stream)
{
    return fgets(str, size, stream->fp);
}


int	osd_fputs(const char *str, OSD_FILE *stream)
{
    return fputs(str, stream->fp);
}



/****************************************************************************
 * ディレクトリ閲覧
 *****************************************************************************/

struct	T_DIR_INFO_STRUCT
{
    int		cur_entry;		/* 上位が取得したエントリ数	*/
    int		nr_entry;		/* エントリの全数		*/
    T_DIR_ENTRY	*entry;			/* エントリ情報 (entry[0]〜)	*/
};



/*
 * ディレクトリ内のファイル名のソーティングに使う関数
 */
static int namecmp(const void *p1, const void *p2)
{
    T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
    T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;

    return strcmp(s1->name, s2->name);
}




/*---------------------------------------------------------------------------
 * T_DIR_INFO *osd_opendir(const char *filename)
 *	opendir()、rewinddir()、readdir()、closedir() を駆使し、
 *	ディレクトリの全てのエントリのファイル名をワークにセットする。
 *	ワークは malloc で確保するが、失敗時はそこでエントリの取得を打ち切る。
 *	処理後は、このワークをファイル名でソートしておく。
 *---------------------------------------------------------------------------*/
T_DIR_INFO	*osd_opendir(const char *filename)
{
    int  len;
    char *p;
    int  i;
    T_DIR_INFO *dir;

    DIR *dirp;
    struct dirent *dp;

				/* T_DIR_INFO ワークを 1個確保 */
    if ((dir = (T_DIR_INFO *)malloc(sizeof(T_DIR_INFO))) == NULL) {
	return NULL;
    }

    if (filename == NULL || filename[0] == '\0') {
	filename = ".";
    }

    dirp = opendir(filename);	/* ディレクトリを開く */
    if (dirp == NULL) {
	free(dir);
	return NULL;
    }


    dir->nr_entry = 0;		/* ファイル数を数える */
    while (readdir(dirp)) {
	dir->nr_entry ++;
    }
    rewinddir(dirp);

				/* T_DIR_ENTRY ワークを ファイル数分 確保 */
    dir->entry = (T_DIR_ENTRY *)malloc(dir->nr_entry * sizeof(T_DIR_ENTRY));
    if (dir->entry == NULL) {
	closedir(dirp);
	free(dir);
	return NULL;
    }
    for (i=0; i<dir->nr_entry; i++) {
	dir->entry[i].name = NULL;
	dir->entry[i].str  = NULL;
    }

				/* ファイル数分、処理ループ (情報を格納) */
    for (i=0; i<dir->nr_entry; i++) {

	dp = readdir(dirp);		/* ファイル名取得 */

	if (dp == NULL) {			/* 取得に失敗したら、中断  */
	    dir->nr_entry = i;			/* (これは正常扱いとする。 */
	    break;				/*  おそらく途中でファイル */
	}					/*  が削除されたのだろう)  */

					/* ファイルの種類をセット */
	{
	    char *fullname;			/* ディレクトリ名(filename) */
	    struct stat sb;			/* とファイル名(dp->d_name) */
						/* からstat関数で属性取得。 */
						/* 失敗しても気にしない     */

	    dir->entry[i].type = FILE_STAT_FILE; /*  (失敗したら FILE 扱い) */

	    fullname = (char*)malloc(strlen(filename)+1 +strlen(dp->d_name)+1);

	    if (fullname) {
		sprintf(fullname, "%s%s%s", filename, "/", dp->d_name);

		if (stat(fullname, &sb) == 0) {
#if 1
		    if (S_ISDIR(sb.st_mode))
#else
		    if ((sb.st_mode & S_IFMT) == S_IFDIR)
#endif
		    {
			dir->entry[i].type = FILE_STAT_DIR;
		    } else {
			dir->entry[i].type = FILE_STAT_FILE;
		    }
		}
		free(fullname);
	    }
	}

					/* ファイル名バッファ確保 */

	len = strlen(dp->d_name) + 1;
	p = (char *)malloc(( len + 1 )  +  ( len + 1 ));
	if (p == NULL) { /* ↑ファイル名 と ↑表示名 のバッファを一気に確保 */
	    dir->nr_entry = i;
	    break;				/* malloc に失敗したら中断 */
	}

					/* ファイル名・表示名セット */
	dir->entry[i].name = &p[0];
	dir->entry[i].str  = &p[len+1];

	strcpy(dir->entry[i].name, dp->d_name);
	strcpy(dir->entry[i].str,  dp->d_name);

	if (dir->entry[i].type == FILE_STAT_DIR) { /* ディレクトリの場合、 */
	    strcat(dir->entry[i].str, "/");	   /* 表示名に / を付加    */
	}

    }


    closedir(dirp);		/* ディレクトリを閉じる */


				/* ファイル名をソート */
    qsort(dir->entry, dir->nr_entry, sizeof(T_DIR_ENTRY), namecmp);


#if 0	/* この処理は無し。表示名 abc/ でも ファイル名は abc のまま */
    /* ディレクトリの場合、ファイル名に / を付加 (ソート後に行う) */
    for (i=0; i<dir->nr_entry; i++) {
	if (dir->entry[i].type == FILE_STAT_DIR) {
	    strcat(dir->entry[i].name, "/");
	}
    }
#endif


				/* osd_readdir に備えて */
    dir->cur_entry = 0;
    return dir;
}



/*---------------------------------------------------------------------------
 * T_DIR_ENTRY *osd_readdir(T_DIR_INFO *dirp)
 *	osd_opendir() の時に確保した、エントリ情報ワークへのポインタを
 *	順次、返していく。
 *---------------------------------------------------------------------------*/
T_DIR_ENTRY	*osd_readdir(T_DIR_INFO *dirp)
{
    T_DIR_ENTRY	*ret_value = NULL;

    if (dirp->cur_entry != dirp->nr_entry) {
	ret_value = &dirp->entry[ dirp->cur_entry ];
	dirp->cur_entry ++;
    }
    return ret_value;
}



/*---------------------------------------------------------------------------
 * void osd_closedir(T_DIR_INFO *dirp)
 *	osd_opendir() 時に確保した全てのメモリを開放する。
 *---------------------------------------------------------------------------*/
void		osd_closedir(T_DIR_INFO *dirp)
{
    int	i;

    for (i=0; i<dirp->nr_entry; i++) {
	if (dirp->entry[i].name) {
	    free(dirp->entry[i].name);
	}
    }
    free(dirp->entry);
    free(dirp);
}



/****************************************************************************
 * パス名の操作
 *****************************************************************************/

/*---------------------------------------------------------------------------
 * int	osd_path_normalize(const char *path, char resolved_path[], int size)
 *
 *	処理内容:
 *		./ は削除、 ../ は親ディレクトリに置き換え、 /…/ は / に置換。
 *		面倒なので、リンクやカレントディレクトリは展開しない。
 *		末尾に / が残った場合、それは削除する。
 *	例:
 *		"../dir1/./dir2///dir3/../../file" → "../dir1/file"
 *---------------------------------------------------------------------------*/
int	osd_path_normalize(const char *path, char resolved_path[], int size)
{
    char *buf, *s, *d, *p;
    int is_abs, is_dir, success = FALSE;
    size_t len = strlen(path);

    if (len == 0) {
	if (size) { resolved_path[0] = '\0';  success = TRUE; }
    } else {

	is_abs = (path[0]     == '/') ? TRUE : FALSE;
	is_dir = (path[len-1] == '/') ? TRUE : FALSE;

	buf = (char *)malloc((len+3) * 2);	/* path と同サイズ位の	*/
	if (buf) {				/* バッファを2個分 確保	*/
	    strcpy(buf, path);
	    d = &buf[ len + 3 ];
	    d[0] = '\0';

	    s = strtok(buf, "/");		/* / で 区切っていく	*/

	    if (s == NULL) {			/* 区切れないなら、	*/
						/* それは / そのものだ  */
		if (size > 1) {
		    strcpy(resolved_path, "/");
		    success = TRUE;
		}

	    } else {				/* 区切れたなら、分析	*/

		for ( ; s ;  s = strtok(NULL, "/")) {

		    if        (strcmp(s, ".")  == 0) {	/* . は無視	*/
			;

		    } else if (strcmp(s, "..") == 0) {	/* .. は直前を削除 */

			p = strrchr(d, '/');		    /* 直前の/を探す */

			if (p && strcmp(p, "/..") != 0) {   /* 見つかれば    */
			    *p = '\0';			    /*    そこで分断 */
			} else {                            /* 見つからない  */
			    if (p == NULL && is_abs) {	    /*   絶対パスなら*/
				;			    /*     無視する  */
			    } else {			    /*   相対パスなら*/
				strcat(d, "/..");	    /*     .. にする */
			    }
			}

		    } else {				/* 上記以外は連結 */
			strcat(d, "/");			    /* 常に / を前置 */
			strcat(d, s);
		    }
		}

		if (d[0] == '\0') {		/* 結果が空文字列になったら */
		    if (is_abs) strcpy(d, "/");	/*   元が絶対パスなら /     */
		    /* else         ;		 *   元が相対パスから 空    */

		} else {
		    if (is_abs == FALSE) {	/* 元が相対パスなら */
			d ++;			/* 先頭の / を削除  */
		    }
#if 0	/* この処理は無し。元が a/b/c/ でも a/b/c とする */
		    if (is_dir) {		/* 元の末尾が / なら */
			strcat(d, "/");		/* 末尾に / を付加   */
		    }
#endif
		}

		if (strlen(d) < (size_t)size) {
		    strcpy(resolved_path, d);
		    success = TRUE;
		}
	    }

	    free(buf);
	}
    }

    /*printf("NORM:\"%s\" => \"%s\"\n",path,resolved_path);*/
    return success;
}



/*---------------------------------------------------------------------------
 * int	osd_path_split(const char *path, char dir[], char file[], int size)
 *
 *	処理内容:
 *		path の最後の / より前を dir、後ろを file にセットする
 *			dir の末尾に / はつかない。
 *		path の末尾が / なら、予め削除してから処理する
 *			よって、 file の末尾にも / はつかない。
 *		path は予め、正規化されているものとする。
 *---------------------------------------------------------------------------*/
int	osd_path_split(const char *path, char dir[], char file[], int size)
{
    int pos = strlen(path);

    /* dir, file は十分なサイズを確保しているはずなので、軽くチェック */
    if (pos == 0 || size <= pos) {
	dir[0]  = '\0';
	file[0] = '\0';
	strncat(file, path, size-1);
	if (pos) fprintf(stderr, "internal overflow %d\n", __LINE__);
	return FALSE;
    }


    if (strcmp(path, "/") == 0) {	/* "/" の場合、別処理	*/
	strcpy(dir, "/");			/* ディレクトリは "/"	*/
	strcpy(file, "");			/* ファイルは ""	*/
	return TRUE;
    }

    if (path[ pos - 1 ] == '/') {	/* path 末尾の / は無視	*/
	pos --;
    }

    do {				/* / を末尾から探す	*/
	if (path[ pos - 1 ] == '/') { break; }
	pos --;
    } while (pos);

    if (pos) {				/* / が見つかったら	*/
	strncpy(dir, path, pos);		/* 先頭〜 / までをコピー*/
	if (pos > 1)
	    dir[ pos - 1 ] = '\0';		/* 末尾の / は削除する	*/
	else					/* ただし		*/ 
	    dir[ pos ] = '\0';			/* "/"の場合は / は残す */

	strcpy(file, &path[pos]);

    } else {				/* / が見つからなかった	*/
	strcpy(dir,  "");			/* ディレクトリは ""	*/
	strcpy(file, path);			/* ファイルは path全て	*/
    }

    pos = strlen(file);			/* ファイル末尾の / は削除 */
    if (pos && file[ pos - 1 ] == '/') { 
	file[ pos - 1 ] = '\0';
    }

    /*printf("SPLT:\"%s\" = \"%s\" + \"%s\")\n",path,dir,file);*/
    return TRUE;
}



/*---------------------------------------------------------------------------
 * int	osd_path_join(const char *dir, const char *file, char path[], int size)
 *
 *	処理内容:
 *		file が / で始まっていたら、そのまま path にセット
 *		そうでなければ、"dir" + "/" + "file" を path にセット
 *		出来上がった path は正規化しておく
 *---------------------------------------------------------------------------*/
int	osd_path_join(const char *dir, const char *file, char path[], int size)
{
    int len;
    char *p;

    if (dir == NULL    ||
	dir[0] == '\0' ||			/* ディレクトリ名なし or  */
	file[0] == '/') {			/* ファイル名が、絶対パス */

	if ((size_t)size <= strlen(file)) { return FALSE; }
	strcpy(path, file);

    } else {					/* ファイル名は、相対パス */

	path[0] = '\0';
	strncat(path, dir, size - 1);

	len = strlen(path);				/* ディレクトリ末尾  */
	if (len && path[ len - 1 ] != '/') {		/* が '/' でないなら */
	    strncat(path, "/", size - len - 1);		/* 付加する          */
	}

	len = strlen(path);
	strncat(path, file, size - len - 1);

    }


    p = (char *)malloc(size);			/* 正規化しておこう */
    if (p) {
	strcpy(p, path);
	if (osd_path_normalize(p, path, size) == FALSE) {
	    strcpy(path, p);
	}
	free(p);
    }

    /*printf("JOIN:\"%s\" + \"%s\" = \"%s\"\n",dir,file,path);*/
    return TRUE;
}



/****************************************************************************
 * ファイル属性の取得
 ****************************************************************************/
#if 1

int	osd_file_stat(const char *pathname)
{
    struct stat sb;

    if (stat(pathname, &sb)) {
	return FILE_STAT_NOEXIST;
    }

    if (S_ISDIR(sb.st_mode)) {
	return FILE_STAT_DIR;
    } else {
	return FILE_STAT_FILE;
    }
}

#else
int	osd_file_stat(const char *pathname)
{
    DIR  *dirp;
    FILE *fp;

    if ((dirp = opendir(pathname))) {		/* ディレクトリとして開く */
	closedir(dirp);				/* 成功したらディレクトリ */
	return FILE_STAT_DIR;
    } else {
	if ((fp = fopen(pathname, "r"))) {	/* ファイルとして開く     */
	    fclose(fp);				/* 成功したらファイル	  */
	    return FILE_STAT_FILE;
	} else {
	    return FILE_STAT_NOEXIST;		/* どちらとも失敗	  */
	}
    }
}
#endif






/****************************************************************************
 * int	osd_file_config_init(void)
 *
 *	この関数は、起動後に1度だけ呼び出される。
 *	正常終了時は真を、 malloc に失敗したなど異常終了時は偽を返す。
 *
 ****************************************************************************/
static int parse_tilda(const char *home, const char *path,
		       char *result_path, int result_size);
static int make_dir(const char *dname);

int	osd_file_config_init(void)
{
    char *s;
    char *home  = NULL;
    char *g_cfg = NULL;
    char *l_cfg = NULL;
    char *state = NULL;

	/* ワークを確保 (固定長で処理する予定なので静的確保でもいいんだけど) */

    dir_cwd   = (char *)malloc(OSD_MAX_FILENAME);
    dir_rom   = (char *)malloc(OSD_MAX_FILENAME);
    dir_disk  = (char *)malloc(OSD_MAX_FILENAME);
    dir_tape  = (char *)malloc(OSD_MAX_FILENAME);
    dir_snap  = (char *)malloc(OSD_MAX_FILENAME);
    dir_state = (char *)malloc(OSD_MAX_FILENAME);
    dir_g_cfg = (char *)malloc(OSD_MAX_FILENAME);
    dir_l_cfg = (char *)malloc(OSD_MAX_FILENAME);


    if (! dir_cwd  || ! dir_rom   || ! dir_disk  || ! dir_tape || 
	! dir_snap || ! dir_state || ! dir_g_cfg || ! dir_l_cfg) return FALSE;



	/* カレントワーキングディレクトリ名 (CWD) を取得する */

    if (getcwd(dir_cwd, OSD_MAX_FILENAME-1)) {
	dir_cwd[ OSD_MAX_FILENAME-1 ] = '\0';
    } else {
	fprintf(stderr, "error: can't get CWD\n");
	strcpy(dir_cwd, "");
    }


	/* ホームディレクトリ $(HOME) を取得する */

    home = getenv("HOME");

    if (home    == NULL ||			/* 未定義とか絶対パスで	*/
	home[0] != '/') {			/* ない場合は NG	*/
	fprintf(stderr, "error: can't get HOME\n");
	home = NULL;

    } else {

	/* $(HOME)/.quasi88/以下のディレクトリを作成 */

#define	HOME_QUASI88		"/.quasi88"
#define	HOME_QUASI88_RC		"/.quasi88/rc"
#define	HOME_QUASI88_STATE	"/.quasi88/state"

	s = malloc(strlen(home) + sizeof(HOME_QUASI88) + 1);
	if (s) {
	    sprintf(s, "%s%s", home, HOME_QUASI88);

	    if (make_dir(s)) {
		g_cfg = s;
	    } else {
		free(s);
	    }
	}

	s = malloc(strlen(home) + sizeof(HOME_QUASI88_RC) + 1);
	if (s) {
	    sprintf(s, "%s%s", home, HOME_QUASI88_RC);

	    if (make_dir(s)) {
		l_cfg = s;
	    } else {
		free(s);
	    }
	}

	s = malloc(strlen(home) + sizeof(HOME_QUASI88_STATE) + 1);
	if (s) {
	    sprintf(s, "%s%s", home, HOME_QUASI88_STATE);

	    if (make_dir(s)) {
		state = s;
	    } else {
		free(s);
	    }
	}

    }



	/* ROMディレクトリを設定する */

    s = getenv("QUASI88_ROM_DIR");		/* $(QUASI88_ROM_DIR)	*/
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_rom, s);
    } else {
	if (parse_tilda(home, ROM_DIR, dir_rom, OSD_MAX_FILENAME) == 0) {
	    strcpy(dir_rom, dir_cwd);
	}
    }


	/* DISKディレクトリを設定する */

    s = getenv("QUASI88_DISK_DIR");		/* $(QUASI88_DISK_DIR) */
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_disk, s);
    } else {
	if (parse_tilda(home, DISK_DIR, dir_disk, OSD_MAX_FILENAME) == 0) {
	    strcpy(dir_disk, dir_cwd);
	}
    }


	/* TAPEディレクトリを設定する */

    s = getenv("QUASI88_TAPE_DIR");		/* $(QUASI88_TAPE_DIR) */
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_tape, s);
    } else {
	if (parse_tilda(home, TAPE_DIR, dir_tape, OSD_MAX_FILENAME) == 0) {
	    strcpy(dir_tape, dir_cwd);
	}
    }


	/* SNAPディレクトリを設定する */

    s = getenv("QUASI88_SNAP_DIR");		/* $(QUASI88_SNAP_DIR) */
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_snap, s);
    } else {
	strcpy(dir_snap, dir_cwd);
    }


	/* STATEディレクトリを設定する */

    s = getenv("QUASI88_STATE_DIR");		/* $(QUASI88_STATE_DIR) */
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_state, s);
    } else {
	if (state && strlen(state) < OSD_MAX_FILENAME) {
	    strcpy(dir_state, state);
	} else {
	    strcpy(dir_state, dir_cwd);
	}
    }


	/* 全体設定ディレクトリを設定する */

    s = g_cfg;
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_g_cfg, s);
    } else {
	strcpy(dir_g_cfg, "");
    }


	/* 個別設定ディレクトリを設定する */

    s = l_cfg;
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_l_cfg, s);
    } else {
	strcpy(dir_l_cfg, "");
    }



    if (g_cfg) free(g_cfg);
    if (l_cfg) free(l_cfg);
    if (state) free(state);

    return TRUE;
}


/*
 * path が ~ で始まっていたら、 home に置き換えて result_path に格納する。
 *	何らかの理由で格納できなかったら、偽を返す。
 */

static int parse_tilda(const char *home, const char *path,
		       char *result_path, int result_size)
{
    int  i;
    char *buf;

    if (home           &&
	home[0] == '/' &&	/* home が / で始まっていて、   */
	path[0] == '~') {	/* path が ~ で始まっている場合	*/

	buf = (char *)malloc(strlen(home) + strlen(path) + 2);
	if (buf == NULL)
	    return FALSE;

	if (path[1] == '/'  ||		/* path が ~/ や ~/xxx や ~ の場合 */
	    path[1] == '\0') {

	    sprintf(buf, "%s%s%s", home, "/", &path[1]);

	} else {			/* path が ~xxx や ~xxx/ の場合 */

	    strcpy(buf, home);			/* home から最後のディレク */
	    i = strlen(buf) - 1;		/* トリ部を削り切り取ろう  */

	    while (0<=i && buf[i] == '/') {i--;}  /* 末尾の / を全てスキップ */
	    while (0<=i && buf[i] != '/') {i--;}  /* / 以外を全てスキップ    */
	    while (0<=i && buf[i] == '/') {i--;}  /* さらに / を全てスキップ */
						  /*   (全部スキップして     */
	    buf[i+1] = '\0';			  /*    しまったら / になる) */

	    strcat(buf, "/");
	    strcat(buf, &path[1]);
	}

	osd_path_normalize(buf, result_path, result_size);

	free(buf);
	return TRUE;

    } else {			/* home が / で始まらない、 path が ~ で…… */

	if (strlen(path) < (size_t)result_size) {

	    strcpy(result_path, path);
	    return TRUE;

	} else {
	    return FALSE;
	}
    }
}



/*
 *	ディレクトリ dname があるかチェック。無ければ作る。
 *		成功したら、真を返す
 */
static int make_dir(const char *dname)
{
    struct stat sb;

    if (stat(dname, &sb)) {

	if (errno == ENOENT) {			/* ディレクトリ存在しない */

	    if (mkdir(dname, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH)) { /*mode==0775*/
		fprintf(stderr, "error: can't make dir %s\n", dname);
		return FALSE;
	    } else {
		printf("make dir \"%s\"\n", dname);
	    }

	} else {				/* その他の異常 */
	    return FALSE;
	}

    } else {					/* ディレクトリあった */

	if (! S_ISDIR(sb.st_mode)) {			/* と思ったらファイル*/
	    fprintf(stderr, "error: not exist dir %s\n", dname);
	    return FALSE;
	}

    }

    return TRUE;
}


/****************************************************************************
 * int	osd_file_config_exit(void)
 *
 *	この関数は、終了後に1度だけ呼び出される。
 *
 ****************************************************************************/
void	osd_file_config_exit(void)
{
    if (dir_cwd)   free(dir_cwd);
    if (dir_rom)   free(dir_rom);
    if (dir_disk)  free(dir_disk);
    if (dir_tape)  free(dir_tape);
    if (dir_snap)  free(dir_snap);
    if (dir_state) free(dir_state);
    if (dir_g_cfg) free(dir_g_cfg);
    if (dir_l_cfg) free(dir_l_cfg);
}
