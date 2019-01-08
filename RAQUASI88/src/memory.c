/************************************************************************/
/*									*/
/* メモリ確保 & ROMファイルロード					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "memory.h"
#include "pc88main.h"

#include "soundbd.h"		/* sound_board, sound2_adpcm	*/

#include "menu.h"		/* menu_lang	*/
#include "file-op.h"
#include "suspend.h"



int	set_version;	/* 起動時のバージョン強制変更 '0' 〜 '9'	*/
int	rom_version;	/* (変更前の) BASIC ROMバージョン		*/

int	use_extram	= DEFAULT_EXTRAM;	/* 拡張RAMのカード数	*/
int	use_jisho_rom	= DEFAULT_JISHO;	/* 辞書ROMを使う	*/
int	use_built_in_font = FALSE;		/* 内蔵フォントを使う	*/
int	use_pcg = FALSE;			/* PCG-8100サポート	*/
int	font_type = 0;				/* フォントの種類	*/
int	font_loaded = 0;			/* ロードしたフォント種	*/

int	memory_wait = FALSE;			/* メモリウェイトの有無	*/

char	*file_compatrom = NULL;			/* P88SR emu のROMを使う*/

int	has_kanji_rom   = FALSE;		/* 漢字ROMの有無	*/

int	linear_ext_ram = TRUE;			/* 拡張RAMを連続させる	*/


/*----------------------------------------------------------------------*/
/* 内蔵フォントデータ							*/
/*----------------------------------------------------------------------*/
#include "font.h"


/*----------------------------------------------------------------------*/
/* 漢字ダミーROM (漢字ROMが無い時のダミー)				*/
/*----------------------------------------------------------------------*/
byte	kanji_dummy_rom[16][2] =
{
  { 0xaa, 0xaa, },	/* o o o o o o o o  */
  { 0x00, 0x00, },	/*                  */
  { 0x80, 0x02, },	/* o             o  */
  { 0x00, 0x00, },	/*                  */
  { 0x80, 0x02, },	/* o             o  */
  { 0x00, 0x00, },	/*                  */
  { 0x80, 0x02, },	/* o             o  */
  { 0x00, 0x00, },	/*                  */
  { 0x80, 0x02, },	/* o             o  */
  { 0x00, 0x00, },	/*                  */
  { 0x80, 0x02, },	/* o             o  */
  { 0x00, 0x00, },	/*                  */
  { 0x80, 0x02, },	/* o             o  */
  { 0x00, 0x00, },	/*                  */
  { 0xaa, 0xaa, },	/* o o o o o o o o  */
  { 0x00, 0x00, },	/*                  */
};


/*----------------------------------------------------------------------*/
/* ROMファイル名							*/
/*----------------------------------------------------------------------*/
enum {
  N88_ROM,  EXT0_ROM, EXT1_ROM,  EXT2_ROM, EXT3_ROM,  N_ROM,     SUB_ROM,
  KNJ1_ROM, KNJ2_ROM, JISHO_ROM, FONT_ROM, FONT2_ROM, FONT3_ROM, ROM_END
};
static char *rom_list[ ROM_END ][5] =
{
  {  "N88.ROM",		"n88.rom",	0,		0,		0, },
  {  "N88EXT0.ROM",	"n88ext0.rom",	"N88_0.ROM",	"n88_0.rom",	0, },
  {  "N88EXT1.ROM",	"n88ext1.rom",	"N88_1.ROM",	"n88_1.rom",	0, },
  {  "N88EXT2.ROM",	"n88ext2.rom",	"N88_2.ROM",	"n88_2.rom",	0, },
  {  "N88EXT3.ROM",	"n88ext3.rom",	"N88_3.ROM",	"n88_3.rom",	0, },
  {  "N88N.ROM",	"n88n.rom",	"N80.ROM",	"n80.rom",	0, },
  {  "N88SUB.ROM",	"n88sub.rom",	"DISK.ROM",	"disk.rom",	0, },
  {  "N88KNJ1.ROM",	"n88knj1.rom",	"KANJI1.ROM",	"kanji1.rom",	0, },
  {  "N88KNJ2.ROM",	"n88knj2.rom",	"KANJI2.ROM",	"kanji2.rom",	0, },
  {  "N88JISHO.ROM",	"n88jisho.rom",	"JISYO.ROM",	"jisyo.rom",	0, },
  {  "FONT.ROM",	"font.rom",	0,		0,		0, },
  {  "FONT2.ROM",	"font2.rom",	0,		0,		0, },
  {  "FONT3.ROM",	"font3.rom",	0,		0,		0, },
};



