/************************************************************************/
/* モニターモードの loadbas、savebas 処理				*/
/*									*/
/*				この機能は peach氏により実装されました	*/
/************************************************************************/
#ifdef	USE_MONITOR


/*
 *  仮想的に CPU と RAM を用意してぶん回します。
 *
 *  具体的に
 *    ・割り込み処理は無視
 *    ・RAM はメイン RAM のみ
 *    ・ポート処理は必要なところだけ処理
 *  などで高速化しています。
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "z80.h"
#include "memory.h"
#include "screen.h"
#include "monitor.h"
#include "basic.h"

#define BASIC_MAX_ERR_NUM	4	/* エラー登録可能数		*/
#define BASIC_MAX_ERR_STR	20	/* エラー表示文字数		*/

#define BASIC_MAX_LINE		256	/* 一行の文字最大数		*/
#define BASIC_MAX_LOOP		200000	/* 仮想プロセスの最大ループ数	*/


/* メモリ読み込み用マクロ */
#define READ_BYTE(mem, addr)						\
	((mem)[addr] & 0xff)

#define READ_WORD(mem, addr)						\
	((READ_BYTE(mem, addr + 1) << 8) + READ_BYTE(mem, addr))

/* メモリ書き込み用マクロ */
#define WRITE_BYTE(mem, addr, data)					\
	(mem)[addr] = ((data) & 0xff)

#define WRITE_WORD(mem, addr, data)					\
	do {								\
	    WRITE_BYTE(mem, (addr) + 1, (data) >> 8);			\
	    WRITE_BYTE(mem, addr, data);				\
	} while (0)


/* レジスタ書き込み用マクロ */
#define SET_REG(reg, data)						\
	((reg).W   = data,						\
	 (reg).B.h = ((data) >> 8) & 0xff,				\
	 (reg).B.l = (data) & 0xff)

/* エラー設定用マクロ */
#define SET_ERR(err, epc, estr)						\
	do {								\
	    (err).pc = epc;						\
	    strncpy((err).str, estr, BASIC_MAX_ERR_STR);		\
	    (err).str[BASIC_MAX_ERR_STR - 1] = '\0';			\
	} while (0)

/* 仮想プロセス時エラー構造体 */
typedef struct {
    word pc;
    char str[BASIC_MAX_ERR_STR];
} basic_err;

int basic_mode = FALSE;

static z80arch pseudo_z80_cpu;			/* 仮想 CPU               */

static byte *pseudo_ram;			/* 仮想メモリ (MAIN RAM)  */
static word pseudo_window_offset;		/* 仮想メモリ用ウインドウ */

static byte *read_pseudo_mem_0000_7fff;		/* 仮想メモリリードポインタ  */
static byte *read_pseudo_mem_8000_83ff;

static byte *write_pseudo_mem_8000_83ff;	/* 仮想メモリライトポインタ  */

static word basic_top_addr_addr;		/* 中間コード始点アドレス格納*/
static word basic_end_addr_addr;		/* 中間コード終点アドレス格納*/
static word basic_top_addr;			/* 中間コード始点アドレス    */
static word basic_end_addr;			/* 中間コード終点アドレス    */
static word basic_buffer_addr;			/* 中間コードバッファアドレス*/
static word basic_buffer_size;			/* 中間コードバッファサイズ  */
static word basic_sp;				/* 開始時のスタックポインタ  */

static word basic_conv_buffer_addr;		/* 変換用バッファアドレス */

static word encode_start_pc;			/* エンコード開始アドレス */
static word encode_end_pc;			/* エンコード終了アドレス */
static basic_err encode_err[BASIC_MAX_ERR_NUM];	/* エンコードエラー       */
static int encode_err_num;			/* エンコードエラー登録数 */

static word decode_start_pc;			/* デコード開始アドレス   */
static word decode_end_pc;			/* デコード終了アドレス   */
static basic_err decode_err[BASIC_MAX_ERR_NUM];	/* デコードエラー         */
static int decode_err_num;			/* デコードエラー登録数   */


/*------------------------------------------------------*/
/* 仮想メモリ割り当て					*/
/*------------------------------------------------------*/
static void pseudo_memory_mapping(void)
{
    if (grph_ctrl & GRPH_CTRL_N){
	read_pseudo_mem_0000_7fff  = &main_rom_n[0x0000];
	read_pseudo_mem_8000_83ff  = &pseudo_ram[0x8000];
	write_pseudo_mem_8000_83ff = &pseudo_ram[0x8000];
    } else {
	read_pseudo_mem_0000_7fff  = &main_rom[0x0000];
	read_pseudo_mem_8000_83ff  = &pseudo_ram[pseudo_window_offset];
	write_pseudo_mem_8000_83ff = &pseudo_ram[pseudo_window_offset];
    }
}

