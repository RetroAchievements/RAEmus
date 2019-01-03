#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED


/***********************************************************************
 * グラフィック処理 (システム依存)
 ************************************************************************/

#include "screen.h"	/* PC88_PALETTE_T */


/***********************************************************************
 * 画面に関する、基本情報をまとめた構造体
 ************************************************************************/
typedef struct {

    int		window_max_width;	/* ウインドウ表示可能な、最大の	     */
    int		window_max_height;	/* 横サイズと縦サイズ (ピクセル数)   */
					/*	ウインドウ表示サイズに制約の */
					/*	無い場合は、適当に大きな値を */
					/*	セットしておく。(640以上)    */

    int		fullscreen_max_width;	/* フルスクリーン表示可能な、最大の  */
    int		fullscreen_max_height;	/* 横サイズと縦サイズ (ピクセル数)   */
					/*	フルスクリーン表示できない場 */
					/*	合は 0 をセットしておく      */

    int		forbid_status;		/* ステータス表示を禁止するなら、真  */

    int		forbid_half;		/* 半分サイズ表示を禁止するなら、真  */

} T_GRAPH_SPEC;


/***********************************************************************
 * 現在 QUASI88 が処理する対象となる画面の、情報をまとめた構造体
 *
 *
 *		<--- byte_per_line --->		(*) width は、実際に表示
 *		<-- width (*) -->		されるエリアのピクセル幅。
 *     buffer =>@---------------+-----+ -	バイト数では、
 *		|               |/////| ^	width * byte_per_pixel バイト
 *		|               |/////| |	となる。
 *		|               |/////| | height
 *		|               |/////| |
 *		|               |/////| v
 *		+---------------+/////| -
 *		|/////////////////////|
 *		+---------------------+
 *
 * (例) byte_per_pixel == 2 の時、画面バッファ上のピクセル (X,Y) には、
 *	((unsigned short *) buffer) + (Y * byte_per_line / byte_per_pixel) + X
 *	として、アクセス可能。
 *
 *	具体的には、 graph_add_color() にて取得した pixel の値をここに
 *	ライトすることになる。
 *
 ************************************************************************/
typedef struct {

    int		fullscreen;	/* 全画面モードなら真、ウインドウなら偽 */

    int		width;		/* 確保した画面の、描画可能エリア幅	*/
    int		height;		/* 確保した画面の、描画可能エリア高さ	*/

    int		byte_per_pixel;	/* 確保した画面のピクセルあたりバイト数	*/
				/*	1, 2, 4 のいずれか		*/

    int		byte_per_line;	/* 確保した画面の1ラインあたりバイト数	*/
				/*	(width * byte_per_line) 以上	*/

    void	*buffer;	/* 確保した画面のバッファ		*/
				/*	byte_per_pixel が、		*/
				/*		1 なら、 unsigned char	*/
				/*		2 なら、 unsigned short	*/
				/*		4 なら、 unsigned int	*/
				/*	で、アクセスできるポインタ	*/

    int		nr_color;	/* 確保した画面にて、利用可能な色の数	*/
				/*	同時に利用できる色の数に制約が	*/
				/*	ある場合、その色の数を返す。	*/
				/*	ただし、以下のいずれかとする。	*/
				/*		16, 24, 144, 255	*/
				/*	色の数に制約がない場合は、	*/
				/*		255 以上		*/

    int		write_only;	/* 画面のバッファが読出不可の場合、真	*/
				/*	直接フレームバッファを確保した	*/
				/*	場合などは、真にしておく。	*/

    int		broken_mouse;	/* マウスカーソル表示問題ありなら、真	*/
				/*	マウスカーソル表示に問題がある	*/
				/*	システムがあるようだ。		*/
				/*	   X11-DGA:マウスカーソルは基本	*/
				/*	           的に、表示されない。	*/
				/*	   SDL-DGA:マウスカーソルを表示	*/
				/*	           させると、画面にゴミ	*/
				/*	           が残る場合がある。	*/
				/*	これらのシステムでは、メニュー	*/
				/*	画面で独自のカーソル制御を行う	*/
				/*	ので、 TRUE をセットしておく。	*/
				/*	普通のシステムでは、問題はない	*/
				/*	と思うので、 FALSE をセットして	*/
				/*	おくこと。			*/

    void	(*draw_start)(void);
    void	(*draw_finish)(void);
				/* 画面のバッファにアクセスする際に、	*/
				/* その前後に呼び出される関数		*/
				/*	buffer のライトに際して、必要な	*/
				/*	内部処理がある場合、それを記述	*/
				/*	した関数をここで設定しておく。	*/
				/*	特に必要がなければ、 NULL を	*/
				/*	設定しておく。			*/

    int		dont_frameskip;	/* フレームスキップを禁止するなら、真	*/
				/*	真ならば、フレーム毎に必ず 	*/
				/*	graph_update が呼び出される。	*/
				/*	描画をスキップさせたい場合は、	*/
				/*	graph_update 内部にて独自に	*/
				/*	スキップ処理を組み込むこと。	*/

} T_GRAPH_INFO;



