/*****************************************************************************/
/* ファイル操作に関する処理						     */
/*									     */
/*	仕様の詳細は、ヘッダファイル file-op.h 参照			     */
/*									     */
/*****************************************************************************/

/*----------------------------------------------------------------------*/
/* ディレクトリの一覧など、 mac 固有のAPIを使った処理は、 		*/
/* apaslothy さん作のコードを使わせてもらいました。			*/
/*							 (c) apaslothy	*/
/*----------------------------------------------------------------------*/


/* でも、いまいちよくわかってない・・・					*/
/* とりあえず、以下の制限で実装してみる。				*/
/*									*/
/*	パスの区切りは ':'						*/
/*	: を複数繋げた相対パス表記は禁止。複数の : は単一の : とみなす	*/
/*	":"  の1文字は、ルートディレクトリとみなす。			*/
/*	"::" の2文字は、親ディレクトリとみなす				*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <MacTypes.h>
#include <Files.h>

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"


/*****************************************************************************/

static char dir_cwd[256];	/* デフォルトのディレクトリ (カレント)	  */
static char dir_rom[256];	/* ROMイメージファイルの検索ディレクトリ  */
static char dir_disk[256];	/* DISKイメージファイルの検索ディレクトリ */
static char dir_tape[256];	/* TAPEイメージファイルの基準ディレクトリ */
static char dir_snap[256];	/* 画面スナップショットファイルの保存先	  */
static char dir_state[256];	/* サスペンドファイルの保存先		  */
static char dir_g_cfg[256];	/* 共通設定ファイルのディレクトリ	  */
static char dir_l_cfg[256];	/* 個別設定ファイルのディレクトリ	  */

/*------------------------------------------------------------------------*/

/* ボリューム番号とディレクトリIDからフルパスを取得する */
static OSErr GetFullPath(short vRefNum, long dirID, UInt8 *pathname);

/* フルパスからボリューム番号とディレクトリIDを取得する */
static OSErr PathToSpec(const char *pathname, short *vRefNum, long *dirID,
			Boolean *is_dir);


/****************************************************************************
 * 各種ディレクトリの取得	(osd_dir_cwd は NULLを返してはだめ !)
 *****************************************************************************/
const char *osd_dir_cwd  (void) { return dir_cwd;   }
const char *osd_dir_rom  (void) { return dir_rom;   }
const char *osd_dir_disk (void) { return dir_disk;  }
const char *osd_dir_tape (void) { return dir_tape;  }
const char *osd_dir_snap (void) { return dir_snap;  }
const char *osd_dir_state(void) { return dir_state; }
const char *osd_dir_gcfg (void) { return dir_g_cfg; }
const char *osd_dir_lcfg (void) { return dir_l_cfg; }