byte	*main_rom;			/* メイン ROM [0x8000] (32KB)	*/
byte	(*main_rom_ext)[0x2000];	/* 拡張 ROM [4][0x2000](8KB *4)	*/
byte	*main_rom_n;			/* N-BASIC [0x8000]    (32KB)	*/
byte	*main_ram;			/* メイン RAM [0x10000](64KB)	*/
byte	*main_high_ram;			/* 高速 RAM(の裏)[0x1000] (4KB)	*/
byte	(*kanji_rom)[65536][2];		/* 漢字ＲＯＭ[2][65536][2]	*/
byte	*sub_romram;			/* サブ ROM/RAM [0x8000] (32KB)	*/

byte	(*ext_ram)[0x8000];		/* 拡張 RAM[4][0x8000](32KB*4〜)*/
byte	(*jisho_rom)[0x4000];		/* 辞書 ROM[32][0x4000](16KB*32)*/

bit8	(*main_vram)[4];		/* VRAM[0x4000][4](=G/R/G/pad)	*/
bit8	*font_rom;			/* フォントイメージROM[8*256*2]	*/

bit8	*font_pcg;			/* フォントイメージROM(PCG用)	*/
bit8	*font_mem;			/* フォントイメージROM(固定 )	*/
bit8	*font_mem2;			/* フォントイメージROM(固定2)	*/
bit8	*font_mem3;			/* フォントイメージROM(固定3)	*/

byte	*dummy_rom;			/* ダミーROM (32KB)		*/
byte	*dummy_ram;			/* ダミーRAM (32KB)		*/





/*----------------------------------------------------------------------
 * メモリアロケート
 *----------------------------------------------------------------------*/
static	int	mem_alloc_result;

static	void	mem_alloc_start( const char *msg )	/* メモリ確保開始 */
{
  if( verbose_proc ){ printf( "%s", msg ); }

  mem_alloc_result = TRUE;
}

static	void	*mem_alloc( size_t size )		/* メモリ確保 */
{
  void	*ptr = malloc( size );

  if( ptr == NULL ){ 
    mem_alloc_result = FALSE;
  }
  return ptr;
}

static	int	mem_alloc_finish( void )		/* メモリ確保完了(偽で失敗) */
{
  if( verbose_proc ){ 
    if( mem_alloc_result == FALSE ){ printf( "FAILED\n" ); }
    else                           { printf( "OK\n" );     }
  }

  return mem_alloc_result;
}



/*----------------------------------------------------------------------
 * ROM イメージファイルをメモリにロード
 *----------------------------------------------------------------------*/

/*
 * 通常のROMイメージロード
 *	int load_rom( char *filelist[], byte *ptr, int size, int disp_flag )
 *		filelist … オープンするファイル名一覧のリスト
 *		ptr      … このポインタの先にロードする
 *		size     … ロードするサイズ
 *		disp_flag… verbose_proc 時に表示するメッセージ形式
 *				DISP_FNAME	開いたファイル名だけ表示
 *							"filename ..."
 *						戻り値をみて"OK\n"等を付加する
 *						ファイルが開けなかったら、
 *							"filename not found"
 *						必要に応じて "\n" を付加する
 *
 *				DISP_RESULT	開いたファイル名と結果を表示
 *							"filename ... OK\n"
 *						ファイルが開けなかったら、
 *							"filename not found\n"
 *
 *				DISP_IF_EXIST	開いたファイル名と結果を表示
 *							"filename ..."
 *						戻り値をみて"OK\n"等を付加する
 *						ファイルが開けなかったら、
 *							なにも表示しない。
 *
 *		戻値     … 読み込んだサイズ。ファイルが開けなかったら -1
 */
#define		DISP_FNAME	(0)
#define		DISP_RESULT	(1)
#define		DISP_IF_EXIST	(2)