/*------------------------------------------------------*/
/* 仮想メモリ・ライト					*/
/*------------------------------------------------------*/
static void pseudo_mem_write(word addr, byte data)
{
    if     (addr < 0x8000) pseudo_ram[addr] = data;
    else if(addr < 0x8400) write_pseudo_mem_8000_83ff[addr & 0x03ff] = data;
    else                   pseudo_ram[addr] = data;
}

/*------------------------------------------------------*/
/* 仮想メモリ・リード					*/
/*------------------------------------------------------*/
static byte pseudo_mem_read(word addr)
{
    if      (addr < 0x8000) return read_pseudo_mem_0000_7fff[addr];
    else if (addr < 0x8400) return read_pseudo_mem_8000_83ff[addr & 0x03ff];
    else                    return pseudo_ram[addr];
}


/*------------------------------------------------------*/
/* 仮想ポート・ライト					*/
/*------------------------------------------------------*/
static void pseudo_io_out(byte port, byte data)
{
    if (port == 0x70) {
        pseudo_window_offset = (word)data << 8;
        pseudo_memory_mapping();
    }
}

/*------------------------------------------------------*/
/* 仮想ポート・リード					*/
/*------------------------------------------------------*/
static byte pseudo_io_in(byte port)
{
    static byte port40_toggle = 0;

    switch (port) {
	/* N-mode decode */
    case 0x09:			/* fake */
	return 0xff;
    case 0x40:			/* fake */
	port40_toggle ^= 0x20;
	return port40_toggle;

	/* V1,V2-mode encode/decode */
    case 0x70:
	return pseudo_window_offset >> 8;
    default:
	return 0;
    }
}


/*------------------------------------------------------*/
/*							*/
/*------------------------------------------------------*/
void	pseudo_intr_update(void){ pseudo_z80_cpu.skip_intr_chk=TRUE; }
int	pseudo_intr_ack(void){ return -1; }

/*------------------------------------------------------*/
/* 仮想 CPU 初期化					*/
/*------------------------------------------------------*/
static void pseudo_z80_init(void)
{
    z80_reset(&pseudo_z80_cpu);

    pseudo_z80_cpu.fetch     = pseudo_mem_read;
    pseudo_z80_cpu.mem_read  = pseudo_mem_read;
    pseudo_z80_cpu.mem_write = pseudo_mem_write;
    pseudo_z80_cpu.io_read   = pseudo_io_in;
    pseudo_z80_cpu.io_write  = pseudo_io_out;

    pseudo_z80_cpu.intr_update = pseudo_intr_update;
    pseudo_z80_cpu.intr_ack    = pseudo_intr_ack;
}

/*------------------------------------------------------*/
/* 仮想メモリ初期化					*/
/*------------------------------------------------------*/
static int pseudo_mem_init(void)
{
    if( verbose_proc ) printf("Allocating 64kB for pseudo ram...");
    pseudo_ram = (byte *)malloc( sizeof(byte)*0x10000 );
    if( pseudo_ram == NULL ){
	if( verbose_proc ){ printf("FAILED\n"); }
	return(0);
    }else{
	if( verbose_proc ){ printf("OK\n");     }
    }
    memset(pseudo_ram, 0x00, 0x10000);
   
    pseudo_memory_mapping();


    if (grph_ctrl & GRPH_CTRL_N) {
	memset(&pseudo_ram[0xf160], 0xc9, 0xc0);
    } else {
	WRITE_BYTE(pseudo_ram, 0xe69d, 0x10);
	memset(&pseudo_ram[0xed00], 0xc9, 0x100);
    }
    return(1);
}

/*------------------------------------------------------*/
/* 中間コードの始点・終点アドレスの書き込み		*/
/*------------------------------------------------------*/
static void write_basic_addr(void)
{
    WRITE_WORD(main_ram, basic_top_addr_addr, basic_top_addr);
    WRITE_WORD(main_ram, basic_end_addr_addr, basic_end_addr);
    if (grph_ctrl & GRPH_CTRL_N) {
	WRITE_WORD(main_ram, basic_end_addr_addr + 2, basic_end_addr);
	WRITE_WORD(main_ram, basic_end_addr_addr + 4, basic_end_addr);
    }
}

