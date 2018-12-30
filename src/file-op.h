/*****************************************************************************/
/* ファイル操作								     */
/*									     */
/*	QUASI88 のファイル操作は、すべてこのヘッダにて定義した関数(マクロ)を */
/*	経由して行われる。						     */
/*									     */
/*****************************************************************************/

#ifndef FILE_OP_H_INCLUDED
#define FILE_OP_H_INCLUDED

#include "filename.h"


/****************************************************************************
 * 扱えるパス名の最大長
 *****************************************************************************/

#ifndef	OSD_MAX_FILENAME
#define	OSD_MAX_FILENAME	(1024)
#endif


/****************************************************************************
 * ファイル操作環境の初期化
 *
 * int	osd_file_config_init(void)
 *	各種ファイル処理に使用する、ディレクトリ名などの設定を行う。この関数の
 *	呼び出し以降、このヘッダに書かれている関数・変数が使用可能となる。
 *
 *	この関数は、 config_init() 内部にて最初に呼び出される。
 *	正常終了時は真を、異常終了時は偽を返す。
 *	(偽を返した場合、 config_init() もエラーとなる)
 *
 * void	osd_file_config_exit(void)
 *	osd_file_config_init() の後片付けを行う。(確保したリソースの解放など)
 *	この関数は、終了時に1回だけ呼び出される。
 *
 *****************************************************************************/
int	osd_file_config_init(void);
void	osd_file_config_exit(void);



/****************************************************************************
 * 各種ファイル名 (グローバル変数)
 *
 *	起動後、関数 quasi88() を呼び出す前に、オープンしたいファイル名を
 *	設定しておく。設定不要の場合は、空文字列 (または NULL) をセット。
 *	なお、関数 quasi88() の呼び出し以後は、変更しないこと !
 *
 *	char file_disk[2][QUASI88_MAX_FILENAME]
 *		ドライブ 1: / 2: にセットするディスクイメージファイル名。
 *		同じ文字列 (ファイル名) がセットされている場合は、両ドライブ
 *		に、同じディスクイメージファイルをセットすることを意味する。
 *
 *	int image_disk[2]
 *		ディスクイメージがセットされている場合、イメージの番号。
 *		0〜31 なら 1〜32番目、 -1 ならば、自動的に番号を割り振る。
 *
 *	int readonly_disk[2]
 *		真なら、リードオンリーでディスクイメージファイルを開く。
 *
 *	char *file_compatrom
 *		このファイルを P88SR.exe の ROMイメージファイルとして開く。
 *		NULL の場合は通常通りの ROMイメージファイルを開く。
 *
 *	char file_tape[2][QUASI88_MAX_FILENAME]
 *		テープロード用/セーブ用ファイル名。
 *
 *	char file_prn[QUASI88_MAX_FILENAME]
 *		プリンタ出力用ファイル名。
 *
 *	char file_sin[QUASI88_MAX_FILENAME]
 *	char file_sout[QUASI88_MAX_FILENAME]
 *		シリアル入力用/出力用ファイル名。
 *
 *	char file_state[QUASI88_MAX_FILENAME]
 *		ステートセーブ/ロード用のファイル名。
 *		空文字列の場合は、デフォルトのファイル名が使用される。
 *
 *	char file_snap[QUASI88_MAX_FILENAME]
 *		画面スナップショット保存用のファイルのベース名。
 *		実際にスナップショットを保存する場合は、このファイル名に
 *		4桁の数字 + 拡張子 ( 0000.bmp など ) を連結したファイル名になる
 *		空文字列の場合は、デフォルトのファイル名が使用される。
 *
 *	char file_wav[QUASI88_MAX_FILENAME]
 *		サウンド出力保存用のファイルのベース名。
 *		実際にサウンド出力を保存する場合は、このファイル名に
 *		4桁の数字 + 拡張子 ( 0000.wav ) を連結したファイル名になる
 *		空文字列の場合は、デフォルトのファイル名が使用される。
 *
 *	char *file_rec / char *file_pb
 *		キー入力の記録用/再生用ファイル名。
 *		使用しないなら、NULL にしておく。
 *****************************************************************************/

extern char file_disk[2][QUASI88_MAX_FILENAME];	/*ディスクイメージファイル名*/
extern int  image_disk[2];	   		/*イメージ番号0〜31,-1は自動*/
extern int  readonly_disk[2];			/*リードオンリーで開くなら真*/

extern char file_tape[2][QUASI88_MAX_FILENAME];	/* テープ入出力のファイル名 */
extern char file_prn[QUASI88_MAX_FILENAME];	/* パラレル出力のファイル名 */
extern char file_sin[QUASI88_MAX_FILENAME];	/* シリアル出力のファイル名 */
extern char file_sout[QUASI88_MAX_FILENAME];	/* シリアル入力のファイル名 */