static int set_new_dir(const char *newdir, char *dir)
{
    if (strlen(newdir) < 256) {
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
 *****************************************************************************/
int	osd_kanji_code(void)
{
    return 2;
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
 * mac ではどうすればいいのか知らないので、ファイル名で区別することにしよう。
 *
 * osd_fopen が呼び出されたときに、ファイル名を保持しておき、
 * すでに開いているファイルのファイル名と一致しないかをチェックする。
 * ここで、ディスクイメージファイルの場合は、すでに開いているファイルの
 * ファイルポインタを返し、他の場合はオープン失敗として NULL を返す。
 */


/*
 * ファイル名 f1 と f2 が同じファイルであれば真を返す
 *
 *	とりあえず、ファイル名だけで比較。大文字小文字の違いは無視。
 *	(つまり、エイリアスだと同じファイルでも不一致になってしまうな・・・)
 */

static int file_cmp(const char *f1, const char *f2)
{
    int is_sjis = FALSE;
    int c;

    if (f1 == NULL || f2 == NULL) return FALSE;
    if (f1 == f2) return TRUE;

    while ((c = (int)*f1)) {

	if (is_sjis) {				/* シフトJISの2バイト目	*/
	    if (*f1 != *f2) return FALSE;
	    is_sjis = FALSE;
	}
	else if ((c >= 0x81 && c <= 0x9f) ||	/* シフトJISの1バイト目 */
		 (c >= 0xe0 && c <= 0xfc)) {
	    if (*f1 != *f2) return FALSE;
	    is_sjis = TRUE;
	}
	else {					/* 英数字半角カナ文字	*/
	    if (my_strcmp(f1, f2) != 0) return FALSE;
	}

	f1 ++;
	f2 ++;
    }

    if (*f2 == '\0') return TRUE;
    else             return FALSE;
}







struct OSD_FILE_STRUCT {

    FILE	*fp;			/* !=NULL なら使用中	*/
    int		type;			/* ファイル種別		*/
    char	*path;			/* ファイル名		*/
    char	mode[4];		/* 開いた際の、モード	*/

};

#define	MAX_STREAM	8
static	OSD_FILE	osd_stream[ MAX_STREAM ];



OSD_FILE *osd_fopen(int type, const char *path, const char *mode)
{
    int i;
    OSD_FILE	*st;

    st = NULL;
    for (i=0; i<MAX_STREAM; i++) {	/* 空きバッファを探す */
	if (osd_stream[i].fp == NULL) {		/* fp が NULL なら空き */
	    st = &osd_stream[i];
	    break;
	}
    }
    if (st == NULL) return NULL;		/* 空きがなければ NG */
    st->path = NULL;



    switch (type) {

    case FTYPE_DISK:		/* "r+b" , "rb"	*/
    case FTYPE_TAPE_LOAD:	/* "rb" 	*/
    case FTYPE_TAPE_SAVE:	/* "ab"		*/
    case FTYPE_PRN:		/* "ab"		*/
    case FTYPE_COM_LOAD:	/* "rb"		*/
    case FTYPE_COM_SAVE:	/* "ab"		*/

	/* すでに開いているファイルかどうかをチェックする */
	for (i=0; i<MAX_STREAM; i++) {
	    if (osd_stream[i].fp) {
		if (file_cmp(osd_stream[i].path, path)) {

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
					/* ファイル名保持用のバッファを確保 */
	st->path = malloc(strlen(path) + 1);
	if (st->path == NULL) {
	    return NULL;
	}
	/* FALLTHROUGH */


    default:
	st->fp = fopen(path, mode);	/* ファイルを開く */

	if (st->fp) {

	    st->type = type;
	    if (st->path)
		strcpy(st->path, path);
	    strncpy(st->mode, mode, sizeof(st->mode));
	    return st;

	} else {

	    if (st->path) {
		free(st->path);
		st->path = NULL;
	    }
	    return NULL;
	}
    }
}



int	osd_fclose(OSD_FILE *stream)
{
    if (stream->fp) {

	FILE *fp = stream->fp;

	stream->fp = NULL;
	if (stream->path) {
	    free(stream->path);
	    stream->path = NULL;
	}

	return fclose(fp);

    }
    return EOF;
}



int	osd_fflush(OSD_FILE *stream)
{
    if (stream == NULL) return fflush(NULL);

    if (stream->fp) {
	return fflush(stream->fp);
    }
    return EOF;
}



int	osd_fseek(OSD_FILE *stream, long offset, int whence)
{
    if (stream->fp) {
	return fseek(stream->fp, offset, whence);
    }
    return -1;
}



long	osd_ftell(OSD_FILE *stream)
{
    if (stream->fp) {
	return ftell(stream->fp);
    }
    return -1;
}



void	osd_rewind(OSD_FILE *stream)
{
    (void)osd_fseek(stream, 0L, SEEK_SET);
    osd_fflush(stream);
}



size_t	osd_fread(void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
{
    if (stream->fp) {
	return fread(ptr, size, nobj, stream->fp);
    }
    return 0;
}



size_t	osd_fwrite(const void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
{
    if (stream->fp) {
	return fwrite(ptr, size, nobj, stream->fp);
    }
    return 0;
}



int	osd_fputc(int c, OSD_FILE *stream)
{
    if (stream->fp) {
	return fputc(c, stream->fp);
    }
    return EOF;
}


int	osd_fgetc(OSD_FILE *stream)
{
    if (stream->fp) {
	return fgetc(stream->fp);
    }
    return EOF;
}


char	*osd_fgets(char *str, int size, OSD_FILE *stream)
{
    if (stream->fp) {
	return fgets(str, size, stream->fp);
    }
    return NULL;
}


int	osd_fputs(const char *str, OSD_FILE *stream)
{
    if (stream->fp) {
	return fputs(str, stream->fp);
    }
    return EOF;
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


/*---------------------------------------------------------------------------
 * トップレベル (ボリュームの一覧)
 *---------------------------------------------------------------------------*/
static	T_DIR_INFO	*openroot(void)
{
    char  *p;
    short i;
    OSErr err;
    int   num;
    T_DIR_INFO	*dir;
    Str63	temp;
    HParamBlockRec pbr;

				/* T_DIR_INFO ワークを 1個確保 */
    if ((dir = (T_DIR_INFO *)malloc(sizeof(T_DIR_INFO))) == NULL) {
	return NULL;
    }

    /* 項目の数を取得する */
    dir->nr_entry = 0;
    i = 1;
    pbr.volumeParam.ioNamePtr = temp;
    do {
	pbr.volumeParam.ioVolIndex = i;
	err = PBHGetVInfoSync(&pbr);
	if (err == noErr) {
	    dir->nr_entry += 1;
	}
	i++;
    } while (err == noErr);

    if (dir->nr_entry == 0) {
	free(dir);
	return NULL;
    }
				/* T_DIR_ENTRY ワークを 項目数分 確保 */
    dir->entry = (T_DIR_ENTRY *)malloc(dir->nr_entry * sizeof(T_DIR_ENTRY));
    if (dir->entry == NULL) {
	free(dir);
	return NULL;
    }
    for (i=0; i<dir->nr_entry; i++) {
	dir->entry[i].name = NULL;
      /*dir->entry[i].str  = NULL;*/
    }


    dir->cur_entry = 0;

    /* フォルダ内の項目を追加 */
    i = 1;
    num = 0;
    pbr.volumeParam.ioNamePtr = temp;
    do {
	pbr.volumeParam.ioVolIndex = i;
	err = PBHGetVInfoSync(&pbr);
	if (err == noErr) {
	    temp[temp[0] + 1] = 0;

	    p = (char *)malloc( (temp[0] + 1) + (temp[0] + 1) );
	    if (p == NULL) { /* ↑ファイル名 と ↑表示名 のバッファを確保 */
		dir->nr_entry = num;
		break;				/* malloc に失敗したら中断 */
	    }

	    dir->entry[num].name = &p[0];
	    dir->entry[num].str  = &p[(temp[0] + 1)];

	    strcpy(dir->entry[num].name, (char*)&temp[1]); /* ファイル名 */
	    strcpy(dir->entry[num].str,  (char*)&temp[1]); /* 表示名     */

	    dir->entry[num].type = FILE_STAT_DIR;
	    num++;
	}
	i++;
    } while (err == noErr && num < dir->nr_entry);

    dir->nr_entry = num;

    return dir;
}


/*---------------------------------------------------------------------------
 * T_DIR_INFO *osd_opendir(const char *filename)
 *---------------------------------------------------------------------------*/
T_DIR_INFO	*osd_opendir(const char *filename)
{
    char  *p;
    short i;
    OSErr err;
    short vRefNum;
    int   num;
    long  dirID;
    T_DIR_INFO	*dir;
    Str63	temp;
    CInfoPBRec	pbr;

    /* : なら、ルート(トップレベル) とみなす */
    if (strcmp(filename, ":") == 0) {
	return openroot();
    }

    /* filenameが無いときも、ルート(トップレベル) とみなす */
    if (filename == NULL || filename[0] == '\0') {
	return openroot();
    }


				/* T_DIR_INFO ワークを 1個確保 */
    if ((dir = (T_DIR_INFO *)malloc(sizeof(T_DIR_INFO))) == NULL) {
	return NULL;
    }

    err = PathToSpec(filename, &vRefNum, &dirID, NULL);
    if (noErr != err) {
	free(dir);
	return NULL;
    }

    /* 項目の数を取得する */
    dir->nr_entry = 2;		/* 最低でも2エントリ (TOPとparent) */
    i = 1;
    pbr.hFileInfo.ioNamePtr = temp;
    pbr.hFileInfo.ioVRefNum = vRefNum;
    do {
	pbr.hFileInfo.ioFDirIndex = i;
	pbr.hFileInfo.ioDirID     = dirID;
	pbr.hFileInfo.ioACUser    = 0;
	err = PBGetCatInfoSync(&pbr);
	if (err == noErr && !(pbr.hFileInfo.ioFlFndrInfo.fdFlags & 0x4000)) {
	    dir->nr_entry += 1;
	}
	i++;
    } while (err == noErr);
				/* T_DIR_ENTRY ワークを 項目数分 確保 */
    dir->entry = (T_DIR_ENTRY *)malloc(dir->nr_entry * sizeof(T_DIR_ENTRY));
    if (dir->entry == NULL) {
	free(dir);
	return NULL;
    }
    for (i=0; i<dir->nr_entry; i++) {
	dir->entry[i].name = NULL;
      /*dir->entry[i].str  = NULL;*/
    }


    dir->cur_entry = 0;

    /* 先頭に <TOP> と <parent> を追加 */

#define	TOP_NAME	":"
#define	TOP_STR		"<< TOP >>"
#define	PAR_NAME	"::"
#define	PAR_STR		"<< parent >>"

    num = 0;
    {
	p = (char *)malloc(sizeof(TOP_NAME) + sizeof(TOP_STR));
	if (p == NULL) {    /* ↑ファイル名 と ↑表示名 のバッファを確保 */
	    dir->nr_entry = num;
	    return dir;
	}
	dir->entry[num].name = &p[0];
	dir->entry[num].str  = &p[sizeof(TOP_NAME)];

	strcpy(dir->entry[num].name, TOP_NAME);
	strcpy(dir->entry[num].str,  TOP_STR);

	dir->entry[num].type = FILE_STAT_DIR;
    }
    num = 1;
    {
	p = (char *)malloc(sizeof(PAR_NAME) + sizeof(PAR_STR));
	if (p == NULL) {    /* ↑ファイル名 と ↑表示名 のバッファを確保 */
	    dir->nr_entry = num;
	    return dir;
	}
	dir->entry[num].name = &p[0];
	dir->entry[num].str  = &p[sizeof(PAR_NAME)];

	strcpy(dir->entry[num].name, PAR_NAME);
	strcpy(dir->entry[num].str,  PAR_STR);

	dir->entry[num].type = FILE_STAT_DIR;
    }

    /* フォルダ内の項目を追加 */
    i = 1;
    num = 2;
    pbr.hFileInfo.ioNamePtr = temp;
    pbr.hFileInfo.ioVRefNum = vRefNum;
    do {
	pbr.hFileInfo.ioFDirIndex = i;
	pbr.hFileInfo.ioDirID     = dirID;
	pbr.hFileInfo.ioACUser    = 0;
	err = PBGetCatInfo(&pbr, 0);
	if (err == noErr &&		/* ↓ 不可視属性のファイルは除く */
	    !(pbr.hFileInfo.ioFlFndrInfo.fdFlags & 0x4000)) {
	    temp[temp[0] + 1] = 0;

	    p = (char *)malloc((temp[0] + 1) + (temp[0] + 3));
	    if (p == NULL) { /* ↑ファイル名 と ↑表示名 のバッファを確保 */
		dir->nr_entry = num;
		break;				/* malloc に失敗したら中断 */
	    }

	    dir->entry[num].name = &p[0];
	    dir->entry[num].str  = &p[(temp[0] + 1)];

	    if (pbr.hFileInfo.ioFlAttrib & 16) {
		sprintf(dir->entry[num].name, "%s",   temp + 1);
		sprintf(dir->entry[num].str,  "[%s]", temp + 1);
		dir->entry[num].type = FILE_STAT_DIR;
	    } else {
		sprintf(dir->entry[num].name, "%s", temp + 1);
		sprintf(dir->entry[num].str,  "%s", temp + 1);
		dir->entry[num].type = FILE_STAT_FILE;
	    }
	    num++;
	}
	i++;
    } while (err == noErr && num < dir->nr_entry);

    dir->nr_entry = num;

    return dir;
}



/*---------------------------------------------------------------------------
 * T_DIR_ENTRY *osd_readdir(T_DIR_INFO *dirp)
 *	osd_opendir() の時に確保した、エントリ情報ワークへのポインタを
 *	順次、返していく。
 *---------------------------------------------------------------------------*/
T_DIR_ENTRY	*osd_readdir(T_DIR_INFO *dirp)
{
    T_DIR_ENTRY *ret_value = NULL;

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
    int i;

    for (i=dirp->nr_entry -1; i>=0; i--) {
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
 *	連続する : は、単一の : に置き換える。つまり相対パスは使えない。
 *---------------------------------------------------------------------------*/
int	osd_path_normalize(const char *path, char resolved_path[], int size)
{
    int i, del = FALSE;

    while (*path) {			/* 先頭の : を全て削除 */
	if (*path == ':') { path ++; }
	else              { break; }
    }


    i = 0;
    while (--size && *path) {		/* path をコピー		  */
	if (*path == ':') {		/* 但し : が連続したら、1個にする */
	    if (del == FALSE) {
		resolved_path[i++] = *path;
		del = TRUE;
	    }
	} else {
	    resolved_path[i++] = *path;
	    del = FALSE;
	}
	path ++;
    }

    if (size == 0 && *path) {
	return FALSE;
    }

    if (i == 0 && size) {		/* 結果が何も残らなければ : とする */
	resolved_path[i++] = ':';	/* (:は、ルート(トップレベル)扱い) */
    } else {
	if (resolved_path[i-1] == ':') {	/* 末尾の : は削除 */
	    i--;
	}
    }

    resolved_path[i] = '\0';

    return TRUE;
}



/*---------------------------------------------------------------------------
 * int	osd_path_split(const char *path, char dir[], char file[], int size)
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


    do {				/* : を末尾から探す 	*/
	if (path[ pos-1 ] == ':') { break; }
	pos --;
    } while (pos);

    if (pos) {				/* : が見つかったら	*/
	strncpy(dir, path, pos);		/* 先頭〜 : までをコピー*/
	dir[pos-1] = '\0';			/* : は含まれません	*/
	strcpy(file, &path[pos]);

    } else {				/* : が見つからなかった	*/
	strcpy(dir,  "");			/* ディレクトリは ""	*/
	strcpy(file, path);			/* ファイルは path全て	*/
    }

    return TRUE;
}



/*---------------------------------------------------------------------------
 * int	osd_path_join(const char *dir, const char *file, char path[], int size)
 *
 *	file が ":"  なら、トップレベルとする。
 *
 *	file が "::" なら、親ディレクトリを手繰る。
 *		dir に : が含まれる場合は、最後の : の手前部分を取り出す。
 *		dir に : が含まれない場合は、 ":" (カレント) とする。
 *
 *	file が それ以外なら、 dir と file を結合する。
 *		dir == ":" なら、 "file" そのものとする。
 *		dir != ":" なら、 "dir:file" とする。
 *		(file が :: で始まる場合も同様に結合する。
 *		 ただし、複数の : は 単一の : として処理される)
 *---------------------------------------------------------------------------*/
int	osd_path_join(const char *dir, const char *file, char path[], int size)
{
    char buf[257];

    if (strcmp(file, ":") == 0) {	/* ":" はルート(トップレベル)扱い) */
	strcpy(buf, ":");
    } else {
	if (strcmp(file, "::") == 0) {	/* "::" は、親ディレクトリを抽出 */
	    char *p;
	    strcpy(buf, dir);
	    if (buf[ strlen(buf) - 1 ] == ':') {
		buf[ strlen(buf) - 1 ] = '\0';
	    }
	    p = strrchr(buf, ':');
	    if (p) { *p = '\0'; }
	    else   { strcpy(buf, ":"); }

	} else {			/* それ以外は */
	    if (strcmp(dir, ":") == 0) {	/* dir==":" なら file とする */
		strcpy(buf, file);
	    } else {				/* 以外は、  dir:file とする */
		sprintf(buf, "%s:%s", dir, file);
	    }
	}
    }

    return osd_path_normalize(buf, path, size);
}



/****************************************************************************
 * ファイル属性の取得
 ****************************************************************************/

int	osd_file_stat(const char *pathname)
{
    if (pathname == NULL || pathname[0] == '\0') {
	return FILE_STAT_NOEXIST;
    }

    if (strcmp(pathname, ":") == 0) {	/* ":" はルート(トップレベル)扱い) */
	return FILE_STAT_DIR;

    } else {				/* osd_opendirと同じことをやってみる */
	short vRefNum;
	long dirID;
	Boolean is_dir;

	if (PathToSpec(pathname, &vRefNum, &dirID, &is_dir) == noErr) {
	    if (is_dir) return FILE_STAT_DIR;
	    else        return FILE_STAT_FILE;
	} else {
	    return FILE_STAT_NOEXIST;
	}
    }
}






/****************************************************************************
 * int	osd_file_config_init(void)
 *
 *	この関数は、起動後に1度だけ呼び出される。
 *	正常終了時は真を、 malloc に失敗したなど異常終了時は偽を返す。
 *
 ****************************************************************************/

int	osd_file_config_init(void)
{
    char buffer[257];

    OSErr err;
    short vRefNum;
    long  dirID;

	/* カレントディレクトリを設定 */

    dir_cwd[0] = '\0';

    err = HGetVol((UInt8*)buffer, &vRefNum, &dirID);
    if (noErr != err) {
	;
    } else {
	err = GetFullPath(vRefNum, dirID, (UInt8*)buffer);
	if (noErr == err) {
	    buffer[ buffer[0]+1 ] = '\0';
	    strcpy(dir_cwd, &buffer[1]);
	} else {
	    ;
	}
    }

	/* ROMディレクトリを設定する */

    if (strlen(dir_rom) + sizeof(":ROM") < 256) {
	dir_rom[0] = '\0';
	strcat(dir_rom, dir_cwd);
	strcat(dir_rom, ":ROM");
    }

	/* DISKディレクトリを設定する */

    if (strlen(dir_disk) + sizeof(":DISK") < 256) {
	dir_disk[0] = '\0';
	strcat(dir_disk, dir_cwd);
	strcat(dir_disk, ":DISK");
    }

	/* TAPEディレクトリを設定する */

    if (strlen(dir_tape) + sizeof(":TAPE") < 256) {
	dir_tape[0] = '\0';
	strcat(dir_tape, dir_cwd);
	strcat(dir_tape, ":TAPE");
    }

	/* SNAPディレクトリを設定する */

    if (strlen(dir_snap) + sizeof(":SNAP") < 256) {
	dir_snap[0] = '\0';
	strcat(dir_snap, dir_cwd);
	strcat(dir_snap, ":SNAP");
    }

	/* STATEディレクトリを設定する */

    if (strlen(dir_state) + sizeof(":STATE") < 256) {
	dir_state[0] = '\0';
	strcat(dir_state, dir_cwd);
	strcat(dir_state, ":STATE");
    }


	/* 全体設定ディレクトリを設定する */

    if (strlen(dir_g_cfg) + sizeof(":Prefs") < 256) {
	dir_g_cfg[0] = '\0';
	strcat(dir_g_cfg, dir_cwd);
/*	strcat(dir_g_cfg, ":Prefs");*/
    }

	/* 個別設定ディレクトリを設定する */

    if (strlen(dir_l_cfg) + sizeof(":Prefs") < 256) {
	dir_l_cfg[0] = '\0';
	strcat(dir_l_cfg, dir_cwd);
/*	strcat(dir_l_cfg, ":Prefs");*/
    }

    return TRUE;
}



/*------------------------------------------------------------------------
 *
 *
 *------------------------------------------------------------------------*/

/* ボリューム番号とディレクトリIDからフルパスを取得する */
static OSErr GetFullPath(short vRefNum, long dirID, UInt8 *pathname)
{
    OSErr  err;
    FSSpec spec;
    char   tmp[257];
    int    i = 0;
    UInt8  nullstr[2] = { 0, 0 };

    pathname[0] = pathname[1] = 0;
	
    spec.vRefNum = vRefNum;
    spec.parID   = dirID;
    do {
	err = FSMakeFSSpec(spec.vRefNum, spec.parID, nullstr, &spec);
	if (noErr == err) {
	    memcpy(tmp, &spec.name[1], spec.name[0]);
	    tmp[spec.name[0]] = 0;
	    if (i > 0) 
		strcat((char*)tmp, ":");
	    strcat(tmp, (char*)&pathname[1]);
	    pathname[0] = strlen(tmp);
	    strcpy((char*)&pathname[1], tmp);
	}
	i++;
    } while (spec.parID != 1 && noErr == err);

    return err;
}

/* フルパスからボリューム番号とディレクトリIDを取得する */
static OSErr PathToSpec(const char *pathname, short *vRefNum, long *dirID,
			Boolean *is_dir)
{
    FSSpec spec;
    OSErr  err;
    UInt8  temp[257];
    int    i;
    CInfoPBRec pbr;
    HParamBlockRec hpbr;

    if (is_dir) *is_dir = FALSE;

    temp[0] = strlen(pathname);
    strcpy((char*)&temp[1], pathname);
    if (! strchr(pathname, ':')) {	/* : が含まれない場合 (ボリューム?) */
	hpbr.volumeParam.ioNamePtr = temp;
	i = 1;
	do {
	    hpbr.volumeParam.ioVolIndex = i;
	    err = PBHGetVInfoSync(&hpbr);
	    temp[temp[0] + 1] = 0;
	    if (err == noErr && !strcmp((char*)&temp[1], pathname)) {
		*vRefNum = hpbr.volumeParam.ioVRefNum;
		*dirID   = fsRtDirID;
		if (is_dir) *is_dir = TRUE;
		return err;
	    }
	    i++;
	} while (err == noErr);
	return err;
    }

    err = FSMakeFSSpec(-1, 1, temp, &spec);
    if (err != noErr) return err;

    /* dirIDを取得する */
    pbr.hFileInfo.ioNamePtr   = spec.name;
    pbr.hFileInfo.ioVRefNum   = spec.vRefNum;
    pbr.hFileInfo.ioFDirIndex = 0;
    pbr.hFileInfo.ioDirID     = spec.parID;
    pbr.hFileInfo.ioACUser    = 0;
    err = PBGetCatInfoSync(&pbr);
    if (err == noErr) {
	*vRefNum = pbr.hFileInfo.ioVRefNum;
	*dirID   = pbr.hFileInfo.ioDirID;
	if (pbr.hFileInfo.ioFlAttrib & 16) {
	    if (is_dir) *is_dir = TRUE;
	}
	return noErr;
    }
    return err;
}


/****************************************************************************
 * int	osd_file_config_exit(void)
 *
 *	この関数は、終了後に1度だけ呼び出される。
 *
 ****************************************************************************/
void	osd_file_config_exit(void)
{
    /* 特にすることなし */
}