/*------------------------------------------------------*/
/* エンコード・デコード用アドレス設定			*/
/*------------------------------------------------------*/
static void pseudo_set_addr(void)
{
    if (grph_ctrl & GRPH_CTRL_N) {
	/* N-mode */

	/* アドレス設定 */
	basic_top_addr_addr    = 0xeb54;
	basic_end_addr_addr    = 0xefa0;
	basic_buffer_addr      = 0x8021;
	basic_buffer_size      = 0x4fff;
	/*basic_buffer_size      = 0x4014;*/
	basic_sp               = 0xe8d1;
	basic_conv_buffer_addr = 0xec96;

	/* エンコード開始・終了アドレス設定 */
	encode_start_pc        = 0x3cf2;
	encode_end_pc          = 0x3d6a;
	
	/* エンコードエラー登録 */
	SET_ERR(encode_err[0], 0x3c9f, "no string");
	SET_ERR(encode_err[1], 0x423b, "no line number");
	SET_ERR(encode_err[2], 0x459e, "only line number");
	encode_err_num = 3;

	/* デコード開始・終了アドレス設定 */
	decode_start_pc        = 0x5718;
	decode_end_pc          = 0x574e;

	/* デコードエラー登録 */
	SET_ERR(decode_err[0], 0x3c82, "no line number");
	SET_ERR(decode_err[1], 0x3c81, "buffer over flow");
	decode_err_num = 1;
    } else {
	/* V1,V2-mode */

	/* アドレス設定 */
	basic_top_addr_addr    = 0xe658;
	basic_end_addr_addr    = 0xeb18;
	basic_buffer_addr      = 0x0001;
	basic_buffer_size      = 0x7fff;
	basic_sp               = 0xe5ff;
	basic_conv_buffer_addr = 0xe9b9;

	/* エンコード開始・終了アドレス設定 */
	encode_start_pc        = 0x04e2;
	encode_end_pc          = 0x05a8;

	/* エンコードエラー登録 */
	SET_ERR(encode_err[0], 0x04a7, "no string");
	SET_ERR(encode_err[1], 0x4c70, "no line number");
	SET_ERR(encode_err[2], 0x0393, "line number is 0");
	SET_ERR(encode_err[3], 0x0c3c, "only line number");
	encode_err_num = 4;
	
	/* デコード開始・終了アドレス設定 */
	decode_start_pc        = 0x18f1;
	decode_end_pc          = 0x1928;

	/* デコードエラー登録 */
	SET_ERR(decode_err[0], 0x047b, "no string");
	SET_ERR(decode_err[1], 0x047a, "buffer over flow");
	decode_err_num = 2;
    }
}


/*------------------------------------------------------*/
/* エンコード用仮想 CPU レジスタ設定			*/
/*------------------------------------------------------*/
static void encode_z80_set_register(void)
{
    SET_REG(pseudo_z80_cpu.PC, encode_start_pc); /* start addr */
    SET_REG(pseudo_z80_cpu.HL, basic_conv_buffer_addr - 1);
    SET_REG(pseudo_z80_cpu.SP, basic_sp);
}


/*------------------------------------------------------*/
/* エンコード用仮想メモリ設定				*/
/*------------------------------------------------------*/
static void encode_set_mem(void)
{
    /* 開始アドレス格納 */
    WRITE_WORD(pseudo_ram, basic_top_addr_addr, basic_buffer_addr);
    /* 終了アドレス格納 */
    WRITE_WORD(pseudo_ram, basic_end_addr_addr, basic_buffer_addr + 2);
}