static	int	load_rom( char *filelist[], byte *ptr, int size, int disp )
{
  OSD_FILE *fp;
  int i=0, load_size = -1;
  char buf[ OSD_MAX_FILENAME ];
  const char *dir = osd_dir_rom();

  if( dir ){
    for( i=0; filelist[i] ; i++ ){

      if( osd_path_join( dir, filelist[i], buf, OSD_MAX_FILENAME )==FALSE )
	break;

      if( (fp = osd_fopen( FTYPE_ROM, buf, "rb" )) ){

	load_size = osd_fread( ptr, sizeof(byte), size, fp );
	osd_fclose( fp );

	break;
      }

    }
  }

  if( load_size < 0 ){			/* ファイル見つからず */
    memset( ptr, 0xff, size );
  }else if( load_size < size ){		/* 見つかったけど、サイズが足りない */
    memset( &ptr[load_size], 0xff, size-load_size );
  }


  if( verbose_proc ){
    if( load_size < 0 ){
      if( disp == DISP_FNAME  ) printf( "  %-12s ... ",           filelist[0]);
      if( disp == DISP_RESULT ) printf( "  %-12s ... Not Found\n",filelist[0]);
    }else{
      printf( "  Found %-12s : Load...", filelist[i] );
      if( disp == DISP_RESULT ){
	if( load_size == size ){ printf( "OK\n" );     }
	else                   { printf( "FAILED\n" ); }
      }
    }
  }

  return load_size;
}



/*
 * 結合形式のROMイメージロード
 *	OSD_FILE *load_compat_rom_open( void )
 *	void load_compat_rom_close( OSD_FILE *fp )
 *		ファイルを開く / 閉じる
 *	int load_compat_rom( byte *ptr, long pos, int size, OSD_FILE *fp )
 *		ファイルfp の先頭 posバイト目から sizeバイトを ptrにロード。
 *		戻値はロードしたサイズ。シーク失敗なら -1。
 *	途中で1度でも処理に失敗したら、 load_compat_rom_success==FALSE になる
 */
static	int	load_compat_rom_success;

static	OSD_FILE *load_compat_rom_open( void )
{
  OSD_FILE *fp = osd_fopen( FTYPE_ROM, file_compatrom, "rb" );

  if( verbose_proc ){
    if( fp ){ printf( "  Found %-12s : Load...", file_compatrom ); }
    else    { printf( "  %-12s ... Not Found\n", file_compatrom ); }
  }

  if( fp==NULL ) load_compat_rom_success = FALSE;
  else           load_compat_rom_success = TRUE;

  return fp;
}

static	int	load_compat_rom( byte *ptr, long pos, int size, OSD_FILE *fp )
{
  int load_size = -1;

  if( fp ){
    if( osd_fseek( fp, pos, SEEK_SET ) == 0 ){

      load_size = osd_fread( ptr, 1, size, fp );

      if( load_size < size ){
	memset( &ptr[load_size], 0xff, size-load_size );
      }
    }

    if( load_size != size ) load_compat_rom_success = FALSE;
  }

  return load_size;
}

static	void	load_compat_rom_close( OSD_FILE *fp )
{
  if( fp ) osd_fclose( fp );
}










/****************************************************************************
 * エミュレーションに使用するメモリの確保と、ROMイメージのロード
 *
 *	peachさんにより、M88 の ROMファイルも使えるように拡張されました	
 *	→ その後の大幅にソースを修正しました。
 *****************************************************************************/

#define	FONT_SZ	(8*256*1)