/****************************************************************************
 * グラフィック処理の初期化／終了
 *
 * const T_GRAPH_SPEC	*graph_init(void)
 *
 *	システム依存のグラフィック処理の初期化を行う。
 *	初期化に成功したら、画面に関する基本情報を T_GRAPH_SPEC 型変数への
 *	ポインタにて返す。処理に失敗した場合は、 NULL を返す。
 *
 *	o QUASI88 は、起動時に 1回だけこの関数を呼び出し、
 *	  全画面モードが可能か、画面サイズ切り替えは可能か、などを判断する。
 *
 * void	graph_exit(void)
 *
 *	システム依存のグラフィック処理の後始末を行う。
 *	この関数は、終了時に 1回だけ呼び出される。
 *
 *****************************************************************************/
const T_GRAPH_SPEC	*graph_init(void);
void			graph_exit(void);



/****************************************************************************
 * グラフィック処理の設定
 *
 * const T_GRAPH_INFO	*graph_setup(int width, int height,
 *				     int fullscreen, double aspect)
 *
 *	ウインドウを作成・リサイズする。または全画面モードに切り替える。
 *	width, height は、ウインドウないし全画面モードのサイズ。
 *	必ず、 width は 8の倍数、 height は 2の倍数がセットされる。
 *	fullscreen が真なら、全画面モードにする。
 *	aspect は、モニターのアスペクト比 (縦横比)。
 *	4:3型なら 1.3333、 16:9型なら 1.7778、設定なしなら 0.0 となる。
 *	fullscreen が真で、 aspect > 0.0 ならアスペクト比を考慮した全画面モード
 *	にするとうれしいかも。(この引数は無視しても、実害はないだろう)
 *
 *	必ずしも、指定通りにならなくてもよい。
 *	全画面モードの切り替えに失敗したら、ウインドウで表示してもよいし、
 *	指定サイズの全画面モードが無理なら、それよりも『大きな』全画面モードに
 *	してもよい。(指定よりも小さなサイズにしてはいけない)
 *
 *	最終的にどのような処理結果になったかは T_GRAPH_INFO 型変数へのポインタ
 *	にて返す。処理に失敗し、続行不能となった場合は、 NULL を返す。
 *
 *	o QUASI88 は、 graph_init() 後に、この関数を呼び出す。
 *	  画面サイズや、全画面モード切替の際にも、この関数を呼び出す。
 *	  QUASI88 はウインドウを1個しか使わないので、2度目以降の呼び出し時は、
 *	  以前のウインドウや色などの、すべての情報は破棄する。
 *
 *		<QUASI88 の画面サイズ切り替え時の処理>
 *
 *		  ：
 *		graph_setup();
 *		graph_add_color(全ての色);
 *		graph_update(全画面領域);
 *		  ：
 *
 ****************************************************************************/
const T_GRAPH_INFO	*graph_setup(int width, int height,
				     int fullscreen, double aspect);