extern char file_state[QUASI88_MAX_FILENAME];	/* ステートファイル名	    */
extern char file_snap[QUASI88_MAX_FILENAME];	/* スナップショットベース部 */
extern char file_wav[QUASI88_MAX_FILENAME];	/* サウンド出力ベース部     */

extern char *file_compatrom;		/* P88SR.exeのROMを使うならファイル名*/
extern char *file_rec;			/* キー入力記録するなら、ファイル名  */
extern char *file_pb;			/* キー入力再生するなら、ファイル名  */



/****************************************************************************
 * 各種ディレクトリの取得と設定
 *	QUASI88 はファイル操作の際に、ここで取得したディレクトリ名と
 *	osd_path_join を使って、パス名を生成する。
 *	osd_dir_cwd() は NULL を返さないこと!  他は……まあ NULL でもいいや。
 *
 * const char *osd_dir_cwd  (void)	デフォルトのディレクトリを返す
 * const char *osd_dir_rom  (void)	ROMイメージのあるディレクトリを返す
 * const char *osd_dir_disk (void)	DISKイメージの基準ディレクトリを返す
 * const char *osd_dir_tape (void)	TAPEイメージの基準ディレクトリを返す
 * const char *osd_dir_snap (void)	スナップショット保存ディレクトリを返す
 * const char *osd_dir_state(void)	ステートファイル保存ディレクトリを返す
 * const char *osd_dir_gcfg (void)	共通設定ファイルのディレクトリを返す
 * const char *osd_dir_lcfg (void)	個別設定ファイルのディレクトリを返す
 *
 * int osd_set_dir_???(const char *new_dir)
 *					それぞれ、各ディレクトリを new_dir に
 *					設定する。失敗した場合は偽が返る。
 *					( この時、各ディレクトリは元のまま )
 *****************************************************************************/

const char *osd_dir_cwd  (void);
const char *osd_dir_rom  (void);
const char *osd_dir_disk (void);
const char *osd_dir_tape (void);
const char *osd_dir_snap (void);
const char *osd_dir_state(void);
const char *osd_dir_gcfg (void);
const char *osd_dir_lcfg (void);

int	osd_set_dir_cwd  (const char *new_dir);
int	osd_set_dir_rom  (const char *new_dir);
int	osd_set_dir_disk (const char *new_dir);
int	osd_set_dir_tape (const char *new_dir);
int	osd_set_dir_snap (const char *new_dir);
int	osd_set_dir_state(const char *new_dir);
int	osd_set_dir_gcfg (const char *new_dir);
int	osd_set_dir_lcfg (const char *new_dir);



/****************************************************************************
 * ファイル名に使用されている漢字コードを取得
 *		0 … ASCII のみ
 *		1 … 日本語EUC
 *		2 … シフトJIS
 *	この情報は、メニュー画面でファイル名を表示する際にのみ利用する。
 *****************************************************************************/
int	osd_kanji_code(void);