int	memory_allocate( void )
{
  int	size;


		/* 標準メモリを確保 */

  mem_alloc_start( "Allocating memory for standard ROM/RAM..." );
  {
    main_rom     = (byte *)           mem_alloc( sizeof(byte) *  0x8000 );
    main_rom_ext = (byte(*)[0x2000])  mem_alloc( sizeof(byte) *  0x2000 *4 );
    main_rom_n   = (byte *)           mem_alloc( sizeof(byte) *  0x8000 );
    sub_romram   = (byte *)           mem_alloc( sizeof(byte) *  0x8000 );

    main_ram     = (byte *)           mem_alloc( sizeof(byte) * 0x10000 );
    main_high_ram= (byte *)           mem_alloc( sizeof(byte) *  0x1000 );
    main_vram    = (byte(*)[0x4])     mem_alloc( sizeof(byte) *  0x4000 *4 );

    kanji_rom    = (byte(*)[65536][2])mem_alloc( sizeof(byte)*2*65536*2 );

    font_pcg     = (byte *)           mem_alloc( sizeof(byte)*8*256*2 );
    font_mem     = (byte *)           mem_alloc( sizeof(byte)*8*256*2 );
    font_mem2    = (byte *)           mem_alloc( sizeof(byte)*8*256*2 );
    font_mem3    = (byte *)           mem_alloc( sizeof(byte)*8*256*2 );
  }
  if( mem_alloc_finish()==FALSE ){
    return 0;
  }




		/* ROMイメージをファイルから読み込む */

  if( file_compatrom == NULL ){		/* 通常のROMイメージファイル */

    load_rom( rom_list[ N88_ROM ],  main_rom,        0x8000, DISP_RESULT );
    load_rom( rom_list[ EXT0_ROM ], main_rom_ext[0], 0x2000, DISP_RESULT );
    load_rom( rom_list[ EXT1_ROM ], main_rom_ext[1], 0x2000, DISP_RESULT );
    load_rom( rom_list[ EXT2_ROM ], main_rom_ext[2], 0x2000, DISP_RESULT );
    load_rom( rom_list[ EXT3_ROM ], main_rom_ext[3], 0x2000, DISP_RESULT );
    load_rom( rom_list[ N_ROM ],    main_rom_n,      0x8000, DISP_RESULT );

    size = load_rom( rom_list[ SUB_ROM ], sub_romram, 0x2000, DISP_FNAME );
    {
      if( verbose_proc ){
	if     ( size <       0 ){ printf( "Not Found\n" );    }
	else if( size ==  0x800 ){ printf( "OK(2D-type)\n" );  }
	else if( size == 0x2000 ){ printf( "OK(2HD-type)\n" ); }
	else                     { printf( "FAILED\n");        }
      }
      if( size <= 0x800 ){
	memcpy( &sub_romram[0x0800], &sub_romram[0x0000], 0x0800 );
	memcpy( &sub_romram[0x1000], &sub_romram[0x0000], 0x1000 );
      }
    }

  }else{				/* 結合形式ROMイメージファイル */

    OSD_FILE *fp = load_compat_rom_open();
    if( fp ){
      load_compat_rom( main_rom,                  0, 0x8000, fp );
      load_compat_rom( main_rom_ext[0],     0x0c000, 0x2000, fp );
      load_compat_rom( main_rom_ext[1],     0x0e000, 0x2000, fp );
      load_compat_rom( main_rom_ext[2],     0x10000, 0x2000, fp );
      load_compat_rom( main_rom_ext[3],     0x12000, 0x2000, fp );
      load_compat_rom( &main_rom_n[0x6000], 0x08000, 0x2000, fp );
      load_compat_rom( sub_romram,          0x14000, 0x2000, fp );

      if( load_compat_rom_success == FALSE ){	/* ここ迄で失敗があれば終了 */
	if( verbose_proc ){ printf( "FAILED\n"); }
      }
      else{					/* 成功なら N-BASIC ロード */
	size = load_compat_rom( main_rom_n, 0x16000, 0x6000, fp );
	if( verbose_proc ){
	  if     ( size ==      0 ){ printf( "OK (Without N-BASIC)\n" ); }
	  else if( size == 0x6000 ){ printf( "OK (With N-BASIC)\n" );    }
	  else                     { printf( "FAILED\n");                }
	}
      }

      load_compat_rom_close( fp );
    }
  }
					/* SUB側 ROM ミラー生成、RAMクリア */
  memcpy( &sub_romram[0x2000], &sub_romram[0x0000], 0x2000 );
  memset( &sub_romram[0x4000], 0xff, 0x4000 );
  


		/* 漢字ROMイメージをファイルから読み込む */

  size=load_rom( rom_list[ KNJ1_ROM ], kanji_rom[0][0], 0x20000, DISP_RESULT );

  if( size == 0x20000 ){
    has_kanji_rom = TRUE;
  }else{
    has_kanji_rom = FALSE;
    menu_lang = MENU_ENGLISH;
  }

  load_rom( rom_list[ KNJ2_ROM ], kanji_rom[1][0], 0x20000, DISP_RESULT );



		/* フォントROMイメージをファイルから読み込む */
		/*   (セミグラフィック文字は内蔵文字を使用)  */

  if( use_built_in_font ){

    memcpy( &font_mem[0], &built_in_font_ANK[0], 0x100*8 );

  }else{

    size = load_rom( rom_list[ FONT_ROM ],  font_mem,  FONT_SZ, DISP_FNAME );
    font_loaded |= 1;

    if( verbose_proc ){
      if( size == FONT_SZ ){ printf( "OK\n" ); }
      else{
	if( size < 0 ){ printf( "Not Found " ); }
	else          { printf( "FAILED ");     }
	if( has_kanji_rom ){ printf( "(Use KANJI-ROM font)\n" ); }
	else               { printf( "(Use built-in font)\n" );  }
      }
    }

    if( size != FONT_SZ ){	/* 異常時は、漢字ROMか内蔵フォントを使用 */
      if( has_kanji_rom ){
	memcpy( &font_mem[0], &kanji_rom[0][(1<<11)][0], 0x100*8 );
      }else{
	memcpy( &font_mem[0], &built_in_font_ANK[0], 0x100*8 );
	font_loaded &= ~1;
      }
    }
  }

  memcpy( &font_mem[0x100*8], &built_in_font_graph[0], 0x100*8 );



		/* 第2フォントイメージをファイルから読み込む */

  if( use_built_in_font ){

    memcpy( &font_mem2[0],       &built_in_font_ANH[0],   0x100*8 );
    memcpy( &font_mem2[0x100*8], &built_in_font_graph[0], 0x100*8 );

  }else{

    size = load_rom( rom_list[FONT2_ROM], font_mem2, FONT_SZ*2, DISP_IF_EXIST);
    font_loaded |= 2;

    if( verbose_proc ){
      if     ( size == -1 ) ;
      else if( size == FONT_SZ*2 ){ printf( "OK(with semi-graphic-font)\n" ); }
      else if( size == FONT_SZ   ){ printf( "OK\n" );                         }
      else                        { printf( "FAILED\n" ); }
    }

    if( size == FONT_SZ*2 ){
      ;
    }else{
      if( size == FONT_SZ ){
	;
      }else{			/* 存在しない場合は、内蔵フォントを使用 */
	memcpy( &font_mem2[0],     &built_in_font_ANH[0],   0x100*8 );
	font_loaded &= ~2;
      }
      memcpy( &font_mem2[0x100*8], &built_in_font_graph[0], 0x100*8 );
    }

  }


		/* 第3フォントイメージをファイルから読み込む */

  if( use_built_in_font ){

    memset( &font_mem3[0], 0, FONT_SZ*2 );

  }else{

    size = load_rom( rom_list[FONT3_ROM], font_mem3, FONT_SZ*2, DISP_IF_EXIST);
    font_loaded |= 4;

    if( verbose_proc ){
      if     ( size == -1 ) ;
      else if( size == FONT_SZ*2 ){ printf( "OK(with semi-graphic-font)\n" ); }
      else if( size == FONT_SZ   ){ printf( "OK\n" );                         }
      else                        { printf( "FAILED\n" ); }
    }

    if( size == FONT_SZ*2 ){
      ;
    }else{
      if( size == FONT_SZ ){
	memcpy( &font_mem3[0x100*8], &built_in_font_graph[0], 0x100*8 );

      }else{			/* 存在しない場合は、透明フォントを使用 */
	memset( &font_mem3[0], 0, FONT_SZ*2 );
	font_loaded &= ~4;
      }
    }

  }


		/* フォントの文字コード 0 は絶対に空白 */
  memset( &font_mem[0],  0, 8 );
  memset( &font_mem2[0], 0, 8 );
  memset( &font_mem3[0], 0, 8 );


  memory_reset_font();



		/* ROMのバージョンを保存 */
  rom_version = ROM_VERSION;

		/* オプショナルなメモリを確保 */

  if( memory_allocate_additional()== FALSE ){
    return 0;
  }


  return 1;
}