/****************************************************************************
 * 色の確保と破棄
 *
 * void	graph_add_color(const PC88_PALETTE_T color[],
 *			int nr_color, unsigned long pixel[])
 *
 *	color は今から使用したい色で、 nr_color 個の配列。
 *	この color に対応するピクセル値を nr_color 個の配列 pixel に返す。
 *
 *	例えば、(T_GRAPH_INFO *)->byte_per_pixel が 2 である場合、QUASI88 は
 *	pixel にて返されたピクセル値を unsigned short 型にキャストして、
 *	(T_GRAPH_INFO *)->buffer に直接ライトすることになる。
 *
 *	o TrueColor の環境では、 color の RGB値をそのままピクセルフォーマットに
 *	  変換して、pixel[] にセットすればよいはず。
 *
 *
 * void	graph_remove_color(int nr_pixel, unsigned long pixel[])
 *
 *	pixel は、使用しなくなった色で、 nr_pixel 個の配列。
 *	graph_add_color() にて取得した pixel の値がセットされている。
 *
 *	o TrueColor の環境では、特に何も処理する必要はないハズ。
 *
 *	o 使用できる色数が、システムによって制限されている場合、色の管理が必要
 *	  になることがある。 (X11 の PseudoColor で共有カラーを使う場合など)
 * 
 *	  QUASI88 が graph_remove_color を呼び出す場合、その引数の pixel は、
 *	  直前の graph_add_color() にて確保した nr_color 個の pixel と
 *	  同じとなるようにしてある。 (つまり、LIFO方式)
 *
 *		  ：
 *		graph_add_color(8色);		ステータス用に8色確保する
 *		  ：
 *		graph_add_color(16色);		メイン用に16色確保する
 *		  ：
 *		graph_remove_color(16色);	直前の16色を破棄する
 *		  ：
 *		graph_remove_color(8色);	さらにその前の8色を破棄する
 *		  ：
 *
 *	  このことを前提として、 graph_add_color(), graph_remove_color() を
 *	  実装する必要がある。
 *
 *****************************************************************************/
void	graph_add_color(const PC88_PALETTE_T color[],
			int nr_color, unsigned long pixel[]);
void	graph_remove_color(int nr_pixel, unsigned long pixel[]);



/***********************************************************************
 * 画面の更新
 *
 * void	graph_update(int nr_rect, T_SCREEN_RECT rect[])
 *	画像データを実際に描画する。
 *	描画したいエリアが、T_SCREEN_RECT 型の配列 rect[] に nr_rect 個分、
 *	セットされているので、このエリアを描画する。
 *
 *	o QUASI88 は、表示の際に以下の順に処理する。
 *
 *	    if (フレームスキップしない?) {		※1
 *		if (パレット変化あり?) {
 *		    graph_remove_color(前回のパレットで使った色);
 *		    graph_add_color(今回のパレットで使う色);
 *		    画面変化ありとする
 *		}
 *		if (画面変化あり?) {
 *		    if (draw_start != NULL) {		※2
 *			(*draw_start)();
 *		    }
 *		    buffer 更新する
 *		    if (draw_finish != NULL) {		※2
 *			(*draw_finish)();
 *		    }
 *		    graph_update(画面上の、更新された矩形領域);
 *	    }
 *
 *		※1 graph_setup() にて、 dont_frameskip が真であった場合、
 *		    常時、フレームスキップしない
 *		※2 graph_setup() にて、 draw_start / draw_finish の
 *		    関数ポインタが設定された場合のみ、呼び出す。
 *
 ************************************************************************/
typedef struct {
    int x, y;
    int width, height;
} T_GRAPH_RECT;

void	graph_update(int nr_rect, T_GRAPH_RECT rect[]);



/***********************************************************************
 * 属性の設定
 *	これらは実装しなくても、おそらくエミュの動作には影響はない。
 *	システムがサポートしている項目があれば、それらを実装すればよい。
 *
 * void	graph_set_window_title(const char *title)
 *	ウインドウのタイトルを設定する。
 *	title がその文字列で、 255文字以内の ASCII。
 *
 * void	graph_set_attribute(int mouse_show, int grab, int keyrepeat_on)
 *	さまざまな属性を設定する。
 *	mouse_show が真なら、マウスカーソルを表示する。偽なら消去する。
 *	grab が 真なら、マウス・キー入力をグラブする。偽ならしない。
 *	keyrepeat_on が真なら、キーリピートをオンにする。偽ならオフにする。
 *
 ************************************************************************/
void	graph_set_window_title(const char *title);
void	graph_set_attribute(int mouse_show, int grab, int keyrepeat_on);



#endif	/* GRAPH_H_INCLUDED */