/****************************************************************************
 * ファイル操作
 *	QUASI88 のファイルアクセスは、全て以下の関数を経由して行われる。
 *	実際に読み書きは、機種依存部側にて、好きにしてもよい。
 *	(排他制御したり、圧縮ファイルとして読み書きしたり……など)
 *	面倒なら、そのまま標準関数のラッパにすればよい。
 *	ちなみに errno は参照しない。ferror、feof なども呼び出さない。
 *
 *	この関数にて扱うファイルの種類およびモードは以下のとおり。
 *	()内はファイルの形式で、記載してないものは生のバイナリデータで扱う。
 *
 *	FTYPE_ROM		ROMイメージ				"rb"
 *	FTYPE_DISK		ディスクイメージ(D88形式)	"r+b" , "rb"
 *	FTYPE_TAPE_LOAD		ロード用テープイメージ(T88/CMT形式)	"rb"
 *	FTYPE_TAPE_SAVE		セーブ用テープイメージ(CMT形式)		"ab"
 *	FTYPE_PRN		プリンタ出力イメージ			"ab"
 *	FTYPE_COM_LOAD		シリアル入力イメージ			"rb"
 *	FTYPE_COM_SAVE		シリアル出力イメージ			"ab"
 *	FTYPE_SNAPSHOT_RAW	画面スナップショット			"wb"
 *	FTYPE_SNAPSHOT_PPM	画面スナップショット(PPM形式)		"wb"
 *	FTYPE_SNAPSHOT_BMP	画面スナップショット(BMP形式)		"wb"
 *	FTYPE_KEY_PB		キー入力再生				"rb"
 *	FTYPE_KEY_REC		キー入力記録				"wb"
 *	FTYPE_STATE_LOAD	レジューム用イメージ			"rb"
 *	FTYPE_STATE_SAVE	サスペンド用イメージ			"wb"
 *	FTYPE_CFG		設定ファイル			"r", "w","a"
 *	FTYPE_READ		バイナリリード汎用			"rb"
 *	FTYPE_WRITE		バイナリリード汎用			"wb"
 *	---------------------------------------------------------------------
 *
 * OSD_FILE *osd_fopen(int type, const char *path, const char *mode)
 *	fopen と同じ。成功時はファイルポインタが、失敗時は NULL を返す。
 *	type は、上記の FTYPE_xxxx を指定する。
 *	type が FTYPE_DISK の場合で、すでに同じファイルが開いてある時は、
 *	違うモードで開こうとした場合は NULL を、同じモードで開こうとした場合は
 *	その開いているファイルのファイルポインタを返す。
 *	(同じファイルが開いているかどうか検知できる場合のみ)
 *
 * int	osd_fclose(OSD_FILE *stream)
 *	fclose と同じ。失敗時でも EOF を返さなくてもかまわない。
 *
 * int	osd_fflush(OSD_FILE *stream)
 *	fflush と同じ。失敗時でも EOF を返さなくてもかまわない。
 *
 * int	osd_fseek(OSD_FILE *stream, long offset, int whence)
 *	fseek と同じ。失敗時には -1 を返す。
 *
 * long	osd_ftell(OSD_FILE *stream)
 *	ftell と同じ。失敗時には -1 を返す。
 *
 * void	osd_rewind(OSD_FILE *stream)
 *	rewind と同じ。
 *
 * size_t osd_fread(void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
 *	fread と同じ。読み込みに成功したブロック数を返す。
 *
 * size_t osd_fwrite(const void *ptr,size_t size,size_t nobj,OSD_FILE *stream)
 *	fwrite と同じ。書き込みに成功したブロック数を返す。
 *
 * int	osd_fputc(int c, OSD_FILE *stream)
 *	fputc と同じ。失敗時には EOF を返す。
 *
 * int	osd_fgetc(OSD_FILE *stream)
 *	fgetc と同じ。失敗時には EOF を返す。
 *
 * char	*osd_fgets(char *str, int size, OSD_FILE *stream)
 *	fgets と同じ。失敗時には NULL を返す。
 *
 * int	osd_fputs(const char *str, OSD_FILE *stream)
 *	fputs と同じ。失敗時には NULL を返す。
 *
 *****************************************************************************/

enum {
  FTYPE_ROM,
  FTYPE_DISK,
  FTYPE_TAPE_LOAD,
  FTYPE_TAPE_SAVE,
  FTYPE_PRN,
  FTYPE_COM_LOAD,
  FTYPE_COM_SAVE,
  FTYPE_SNAPSHOT_RAW,
  FTYPE_SNAPSHOT_PPM,
  FTYPE_SNAPSHOT_BMP,
  FTYPE_KEY_PB,
  FTYPE_KEY_REC,
  FTYPE_STATE_LOAD,
  FTYPE_STATE_SAVE,
  FTYPE_CFG,
  FTYPE_READ,
  FTYPE_WRITE,
  FTYPE_END
};


/* ファイル情報の構造体。実体は、 file-op.c にて定義する */

typedef struct OSD_FILE_STRUCT		OSD_FILE;



OSD_FILE *osd_fopen(int type, const char *path, const char *mode);
int	osd_fclose (OSD_FILE *stream);
int	osd_fflush (OSD_FILE *stream);
int	osd_fseek  (OSD_FILE *stream, long offset, int whence);
long	osd_ftell  (OSD_FILE *stream);
void	osd_rewind (OSD_FILE *stream);
size_t	osd_fread  (void *ptr, size_t size, size_t nobj, OSD_FILE *stream);
size_t	osd_fwrite (const void *ptr,size_t size,size_t nobj,OSD_FILE *stream);
int	osd_fputc  (int c, OSD_FILE *stream);
int	osd_fgetc  (OSD_FILE *stream);
char	*osd_fgets (char *str, int size, OSD_FILE *stream);
int	osd_fputs  (const char *str, OSD_FILE *stream);



