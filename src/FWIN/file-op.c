/*****************************************************************************/
/* ファイル操作に関する処理						     */
/*									     */
/*	仕様の詳細は、ヘッダファイル file-op.h 参照			     */
/*									     */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>	/* _fullpath */
#include <string.h>
#include <sys/stat.h>	/* _stat */
#include <direct.h>	/* _getcwd, _getdrives */
#include <io.h>		/* _findfirst */
#include <errno.h>
#include <mbstring.h>	/* _mbsicmp */

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"


/* シフトJISコードかどうかの判定 */
#define	IS_SJIS(h, l)							\
	((BETWEEN(0x81, (unsigned char)(h), 0x9F) ||			\
	  BETWEEN(0xE0, (unsigned char)(h), 0xFC))   &&			\
	 (BETWEEN(0x40, (unsigned char)(l), 0x7E) ||			\
	  BETWEEN(0x80, (unsigned char)(l), 0xFC)))

#define	IS_SJIS_H(h)							\
	(BETWEEN(0x81, (h), 0x9F) || BETWEEN(0xE0, (h), 0xFC))



/*****************************************************************************/

static char *dir_cwd;	/* デフォルトのディレクトリ (カレント)		*/
static char *dir_rom;	/* ROMイメージファイルの検索ディレクトリ	*/
static char *dir_disk;	/* DISKイメージファイルの検索ディレクトリ	*/
static char *dir_tape;	/* TAPEイメージファイルの基準ディレクトリ	*/
static char *dir_snap;	/* 画面スナップショットファイルの保存先		*/
static char *dir_state;	/* サスペンドファイルの保存先			*/
static char *dir_home;	/* 共通設定ファイルを置いてるディレクトリ	*/
static char *dir_ini;	/* 個別設定ファイルを置いてるディレクトリ	*/



/****************************************************************************
 * 各種ディレクトリの取得	(osd_dir_cwd は NULLを返してはだめ !)
 *****************************************************************************/
const char *osd_dir_cwd  (void) { return dir_cwd;   }
const char *osd_dir_rom  (void) { return dir_rom;   }
const char *osd_dir_disk (void) { return dir_disk;  }
const char *osd_dir_tape (void) { return dir_tape;  }
const char *osd_dir_snap (void) { return dir_snap;  }
const char *osd_dir_state(void) { return dir_state; }
const char *osd_dir_gcfg (void) { return dir_home;  }
const char *osd_dir_lcfg (void) { return dir_ini;   }

static int set_new_dir(const char *newdir, char **dir)
{
    char *p;
    p = malloc(strlen(newdir) + 1);
    if (p) {
	free(*dir);
	*dir = p;
	strcpy(*dir, newdir);
	return TRUE;
    }
    return FALSE;
}

int osd_set_dir_cwd  (const char *d) { return set_new_dir(d, &dir_cwd);   }
int osd_set_dir_rom  (const char *d) { return set_new_dir(d, &dir_rom);   }
int osd_set_dir_disk (const char *d) { return set_new_dir(d, &dir_disk);  }
int osd_set_dir_tape (const char *d) { return set_new_dir(d, &dir_tape);  }
int osd_set_dir_snap (const char *d) { return set_new_dir(d, &dir_snap);  }
int osd_set_dir_state(const char *d) { return set_new_dir(d, &dir_state); }
int osd_set_dir_gcfg (const char *d) { return set_new_dir(d, &dir_home);  }
int osd_set_dir_lcfg (const char *d) { return set_new_dir(d, &dir_ini);   }







/****************************************************************************
 * ファイル名に使用されている漢字コードを取得
 *		0 … ASCII のみ
 *		1 … 日本語EUC
 *		2 … シフトJIS
 *****************************************************************************/
