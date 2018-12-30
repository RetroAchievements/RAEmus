#ifndef GETCONF_H_INCLUDED
#define GETCONF_H_INCLUDED

#include "initval.h"

/*------------------------------------------------------------------------*/

typedef struct T_CONFIG_TABLE {

    int		group;	/* 分類 (排他的なオプションには同じ値を割り振る)  */
	 		/* QUASI88共通オプションは、    1〜299 を使用する */
	 		/* システム依存オプションは、 300〜349 を使用する */
	 		/* サウンド系のオプションは、 350〜399 を使用する */

    char	*name;	/* オプションの名前 (ハイフンなし)                */

    enum {		/* オプションの種類				  */
			/* ・エラー時は、変数へのセットは行われない	  */

	X_FIX,		/* 変数 var に int型の定数 var1 をセット	  */
			/* ・オプション引数は不可			  */
			/* ・var2 は使用しない				  */
			/*	*(int*)*var = (int)val1;		  */

	X_INT,		/* 変数 var に int型のオプション引数をセット	  */
			/* ・オプション引数の有効範囲 var1 ≦ 値 ≦ var2  */
			/* ・範囲外／オプション引数未指定時はエラー	  */
			/*	*((int*)var) = ARGV;			  */

	X_DBL,		/* 変数 var に double型のオプション引数をセット	  */
			/* ・オプション引数の有効範囲 var1 ≦ 値 ≦ var2  */
			/* ・範囲外／オプション引数未指定時はエラー	  */
			/*	*((double)var) = ARGV;			  */

	X_STR,		/* 変数 var に オプション引数の文字列をセット	  */
			/* ・実際は malloc された領域に、オプション引数   */
			/*   の文字列をコピーし、そのポインタをセット	  */
			/* ・オプション引数未指定時はエラー		  */
			/* ・var が NULL の場合は処理せずに正常とみなす	  */
			/* ・var1, var2 は使用しない			  */
			/*	wk = malloc(); strcpy(wk, argv);	  */
			/*	*(char **)var = wk;                	  */

	X_NOP,		/* なにも処理しない				  */
			/* ・変数 var が NULL なら、オプション引数不可	  */
			/* ・NULL でない場合は、オプション引数が必須で    */
			/*   未指定時はエラー				  */
			/* ・エラーがなければ、正常とする		  */
			/* ・var1, var2 は使用しない			  */

	X_INV		/* なにも処理しない				  */
			/* ・変数 var が NULL なら、オプション引数不可	  */
			/* ・NULL でない場合は、オプション引数が必須で    */
			/*   未指定時はエラー				  */
			/* ・常にエラーとする				  */
			/* ・var1, var2 は使用しない			  */
    }	type;

    void	*var;	/* ここで示す変数に値をセットする                 */
    double	val1;	/* 指定可能な最小値 または セットする定数         */
    double	val2;	/* 指定可能な最大値                               */

    int	(*func)(char *argv);	/* オプションの処理が正常であれば、
				 * 処理後にこの関数が呼び出される。
				 * argv には パラメータがセットされる。
				 * (ex. -opt 12 なら、"12")
				 * 
				 * この関数は、異常終了時は 0 でない値を
				 * 返し、その場合はエラーを表示する */

    int (*save_func)(const struct T_CONFIG_TABLE *op, char opt_arg[255]);

				/* 設定保存時に、呼び出される関数。
				     NULL の場合、そのオプションに該当する
				   設定は保存されない。
				     OPT_SAVE の場合、デフォルトの処理により
				   設定が保存される。
					X_FIX は、変数と定数が一致する場合に
					そのオプションが設定される。
					X_INT と X_DBL と X_STR は、変数の値を
					引数として、オプションが設定される)
				     それ以外の場合は、指定した関数が呼び出さ
				   れる。引数は、オプションテーブルのポインタ
				   と、0埋めされた opt_arg へのポインタ。
				   この関数は、オプションを設定に保存する場合
				   は、真を返す。この時 opt_arg にはオプション
				   引数をセットする。オプション引数が不要の
				   場合は、0埋めされたままにしておく */

	/* テーブルの終端は、すべて 0 / NULL をセットする */

} T_CONFIG_TABLE;


#define	OPT_SAVE	(int (*)(const struct T_CONFIG_TABLE *, char *))(-1)


/*------------------------------------------------------------------------*/

typedef struct {

    char	*d[NR_DRIVE];		/* ディスクイメージファイル名 */
    int		n[NR_DRIVE];		/* イメージ番号 */
    int		ro[NR_DRIVE];		/* ReadOnlyフラグ */

    char	*t[NR_TAPE];		/* テープイメージファイル名 */
    char	*prn;			/* プリンタ出力ファイル名 */
    char	*sin;			/* シリアル入力ファイル名 */
    char	*sout;			/* シリアル出力ファイル名 */

} T_CONFIG_IMAGE;

extern	T_CONFIG_IMAGE	config_image;	/* 引数で指定されたイメージファイル */

/*------------------------------------------------------------------------*/

extern	int	file_coding;	/* ファイル名の漢字コード 0:EUC/1:SJIS/2:UTF8*/
extern	int	save_config;	/* 真で、終了時に設定保存する		*/

int	config_init(int argc,
		    char *argv[],
		    const T_CONFIG_TABLE *osd_options,
		    void	(*osd_help)(void));
void	config_exit(void);

int	config_save(const char *fname);

#endif	/* GETCONF_H_INCLUDED */