/****************************************************************************
 * ファイル属性の取得
 *	filename で与えられたファイルの属性を返す
 *
 *	呼び出し元は、返って来た内容により以下の処理を行う。
 *
 *		FILE_STAT_NOEXIST … ファイルの新規作成を試みる。
 *						( fopen("filename", "w") ) 
 *		FILE_STAT_DIR     … ディレクトリを読み込みを試みる。
 *						( osd_opendir ) 
 *		FILE_STAT_FILE    … ファイルを読み書きを試みる。
 *				 ( fopen("filename", "r" or "r+" or "a") )
 *
 *	属性が不明の場合は・・・どうしよう・・・??
 *	とりあえず、 FILE_STAT_FILE を返すことにしておけば大きな問題はない ?
 *****************************************************************************/
#define	FILE_STAT_NOEXIST	(0)	/* 存在しない			*/
#define	FILE_STAT_DIR		(1)	/* ディレクトリ			*/
#define	FILE_STAT_FILE		(2)	/* ファイル			*/
int	osd_file_stat(const char *filename);

/****************************************************************************
 * 上書き用ファイルのパスの取得
 *  fullname で与えられたファイル名に相当する上書き用ファイルのパスを
 *  localname にセットする
 *
 * メモリ確保は呼び出し元から行う。
 ****************************************************************************/
void osd_file_localname(const char *fullname, char *localname);



/****************************************************************************
 * ディレクトリ閲覧
 *
 * T_DIR_INFO *osd_opendir(const char *filename)
 *	filename で指定されたディレクトリを開き、その情報をセットしたワーク
 *	へのポインタを返す。このポインタは、osd_readdir、osd_closedir にて使用
 *	呼び出し側が直接中身を参照することはない。
 *
 * const T_DIR_ENTRY *osd_readdir(T_DIR_INFO *dirp)
 *	osd_opendir にて開いたディレクトリからエントリを一つ読み取って、内容を
 *	T_DIR_ENTRY型のワークにセットし、そのポインタを返す。
 *
 *		typedef	struct {
 *		  int	 type; 		ファイルの種類 (下参照)
 *		  char	*name;		ファイル名 (アクセス用)
 *		  char	*str;		ファイル名 (表示用)
 *		} T_DIR_ENTRY;
 *
 *	ファイルの種類は、以下のいずれか
 *		FILE_STAT_DIR		osd_opendir で開くことが可能
 *						(ディレクトリなど)
 *		FILE_STAT_FILE		osd_opendir で開くことが出来ない
 *						(ファイルなど)
 *
 *	name は、osd_fopen でファイルを開く際に使用する、ファイル名。
 *	str  は、画面に表示する際の文字列で、例えばディレクトリ名は <> で囲む、
 *	といった装飾を施した文字列でもかまわない。(EUC or SJIS の漢字も可)
 *	なお、画面の表示は、 osd_readdir にて取得した順に行うので、予め適切な
 *	順序でソーティングされているのがよい。
 *
 * void osd_closedir(T_DIR_INFO *dirp)
 *	ディレクトリを閉じる。以降 dirp は使わない。
 *
 *****************************************************************************/

/* ディレクトリ情報構造体。実体は、file-op.c にて定義する */

typedef	struct	T_DIR_INFO_STRUCT	T_DIR_INFO;



/* ディレクトリエントリ構造体 */

typedef	struct {
  int	 type; 		/* ファイルの種類 (下参照)	*/
  char	*name;		/* ファイル名 (アクセス用)	*/
  char	*str;		/* ファイル名 (表示用)		*/
} T_DIR_ENTRY;


T_DIR_INFO	*osd_opendir(const char *filename);
T_DIR_ENTRY	*osd_readdir(T_DIR_INFO *dirp);
void		osd_closedir(T_DIR_INFO *dirp);



/****************************************************************************
 * パス名の操作
 *	パス名を、ディレクトリ名とファイル名に分離したり、つなげたりする。
 *	いずれの関数も、処理に失敗した場合は偽を返す。
 *
 * int	osd_path_normalize(const char *path, char resolved_path[], int size)
 *	path で与えられたパス名を展開し、 resolved_path にセットする。
 *	resolved_path のバッファサイズは、 size バイト。
 *
 * int	osd_path_split(const char *path, char dir[], char file[], int size)
 *	path で与えられたパス名をディレクトリ名とファイル名に分割して、
 *	dir, file にセットする。 dir, file のバッファサイズはともにsize バイト
 *
 * int	osd_path_join(const char *dir, const char *file, char path[], int size)
 *	dir, file 与えられたディレクトリ名とファイル名を結合したパス名を
 *	path にセットする。 path のバッファサイズは、size バイト
 *
 *****************************************************************************/
int osd_path_normalize(const char *path, char resolved_path[], int size);
int osd_path_split(const char *path, char dir[], char file[], int size);
int osd_path_join(const char *dir, const char *file, char path[], int size);



#endif	/* FILE_OP_H_INCLUDED */