/****************************************************************************
 *
 *
 *****************************************************************************/
static	int	alloced_extram = 0;		/* 確保した拡張RAMの数	*/

int	memory_allocate_additional( void )
{

		/* 拡張メモリを確保 */

  if( use_extram ){

    if (use_extram <= 4 ||
	BETWEEN(8, use_extram, 10) ||
	use_extram == 16) {
	;
    } else {
	linear_ext_ram = TRUE;
    }

				/* 確保済みサイズが小さければ、確保しなおし */
    if( ext_ram && alloced_extram < use_extram ){
      free( ext_ram );
      ext_ram = NULL;
    }

    if( ext_ram == NULL ){

      char msg[80];
      sprintf( msg, "Allocating memory for Extended RAM(%dKB)...",
			use_extram * 128 );

      mem_alloc_start( msg );

      ext_ram = (byte(*)[0x8000])mem_alloc( sizeof(byte)*0x8000 *4*use_extram);

      if( dummy_rom == NULL )
	dummy_rom = (byte *)     mem_alloc( sizeof(byte) * 0x8000 );
      if( dummy_ram == NULL )
	dummy_ram = (byte *)     mem_alloc( sizeof(byte) * 0x8000 );

      if( mem_alloc_finish()==FALSE ){
	return 0;
      }

      alloced_extram = use_extram;
    }

    memset( &ext_ram[0][0], 0xff, 0x8000 * 4*use_extram );
    memset( &dummy_rom[0],  0xff, 0x8000 );
  }


		/* 辞書ROM用メモリを確保 */
		/* 辞書ROMイメージをファイルから読み込む */

  if( use_jisho_rom ){

    if( jisho_rom == NULL ){

      mem_alloc_start( "Allocating memory for Jisho ROM..." );

      jisho_rom = (byte(*)[0x4000])mem_alloc( sizeof(byte) * 0x4000*32 );

      if( mem_alloc_finish()==FALSE ){
	return 0;
      }

      load_rom( rom_list[ JISHO_ROM ], jisho_rom[0], 0x4000*32, DISP_RESULT );

    }
  }


		/* サウンドボードII ADPCM用RAMを確保 */

  if( sound_board==SOUND_II ){

    if( sound2_adpcm == NULL ){

      mem_alloc_start( "Allocating memory for ADPCM RAM..." );

      sound2_adpcm = (byte *)mem_alloc( sizeof(byte) * 0x40000 );

      if( mem_alloc_finish()==FALSE ){
	return 0;
      }

    }

    memset( &sound2_adpcm[0],  0xff, 0x40000 );
  }

  return 1;
}