/*------------------------------------------------------*/
/* テキストリストから中間コードに変換			*/
/*------------------------------------------------------*/
int basic_encode_list(FILE *fp)
{
    char *ptr;
    char buf[BASIC_MAX_LINE];
    long loop;
    int i;
    int size;
    int text_line_num;

    pseudo_window_offset = 0;
    pseudo_set_addr();
    pseudo_z80_init();
    if (!pseudo_mem_init()) return(0);

    basic_mode = TRUE;
    size = 0;

    encode_set_mem();
    text_line_num = 0;
    while (fgets(buf, BASIC_MAX_LINE, fp) != NULL) {
	text_line_num++;
	/* 改行コード削除 */
	if ((ptr = strchr(buf, '\r')) != NULL) *ptr = '\0';
	if ((ptr = strchr(buf, '\n')) != NULL) *ptr = '\0';

	/* 先頭が数字でないならとばす */
	ptr = buf;
	while (*ptr == ' ') ptr++;
	if (!isdigit(*ptr)) continue;
	encode_z80_set_register();

	/* 変換用バッファにリストをコピー */
	strncpy(&pseudo_ram[basic_conv_buffer_addr], ptr, strlen(ptr));
	pseudo_ram[basic_conv_buffer_addr + strlen(ptr)] = 0x00; /* 終端 */

	/* 仮想プロセス開始 */
	for (loop = 0; loop < BASIC_MAX_LOOP; loop++) {
            z80_emu(&pseudo_z80_cpu, 1);
	    if (pseudo_z80_cpu.PC.W == encode_end_pc) break;
	    for (i = 0; i < encode_err_num; i++) {
		if (pseudo_z80_cpu.PC.W == encode_err[i].pc) {
		    printf("Error in line %d : %s.\n",
			   text_line_num, encode_err[i].str);
		    size = 0;
		    goto end_basic_encode_list;
		}
	    }
        }
        if (loop >= BASIC_MAX_LOOP) {
            printf("Error in line %d : failed to encode.\n", text_line_num);
            break;
        }
    }

    /* エンコード結果をメイン RAM にコピー */
    basic_top_addr = READ_WORD(pseudo_ram, basic_top_addr_addr);
    basic_end_addr = READ_WORD(pseudo_ram, basic_end_addr_addr);
    if (basic_end_addr < basic_top_addr) {
	printf("Error : failed to encode.\n");
	return(0);
    }
    size = basic_end_addr - basic_top_addr + 1;
    memcpy(&main_ram[basic_top_addr], &pseudo_ram[basic_top_addr], size);
    write_basic_addr();

 end_basic_encode_list:
    basic_mode = FALSE;
    free(pseudo_ram);

    return(size);
}



/*------------------------------------------------------*/
/* デコード用仮想メモリ設定 1				*/
/*------------------------------------------------------*/
static int decode_mem_set1(void)
{
    /* 中間コードをコピー */
    basic_top_addr = READ_WORD(main_ram, basic_top_addr_addr);
    basic_end_addr = READ_WORD(main_ram, basic_end_addr_addr);
    WRITE_WORD(pseudo_ram, basic_top_addr_addr, basic_top_addr);
    WRITE_WORD(pseudo_ram, basic_end_addr_addr, basic_end_addr);
    if (basic_end_addr < basic_top_addr) {
	printf("Error : no basic code.\n");
	return(FALSE);
    }
    memcpy(&pseudo_ram[basic_top_addr], &main_ram[basic_top_addr],
	   basic_end_addr - basic_top_addr + 1);

    if (grph_ctrl & GRPH_CTRL_N) {
	WRITE_BYTE(pseudo_ram, 0xea59, 0xff);
	WRITE_BYTE(pseudo_ram, 0xea65, 0x28); /* 横の最大文字数 */
	WRITE_BYTE(pseudo_ram, 0xeb4a, 0x28); /* 横の最大文字数 */
	WRITE_WORD(pseudo_ram, 0xef56, 0xef58);
	WRITE_WORD(pseudo_ram, 0xef79, 0xe9ff);
    } else {
	WRITE_BYTE(pseudo_ram, 0xe6a0, 0xff); /* エラー関係？ */
	WRITE_BYTE(pseudo_ram, 0xe6a2, 0xff);
	WRITE_WORD(pseudo_ram, 0xe6c4, 0xf3c8);
	WRITE_WORD(pseudo_ram, 0xeace, 0xead0);
	WRITE_WORD(pseudo_ram, 0xeaf1, 0xe3fd);
	WRITE_BYTE(pseudo_ram, 0xef89, 0x50); /* 横の最大文字数 */
    }
    return(TRUE);
}


/*------------------------------------------------------*/
/* デコード用仮想メモリ設定 2				*/
/*------------------------------------------------------*/
static void decode_mem_set2(void)
{
/*    pseudo_window_offset = 0;*/
/*    pseudo_memory_mapping();*/

    WRITE_WORD(pseudo_ram, basic_sp, 0xfffa); /* POP DE */

    if (grph_ctrl & GRPH_CTRL_N) {
	WRITE_WORD(pseudo_ram, 0xea63, 0x0101); /* カーソル位置 */
    } else {
	WRITE_WORD(pseudo_ram, 0xef86, 0x0101); /* カーソル位置 */
    }

}