int	osd_kanji_code(void)
{
    return 2;			/* WIN なのできめうちで SJIS */
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
 * いい方法を知らないので、ファイル名で区別することにしよう。
 *
 * osd_fopen が呼び出されたときに、ファイル名を保持しておき、
 * すでに開いているファイルのファイル名と一致しないかをチェックする。
 * ここで、ディスクイメージファイルの場合は、すでに開いているファイルの
 * ファイルポインタを返し、他の場合はオープン失敗として NULL を返す。
 */


/*
 * ファイル名 f1 と f2 が同じファイルであれば真を返す
 */
static int file_cmp(const char *f1, const char *f2);

#if 0

/*
 * WinAPI を使う方法。正直、よくわからない。
 */

#include <windows.h>
static int file_cmp(const char *f1, const char *f2)
{
    HANDLE h1, h2;
    BY_HANDLE_FILE_INFORMATION fi1, fi2;

    if (f1 == NULL || f2 == NULL) return FALSE;
    if (f1 == f2) return TRUE;

    h1 = CreateFile(f1, GENERIC_READ, FILE_SHARE_READ, NULL,
		    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h1 == INVALID_HANDLE_VALUE) {
	return FALSE;
    }

    h2 = CreateFile(f2, GENERIC_READ, FILE_SHARE_READ, NULL,
		    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h2 == INVALID_HANDLE_VALUE) {
	CloseHandle(h1); return FALSE; 
    }

    if (! GetFileInformationByHandle(h1, &fi1)) {
	CloseHandle(h1);
	CloseHandle(h2);
	return FALSE;
    }
    if (! GetFileInformationByHandle(h2, &fi2)) {
	CloseHandle(h1);
	CloseHandle(h2);
	return FALSE;
    }

    return (fi1.dwVolumeSerialNumber == fi2.dwVolumeSerialNumber &&
	    fi1.nFileIndexHigh == fi2.nFileIndexHigh && 
	    fi1.nFileIndexLow  == fi2.nFileIndexLow)     ? TRUE : FALSE;
}
#elif 0

/*
 * ファイル名を比較する方法。英字の大文字小文字の無視だけ、一応処理してある。
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
	    if (_strnicmp(f1, f2, 1) != 0) return FALSE;
	}

	f1 ++;
	f2 ++;
    }

    if (*f2 == '\0') return TRUE;
    else             return FALSE;
}
#else

/*
 * _mbsicmp を使ってお手軽に。
 */

static int file_cmp(const char *f1, const char *f2)
{
    if (f1 == NULL || f2 == NULL) return FALSE;
    if (f1 == f2) return TRUE;

    return (_mbsicmp(f1, f2) == 0) ? TRUE : FALSE;
}
#endif







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
    char	*fullname, *localname;
    FILE    *origfile, *localfile;
    char    *filebuf;
    unsigned long filelen;

    st = NULL;
    for (i = 0; i < MAX_STREAM; i++) {	/* 空きバッファを探す */
        if (osd_stream[i].fp == NULL) {		/* fp が NULL なら空き */
            st = &osd_stream[i];
            break;
        }
    }
    if (st == NULL) return NULL;		/* 空きがなければ NG */
    st->path = NULL;


    fullname = _fullpath(NULL, path, 0);	/* ファイル名を取得する */
    if (fullname == NULL) return NULL;

    if ((type == FTYPE_DISK) && osd_file_stat(fullname))
    {
        localname = calloc(_MAX_PATH, sizeof(char));
        osd_file_localname(fullname, localname);

        if (!osd_file_stat(localname) && !file_cmp(fullname, localname)) /* 上書き用ファイルの有無をチェック */
        {
            if ((origfile = fopen(fullname, "rb")) && (localfile = fopen(localname, "wb")))
            {
                fseek(localfile, 0, SEEK_SET);
                fseek(origfile, 0, SEEK_END);
                filelen = ftell(origfile);
                fseek(origfile, 0, SEEK_SET);

                filebuf = malloc(filelen);

                fread(filebuf, 1, filelen, origfile);
                fflush(origfile);
                fwrite(filebuf, 1, filelen, localfile);

                fclose(origfile);
                fclose(localfile);

                free(filebuf);
            }
            else
            {
                free(localname); /* 上書き用ファイルを利用しない */
            }
        }

        if (localname)
        {
            free(fullname); /* 上書き用ファイルをロードする */
            fullname = localname;
        }
    }


    switch (type) {

    case FTYPE_DISK:		/* "r+b" , "rb"	*/
    case FTYPE_TAPE_LOAD:	/* "rb" 	*/
    case FTYPE_TAPE_SAVE:	/* "ab"		*/
    case FTYPE_PRN:			/* "ab"		*/
    case FTYPE_COM_LOAD:	/* "rb"		*/
    case FTYPE_COM_SAVE:	/* "ab"		*/

	/* すでに開いているファイルかどうかをチェックする */
	for (i=0; i<MAX_STREAM; i++) {
	    if (osd_stream[i].fp) {
		if (file_cmp(osd_stream[i].path, fullname)) {

		    free(fullname);

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
	st->path = fullname;		/* ファイル名を保持する */
	/* FALLTHROUGH */


    default:
	st->fp = fopen(fullname, mode);	/* ファイルを開く */

	if (st->fp) {

	    st->type = type;
	    strncpy(st->mode, mode, sizeof(st->mode));
	    return st;

	} else {

	    free(fullname);
	    return NULL;
	}
    }
}



int	osd_fclose(OSD_FILE *stream)
{
    FILE *fp = stream->fp;

    stream->fp = NULL;
    if (stream->path) {
	free(stream->path);
	stream->path = NULL;
    }

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



/*---------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
/* パスがルートディレクトリかどうかを判定。 path は正規化されている想定 */
static	int	is_root_dir(const char *path)
{
    int len = strlen(path);

    if (len == 1 && strcmp(&path[0], "\\")  == 0) {	    /* \   → ◎ */
	return TRUE;
    }
    if (len == 3 && strcmp(&path[1], ":\\") == 0) {	    /* x:\ → ◎ */
	return TRUE;
    }
    if (len >= 4 && strncmp(&path[0], "\\\\", 2) == 0) {
	int i, j;

	if (path[2] == '\\') { return FALSE; }		    /* \\\ → × */

	/* ネットワークディレクトリだろうから、解析する	      (\\… で始まる)*/
	for (i = 3; i < len-1; i++) {

	    /* SJIS文字はスキップする */
	    if (IS_SJIS(path[i], path[i+1])) { i++; continue; }

	    /* \ が見つかれば、後続するパスの内容をチェック */

	    if (path[i] == '\\') {

		/* いきなりパスが終端の場合は、ホスト名だけ
		   なので、ルートとみなさない */

		if (path[i+1] == '\0') { return FALSE; }    /* \\x…\ → × */

		/* 後続するパスが \ で終わるかチェック */

		for (j = i+1; j < len-1; j++) {

		    if (IS_SJIS(path[j], path[j+1])) { i++; continue; }

		    if (path[j] == '\\') {
			if (path[j+1] == '\0') {	/* \\x…\y…\   → ◎*/
			    return TRUE;
			} else {			/* \\x…\y…\… → ×*/
			    return FALSE;
			}
		    }
		}

		/* 後続するパスに \ が含まれない場合は、
		   \ で終わってないがルートとみなす */

		return TRUE;				/* \\x…\y…    → ◎*/
	    }
	}

	/* ホスト名だけなので、ルートとみなさない */
	return FALSE;					    /* \\x…  → × */
    }

    return FALSE;
}

/* パスの末尾が \ かどうかを判定 */
static	int	backslash_terminated(const char *path)
{
    int i, len = strlen(path);

    for (i = 0; i < len; i++) {

	if (i < len-1 ){
	    if (IS_SJIS(path[i], path[i+1])) { i++; continue; }
	}

	if (path[i] == '\\' &&
	    path[i+1] == '\0') {
	    return TRUE;
	}
    }

    return FALSE;
}

/****************************************************************************
 * ディレクトリ閲覧
 *****************************************************************************/

struct	T_DIR_INFO_STRUCT
{
    int		cur_entry;		/* 上位が取得したエントリ数	*/
    int		nr_entry;		/* エントリの全数		*/
    int		nr_total;		/* エントリの全数 + ドライブ数	*/
    T_DIR_ENTRY	*entry;			/* エントリ情報 (entry[0]〜)	*/
};



/*
 * ディレクトリ内のファイル名のソーティングに使う関数
 */

	/* 大文字小文字を無視してファイル名ソート  */
static int namecmp(const void *p1, const void *p2)
{
    T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
    T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;
#if 0
    return _stricmp(s1->name, s2->name);
#else
    return _mbsicmp(s1->name, s2->name);
#endif
}
	/* 大文字小文字を無視してディレクトリ名ソート  */
static int dircmp(const void *p1, const void *p2)
{
    T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
    T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;
    if (s1->str[0] == '<' && s2->str[0] != '<') return -1; /* <〜>はドライブ */
    if (s1->str[0] != '<' && s2->str[0] == '<') return +1; /* なので高優先   */
#if 0
    return _stricmp(s1->name, s2->name);
#else
    return _mbsicmp(s1->name, s2->name);
#endif
}
	/* ファイルとディレクトリとを分離 */
static int typecmp(const void *p1, const void *p2)
{
    T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
    T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;

    if (s1->type == s2->type) return 0;
    if (s1->type == FILE_STAT_DIR) return -1;
    else                           return +1;
}



/*---------------------------------------------------------------------------
 * T_DIR_INFO *osd_opendir(const char *filename)
 *	_findfirst(), _findnext(), _findclose() を駆使し、
 *	ディレクトリの全てのエントリの ファイル名と属性をワークにセットする。
 *	ワークは malloc で確保するが、失敗時はそこでエントリの取得を打ち切る。
 *	処理後は、このワークをファイル名でソートしておく。
 *	また、ディレクトリの場合は、表示用の名の前後に [ と ] を付加しておく。
 *---------------------------------------------------------------------------*/
T_DIR_INFO	*osd_opendir(const char *filename)
{
    int i;
    T_DIR_INFO *dir;

    long dirp;
    struct _finddata_t dp;

    int len;
    char *p;
    char *fname;
    long drv_list    = _getdrives();
    char drv_name[4] = "A:\\";
    char drv_str[5]  = "<A:>";
    int top_dir = FALSE;

				/* T_DIR_INFO ワークを 1個確保 */
    if ((dir = (T_DIR_INFO *)malloc(sizeof(T_DIR_INFO))) == NULL) {
	return NULL;
    }

    if (filename == NULL || filename[0] == '\0') {
	filename = ".";
    }

				/* ルートディレクトリかどうかを判定 */
    top_dir = is_root_dir(filename);


				/* ディレクトリ検索名 "filename\\*" をセット */
    len = strlen(filename) + sizeof("\\*");
    if (len >= OSD_MAX_FILENAME ||
	((fname = (char*)malloc(len)) == NULL)) {	/* バッファ確保 */
	free(dir);
	return NULL;
    }
    strcpy(fname, filename);
    if (backslash_terminated(fname) == FALSE) {
	strcat(fname, "\\");				/* 末尾に \ を付加 */
    }
    strcat(fname, "*");					/* さらに * を付加 */


				/* ディレクトリ内のファイル数を数える */
    dir->nr_entry = 0;
    dirp = _findfirst(fname, &dp);
    if (dirp != -1) {
	do {
	    dir->nr_entry ++;
	} while (_findnext(dirp, &dp) == 0);
	_findclose(dirp);
    }


				/* T_DIR_ENTRY ワークを ファイル数分 確保 */
    dir->nr_total = dir->nr_entry + 26;		/* +26 はドライブ名の分 */
    dir->entry = (T_DIR_ENTRY *)malloc(dir->nr_total * sizeof(T_DIR_ENTRY));
    if (dir->entry == NULL) {
	free(dir);
	free(fname);
	return NULL;
    }
    for (i=0; i<dir->nr_total; i++) {
	dir->entry[i].name = NULL;
	dir->entry[i].str  = NULL;
    }


				/* ファイル数分、処理ループ (情報を格納) */
    dirp = -1;
    for (i=0; i<dir->nr_entry; i++) {

	if (i == 0) {			/* ファイル名取得(初回) */
	    dirp = _findfirst(fname, &dp);
	    if (dirp == -1) {
		dir->nr_entry = i;		/* 取得に失敗したら、中断  */
		break;
	    }
	} else {			/* ファイル名取得(次回以降) */
	    if (_findnext(dirp, &dp) != 0) {
		dir->nr_entry = i;		/* 取得に失敗したら、中断  */
		break;
	    }
	}

					/* ファイルの種類をセット */
	if (dp.attrib & _A_SUBDIR) {
	    dir->entry[i].type = FILE_STAT_DIR;
	} else {
	    dir->entry[i].type = FILE_STAT_FILE;
	}

					/* ファイル名バッファ確保 */
	len = strlen(dp.name) + 1;
	p = (char *)malloc(( len )   +  ( len + 2 ));
	if (p == NULL) { /* ↑ファイル名 と ↑表示名 のバッファを一気に確保 */
	    dir->nr_entry = i;
	    break;				/* malloc に失敗したら中断 */
	}

					/* ファイル名・表示名セット */
	dir->entry[i].name = &p[0];
	dir->entry[i].str  = &p[len];

	strcpy(dir->entry[i].name, dp.name);

	if (dir->entry[i].type == FILE_STAT_DIR) {
	    sprintf(dir->entry[i].str, "[%s]", dp.name);
	} else {
	    sprintf(dir->entry[i].str, "%s",   dp.name);
	}

    }


    free(fname);
    if (dirp != -1) {
	_findclose(dirp);		/* ディレクトリを閉じる */
    }


	/* エントリがない(取得失敗)場合や、ルートディレクトリの場合は、
	   ドライブをエントリに追加してあげよう */

    if (dir->nr_entry == 0 || top_dir) {
	for (i=0; i<26; i++) {
	    if (drv_list & (1L<<i)) {

		p = (char *)malloc(sizeof(drv_name) + sizeof(drv_str));
		if (p) {
		    dir->entry[ dir->nr_entry ].name = &p[0];
		    dir->entry[ dir->nr_entry ].str  = &p[sizeof(drv_name)];

		    strcpy(dir->entry[ dir->nr_entry ].name, drv_name);
		    strcpy(dir->entry[ dir->nr_entry ].str,  drv_str);

		    dir->entry[ dir->nr_entry ].type = FILE_STAT_DIR;
		    dir->nr_entry ++;
		}
	    }
	    drv_name[0] ++;	/* "x:\\" の x を A→Zに置換していく */
	    drv_str[1] ++;	/* "<x:>" の x を A→Zに置換していく */
	}
    }


				/* ファイル名をソート */

					/* まずファイルとディレクトリを分離 */
    qsort(dir->entry, dir->nr_entry, sizeof(T_DIR_ENTRY), typecmp);
    {
	T_DIR_ENTRY *p = dir->entry;
	for (i=0; i<dir->nr_entry; i++, p++) {
	    if (p->type == FILE_STAT_FILE) break;
	}
					/* 各々をファイル名でソート */
	qsort(&dir->entry[0], i, sizeof(T_DIR_ENTRY), dircmp);
	qsort(&dir->entry[i], dir->nr_entry-i, sizeof(T_DIR_ENTRY), namecmp);
    }


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
 *		_fullpath()の結果をそのまま返す。
 *			末尾に \ が残った場合、それは削除する。
 *		_fullpath()の結果が NULL なら、pathname をそのまま返す
 *---------------------------------------------------------------------------*/
int	osd_path_normalize(const char *path, char resolved_path[], int size)
{
    if (_fullpath(resolved_path, path, size) != NULL) {

	int i = strlen(resolved_path);
	if (i) {
	    if (is_root_dir(resolved_path)) {
		/* ルートディレクトリなのに末尾が \ でないなら、付加 */
		if (backslash_terminated(resolved_path) == FALSE) {
		    strcat(resolved_path, "\\");
		}
	    } else {
		/* ルートディレクトリ以外は、末尾の \ は削除 */
		if (backslash_terminated(resolved_path)) {
		    resolved_path[i - 1] = '\0';
		}
	    }
	}
	/* resolved_path には絶対パス格納済み */
	/*printf("NORM:\"%s\" => \"%s\"\n",path,resolved_path);*/
	return TRUE;
    } else {
	return FALSE;
    }
}



/*---------------------------------------------------------------------------
 * int	osd_path_split(const char *path, char dir[], char file[], int size)
 *
 *	処理内容:
 *		path の最後の \ より前を dir、後ろを file にセットする
 *			dir の末尾に \ はつかない。
 *		path の末尾が \ なら、予め削除してから処理する
 *			よって、 file の末尾にも \ はつかない。
 *		path は予め、正規化されているものとする。
 *---------------------------------------------------------------------------*/
int	osd_path_split(const char *path, char dir[], char file[], int size)
{
    int skip = FALSE;
    int i, backslash_pos;
    int len = strlen(path);

    /* dir, file は十分なサイズを確保しているはずなので、軽くチェック */
    if (len == 0 || size <= len) {
	dir[0]  = '\0';
	file[0] = '\0';
	strncat(file, path, size-1);
	if (len) fprintf(stderr, "internal overflow %d\n", __LINE__);
	return FALSE;
    }


					/* ルートディレクトリの場合、別処理 */
    if (is_root_dir(path)) {		/* (末尾の \ を残すため)            */
	strcpy(dir, path);			/* ディレクトリは path	*/
	strcpy(file, "");			/* ファイルは ""	*/
	return TRUE;
    }

    if (backslash_terminated(path)) {	/* path 末尾が \ なら予めスキップ */
	skip = TRUE;
	len --;
    }


    backslash_pos = 0;			/* 先頭から、最後の \ を探す */
    for (i = 0; i < len; i++) {

	if (path[i] == '\\') { backslash_pos = i+1; }

	if (i < len-1 ){
	    if (IS_SJIS(path[i], path[i+1])) { i++; continue; }
	}
    }

    if (backslash_pos) {		/* \ が見つかったら	*/
	strncpy(dir, path, backslash_pos);	/* 先頭〜 \ までをコピー*/
	dir[backslash_pos] = '\0';		/* \ も含まれます	*/

	/* dir の 末尾が \ なら、削除 */
	if (is_root_dir(dir)) {		/* ルートディレクトリなら削除しない */
	    /* DO NOTHING */
	} else {
	    if (backslash_terminated(dir)) {
		dir[backslash_pos - 1] = '\0';
	    }
	}

	strcpy(file, &path[backslash_pos]);

    } else {				/* \ が見つからなかった	*/
	strcpy(dir,  "");			/* ディレクトリは ""	*/
	strcpy(file, path);			/* ファイルは path全て	*/
    }

    if (skip) {				/* ファイル末尾の / は削除 */
	file[ strlen(file) - 1 ] = '\0';
    }

    /*printf("SPLT:\"%s\" = \"%s\" + \"%s\")\n",path,dir,file);*/
    return TRUE;
}



/*---------------------------------------------------------------------------
 * int	osd_path_join(const char *dir, const char *file, char path[], int size)
 *
 *	処理内容:
 *		file が \\ で始まっていたら、そのまま path にセット
 *		file が x:\\ の場合も、      そのまま path にセット
 *		そうでなければ、"dir" + "\\" + "file" を path にセット
 *---------------------------------------------------------------------------*/
int	osd_path_join(const char *dir, const char *file, char path[], int size)
{
    int len = strlen(file);

    if (file[0] == '\\' ||			/* ファイル名が、絶対パス */
	file[1] == ':') {

	if ((size_t)size <= strlen(file)) { return FALSE; }
	strcpy(path, file);

    } else {					/* ファイル名は、相対パス */

	path[0] = '\0';
	strncat(path, dir, size - 1);

	if (backslash_terminated(path) == FALSE) {	/* ディレクトリ末尾  */
	    len = strlen(path);				/* が'\\' でないなら */
	    strncat(path, "\\", size - len - 1);	/* 付加する          */
	}

	len = strlen(path);
	strncat(path, file, size - len - 1);

    }

    /*printf("JOIN:\"%s\" + \"%s\" = \"%s\"\n",dir,file,path);*/
    return TRUE;
}



/****************************************************************************
 * ファイル属性の取得
 ****************************************************************************/
int	osd_file_stat(const char *pathname)
{
    struct _stat sb;

    if (_stat(pathname, &sb)) {
	return FILE_STAT_NOEXIST;
    }

    if (sb.st_mode & _S_IFDIR) {
	return FILE_STAT_DIR;
    } else {
	return FILE_STAT_FILE;
    }
}

/****************************************************************************
 * 上書き用ファイルのパスの取得
 ****************************************************************************/
void osd_file_localname(const char *fullname, char *localname)
{
    char    filename[_MAX_FNAME], fileext[_MAX_EXT];
    char    fullpath[_MAX_DIR];

    _splitpath(fullname, NULL, NULL, filename, fileext); /* 上書き用ファイル名を取得する */
    strcpy(fullpath, osd_dir_disk());
    strcat(fullpath, "\\");
    strcat(fullpath, filename);
    strcat(fullpath, fileext);

    strcpy(localname, fullpath);
}






/****************************************************************************
 * int	osd_file_config_init(void)
 *
 *	この関数は、起動後に1度だけ呼び出される。
 *	正常終了時は真を、 malloc に失敗したなど異常終了時は偽を返す。
 *
 ****************************************************************************/

static int make_dir(const char *dname);
static int check_dir(const char *dname);

/*
 * 環境変数 *env_dir にセットされたディレクトリを **dir にセットする。
 * 環境変数が未定義なら、カレントディレクトリ + *alt_dir で表される
 * ディレクトリを **dir にセットする。
 * いずれの場合も、 **dir には malloc された領域 (ないし NULL) がセットされる
 */

static void set_dir(char **dir, char *env_dir, char *alt_dir)
{
    char *s;

    s = getenv(env_dir);
    if (s) {

	*dir = _fullpath(NULL, s, 0);

    } else {

	if (alt_dir) {
	    if (dir_home) {

		s = (char*)malloc(strlen(dir_home) + strlen(alt_dir) + 2);

		if (s) {
		    s[0] = '\0';
		    if (dir_home[0]) {
			strcat(s, dir_home);
			strcat(s, "\\");
		    }
		    strcat(s, alt_dir);

		    *dir = _fullpath(NULL, s, 0);

		    free(s);
		} else {
		    *dir = NULL;
		}

		if (*dir) {
#if 0
		    if (make_dir(*dir)) return;  /* ディレクトリなければ作る */
#else
		    if (check_dir(*dir)) return; /* ディレクトリあれば成功 */
#endif
		    free(*dir);
		}
	    }
	}

	*dir = _getcwd(NULL, 0);
    }
}


int	osd_file_config_init(void)
{
    char *s;


	/* カレントワーキングディレクトリ名 (CWD) を取得する */

    dir_cwd = _getcwd(NULL, 0);


	/* ホームディレクトリ $(QUASI88_HOME) を取得する */

    s = getenv("QUASI88_HOME");
    if (s) {
	dir_home = _fullpath(NULL, s, 0);
    } else {
	dir_home = _getcwd(NULL, 0);
    }


    /* いろんなディレクトリを設定する				*/
    /*	第2引数の環境変数が設定してあれば、そのディレクトリ。	*/
    /*	未設定なら、第3引数のディレクトリが dir_home 以下に	*/
    /*	あるかチェックし、あればそれ。なければ dir_cwd		*/


	/* 設定ディレクトリ */

    set_dir(&dir_ini, "QUASI88_INI_DIR", "INI");


	/* ROMディレクトリ */

    set_dir(&dir_rom, "QUASI88_ROM_DIR", "ROM");


	/* DISKディレクトリ */

    set_dir(&dir_disk, "QUASI88_DISK_DIR", "DISK");


	/* TAPEディレクトリ */

    set_dir(&dir_tape, "QUASI88_TAPE_DIR", "TAPE");


	/* SNAPディレクトリ */

    set_dir(&dir_snap, "QUASI88_SNAP_DIR", "SNAP");


	/* STATEディレクトリ */

    set_dir(&dir_state, "QUASI88_STATE_DIR", "STATE");



	/* 各ディレクトリが設定できなければ異常終了 */

    if (! dir_cwd  || ! dir_home || ! dir_ini  || ! dir_rom  ||
	! dir_disk || ! dir_tape || ! dir_snap || ! dir_state)  return FALSE;


    return TRUE;
}



/*
 *	ディレクトリ dname があるかチェック。無ければ作る。
 *		成功したら、真を返す
 */
static int make_dir(const char *dname)
{
    struct _stat sb;

    if (_stat(dname, &sb)) {

	if (errno == ENOENT) {			/* ディレクトリ存在しない */

	    if (_mkdir(dname)) {
		fprintf(stderr, "error: can't make dir %s\n", dname);
		return FALSE;
	    } else {
		printf("make dir \"%s\"\n", dname);
	    }

	} else {				/* その他の異常 */
	    return FALSE;
	}

    } else {					/* ディレクトリあった */

	if (! (sb.st_mode & _S_IFDIR)) {		/* と思ったらファイル*/
	    fprintf(stderr, "error: not exist dir %s\n", dname);
	    return FALSE;
	}

    }

    return TRUE;
}



/*
 *	ディレクトリ dname があるかチェック。あれば 真
 */
static int check_dir(const char *dname)
{
    struct _stat sb;

    if (_stat(dname, &sb)) {

	return FALSE;				/* チェック失敗 */

    } else {					/* ディレクトリあった */

	if (! (sb.st_mode & _S_IFDIR)) {		/* と思ったらファイル*/
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
    if (dir_home)  free(dir_home);
    if (dir_ini)   free(dir_ini);
    if (dir_rom)   free(dir_rom);
    if (dir_disk)  free(dir_disk);
    if (dir_tape)  free(dir_tape);
    if (dir_snap)  free(dir_snap);
    if (dir_state) free(dir_state);
}