/****************************************************************************
 * フォント初期化
 *	PCGフォントデータを通常のフォントで初期化
 *****************************************************************************/
void	memory_reset_font( void )
{
  memcpy( font_pcg, font_mem, sizeof(byte)*8*256*2 );

  memory_set_font();
}



/****************************************************************************
 * 使用するフォントを決定
 *
 *****************************************************************************/
void	memory_set_font( void )
{
  if( use_pcg ){
    font_rom = font_pcg;
  }else{
    if     ( font_type == 0 ) font_rom = font_mem;
    else if( font_type == 1 ) font_rom = font_mem2;
    else if( font_type == 2 ) font_rom = font_mem3;
  }
}



/****************************************************************************
 * 確保したメモリの解放
 *	終了するんなら解放する必要もないけど…
 *****************************************************************************/
void	memory_free( void )
{
  if( main_rom )     free( main_rom );
  if( main_rom_ext ) free( main_rom_ext );
  if( main_rom_n )   free( main_rom_n );
  if( sub_romram)    free( sub_romram );

  if( main_ram)      free( main_ram );
  if( main_high_ram )free( main_high_ram );
  if( main_vram )    free( main_vram );

  if( kanji_rom )    free( kanji_rom );

  if( font_pcg )     free( font_pcg );
  if( font_mem )     free( font_mem );
  if( font_mem2 )    free( font_mem2 );
  if( font_mem3 )    free( font_mem3 );

  if( use_extram )    free( ext_ram );
  if( use_jisho_rom ) free( jisho_rom );
  if( sound_board==SOUND_II ) free( sound2_adpcm );
}