/*------------------------------------------------------*/
/* デコード用仮想 CPU レジスタ設定			*/
/*------------------------------------------------------*/
static void decode_z80_set_register(word top_addr)
{
    SET_REG(pseudo_z80_cpu.PC, decode_start_pc); /* start addr */
    SET_REG(pseudo_z80_cpu.HL, top_addr);
    SET_REG(pseudo_z80_cpu.SP, basic_sp);
}

/*------------------------------------------------------*/
/* 中間コードからテキストリストへ変換			*/
/*------------------------------------------------------*/
/* list コマンドを流用 */
int basic_decode_list(FILE *fp)
{
    char buf[BASIC_MAX_LINE];
    int i;
    int size = 0;
    int text_line_num;
    long loop;
    word line_top_addr, line_end_addr;
    word line_num;


    pseudo_window_offset = 0;
    pseudo_set_addr();
    pseudo_z80_init();
    if (!pseudo_mem_init()) return(0);
 
    if (!decode_mem_set1()) goto end_basic_decode_list;

    basic_mode = TRUE;
    size = 0;

    line_top_addr = basic_top_addr;
    text_line_num = 0;
    while (line_top_addr < basic_end_addr) {
	text_line_num++;
	line_end_addr = READ_WORD(pseudo_ram, line_top_addr);
	if (line_end_addr == 0x0000) break;
	line_num = READ_WORD(pseudo_ram, line_top_addr + 2);

	decode_mem_set2();
	decode_z80_set_register(line_top_addr);

	/* 仮想プロセス開始 */
	for (loop = 0; loop < BASIC_MAX_LOOP; loop++) {
            z80_emu(&pseudo_z80_cpu, 1);
	    if (pseudo_z80_cpu.PC.W == decode_end_pc) break;
	    for (i = 0; i < decode_err_num; i++) {
		if (pseudo_z80_cpu.PC.W == decode_err[i].pc) {
		    printf("Error in line %d : %s.\n",
			   text_line_num, decode_err[i].str);
		    size = 0;
		    goto end_basic_decode_list;
		}
	    }
        }
        if (loop >= BASIC_MAX_LOOP) {
            printf("Error in line %d : failed to decode.\n", text_line_num);
            break;
        }

	/* 結果表示 */
	size += fprintf(fp, "%d ", line_num);
	for (i = 0; i < BASIC_MAX_LINE - 1; i++) {
	    if (READ_BYTE(pseudo_ram, basic_conv_buffer_addr + i) == 0x00) break;
	    buf[i] = READ_BYTE(pseudo_ram, basic_conv_buffer_addr + i);
	    size++;
	}
	buf[i] = '\0';
	print_hankaku(fp, buf, alt_char);
	fprintf(fp, "\r\n");
	size += 2;
	line_top_addr = line_end_addr;

    }

 end_basic_decode_list:

    basic_mode = FALSE;
    free(pseudo_ram);

    return(size);
}


/*------------------------------------------------------*/
/* ファイルから中間コードをメイン RAM に読み込む	*/
/*------------------------------------------------------*/
int basic_load_intermediate_code(FILE *fp)
{
    int size;

    pseudo_set_addr();

    size = fread(&main_ram[basic_buffer_addr - 1], 1, basic_buffer_size + 1, fp);
    basic_top_addr = basic_buffer_addr;
    basic_end_addr = basic_buffer_addr + size - 2;
    write_basic_addr();
    
    return(size);
}


/*------------------------------------------------------*/
/* メイン RAM からファイルに中間コードを書き込む	*/
/*------------------------------------------------------*/
int basic_save_intermediate_code(FILE *fp)
{
    int size, wsize;

    pseudo_set_addr();

    basic_top_addr = READ_WORD(main_ram, basic_top_addr_addr);
    basic_end_addr = READ_WORD(main_ram, basic_end_addr_addr);
    if (basic_end_addr < basic_top_addr) {
	printf("Error : no basic code.\n");
	return(0);
    }
    /* p88make, p80make でそのまま使えます */
    size = basic_end_addr - basic_top_addr + 2;
    wsize = fwrite(&main_ram[basic_top_addr - 1], 1, size, fp);
    if (wsize < size) {
	printf("Error : basic size is %d.\n", size);
    }

    return(wsize);
}

#endif	/* USE_MONITOR */