/***********************************************************************
 * ステートロード／ステートセーブ
 ************************************************************************/

#define	SID		"MEM "
#define	SID_MAIN	"MEM0"
#define	SID_HIGH	"MEM1"
#define	SID_SUB		"MEM2"
#define	SID_VRAM	"MEM3"
#define	SID_PCG		"MEM4"
#define	SID_ADPCM	"MEMA"
#define	SID_ERAM	"MEMB"
#define	SID2		"MEM5"

static	T_SUSPEND_W	suspend_memory_work[]=
{
  { TYPE_INT,	&set_version,		},
  { TYPE_INT,	&use_extram,		},
  { TYPE_INT,	&use_jisho_rom,		},

  { TYPE_INT,	&use_pcg,		},

  { TYPE_END,	0			},
};

static	T_SUSPEND_W	suspend_memory_work2[]=
{
  { TYPE_INT,	&linear_ext_ram,	},
  { TYPE_END,	0			},
};


int	statesave_memory( void )
{
  if( statesave_table( SID, suspend_memory_work ) != STATE_OK ) return FALSE;

  if( statesave_table( SID2, suspend_memory_work2 ) != STATE_OK ) return FALSE;

  /* 通常メモリ */

  if( statesave_block( SID_MAIN, main_ram,           0x10000  ) != STATE_OK )
								return FALSE;
  if( statesave_block( SID_HIGH, main_high_ram,      0x1000   ) != STATE_OK )
								return FALSE;
  if( statesave_block( SID_SUB, &sub_romram[0x4000], 0x4000   ) != STATE_OK )
								return FALSE;
  if( statesave_block( SID_VRAM, main_vram,          4*0x4000 ) != STATE_OK )
								return FALSE;
  if( statesave_block( SID_PCG,  font_pcg,           8*256*2  ) != STATE_OK )
								return FALSE;

  /* オプショナルなメモリ */

  if( sound_board==SOUND_II ){
    if( statesave_block( SID_ADPCM, sound2_adpcm,    0x40000  ) != STATE_OK )
								return FALSE;
  }
  if( use_extram ){
    if( statesave_block( SID_ERAM, ext_ram, 0x8000*4*use_extram ) != STATE_OK )
								return FALSE;
  }
  return TRUE;
}

int	stateload_memory( void )
{
  if( stateload_table( SID, suspend_memory_work ) != STATE_OK ) return FALSE;

  /* 〜0.6.3 はステートセーブ保存時に、 ROMバージョン値をバージョン強制変更値
     として 保存している。(つまり、オプション -server X 指定状態になっている)
     0.6.4〜 では、そのようなことは行わない。(別途 ROM バージョンを管理)
     特に互換性に実害はないと思うが…。以下を加えればさらに安全? */
  /* if (set_version == rom_version) set_version = 0; */


  if( stateload_table( SID2, suspend_memory_work2 ) != STATE_OK ) {

    /* 旧バージョンなら、みのがす */

    printf( "stateload : Statefile is old. (ver 0.6.0, 1, 2 or 3?)\n" );

    linear_ext_ram = TRUE;
  }


  /* 通常メモリ */

  if( stateload_block( SID_MAIN, main_ram,           0x10000  ) != STATE_OK )
								return FALSE;
  if( stateload_block( SID_HIGH, main_high_ram,      0x1000   ) != STATE_OK )
								return FALSE;
  if( stateload_block( SID_SUB, &sub_romram[0x4000], 0x4000   ) != STATE_OK )
								return FALSE;
  if( stateload_block( SID_VRAM, main_vram,          4*0x4000 ) != STATE_OK )
								return FALSE;
  if( stateload_block( SID_PCG,  font_pcg,           8*256*2  ) != STATE_OK )
								return FALSE;

  /* オプショナルなメモリ */

  if( memory_allocate_additional()== FALSE ){
    return FALSE;
  }
  if( sound_board==SOUND_II ){
    if( stateload_block( SID_ADPCM, sound2_adpcm,    0x40000  ) != STATE_OK )
								return FALSE;
  }
  if( use_extram ){
    if( stateload_block( SID_ERAM, ext_ram, 0x8000*4*use_extram ) != STATE_OK )
								return FALSE;
  }
  return TRUE;
}
