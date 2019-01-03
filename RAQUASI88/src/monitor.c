/************************************************************************/
/*									*/
/* モニターモード							*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include "quasi88.h"
#include "initval.h"
#include "monitor.h"

#include "pc88cpu.h"
#include "pc88main.h"
#include "pc88sub.h"
#include "crtcdmac.h"
#include "memory.h"
#include "graph.h"
#include "intr.h"
#include "keyboard.h"
#include "pio.h"
#include "soundbd.h"
#include "screen.h"
#include "fdc.h"

#include "emu.h"
#include "drive.h"
#include "image.h"
#include "file-op.h"
#include "status.h"
#include "menu.h"
#include "pause.h"
#include "snddrv.h"
#include "wait.h"
#include "suspend.h"
#include "snapshot.h"
#include "event.h"

#include "basic.h"


#ifdef USE_GNU_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#ifdef USE_LOCALE
#include <locale.h>
#include <langinfo.h>
#endif /* USE_LOCALE */



/************************************************************************/
/* SIGINT発生時 (Ctrl-C) 、モニターモードへ遷移するように設定		*/
/*	起動時に -debug オプション指定時のみ。未指定時は終了する。	*/
/************************************************************************/

int	debug_mode	= FALSE;		/* デバッグ機能(モニター)  */

char	alt_char	= 'X';			/* 代替文字 */



/* モニターモード以外の時に SIGINT(Ctrl-C)を受け取ったらモニターモードに移行 */

static	void	sigint_handler(int dummy)
{
    quasi88_monitor();
    signal(SIGINT, sigint_handler);
}

/* SIGTERM を受けとったら、終了する */

static	void	sigterm_handler(int dummy)
{
    quasi88_quit();
    z80main_cpu.icount = 0;
    z80sub_cpu.icount = 0;
}

/*-------- 割り込み設定 -------- */

void	set_signal(void)
{
    if (debug_mode) {
	signal(SIGINT,  sigint_handler);
	signal(SIGTERM, sigterm_handler);
    } else {
	signal(SIGINT,  sigterm_handler);
	signal(SIGTERM, sigterm_handler);
    }
}


/************************************************************************/
/*									*/
/************************************************************************/

#ifdef	USE_MONITOR



enum MonitorJob
{
  MONITOR_LINE_INPUT,

  MONITOR_HELP,
  MONITOR_MENU,
  MONITOR_QUIT,

  MONITOR_GO,
  MONITOR_TRACE,
  MONITOR_STEP,
  MONITOR_STEPALL,
  MONITOR_BREAK,

  MONITOR_READ,
  MONITOR_WRITE,
  MONITOR_DUMP,
  MONITOR_DUMPEXT,
  MONITOR_FILL,
  MONITOR_MOVE,
  MONITOR_SEARCH,
  MONITOR_IN,
  MONITOR_OUT,
  MONITOR_LOADMEM,
  MONITOR_SAVEMEM,

  MONITOR_RESET,
  MONITOR_REG,
  MONITOR_DISASM,

  MONITOR_SET,
  MONITOR_SHOW,
  MONITOR_REDRAW,
  MONITOR_RESIZE,
  MONITOR_DRIVE,
  MONITOR_FILE,

  MONITOR_STATESAVE,
  MONITOR_STATELOAD,
  MONITOR_SNAPSHOT,
  MONITOR_LOADFONT,
  MONITOR_SAVEFONT,

  MONITOR_MISC,

  MONITOR_FBREAK,
  MONITOR_TEXTSCR,
  MONITOR_LOADBAS,
  MONITOR_SAVEBAS,

  MONITOR_TAPELOAD,
  MONITOR_TAPESAVE,
  MONITOR_PRINTER,
  MONITOR_SERIALIN,
  MONITOR_SERIALOUT,

  EndofMONITOR
};



/****************************************************************/
/* ヘルプメッセージ表示関数					*/
/****************************************************************/
static	void	help_help(void)
{
  printf
  (
   "  help [<cmd>]\n"
   "    print help\n"
   "    <cmd> ... command for help\n"
   "              [omit]... print short help for all commands.\n"
   );
}
static	void	help_menu(void)
{
  printf
  (
   "  menu\n"
   "    enter menu-mode.\n"
   );
}
static	void	help_quit(void)
{
  printf
  (
   "  quit\n"
   "    quit QUASI88.\n"
   );
}
static	void	help_go(void)
{
  printf
  (
   "  go\n"
   "    execute MAIN and|or SUB program\n"
   );
}
static	void	help_trace(void)
{
  printf
  (
   "  trace [#<steps>|<steps>|change]\n"
   "    execute MAIN ane|or SUB program specityes times\n"
   "    [all omit]        ... trace some steps (previous steps)\n"
   "    #<steps>, <steps> ... step counts of trace  ( you can omit '#' )\n"
   "	change            ... trace while change CPU job. ( main<->sub )\n"
   "                          this is work under condition -cpu 0 or -cpu 1\n"
   );
}
static	void	help_step(void)
{
  printf
  (
   "  step [call][jp][rep]\n"
   "    execute MAIN and|or SUB program 1 time\n"
   "    [all omit] ... execute 1 step\n"
   "    call       ... not trace CALL instruction\n"
   "    jp         ... not trace DJNZ instruction\n"
   "    rep        ... not trace LD*R/CP*R/IN*R/OT*R instruction\n"
   "    CAUTION)\n"
   "         call/jp/rep are work under condition -cpu 0 or -cpu 1\n"
   "         call/jp/rep are use break-point #10.\n"
   );
}
static	void	help_stepall(void)
{
  printf
  (
   "  S\n"
   "    mean 'step all'   (see. step)\n"
   );
}
static	void	help_break(void)
{
  printf
  (
   "  break [<cpu>] [<action>] <addr|port> [#<No>]\n"
   "  break [<cpu>] CLEAR [#<No>]\n"
   "  break\n"
   "    set break point\n"
   "    [all omit]  ... show all break points\n"
   "    <cpu>       ... CPU select MAIN|SUB\n"
   "                    [omit]... select MAIN\n"
   "    <action>    ... set action of conditon PC|READ|WRITE|IN|OUT or CLEAR\n"
   "                    PC    ... break if PC reach addr\n"
   "                    READ  ... break if data is read\n"
   "                    WRITE ... break if data is written\n"
   "                    IN    ... break if data is input\n"
   "                    OUT   ... break if data is output\n"
   "                    CLEAR ... clear all break point\n"
   "                    [omit]... select PC\n"
   "	<addr|port> ... specify address or port\n"
   "                    if <action> is CLEAR, this argument is invalid\n"
   "    #<No>       ... number of break point. (#1..#10)\n"
   "                    #0    ... all break point when <action> is CLEAR\n"
   "                    [omit]... select #1\n"
   "                    CAUTION).. #10 is used by system\n"
   );
}  



static	void	help_read(void)
{
  printf
  (
   "  read [<bank>] <addr>\n"
   "    read memory.\n"
   "    <bank> ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "               [omit]... current memory bank of MAIN.\n"
   "    <addr> ... specify address\n");
}
static	void	help_write(void)
{
  printf
  (
   "  write [<bank>] <addr> <data>\n"
   "    write memory.\n"
   "    <bank> ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "               [omit]... current memory bank of MAIN.\n"
   "    <addr> ... specify address\n"
   "    <data> ... write data\n"
   );
}
static	void	help_dump(void)
{
  printf
  (
   "  dump [<bank>] <start-addr> [<end-addr>]\n"
   "  dump [<bank>] <start-addr> [#<size>]\n"
   "    dump memory.\n"
   "    <bank>       ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "                     [omit]... current memory bank of MAIN.\n"
   "    <start-addr> ... dump start address\n"
   "    <end-addr>   ... dump end address\n"
   "                     [omit]... <start-address>+256\n"
   "    #<size>      ... dump size\n"
   "                     [omit]... 256 byte\n"
   );
}
static	void	help_dumpext(void)
{
  printf
  (
   "  dumpext [<bank>] [#<board>] <start-addr> [<end-addr>]\n"
   "  dumpext [<bank>] [#<board>] <start-addr> [#<size>]\n"
   "    dump external ram memory.\n"
   "    <bank>       ... memory bank EXT0|EXT1|EXT2|EXT3\n"
   "                     [omit]... current memory bank of EXT0.\n"
   "    #<board>     ... board number (1..16).\n"
   "                     [omit]... board #1.\n"
   "    <start-addr> ... dump start address(0x0000..0x7fff)\n"
   "    <end-addr>   ... dump end address(0x0000..0x7fff)\n"
   "                     [omit]... <start-address>+256\n"
   "    #<size>      ... dump size\n"
   "                     [omit]... 256 byte\n"
   );
}
static	void	help_fill(void)
{
  printf
  (
   "  fill [<bank>] <start-addr> <end-addr> <value>\n"
   "  fill [<bank>] <start-addr> #<size>    <value>\n"
   "    fill memory by specify value. \n"
   "    <bank>       ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "                     [omit]... current memory bank of MAIN.\n"
   "    <start-addr> ... fill start address\n"
   "    <end-addr>   ... fill end address\n"
   "    #<size>      ... fill size\n"
   "    <value>      ... fill value\n"
   );
}
static	void	help_move(void)
{
  printf
  (
   "  move [<src-bank>] <src-addr> <end-addr> [<dist-bank>] <dist-addr>\n"
   "  move [<src-bank>] <src-addr> #<size>    [<dist-bank>] <dist-addr>\n"
   "    move memory. \n"
   "    <src-bank>  ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "                    [omit]... current memory bank of MAIN.\n"
   "    <src-addr>  ... move source start address\n"
   "    <end-addr>  ... move source end   address\n"
   "    #<size>     ... move size\n"
   "    <dist-bank> ... memory bank\n"
   "                    [omit]... same as <src-bank>\n"
   "    <dist-addr> ... move distination address\n"
   );
}
static	void	help_search(void)
{
  printf
  (
   "  search [<value> [[<bank>] <start-addr> <end-addr>]]\n"
   "    search memory. \n"
   "    <value>      ... search value\n"
   "    <bank>       ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "                     [omit]... current memory bank of MAIN.\n"
   "    <start-addr> ... search start address\n"
   "    <end-addr>   ... search end address\n"
   "    [omit-all]   ... search previous value\n"
   );
}
static	void	help_in(void)
{
  printf
  (
   "  in [<cpu>] <port>\n"
   "    input I/O port.\n"
   "    <cpu>  ... CPU select MAIN|SUB\n"
   "               [omit]... select MAIN\n"
   "    <port> ... in port address\n"
   );
}
static	void	help_out(void)
{
  printf
  (
   "  out [<cpu>] <port> <data>\n"
   "    output I/O port.\n"
   "    <cpu>  ... CPU select MAIN|SUB\n"
   "               [omit]... select MAIN\n"
   "    <port> ... out port address\n"
   "    <data> ... output data\n"
   );
}
static	void	help_loadmem(void)
{
  printf
  (
   "  loadmem <filename> <bank> <start-addr> [<end-addr>]\n"
   "  loadmem <filename> <bank> <start-addr> [#<size>]\n"
   "    load memory from binary file.\n"
   "    <filename>   ... binary filename.\n"
   "    <bank>       ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "    <start-addr> ... load start addr\n"
   "    <end-addr>   ... load end addr\n"
   "    #<size>      ... load size\n"
   "                     [omit] set filesize as binary size\n"
   );
}
static	void	help_savemem(void)
{
  printf
  (
   "  savemem <filename> <bank> <start-addr> <end-addr>\n"
   "  savemem <filename> <bank> <start-addr> #<size>\n"
   "    save memory image to file.\n"
   "    <filename>   ... filename.\n"
   "    <bank>       ... memory bank ROM|RAM|N|EXT0|EXT1|EXT2|EXT3|B|R|G|HIGH|SUB\n"
   "    <start-addr> ... save start addr\n"
   "    <end-addr>   ... save end addr\n"
   "    #<size>      ... save size\n"
   );
}
static	void	help_reset(void)
{
  printf
  (
   "  reset [<basic-mode>] [<clock-mode>] [<sound-board>] [<dipsw>]\n"
   "    reset PC8800 (not execute)\n"
   "	<basic-mode>  ... BASIC mode N|V1S|V1H|V2\n"
   "                      [omit] select current BASIC mode\n"
   "	<clock-mode>  ... CPU Clock 4MHZ|8MHZ\n"
   "                      [omit] select current CLOCK mode\n"
   "	<sound-board> ... sound-board type SD|SD2\n"
   "                      [omit] select current sound-board\n"
   "    <dipsw>       ... dip-switch setting \n"
   "                      [omit] select current dip-switch setting\n"
   );
}
static	void	help_reg(void)
{
  printf
  (
   "  reg [[<cpu>] [<name> <value>]]\n"
   "    show & set register.\n"
   "    [all omit] ... show all register (MAIN and|or SUB).\n"
   "    <cpu>      ... CPU select MAIN|SUB\n"
   "                   [omit]... select MAIN\n"
   "    <name>     ... specity register name.\n"
   "                   AF|BC|DE|HL|AF'|BC'|DE'|HL'|IX|IY|SP|PC|I|R|IFF|IM\n"
   "    <value>    ... set value\n"
   "                   [omit]... show value of register\n"
   );
}
static	void	help_disasm(void)
{
  printf
  (
   "  disasm [[<cpu>] [<start-addr>][#<steps>]]\n"
   "    disassemble.\n"
   "    [all omit]   ... disasmble 16 steps from MAIN CPU PC address.\n"
   "    <cpu>        ... CPU select MAIN|SUB\n"
   "                     [omit]... select MAIN\n"
   "    <start-addr> ... disassemble start address\n"
   "                     [omit]... reg PC address\n"
   "    #<steps>     ... disassemble steps\n"
   "                     [omit]... 16 steps\n"
   );
}
static	void	help_set(void)
{
  printf
  (
   "  set [[<variabe-name> [<value>]]]\n"
   "    show & set variables.\n"
   "    [all omit]     ... show all variable.\n"
   "    <variabe-name> ... specify variable name.\n"
   "    <value>        ... set value\n"
   "                       [omit]... show value of variable\n"
   );
}
static	void	help_show(void)
{
  printf
  (
   "  show [<variabe-name>]\n"
   "    show variables.\n"
   "    [all omit]     ... show all variable.\n"
   "    <variabe-name> ... specify variable name.\n"
   );
}
static	void	help_redraw(void)
{
  printf
  (
   "  redraw\n"
   "    redraw QUASI88 screen.\n"
   );
}
static	void	help_resize(void)
{
  printf
  (
   "  resize [<screen-size>]\n"
   "    resize screen.\n"
#ifdef	SUPPORT_DOUBLE
   "    <screen_size> ... screen size FULL|HALF|DOUBLE|FULLSCREEN|WINDOW\n"
   "                      [omit]... change screen size HALF,FULL,DOUBLE...\n"
#else
   "    <screen_size> ... screen size FULL|HALF|FULLSCREEN|WINDOW\n"
   "                      [omit]... change screen size HALF<->FULL\n"
#endif
   );
}
static	void	help_drive(void)
{
  printf
  (
   "  drive\n"
   "  drive show\n"
   "  drive empty [<drive_no>]\n"
   "  drive eject [<drive_no>]\n"
   "  drive set <drive_no> <filename> [<image_no>]\n"
   "    Show drive information, Eject Disk, Set Disk.\n"
   "      drive [show] ... Show now drive information.\n"
   "      drive empty  ... Set/Unset drive <drive_no> empty.\n"
   "      drive eject  ... Eject disk from drive <drive_no>\n"
   "                       <drive_no> omit, eject all disk.\n"
   "      drive set    ... Eject disk and insert new disk\n"
   "        <drive_no> ... 1 | 2  mean  DRIVE 1: | DRIVE 2:\n"
   "        <filename> ... if filename is '-' , disk not change\n"
   "        <image_no> ... image number (1..%d max)\n"
   "                       <image_no> omit, set image number 1.\n"
   , MAX_NR_IMAGE
   );
}
static	void	help_file(void)
{
  printf
  (
   "  file show <filename>\n"
   "  file create <filename>\n"
   "  file protect <filename> <image_no>\n"
   "  file unprotect <filename> <image_no>\n"
   "  file format <filename> <image_no>\n"
   "  file unformat <filename> <image_no>\n"
   "  file rename <filename> <image_no> <image_name>\n"
   "    Disk image file utility.\n"
   "      file show      ... Show file information.\n"
   "      file create    ... Create / Append blank disk image in file.\n"
   "      file protect   ... Set protect.\n"
   "      file unprotect ... Unset protect.\n"
   "      file format    ... format image by N88DISK-BASIC DATA DISK format.\n"
   "      file unformat  ... Unformat image.\n"
   "      file rename    ... Rename disk image.\n"
   "        <filename>   ... filename\n"
   "        <image_no>   ... image number (1..%d max)\n"
   "        <image_name> ... image name (MAX 16chars)\n"
   , MAX_NR_IMAGE
   );
}

static	void	help_statesave(void)
{
  printf
  (
   "  statesave [<filename>]\n"
   "    statesave QUASI88\n"
   "    <filename> ... specify state-file filename.\n"
   "                   omit, default filename\n"
   );
}

static	void	help_stateload(void)
{
  printf
  (
   "  stateload [<filename>]\n"
   "    stateload QUASI88\n"
   "    <filename> ... specify state-file filename.\n"
   "                   omit, default filename\n"
   );
}

static	void	help_snapshot(void)
{
  printf
  (
   "  snapshot [<format>]\n"
   "    save screen snapshot\n"
   "    <format> ... select format \"BMP\", \"PPM\", \"RAW\".\n"
   "                 omit, current format\n"
   );
}

static	void	help_loadfont(void)
{
  printf
  (
   "  loadfont <filename> <format> <type>\n"
   "    load text-font file\n"
   "    <filename> ... specify text-font-file filename.\n"
   "    <format>   ... select format\n"
   "                   0 ... ROM-image\n"
   "                   1 ... bitmap format (horizontal)\n"
   "                   2 ... bitmap format (vertical)\n"
   "    <type>     ... select font kind\n"
   "                   0 ... PCG font\n"
   "                   1 ... Standard font\n"
   "                   2 ... 2nd font (usually hiragana-font)\n"
   "                   3 ... 3rd font (usually transparent-font)\n"
   );
}

static	void	help_savefont(void)
{
  printf
  (
   "  savefont <filename> <format> <type>\n"
   "    save text-font file\n"
   "    <filename> ... specify text-font-file filename.\n"
   "    <format>   ... select format\n"
   "                   0 ... ROM-image\n"
   "                   1 ... bitmap format (horizontal)\n"
   "                   2 ... bitmap format (vertical)\n"
   "    <type>     ... select font kind\n"
   "                   0 ... PCG font\n"
   "                   1 ... Standard font\n"
   "                   2 ... 2nd font (usually hiragana-font)\n"
   "                   3 ... 3rd font (usually transparent-font)\n"
   );
}

static	void help_fbreak(void)
{
  printf
  (
   "  fbreak [<action>] <drive> <track> [<sector>] [#<No>]\n"
   "  fbreak CLEAR [#<No>]\n"
   "  fbreak\n"
   "    set fdc break point\n"
   "    [all omit] ... show all break points\n"
   "    <action>   ... set action of conditon READ|WRITE|DIAG or CLEAR\n"
   "                   READ  ... break if fdc command is read\n"
   "                   WRITE ... break if fdc command is write\n"
   "                   DIAG  ... break if fdc command is diag\n"
   "                   CLEAR ... clear all break point\n"
   "                   [omit]... select READ\n"
   "    <drive>    ... specify drive (1 or 2)\n"
   "                   if <action> is CLEAR, this argument is invalid\n"
   "    <track>    ... specify track (0...)\n"
   "    <sector>   ... specify sector (1...)\n"
   "                   [omit]... select all sector\n"
   "    #<No>      ... number of break point. (#1..#10)\n"
   "                   #0    ... all break point when <action> is CLEAR\n"
   "                   [omit]... select #1\n"
   );
}  

static	void help_textscr(void)
{
  printf
  (
   "  textscr [<char>]\n"
   "    print text screen in the console screen\n"
   "    <char> ... alternative character to unprintable one.\n"
   "               [omit] ... use 'X'\n"
   );
}  

static	void help_loadbas(void)
{
  printf
  (
   "  loadbas <filename> [<type>]\n"
   "    load basic list\n"
   "    <filename> ... filename of basic list.\n"
   "    <type>     ... set type of basic list ASCII or BINARY\n"
   "                   ASCII  ... load as text list\n"
   "                   BINARY ... load as intermediate code\n"
   "                   [omit] ... select ASCII\n"
   );
}  

static	void help_savebas(void)
{
  printf
  (
   "  savebas [<filename> [<type>]]\n"
   "    print or save basic list\n"
   "    <filename> ... filename of basic list.\n"
   "    <type>     ... set type of basic list ASCII or BINARY\n"
   "                   ASCII  ... save as text list\n"
   "                   BINARY ... save as intermediate code\n"
   "                   [omit] ... select ASCII\n"
   );
}  

static	void	help_tapeload(void)
{
  printf
  (
   "  tapeload <filename>\n"
   "    set tape-image-file as load\n"
   "    <filename> ... specify tape-image-file filename.\n"
   "                   filename \"-\" mean \'unset image\'\n"
   );
}

static	void	help_tapesave(void)
{
  printf
  (
   "  tapesave <filename>\n"
   "    set tape-image-file as save\n"
   "    <filename> ... specify tape-image-file filename.\n"
   "                   filename \"-\" mean \'unset image\'\n"
   );
}

static	void	help_printer(void)
{
  printf
  (
   "  printer <filename>\n"
   "    set printout-image-file\n"
   "    <filename> ... specify printout-image-file filename.\n"
   "                   filename \"-\" mean \'unset image\'\n"
   );
}

static	void	help_serialin(void)
{
  printf
  (
   "  serialin <filename>\n"
   "    set serial-in-image-file\n"
   "    <filename> ... specify serial-in-image-file filename.\n"
   "                   filename \"-\" mean \'unset image\'\n"
   );
}

static	void	help_serialout(void)
{
  printf
  (
   "  serialout <filename>\n"
   "    set serial-out-image-file\n"
   "    <filename> ... specify serial-out-image-file filename.\n"
   "                   filename \"-\" mean \'unset image\'\n"
   );
}

static	void	help_misc(void)
{
  printf
  (
   "  misc ... this is for debug. don't mind!\n"
   );
}


/****************************************************************/
/* 命令の種類判定テーブル					*/
/****************************************************************/
static struct {
    int		job;
    char	*cmd;
    void	(*help)(void);
    char	*help_mes;
} monitor_cmd[]=
{
{ MONITOR_HELP,     "help",    	help_help,     "print help",               },
{ MONITOR_HELP,     "?",       	help_help,     "     ''   ",               },
{ MONITOR_MENU,     "menu",    	help_menu,     "enter menu-mode",          },
{ MONITOR_MENU,     "m",       	help_menu,     "     ''        ",          },
{ MONITOR_QUIT,     "quit",    	help_quit,     "quit quasi88",             },
{ MONITOR_QUIT,     "q",       	help_quit,     "     ''     ",             },
{ MONITOR_GO,       "go",      	help_go,       "exec emu",                 },
{ MONITOR_GO,       "g",       	help_go,       "    ''  ",                 },
{ MONITOR_TRACE,    "trace",   	help_trace,    "trace emu",                },
{ MONITOR_TRACE,    "t",       	help_trace,    "    ''   ",                },
{ MONITOR_STEP,     "step",    	help_step,     "step emu",                 },
{ MONITOR_STEP,     "s",       	help_step,     "    ''   ",                },
{ MONITOR_STEPALL,  "S",       	help_stepall,  "    ''   ",                },
{ MONITOR_BREAK,    "break",   	help_break,    "set break point",          },
{ MONITOR_READ,     "read",    	help_read,     "read memory",              },
{ MONITOR_WRITE,    "write",   	help_write,    "write memory ",            },
{ MONITOR_DUMP,     "dump",    	help_dump,     "dump memory",              },
{ MONITOR_DUMPEXT,  "dumpext", 	help_dumpext,  "dump external ram memory", },
{ MONITOR_FILL,     "fill",    	help_fill,     "fill memory",              },
{ MONITOR_MOVE,     "move",    	help_move,     "move memory",              },
{ MONITOR_SEARCH,   "search",  	help_search,   "search memory",            },
{ MONITOR_IN,       "in",      	help_in,       "input port",               },
{ MONITOR_OUT,      "out",     	help_out,      "output port",              },
{ MONITOR_LOADMEM,  "loadmem", 	help_loadmem,  "load memory from file",    },
{ MONITOR_SAVEMEM,  "savemem", 	help_savemem,  "save memory to file",      },
{ MONITOR_RESET,    "reset",   	help_reset,    "reset PC8800 system",      },
{ MONITOR_REG,      "reg",     	help_reg,      "show/set CPU register",    },
{ MONITOR_DISASM,   "disasm",  	help_disasm,   "disassemble",              },
{ MONITOR_SET,      "set",     	help_set,      "show variable",            },
{ MONITOR_SHOW,     "show",    	help_show,     "show/set variable",        },
{ MONITOR_REDRAW,   "redraw",  	help_redraw,   "redraw screen",            },
{ MONITOR_RESIZE,   "resize",  	help_resize,   "resize screen",            },
{ MONITOR_DRIVE,    "drive",   	help_drive,    "operate disk drive",       },
{ MONITOR_FILE,     "file",    	help_file,     "disk image file utility",  },
{ MONITOR_STATESAVE,"statesave",help_statesave,"state-save QUASI88",       },
{ MONITOR_STATELOAD,"stateload",help_stateload,"state-load QUASI88",       },
{ MONITOR_SNAPSHOT, "snapshot",	help_snapshot, "save screen snapshot",     },
{ MONITOR_LOADFONT, "loadfont",	help_loadfont, "load text-font file",      },
{ MONITOR_SAVEFONT, "savefont",	help_savefont, "save text-font file",      },
{ MONITOR_MISC,     "misc",    	help_misc,     "for debug",                },
{ MONITOR_FBREAK,   "fbreak",  	help_fbreak,   "set fdc break point",      },
{ MONITOR_TEXTSCR,  "textscr", 	help_textscr,  "print text screen",        },
{ MONITOR_LOADBAS,  "loadbas", 	help_loadbas,  "load basic list",          },
{ MONITOR_SAVEBAS,  "savebas", 	help_savebas,  "save basic list",          },
{ MONITOR_TAPELOAD, "tapeload", help_tapeload, "set tape-image as load",   },
{ MONITOR_TAPESAVE, "tapesave", help_tapesave, "set tape-image as save",   },
{ MONITOR_PRINTER,  "printer",  help_printer,  "set printout-image-file",  },
{ MONITOR_SERIALIN, "serialin", help_serialin, "set serial-in-image-file", },
{ MONITOR_SERIALOUT,"serialout",help_serialout,"set serial-out-image-file",},
};



/****************************************************************/
/* 引数の種類判定テーブル					*/
/****************************************************************/

enum ArgvType {
  ARGV_END     = 0x0000000,
  ARGV_STR     = 0x0000001,		/* strings			*/
  ARGV_PORT    = 0x0000002,		/* 0〜0xff			*/
  ARGV_ADDR    = 0x0000004,		/* 0〜0xffff			*/
  ARGV_NUM     = 0x0000008,		/* 0〜0x7fffffff		*/
  ARGV_INT     = 0x0000010,		/* -0x7fffffff〜0x7fffffff	*/
  ARGV_DRV     = 0x0000020,		/* 1〜2				*/
  ARGV_IMG     = 0x0000040,		/* 1〜MAX_NR_IMAGE		*/
  ARGV_SIZE    = 0x0000080,		/* #1〜#0x7fffffff		*/
  ARGV_CPU     = 0x0000100,		/* CpuName			*/
  ARGV_BANK    = 0x0000200,		/* MemoryName			*/
  ARGV_REG     = 0x0000400,		/* RegisterName			*/
  ARGV_BREAK   = 0x0000800,		/* BreakAction			*/
  ARGV_BASMODE = 0x0001000,		/* ResetCommand			*/
  ARGV_CKMODE  = 0x0002000,		/* ResetCommand			*/
  ARGV_SDMODE  = 0x0004000,		/* ResetCommand			*/
  ARGV_CHANGE  = 0x0008000,		/* TraceCommand			*/
  ARGV_STEP    = 0x0010000,		/* StepCommand			*/
  ARGV_ALL     = 0x0020000,		/* RegCommand			*/
  ARGV_RESIZE  = 0x0040000,		/* ResizeCommand		*/
  ARGV_FILE    = 0x0080000,		/* FileCommand			*/
  ARGV_DRIVE   = 0x0100000,		/* DriveCommand			*/
  ARGV_FBREAK  = 0x0200000,		/* FBreakAction			*/
  ARGV_BASIC   = 0x0400000,		/* BasicCodeType		*/
  ARGV_SNAPSHOT= 0x0800000,		/* SnapshotFormatType		*/

  EndofArgvType
};

enum ArgvName {

  ARG_MAIN,							/* <cpu> */
  ARG_SUB,

  /*ARG_MAIN,*/							/* <bank> */
  ARG_ROM,	ARG_RAM,	ARG_N,		ARG_HIGH,
  ARG_EXT0,	ARG_EXT1,	ARG_EXT2,	ARG_EXT3,
  ARG_B,	ARG_R,		ARG_G,		ARG_PCG,
  /*ARG_SUB*/

  ARG_AF,	ARG_BC,		ARG_DE,		ARG_HL,		/* <reg> */
  ARG_IX,	ARG_IY,		ARG_SP,		ARG_PC,
  ARG_AF1,	ARG_BC1,	ARG_DE1,	ARG_HL1,
  ARG_I,	/*ARG_R,*/
  ARG_IFF,	ARG_IM,		ARG_HALT,

  /*ARG_PC,*/	ARG_READ,	ARG_WRITE,	ARG_IN,		/* <action>*/
  ARG_OUT,	ARG_DIAG,	ARG_CLEAR,

  ARG_V2,	ARG_V1H,	ARG_V1S,	/*ARG_N,*/	/* <mode> */
  ARG_8MHZ,	ARG_4MHZ,	ARG_SD,		ARG_SD2,

  ARG_CHANGE,						/* trace change */

  /*ARG_ALL*/
  ARG_CALL,	ARG_JP,		ARG_REP,		/* step <cmd>   */

  ARG_ALL,						/* reg all      */

  ARG_FULL,	ARG_HALF,	ARG_DOUBLE,		/* resize <arg> */
  ARG_FULLSCREEN,ARG_WINDOW,

  ARG_SHOW,	ARG_EJECT,				/* drive <cmd>  */
  ARG_EMPTY,	ARG_SET,

  /*ARG_SHOW,*/						/* file <cmd>   */
  ARG_CREATE,	ARG_RENAME,
  ARG_PROTECT,	ARG_UNPROTECT,
  ARG_FORMAT,	ARG_UNFORMAT,

  ARG_BINARY,	ARG_ASCII,				/* savebas <type> */
  
  ARG_BMP,	ARG_PPM,	ARG_RAW,		/* snapshot <fmt> */

  EndofArgName
};


static	struct {
    char	*str;
    int		type;
    int		val;
} monitor_argv[]=
{
  { "MAIN",		ARGV_CPU,	ARG_MAIN,	}, /* <cpu> */
  { "SUB",		ARGV_CPU,	ARG_SUB,	},

  { "MAIN",		ARGV_BANK,	ARG_MAIN,	}, /* <bank> */
  { "ROM",		ARGV_BANK,	ARG_ROM,	},
  { "RAM",		ARGV_BANK,	ARG_RAM,	},
  { "N",		ARGV_BANK,	ARG_N,		},
  { "HIGH",		ARGV_BANK,	ARG_HIGH,	},
  { "EXT0",		ARGV_BANK,	ARG_EXT0,	},
  { "EXT1",		ARGV_BANK,	ARG_EXT1,	},
  { "EXT2",		ARGV_BANK,	ARG_EXT2,	},
  { "EXT3",		ARGV_BANK,	ARG_EXT3,	},
  { "B",		ARGV_BANK,	ARG_B,		},
  { "R",		ARGV_BANK,	ARG_R,		},
  { "G",		ARGV_BANK,	ARG_G,		},
  { "SUB",		ARGV_BANK,	ARG_SUB,	},
  { "PCG",		ARGV_BANK,	ARG_PCG,	},

  { "AF",		ARGV_REG,	ARG_AF,		}, /* <reg> */
  { "BC",		ARGV_REG,	ARG_BC,		},
  { "DE",		ARGV_REG,	ARG_DE,		},
  { "HL",		ARGV_REG,	ARG_HL,		},
  { "IX",		ARGV_REG,	ARG_IX,		},
  { "IY",		ARGV_REG,	ARG_IY,		},
  { "SP",		ARGV_REG,	ARG_SP,		},
  { "PC",		ARGV_REG,	ARG_PC,		},
  { "AF'",		ARGV_REG,	ARG_AF1,	},
  { "BC'",		ARGV_REG,	ARG_BC1,	},
  { "DE'",		ARGV_REG,	ARG_DE1,	},
  { "HL'",		ARGV_REG,	ARG_HL1,	},
  { "I",		ARGV_REG,	ARG_I,		},
  { "R",		ARGV_REG,	ARG_R,		},
  { "IFF",		ARGV_REG,	ARG_IFF,	},
  { "IM",		ARGV_REG,	ARG_IM,		},
  { "HALT",		ARGV_REG,	ARG_HALT,	},

  { "PC",		ARGV_BREAK,	ARG_PC,		}, /*<action>*/
  { "READ",		ARGV_BREAK,	ARG_READ,	},
  { "WRITE",		ARGV_BREAK,	ARG_WRITE,	},
  { "IN",		ARGV_BREAK,	ARG_IN,		},
  { "OUT",		ARGV_BREAK,	ARG_OUT,	},
  { "CLEAR",		ARGV_BREAK,	ARG_CLEAR,	},

  { "READ",		ARGV_FBREAK,	ARG_READ,	}, /*<action>*/
  { "WRITE",		ARGV_FBREAK,	ARG_WRITE,	},
  { "DIAG",		ARGV_FBREAK,	ARG_DIAG,	},
  { "CLEAR",		ARGV_FBREAK,	ARG_CLEAR,	},

  { "V2",		ARGV_BASMODE,	ARG_V2,		}, /* <basic-mode> */
  { "V1H",		ARGV_BASMODE,	ARG_V1H,	},
  { "V1S",		ARGV_BASMODE,	ARG_V1S,	},
  { "N",		ARGV_BASMODE,	ARG_N,		},
  { "8MHZ",		ARGV_CKMODE,	ARG_8MHZ,	}, /* <clock-mode> */
  { "4MHZ",		ARGV_CKMODE,	ARG_4MHZ,	},
  { "SD",		ARGV_SDMODE,	ARG_SD,		}, /* <sound-board> */
  { "SD2",		ARGV_SDMODE,	ARG_SD2,	},

  { "CHANGE",		ARGV_CHANGE,	ARG_CHANGE,	}, /* trace */

  { "CALL",		ARGV_STEP,	ARG_CALL,	}, /* step  */
  { "JP",		ARGV_STEP,	ARG_JP,		},
  { "REP",		ARGV_STEP,	ARG_REP,	},
  { "ALL",		ARGV_STEP,	ARG_ALL,	},

  { "ALL",		ARGV_ALL,	ARG_ALL,	}, /* reg   */

  { "FULL",		ARGV_RESIZE,	ARG_FULL,	}, /* resize*/
  { "HALF",		ARGV_RESIZE,	ARG_HALF,	},
#ifdef	SUPPORT_DOUBLE
  { "DOUBLE",		ARGV_RESIZE,	ARG_DOUBLE,	},
#endif
  { "FULLSCREEN",	ARGV_RESIZE,	ARG_FULLSCREEN,	},
  { "WINDOW",		ARGV_RESIZE,	ARG_WINDOW,	},

  { "SHOW",		ARGV_DRIVE,	ARG_SHOW,	}, /* drive */
  { "EJECT",		ARGV_DRIVE,	ARG_EJECT,	},
  { "EMPTY",		ARGV_DRIVE,	ARG_EMPTY,	},
  { "SET",		ARGV_DRIVE,	ARG_SET,	},

  { "SHOW",		ARGV_FILE,	ARG_SHOW,	}, /* file  */
  { "CREATE",		ARGV_FILE,	ARG_CREATE,	},
  { "RENAME",		ARGV_FILE,	ARG_RENAME,	},
  { "PROTECT",		ARGV_FILE,	ARG_PROTECT,	},
  { "UNPROTECT",	ARGV_FILE,	ARG_UNPROTECT,	},
  { "FORMAT",		ARGV_FILE,	ARG_FORMAT,	},
  { "UNFORMAT",		ARGV_FILE,	ARG_UNFORMAT,	},

  { "BINARY",		ARGV_BASIC,	ARG_BINARY,	}, /*savebas*/
  { "ASCII",		ARGV_BASIC,	ARG_ASCII,	},

  { "BMP",		ARGV_SNAPSHOT,	ARG_BMP,	}, /*snapshot*/
  { "PPM",		ARGV_SNAPSHOT,	ARG_PPM,	},
  { "RAW",		ARGV_SNAPSHOT,	ARG_RAW,	},
};



enum SetType
{
  MTYPE_NEWLINE,
  MTYPE_INT,		MTYPE_BYTE,	MTYPE_WORD,	MTYPE_DOUBLE,
  MTYPE_INT_C,		MTYPE_BYTE_C,	MTYPE_WORD_C,	MTYPE_DOUBLE_C,
  MTYPE_KEY,		MTYPE_PALETTE,	MTYPE_CRTC,	MTYPE_PIO,
  MTYPE_MEM,		MTYPE_FONT,	MTYPE_FRAMESKIP,MTYPE_INTERLACE,
  MTYPE_INTERP,		MTYPE_CLOCK,	MTYPE_BEEP,
  MTYPE_VOLUME,		MTYPE_FMMIXER,	MTYPE_PSGMIXER,	MTYPE_BEEPMIXER,
  MTYPE_RHYTHMMIXER,	MTYPE_ADPCMMIXER,	MTYPE_FMGENMIXER,
  MTYPE_SAMPLEMIXER,	MTYPE_MIXER,
  EndofTYPE
};
static struct {
    char	*var_name;
    char	*port_mes;
    int		var_type;
    void	*var_ptr;
} monitor_variable[]=
{
{ "boot_dipsw",		"(boot:3031)",	MTYPE_INT_C,	&boot_dipsw,	    },
{ "boot_from_rom",	"(boot:40>>3)",	MTYPE_INT_C,	&boot_from_rom,	    },
{ "boot_clock_4mhz",	"(boot:6E>>7)",	MTYPE_INT_C,	&boot_clock_4mhz,   },
{ "boot_basic",		"(boot:3031)",	MTYPE_INT_C,	&boot_basic,	    },
{ "sound_board",	"(-sd/-sd2)",	MTYPE_INT_C,	&sound_board,	    },
{ "use_extram",		"(-extram)",	MTYPE_INT_C,	&use_extram,	    },
{ "use_jisho_rom",	"(-jisho)",	MTYPE_INT_C,	&use_jisho_rom,	    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },

{ "sys_ctrl",		"(OUT:30)",	MTYPE_BYTE_C,	&sys_ctrl,	    },
{ "grph_ctrl",		"(OUT:31)",	MTYPE_BYTE_C,	&grph_ctrl,	    },
{ "misc_ctrl",		"(I/O:32)",	MTYPE_BYTE_C,	&misc_ctrl,	    },
{ "ALU1_ctrl",		"(OUT:34)",	MTYPE_BYTE,	&ALU1_ctrl,	    },
{ "ALU2_ctrl",		"(OUT:35)",	MTYPE_BYTE_C,	&ALU2_ctrl,	    },
{ "ctrl_signal",	"(OUT:40)",	MTYPE_BYTE_C,	&ctrl_signal,	    },
{ "grph_pile",		"(OUT:53)",	MTYPE_BYTE_C,	&grph_pile,	    },
{ "intr_level",		"(OUT:E4&07)",	MTYPE_INT_C,	&intr_level,	    },
{ "intr_priority",	"(OUT:E4&08)",	MTYPE_INT_C,	&intr_priority,	    },
{ "intr_sio_enable",	"(OUT:E6&04)",	MTYPE_INT_C,	&intr_sio_enable,   },
{ "intr_vsync_enable",	"(OUT:E6&02)",	MTYPE_INT_C,	&intr_vsync_enable, },
{ "intr_rtc_enable",	"(OUT:E6&01)",	MTYPE_INT_C,	&intr_rtc_enable,   },
{ "intr_sound_enable",	"(IO:~32AA&80)",MTYPE_INT_C,	&intr_sound_enable, },
{ "sound_ENABLE_A",	"(sd[27])",	MTYPE_INT,	&sound_ENABLE_A,    },
{ "sound_ENABLE_B",	"(sd[27])",	MTYPE_INT,	&sound_ENABLE_B,    },
{ "sound_LOAD_A",	"(sd[27])",	MTYPE_INT,	&sound_LOAD_A,	    },
{ "sound_LOAD_B",	"(sd[27])",	MTYPE_INT,	&sound_LOAD_B,	    },
{ "sound_FLAG_A",	"(IN:44)",	MTYPE_INT,	&sound_FLAG_A,	    },
{ "sound_FLAG_B",	"(IN:44)",	MTYPE_INT,	&sound_FLAG_B,	    },
{ "sound_TIMER_A",	"(sd[24]-[25])",MTYPE_INT,	&sound_TIMER_A,	    },
{ "sound_TIMER_B",	"(sd[26])",	MTYPE_INT,	&sound_TIMER_B,	    },
{ "sound_prescaler",	"(sd[2D]-[2F])",MTYPE_INT,	&sound_prescaler,   },
{ "sound_reg[27]",	"(sd[27])",	MTYPE_BYTE,	&sound_reg[0x27],   },
{ "sound2_EN_EOS",	"(sd[29])",	MTYPE_INT,	&sound2_EN_EOS,     },
{ "sound2_EN_BRDY",	"(sd[29])",	MTYPE_INT,	&sound2_EN_BRDY,    },
{ "sound2_EN_ZERO",	"(sd[29])",	MTYPE_INT,	&sound2_EN_ZERO,    },
{ "use_cmdsing",	"",		MTYPE_BEEP,	&use_cmdsing,	    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },
{ "RS232C_flag",	"",		MTYPE_INT,	&RS232C_flag,	    },
{ "VSYNC_flag",		"",		MTYPE_INT,	&VSYNC_flag,	    },
{ "ctrl_vrtc",		"",		MTYPE_INT,	&ctrl_vrtc,	    },
{ "RTC_flag",		"",		MTYPE_INT,	&RTC_flag,	    },
{ "SOUND_flag",		"",		MTYPE_INT,	&SOUND_flag,	    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },

{ "mem",		"",		MTYPE_MEM,	NULL,		    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },
{ "key",		"(IN:00..0F)",	MTYPE_KEY,	NULL,		    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },
{ "palette",		"(OUT:5254..5B)",MTYPE_PALETTE,	NULL,		    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },
{ "crtc",		"",		MTYPE_CRTC,	NULL,		    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },
{ "pio",		"(IO:FC..FF)",	MTYPE_PIO,	NULL,		    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },

#ifdef	USE_SOUND
{ "volume",		"(-vol)",	MTYPE_VOLUME,		NULL,	    },
{ "fm-mixer",		"(-fmvol)",	MTYPE_FMMIXER,		NULL,	    },
{ "psg-mixer",		"(-psgvol)",	MTYPE_PSGMIXER,		NULL,	    },
{ "beep-mixer",		"(-beepvol)",	MTYPE_BEEPMIXER,	NULL,	    },
{ "rhythm-mixer",	"(-rhythmvol)",	MTYPE_RHYTHMMIXER,	NULL,	    },
{ "adpcm-mixer",	"(-adpcmvol)",	MTYPE_ADPCMMIXER,	NULL,	    },
#ifdef	USE_FMGEN
{ "fmgen-mixer",	"(-fmgenvol)",	MTYPE_FMGENMIXER,	NULL,	    },
#endif
{ "sample-mixer",	"(-samplevol)",	MTYPE_SAMPLEMIXER,	NULL,	    },
{ "",			"",		MTYPE_MIXER,		NULL,	    },
{ "",			"",		MTYPE_NEWLINE,		NULL,	    },
#endif

{ "cpu_timing",		"(-cpu)",	MTYPE_INT,	&cpu_timing,	    },
{ "select_main_cpu",	"",		MTYPE_INT,	&select_main_cpu,   },
{ "dual_cpu_count",	"",		MTYPE_INT,	&dual_cpu_count,    },
{ "CPU_1_COUNT",	"",		MTYPE_INT,	&CPU_1_COUNT,	    },
{ "cpu_slice_us",       "(-cpu2us)",	MTYPE_INT,	&cpu_slice_us,	    },
{ "calendar_stop",	"(-timestop)",	MTYPE_INT,	&calendar_stop,	    },
{ "cmt_speed",		"(-cmt_speed)",	MTYPE_INT,	&cmt_speed,	    },
{ "cmt_intr",		"(-cmt_intr)",	MTYPE_INT,	&cmt_intr,	    },
{ "cmt_wait",		"(-cmt_wait)",	MTYPE_INT,	&cmt_wait,	    },
{ "highspeed_mode",	"(-hsbasic)",	MTYPE_INT,	&highspeed_mode,    },
{ "monitor_15k",	"(-15k)",	MTYPE_INT,	&monitor_15k,	    },
{ "use_pcg",		"(-pcg)",	MTYPE_FONT,	&use_pcg,	    },
{ "font_type",		"",		MTYPE_FONT,	&font_type,	    },
{ "memory_wait",	"(-mem_wait)",	MTYPE_INT,	&memory_wait,	    },
{ "sub_load_rate",	"(-subload)",	MTYPE_INT,	&sub_load_rate,	    },
{ "disk_exchange",	"(-exchange)",	MTYPE_INT,	&disk_exchange,	    },
{ "fdc_debug_mode",	"(-fdcdebug)",	MTYPE_INT,	&fdc_debug_mode,    },
{ "fdc_ignore_readonly","(-ignore_ro)",	MTYPE_INT,	&fdc_ignore_readonly},
{ "fdc_wait",		"(-fdc_wait)",	MTYPE_INT,	&fdc_wait,	    },
{ "frameskip_rate",	"(-frameskip)",	MTYPE_FRAMESKIP,&frameskip_rate,    },
{ "monitor_analog",	"(-analog)",	MTYPE_INT,	&monitor_analog,    },
{ "use_auto_skip",	"(-autoskip)",	MTYPE_INT,	&use_auto_skip,	    },
{ "use_interlace",	"(-interlace)",	MTYPE_INTERLACE,&use_interlace,	    },
{ "use_half_interp",	"(-interp)",	MTYPE_INTERP,	&use_half_interp,   },
{ "mon_aspect",		"(-aspect)",	MTYPE_DOUBLE,	&mon_aspect,	    },
{ "hide_mouse",		"(-hide_mouse)",MTYPE_INT,	&hide_mouse,	    },
{ "grab_mouse",		"(-grab_mouse)",MTYPE_INT,	&grab_mouse,	    },
{ "mouse_mode",		"(-mouse)",	MTYPE_INT,	&mouse_mode,	    },
{ "mouse_swap_button",	"(-mouseswap)",	MTYPE_INT,	&mouse_swap_button, },
{ "mouse_sensitivity",	"(-mousespeed)",MTYPE_INT,	&mouse_sensitivity, },
{ "joy_key_mode",	"(-joykey)",	MTYPE_INT,	&joy_key_mode,	    },
{ "joy_swap_button",	"(-joyswap)",	MTYPE_INT,	&joy_swap_button,   },
{ "joy2_key_mode",	"",		MTYPE_INT,	&joy2_key_mode,	    },
{ "joy2_swap_button",	"",		MTYPE_INT,	&joy2_swap_button,  },
{ "tenkey_emu",		"(-tenkey)",	MTYPE_INT,	&tenkey_emu,	    },
{ "numlock_emu",	"(-numlock)",	MTYPE_INT,	&numlock_emu,	    },
{ "function_f[1]",	"",		MTYPE_INT,	&function_f[1],     },
{ "function_f[2]",	"",		MTYPE_INT,	&function_f[2],     },
{ "function_f[3]",	"",		MTYPE_INT,	&function_f[3],     },
{ "function_f[4]",	"",		MTYPE_INT,	&function_f[4],     },
{ "function_f[5]",	"",		MTYPE_INT,	&function_f[5],     },
{ "function_f[6]",	"(-f6)",	MTYPE_INT,	&function_f[6],     },
{ "function_f[7]",	"(-f7)",	MTYPE_INT,	&function_f[7],     },
{ "function_f[8]",	"(-f8)",	MTYPE_INT,	&function_f[8],     },
{ "function_f[9]",	"(-f9)",	MTYPE_INT,	&function_f[9],     },
{ "function_f[10]",	"(-f10)",	MTYPE_INT,	&function_f[10],    },
{ "function_f[11]",	"",		MTYPE_INT,	&function_f[11],    },
{ "function_f[12]",	"",		MTYPE_INT,	&function_f[12],    },
{ "function_f[13]",	"",		MTYPE_INT,	&function_f[13],    },
{ "function_f[14]",	"",		MTYPE_INT,	&function_f[14],    },
{ "function_f[15]",	"",		MTYPE_INT,	&function_f[15],    },
{ "function_f[16]",	"",		MTYPE_INT,	&function_f[16],    },
{ "function_f[17]",	"",		MTYPE_INT,	&function_f[17],    },
{ "function_f[18]",	"",		MTYPE_INT,	&function_f[18],    },
{ "function_f[19]",	"",		MTYPE_INT,	&function_f[19],    },
{ "function_f[20]",	"",		MTYPE_INT,	&function_f[20],    },
{ "romaji_type",	"(-romaji)",	MTYPE_INT,	&romaji_type,	    },
{ "need_focus",		"(-focus)",	MTYPE_INT,	&need_focus,	    },
{ "cpu_clock_mhz",	"(-clock)",	MTYPE_CLOCK,	&cpu_clock_mhz,	    },
{ "sound_clock_mhz",	"(-soundclock)",MTYPE_CLOCK,	&sound_clock_mhz,   },
{ "vsync_freq_hz",	"(-vsync)",	MTYPE_CLOCK,	&vsync_freq_hz,     },
{ "wait_rate",		"(-speed)",	MTYPE_INT,	&wait_rate,	    },
{ "wait_by_sleep",	"(-sleep)",	MTYPE_INT,	&wait_by_sleep,	    },
{ "no_wait",		"(-nowait)",	MTYPE_INT,	&no_wait,	    },
/*{ "boost",		"(-boost)",	MTYPE_BOOST,	&boost,		    },*/
/*{ "wait_sleep_min_us",	"(-sleepparm)",	MTYPE_INT,	&wait_sleep_min_us, },*/
{ "status_imagename",	"(-statusimage)",MTYPE_INT,	&status_imagename,  },
{ "menu_lang",		"(-english)",	MTYPE_INT,	&menu_lang,	    },
{ "menu_readonly",	"(-ro)",	MTYPE_INT,	&menu_readonly,	    },
{ "menu_swapdrv",	"(-swapdrv)",	MTYPE_INT,	&menu_swapdrv,	    },
{ "file_coding",	"(-euc/-sjis)",	MTYPE_INT,	&file_coding,	    },
{ "filename_synchronize","",		MTYPE_INT,	&filename_synchronize},
{ "verbose_proc",	"(-verbose&01)",MTYPE_INT,	&verbose_proc,	    },
{ "verbose_z80",	"(-verbose&02)",MTYPE_INT,	&verbose_z80,       },
{ "verbose_io",		"(-verbose&04)",MTYPE_INT,	&verbose_io,	    },
{ "verbose_pio",	"(-verbose&08)",MTYPE_INT,	&verbose_pio,	    },
{ "verbose_fdc",	"(-verbose&10)",MTYPE_INT,	&verbose_fdc,	    },
{ "verbose_wait",	"(-verbose&20)",MTYPE_INT,	&verbose_wait,	    },
{ "verbose_suspend",	"(-verbose&40)",MTYPE_INT,	&verbose_suspend,   },
{ "verbose_snd",	"(-verbose&80)",MTYPE_INT,	&verbose_snd,	    },
{ "",			"",		MTYPE_NEWLINE,	NULL,		    },

#ifdef	PROFILER
{ "debug_profiler",	"for debug",    MTYPE_INT,	&debug_profiler,    },
#endif
#ifdef	DEBUGLOG
{ "pio_debug",		"for debug",    MTYPE_INT,	&pio_debug,	    },
{ "fdc_debug",		"for debug",    MTYPE_INT,	&fdc_debug,	    },
{ "main_debug",		"for debug",    MTYPE_INT,	&main_debug,	    },
{ "sub_debug",		"for debug",    MTYPE_INT,	&sub_debug,	    },
#endif

};

static struct {
    char	*block_name;
    int		start;
    int		end;
} monitor_variable_block[] =
{
  { "boot",	0,	0 },
  { "main",	1,	2 },
  { "intr",	2,	2 },
#ifdef	USE_SOUND
  { "vol",	8,	8 },
  { "emu",	9,	10 },
#else
  { "emu",	8,	9 },
#endif
};



/*--------------------------------------------------------------*/
/* メモリ READ/WRITE 関数					*/
/*--------------------------------------------------------------*/
static	byte	peek_memory( int bank, word addr )
{
  int	verbose_save;
  byte	wk;

  switch( bank ){
  case ARG_MAIN:
    return main_mem_read(addr);
  case ARG_ROM:
    if( addr<0x8000 ) return main_rom[addr];
    else              return main_mem_read(addr);
  case ARG_RAM:
    if( 0xf000<=addr && high_mode ) return main_high_ram[addr-0xf000];
    else                            return main_ram[addr];
  case ARG_N:
    if( addr<0x8000 ) return main_rom_n[addr];
    else              return main_mem_read(addr);
  case ARG_EXT0:
    if( 0x6000<=addr && addr<0x8000 ) return main_rom_ext[0][addr-0x6000];
    else                              return main_mem_read(addr);
  case ARG_EXT1:
    if( 0x6000<=addr && addr<0x8000 ) return main_rom_ext[1][addr-0x6000];
    else                              return main_mem_read(addr);
  case ARG_EXT2:
    if( 0x6000<=addr && addr<0x8000 ) return main_rom_ext[2][addr-0x6000];
    else                              return main_mem_read(addr);
  case ARG_EXT3:
    if( 0x6000<=addr && addr<0x8000 ) return main_rom_ext[3][addr-0x6000];
    else                              return main_mem_read(addr);
  case ARG_B:
    if( 0xc000<=addr ) return main_vram[addr-0xc000][0];
    else               return main_mem_read(addr);
  case ARG_R:
    if( 0xc000<=addr ) return main_vram[addr-0xc000][1];
    else               return main_mem_read(addr);
  case ARG_G:
    if( 0xc000<=addr ) return main_vram[addr-0xc000][2];
    else               return main_mem_read(addr);
  case ARG_HIGH:
    if( 0xf000<=addr ){
      if( high_mode )  return main_ram[addr];
      else             return main_high_ram[addr-0xf000];
    }else              return main_mem_read(addr);
  case ARG_SUB:
    verbose_save = verbose_io;
    verbose_io = 0;
    wk = sub_mem_read(addr);
    verbose_io = verbose_save;
    return wk;
  case ARG_PCG:
    if( addr<8*256*2 ) return font_pcg[addr];
    else               return 0xff;
  }
  return 0xff;
}
static	void	poke_memory( int bank, word addr, byte data )
{
  int	verbose_save;

  switch( bank ){
  case ARG_MAIN:
    main_mem_write(addr,data);
    return;
  case ARG_ROM:
    if( addr<0x8000 ) main_rom[addr] = data;
    else              main_mem_write(addr,data);
    return;
  case ARG_RAM:
    if( 0xf000<=addr && high_mode ) main_high_ram[addr-0xf000] = data;
    else                            main_ram[addr] = data;
    return;
  case ARG_N:
    if( addr<0x8000 ) main_rom_n[addr] = data;
    else              main_mem_write(addr,data);
    return;
  case ARG_EXT0:
    if( 0x6000<=addr && addr<0x8000 ) main_rom_ext[0][addr-0x6000] = data;
    else                              main_mem_write(addr,data);
    return;
  case ARG_EXT1:
    if( 0x6000<=addr && addr<0x8000 ) main_rom_ext[1][addr-0x6000] = data;
    else                              main_mem_write(addr,data);
    return;
  case ARG_EXT2:
    if( 0x6000<=addr && addr<0x8000 ) main_rom_ext[2][addr-0x6000] = data;
    else                              main_mem_write(addr,data);
    return;
  case ARG_EXT3:
    if( 0x6000<=addr && addr<0x8000 ) main_rom_ext[3][addr-0x6000] = data;
    else                              main_mem_write(addr,data);
    return;
  case ARG_B:
    if( 0xc000<=addr ) main_vram[addr-0xc000][0] = data;
    else               main_mem_write(addr,data);
    return;
  case ARG_R:
    if( 0xc000<=addr ) main_vram[addr-0xc000][1] = data;
    else               main_mem_write(addr,data);
    return;
  case ARG_G:
    if( 0xc000<=addr ) main_vram[addr-0xc000][2] = data;
    else               main_mem_write(addr,data);
    return;
  case ARG_HIGH:
    if( 0xf000<=addr ){
      if( high_mode )  main_ram[addr] = data;
      else             main_high_ram[addr-0xf000] = data;
    }else              main_mem_write(addr,data);
    return;
  case ARG_SUB:
    verbose_save = verbose_io;
    verbose_io = 0;
    sub_mem_write(addr,data);
    verbose_io = verbose_save;
    return;
  case ARG_PCG:
    if( addr<8*256*2 ) font_pcg[addr] = data;
    return;
  }
}








/*==============================================================*/
/* 引数処理							*/
/*==============================================================*/

/*
 * buf[] の文字列から、単語を取り出す。区切りは、SPC と TAB。
 * 取り出した各単語の先頭アドレスが、*d_argv[] に格納される。
 * 単語は最大 MAX_ARGS 個取り出す。単語の数は、d_argc にセット。
 * 単語の数が MAX_ARGS よりも多い時は、d_argc に MAX_ARGS+1 をセット、
 * この時、MAX_ARGS 個までは、*d_argv[] が格納されている。
 */

#define MAX_ARGS	(8)
#define	MAX_CHRS	(256)

static	char	buf[MAX_CHRS];
static	int	d_argc;
static	char	*d_argv[MAX_ARGS];

static	int	argv_counter;

static	void	getarg(void)
{
    char *p = &buf[0];

    argv_counter = 1;

    d_argc = 0;
    for (;;) {

	for (;;) {
	    if      (*p == '\n' || *p == '\0') return;
	    else if (*p == ' '  || *p == '\t') p++;
	    else {
		if (d_argc == MAX_ARGS) { d_argc++;                return; }
		else                    { d_argv[ d_argc++ ] = p;  break; }
	    }
	}
	for (;;) {
	    if      (*p == '\n' || *p == '\0') { *p   = '\0';  return; }
	    else if (*p == ' '  || *p == '\t') { *p++ = '\0';  break; }
	    else                               {  p++; }
	}

    }

    return;
}


/*
 * getarg() により、main()の引数と同じような形式で、int d_argc, char *d_argv[]
 * が設定されるが、これをもう少し簡単に処理したいので、shift() 関数を用意した。
 *
 * shift() 関数を呼ぶと、一番最初の引数が解析され、その結果が argv ワークに
 * 格納される。この後で、argv.type をチェックすれば、その引数の種類が、
 * argv.val をチェックすれば、その引数の値がわかる。
 *
 * shift() 関数により、引数が見かけ上一つずつ前にずれていく。
 * ゆえに、shift() 関数を連続して呼べば、常に次の引数が解析される。
 *
 *   shift();
 *   if( argv.type == XXX ){ 処理() };
 *   shift();
 *   if( argv.type == YYY ){ 処理() };
 *   ...
 */

static struct {
    int		type;			/* 引数の種類	ARGV_xxx	   */
    int		val;			/* 引数の値	ARG_xxx または、数 */
    char	*str;			/* 引数の文字列	d_argv[xxx]と同じ  */
} argv;


static	void	shift(void)
{
    int i, size = FALSE;
    char *p, *chk;


    if (argv_counter > MAX_ARGS ||		/* これ以上引数が無い */
	argv_counter >= d_argc) {

	argv.type = ARGV_END;

    } else {					/* まだ引数があるので解析 */

	p = d_argv[ argv_counter ];
	if (*p == '#') { size = TRUE; p++; }

	argv.type = 0;
	argv.val  = strtol(p, &chk, 0);
	argv.str  = d_argv[ argv_counter ];

	if (p != chk && *chk == '\0') {			/* 数値の場合 */

	    if (size) {						/* #で始まる */
		if (argv.val <= 0) argv.type = ARGV_STR;
		else               argv.type = ARGV_SIZE;
	    } else {						/*数で始まる */
		argv.type |= ARGV_INT;
		if (argv.val >= 0)      argv.type |= ARGV_NUM;
		if (argv.val <= 0xff)   argv.type |= ARGV_PORT;
		if (argv.val <= 0xffff) argv.type |= ARGV_ADDR;
		if (BETWEEN(1, argv.val, NR_DRIVE))     argv.type |= ARGV_DRV;
		if (BETWEEN(1, argv.val, MAX_NR_IMAGE)) argv.type |= ARGV_IMG;
	    }

	} else {					/* 文字列の場合 */

	    if (size) {						/* #で始まる */
		argv.type = ARGV_STR;
	    } else {						/*字で始まる */
		for (i=0; i<COUNTOF(monitor_argv); i++) {
		    if (my_strcmp(p, monitor_argv[i].str) == 0) {
			argv.type |= monitor_argv[i].type;
			argv.val   = monitor_argv[i].val;
		    }
		}
		if (argv.type == 0) argv.type = ARGV_STR;
	    }

	}

	argv_counter ++;

    }
}


/* shift() した結果、引数が設定されたかどうかをチェック */

#define	exist_argv()	(argv.type)

/* shift() した結果、処理された引数の種類をチェック */

#define	argv_is(x)	(argv.type & (x))


/* 引数の値 (ARG_xxx) から、引数の文字列 (大文字) を得る */

static	char	*argv2str(int argv_val)
{
    int i;

    for (i=0; i<COUNTOF(monitor_argv); i++) {
	if (argv_val == monitor_argv[i].val) return monitor_argv[i].str;
    }
    return "";
}




/*==============================================================*/
/* エラー関連							*/
/*==============================================================*/
#define error()							\
	do {							\
	    printf("Invalid argument (arg %d)\n",argv_counter);	\
	    return;						\
	} while(0)




/*==============================================================*/
/* テキストスクリーン関連			by peach	*/
/*==============================================================*/
#define PUT_JIS_IN(fp)	fprintf(fp, "%c%c%c", 0x1b, 0x28, 0x49);
#define PUT_JIS_OUT(fp)	fprintf(fp, "%c%c%c", 0x1b, 0x28, 0x42);

enum { LANG_EUC, LANG_JIS, LANG_SJIS };

static int lang = -1;

static	void	set_lang(void)
{
    /*char *p;*/

    if (lang >= 0) return;

#ifdef USE_LOCALE
    setlocale(LC_ALL, "");
    p = nl_langinfo(CODESET);
    if (strncmp(p, "JIS_", 4) == 0) {
	lang = LANG_JIS;
    } else if (strncmp(p, "SHIFT_JIS", 9) == 0) {
	lang = LANG_SJIS;
    } else {			/* EUC_JP (default)*/
	lang = LANG_EUC;
    }
#else
    lang = LANG_EUC;
#endif
}

void	print_hankaku(FILE *fp, Uchar *str, char ach)
{
    Uchar *ptr;

    /* 標準出力じゃないならそのまま */
    if (fp != stdout) {
	fprintf(fp, "%s", str);
	return;
    }

    ptr = str;
    set_lang();

    if (lang == LANG_JIS) PUT_JIS_IN(fp);
    while (*ptr != '\0') {
	if (*ptr == '\n') {
	    if (lang == LANG_JIS) {
		PUT_JIS_OUT(fp);
		fputc('\n', fp);
		PUT_JIS_IN(fp);
	    } else {
		fputc('\n', fp);
	    }
	} else if (0xa1u <= *ptr && *ptr <= 0xdfu) {
	    switch (lang) {
	    case LANG_EUC:
		fputc(0x8eu, fp);	fputc(*ptr, fp);	break;
	    case LANG_JIS:
		fputc(*ptr - 0x80u, fp);	break;
	    case LANG_SJIS:
		fputc(*ptr, fp);	break;
	    }
	} else if (isprint(*ptr)) {
	    fputc(*ptr, fp);
	} else {		/* 表示不能 */
	    fputc(ach, fp);
	    /*fprintf(fp, "0x%x", *ptr);*/
	}
	ptr++;
    }
    if (lang == LANG_JIS) PUT_JIS_OUT(fp);
}


/****************************************************************/
/* 命令別処理							*/
/****************************************************************/

/*--------------------------------------------------------------*/
/* help [<cmd>]							*/
/*	ヘルプを表示する					*/
/*--------------------------------------------------------------*/
static	void	monitor_help(void)
{
    int i;
    char *cmd = NULL;

    if (exist_argv()) {					/* [cmd] */
	cmd = argv.str;
	shift();
    }
    if (exist_argv()) error();



    if (cmd == NULL) {			/* 引数なし。全ヘルプ表示 */

	printf("help\n");
	for (i=0; i<COUNTOF(monitor_cmd); i++) {
	    printf("  %-10s %s\n", monitor_cmd[i].cmd,
				   monitor_cmd[i].help_mes);
	}
	printf("     Note: type \"help <command-name>\" for more details.\n");

    } else {				/* 引数のコマンドのヘルプ表示 */

	for (i=0; i<COUNTOF(monitor_cmd); i++) {
	    if (strcmp(cmd, monitor_cmd[i].cmd) == 0) break;
	}
	if (i == COUNTOF(monitor_cmd)) error();
	(monitor_cmd[i].help)();

    }

}


/*--------------------------------------------------------------*/
/* dump [<bank>] <start-addr> [<end-addr>]			*/
/* dump [<bank>] <start-addr> [#<size>]				*/
/*	メモリダンプを表示する					*/
/*--------------------------------------------------------------*/
static	int	save_dump_addr = -1;
static	int	save_dump_bank = ARG_MAIN;
static	void	monitor_dump( void )
{
  int	i, j;
  byte	c;
  int	bank  = save_dump_bank;
  int	start = save_dump_addr;
  int	size  = 256;


  if( !exist_argv() ){
    if( save_dump_addr == -1 ) error();
    /* else   skip */
  }else{

    if( argv_is( ARGV_BANK ) ){				/* [<bank>] */
      bank = argv.val;
      shift();
    }

    if( !argv_is( ARGV_ADDR ) ) error();		/* <addr> */
    start = argv.val;
    shift();						

    if     ( !exist_argv() )	    /* skip */;		/* [<addr|#size>] */
    else if( argv_is( ARGV_SIZE ) ) size = argv.val;
    else if( argv_is( ARGV_ADDR ) ) size = argv.val - start +1;
    else                            error();
    shift();
  }

  if( exist_argv() ) error();


	/*================*/

  save_dump_addr = start + size;		/* 毎回ダンプしたアドレスを */
  save_dump_bank = bank;			/* 覚えておく (連続ダンプ用)*/

  size = ( size+15 ) /16;

  printf("addr : +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n");
  printf("---- : -----------------------------------------------\n");
  for( i=0; i<size; i++ ){
    printf("%04X : ",(start+i*16)&0xffff);
    for( j=0; j<16; j++ ){
      printf("%02X ", peek_memory( bank, start+i*16+j ) );
    }
    printf("|");
    for( j=0; j<16; j++ ){
      c = peek_memory( bank, start+i*16+j );
      if( !isprint( c ) ) c = '.';
      printf("%c", c );
    }
    printf("|\n");
  }
  printf("\n");

  return;
}

/*----------------------------------------------------------------------*/
/* dumpext [<bank>] [#<board>] <start-addr> [<end-addr>]		*/
/* dumpext [<bank>] [#<board>] <start-addr> [#<size>]			*/
/*	拡張RAMのメモリダンプを表示する					*/
/*				この機能は peach氏により実装されました	*/
/*----------------------------------------------------------------------*/
static	int	save_dumpext_addr = -1;
static	int	save_dumpext_bank = ARG_EXT0;
static	int	save_dumpext_board = 1;
static	void	monitor_dumpext( void )
{
  int	i, j;
  byte	c;
  int	bank  = save_dumpext_bank;
  int	start = save_dumpext_addr;
  int	board = save_dumpext_board;
  int	size  = 256;


  if( !exist_argv() ){
    if( save_dumpext_addr == -1 ) error();
    /* else   skip */
  }else{

    if( argv_is( ARGV_BANK ) ){				/* [<bank>] */
      bank = argv.val;
      if ( bank < ARG_EXT0 || ARG_EXT3 < bank ) error();
      shift();
    }

    if( argv_is( ARGV_SIZE ) ) {			/* [#<board>] */
      board = argv.val;
      if ( board < 1 || use_extram < board ) error();
      shift();
    }

    if( !argv_is( ARGV_ADDR ) ) error();		/* <addr> */
    start = argv.val;
    if ( start >= 0x8000 ) error();
    shift();						

    if     ( !exist_argv() )	    /* skip */;		/* [<addr|#size>] */
    else if( argv_is( ARGV_SIZE ) ) size = argv.val;
    else if( argv_is( ARGV_ADDR ) ) size = argv.val - start +1;
    else                            error();
    if ( start + size >= 0x8000 ) error();
    shift();
  }

  if( exist_argv() ) error();


	/*================*/

  save_dumpext_addr  = start + size;		/* 毎回ダンプしたアドレスを */
  save_dumpext_bank  = bank;			/* 覚えておく (連続ダンプ用)*/
  save_dumpext_board = board;

  size = ( size+15 ) /16;
  bank = bank - ARG_EXT0 + (board - 1) * 4;

  printf("addr : +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n");
  printf("---- : -----------------------------------------------\n");
  for( i=0; i<size; i++ ){
    printf("%04X : ",(start+i*16)&0xffff);
    for( j=0; j<16; j++ ){
      printf("%02X ", ext_ram[bank][start+i*16+j]) ;
    }
    printf("|");
    for( j=0; j<16; j++ ){
      c = ext_ram[bank][start+i*16+j];
      if( !isprint( c ) ) c = '.';
      printf("%c", c );
    }
    printf("|\n");
  }
  printf("\n");

  return;
}



/*--------------------------------------------------------------*/
/* fill [<bank>] <start-addr> <end-addr> <value>		*/
/* fill [<bank>] <start-addr> #<size>	 <value>		*/
/*	メモリを埋める						*/
/*--------------------------------------------------------------*/
static	void	monitor_fill( void )
{
  int	i;
  int	bank  = ARG_MAIN;
  int	start, size, value;


  if( !exist_argv() ) error();

  if( argv_is( ARGV_BANK ) ){				/* [<bank>] */
    bank = argv.val;
    shift();
  }

  if( !argv_is( ARGV_ADDR ) ) error();			/* <addr> */
  start = argv.val;
  shift();						

  if     ( argv_is( ARGV_SIZE ) ) size = argv.val;	/* [<addr|#size>] */
  else if( argv_is( ARGV_ADDR ) ) size = argv.val - start +1;
  else                            error();
  shift();

  if( !argv_is( ARGV_INT )) error();			/* <data> */
  value = argv.val;
  shift();

  if( exist_argv() ) error();


	/*================*/

  for( i=0; i<size; i++ ){
    poke_memory( bank, start+i, value );
  }

  return;
}



/*--------------------------------------------------------------*/
/* move [<bank>] <src-addr> <end-addr> [<bank>] <dist-addr>	*/
/* move [<bank>] <src-addr> #size      [<bank>] <dist-addr>	*/
/*	メモリ転送						*/
/*--------------------------------------------------------------*/
static	void	monitor_move( void )
{
  int	i;
  int	s_bank  = ARG_MAIN;
  int	d_bank  = ARG_MAIN;
  int	start, size, dist;
  byte	data;


  if( !exist_argv() ) error();

  if( argv_is( ARGV_BANK ) ){				/* [<bank>] */
    s_bank = argv.val;
    d_bank = s_bank;
    shift();
  }

  if( !argv_is( ARGV_ADDR ) ) error();			/* <addr> */
  start = argv.val;
  shift();						

  if     ( argv_is( ARGV_SIZE ) ) size = argv.val;	/* [<addr|#size>] */
  else if( argv_is( ARGV_ADDR ) ) size = argv.val - start +1;
  else                            error();
  shift();

  if( argv_is( ARGV_BANK ) ){				/* [<bank>] */
    d_bank = argv.val;
    shift();
  }

  if( !argv_is( ARGV_ADDR ) ) error();			/* <addr> */
  dist = argv.val;
  shift();						

  if( exist_argv() ) error();


	/*================*/

  if( start+size <= dist ){			/* 転送元-転送先が 重ならない*/
    for( i=0; i<size; i++ ){
      data = peek_memory( s_bank, start+i );
      poke_memory( d_bank, dist+i, data );
    }
  }else{					/* 転送元-転送先が 重なる */
    for( i=size-1; i>=0; i-- ){
      data = peek_memory( s_bank, start+i );
      poke_memory( d_bank, dist+i, data );
    }
  }

  return;
}



/*--------------------------------------------------------------*/
/* search [<value> [[<bank>] <start-addr> <end-addr>]]		*/
/*	特定の定数 (1バイト) をサーチ				*/
/*--------------------------------------------------------------*/
static	int	save_search_addr = -1;
static	int	save_search_size = -1;
static	int	save_search_bank = ARG_MAIN;
static	byte	save_search_data;
static	void	monitor_search( void )
{
  int	i, j;

  if( !exist_argv() ){
    if( save_search_addr == -1 ) error();
    /* else   skip */
  }else{

    if( !argv_is( ARGV_INT )) error();			/* <value> */
    save_search_data = argv.val;
    shift();

    if( !exist_argv() ){
      if( save_search_addr == -1 ) error();
      /* else   skip */
    }else{

      if( argv_is( ARGV_BANK ) ){			/* [<bank>] */
	save_search_bank = argv.val;
	shift();
      }

      if( !argv_is( ARGV_ADDR ) ) error();		/* <addr> */
      save_search_addr = argv.val;
      shift();						

      if     ( argv_is( ARGV_SIZE ) )			/* <end-addr|#size>*/
			save_search_size = argv.val;
      else if( argv_is( ARGV_ADDR ) )
			save_search_size = argv.val - save_search_addr +1;
      else error();
      shift();
    }

  }

  if( exist_argv() ) error();


	/*================*/

  j=0;						/* 一致したアドレスを列挙 */
  for( i=0; i<save_search_size; i++ ){
    if( peek_memory( save_search_bank, save_search_addr+i )
        == save_search_data ){
      printf("[%04X] ",save_search_addr+i);
      if( ++j == 11 ){ printf("\n"); j=0; }
    }
  }
  if( j!=0 ) printf("\n");

  return;
}



/*--------------------------------------------------------------*/
/* read [<bank>] <addr>						*/
/*	特定のアドレスをリード					*/
/*--------------------------------------------------------------*/
static	void	monitor_read( void )
{
  int	i, j;
  int	addr;
  byte	data;
  int	bank = ARG_MAIN;



  if( !exist_argv() ) error();

  if( argv_is( ARGV_BANK )){				/* [<bank>] */
    bank = argv.val;
    shift();
  }

  if( !argv_is( ARGV_ADDR )) error();			/* <addr> */
  addr = argv.val;
  shift();

  if( exist_argv() ) error();


	/*================*/

  data = peek_memory( bank, addr );

  printf("READ memory %s[ %04XH ] -> %02X  (= %d | %+d | ",argv2str(bank),
	 addr,(Uint)data,(Uint)data,(int)((char)data));
  for( i=0, j=0x80; i<8; i++, j>>=1 ){
    printf("%d", (data & j) ? 1 : 0 );
  }
  printf("B )\n");

  return;
}
/*--------------------------------------------------------------*/
/* write [<bank>] <addr> <data>					*/
/*	特定のアドレスにライト					*/
/*--------------------------------------------------------------*/
static	void	monitor_write( void )
{
  int	i, j;
  int	addr;
  byte	data;
  int	bank = ARG_MAIN;


  if( !exist_argv() ) error();

  if( argv_is( ARGV_BANK )){				/* [<bank>] */
    bank = argv.val;
    shift();
  }

  if( !argv_is( ARGV_ADDR )) error();			/* <addr> */
  addr = argv.val;
  shift();

  if( !argv_is( ARGV_INT )) error();			/* <data> */
  data = argv.val;
  shift();

  if( exist_argv() ) error();


	/*================*/

  poke_memory( bank, addr, data );

  printf("WRITE memory %s[ %04XH ] <- %02X  (= %d | %+d | ",argv2str(bank),
	 addr,(Uint)data,(Uint)data,(int)((char)data));
  for( i=0, j=0x80; i<8; i++, j>>=1 ){
    printf("%d", (data & j) ? 1 : 0 );
  }
  printf("B )\n");

  return;
}



/*--------------------------------------------------------------*/
/* in [<cpu>] <port>						*/
/*	特定のポートから入力					*/
/*--------------------------------------------------------------*/
static	void	monitor_in( void )
{
  int	i, j;
  int	cpu = ARG_MAIN, port;
  byte	data;


  if( !exist_argv() ) error();

  if( argv_is( ARGV_CPU )){				/* [<cpu>] */
    cpu = argv.val;
    shift();
  }

  if( !argv_is( ARGV_PORT )) error();			/* <port> */
  port = argv.val;
  shift();

  if( exist_argv() ) error();


	/*================*/

  if( cpu==ARG_MAIN ) data = main_io_in(port) & 0xff;
  else                data = sub_io_in(port) & 0xff;

  printf("IN port %s[ %02X ] -> %02X (",argv2str(cpu),port,data);
  for( i=0, j=0x80; i<8; i++, j>>=1 ){
    printf("%d", (data & j) ? 1 : 0 );
  }
  printf(")\n");

  return;
}



/*--------------------------------------------------------------*/
/* out [<cpu>] <port> <data>					*/
/*	特定のポートに出力					*/
/*--------------------------------------------------------------*/
static	void	monitor_out( void )
{
  int	i, j;
  int	cpu = ARG_MAIN, port;
  byte	data;


  if( !exist_argv() ) error();

  if( argv_is( ARGV_CPU )){				/* [<cpu>] */
    cpu = argv.val;
    shift();
  }

  if( !argv_is( ARGV_PORT )) error();			/* <port> */
  port = argv.val;
  shift();

  if( !argv_is( ARGV_INT )) error();			/* <data> */
  data = argv.val & 0xff;
  shift();

  if( exist_argv() ) error();


	/*================*/

  if( cpu==ARG_MAIN ) main_io_out(port,data);
  else                sub_io_out(port,data);

  printf("OUT port %s[ %02X ] <- %02X (",argv2str(cpu),port,data);
  for( i=0, j=0x80; i<8; i++, j>>=1 ){
    printf("%d", (data & j) ? 1 : 0 );
  }
  printf(")\n");

  return;
}


/*--------------------------------------------------------------*/
/* reset [<bas-mode>] [<clock-mode>] [<sound-board>] [<dipsw>]	*/
/*	リセット。モードとサウンドボードとディップを設定できる	*/
/*--------------------------------------------------------------*/
static	void	monitor_reset(void)
{
    T_RESET_CFG cfg;
    int dipsw = -1, bas_mode = -1, ck_mode = -1, sd_mode = -1;


    while (exist_argv()) {
	if (argv_is(ARGV_BASMODE)) {
	    if (bas_mode != -1) error();		/* <bas-mode> */
	    bas_mode = argv.val;
	    shift();
	} else if (argv_is(ARGV_CKMODE)) {
	    if (ck_mode != -1) error();			/* <ck-mode> */
	    ck_mode = argv.val;
	    shift();
	} else if (argv_is(ARGV_SDMODE)) {
	    if (sd_mode != -1) error();			/* <sound-board> */
	    sd_mode = argv.val;
	    shift();
	} else if (argv_is(ARGV_NUM)) {			/* <dipsw> */
	    if (dipsw != -1) error();
	    dipsw = argv.val & 0xffff;
	    shift();
	} else {
	    error();
	}
    }

    if (exist_argv()) error();


	/*================*/
    quasi88_get_reset_cfg(&cfg);

    switch (bas_mode) {
    case ARG_N:		cfg.boot_basic = BASIC_N;		break;
    case ARG_V1S:	cfg.boot_basic = BASIC_V1S;		break;
    case ARG_V1H:	cfg.boot_basic = BASIC_V1H;		break;
    case ARG_V2:	cfg.boot_basic = BASIC_V2;		break;
    }

    switch (ck_mode) {
    case ARG_8MHZ:	cfg.boot_clock_4mhz = FALSE;		break;
    case ARG_4MHZ:	cfg.boot_clock_4mhz = TRUE;		break;
    }

    switch (sd_mode) {
    case ARG_SD:	cfg.sound_board = SOUND_I;		break;
    case ARG_SD2:	cfg.sound_board = SOUND_II;		break;
    }

    if (dipsw != -1) cfg.boot_dipsw = dipsw;

    quasi88_reset(&cfg);

    /* quasi88_exec(); */
    /* go はしない。 reset-go というコマンドがあると便利？ */
}

/*--------------------------------------------------------------*/
/*  reg [[<cpu>] [<name> <value>]]				*/
/*  reg all							*/
/*	レジスタの内容を表示／変更				*/
/*--------------------------------------------------------------*/
static	void	monitor_reg( void )
{
  int	all = FALSE;
  int	cpu = -1, reg = -1, value=0;
  z80arch	*z80;


  if( exist_argv() ){

    if( argv_is( ARGV_ALL )){				/* all */
      all = TRUE;
      shift();
    }else{

      if( argv_is( ARGV_CPU )){				/* [<cpu>] */
	cpu = argv.val;
	shift();
      }

      if( exist_argv() ){
	if( !argv_is( ARGV_REG )) error();		/* [<name>] */
	reg = argv.val;
	shift();
	if( !argv_is( ARGV_INT )) error();		/* [<value>] */
	value = argv.val;
	shift();
      }
    }
  }

  if( exist_argv() ) error();


	/*================*/

  if( reg==-1 ){				/* レジスタ表示 */
    if( !all  &&  cpu==-1 ){
      if( cpu_timing >= 2 )   all = TRUE;
      else{
	if( select_main_cpu ) cpu = ARG_MAIN;
	else                  cpu = ARG_SUB;
      }
    }
    if( all ){
      z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
      z80_debug( &z80sub_cpu,  "[SUB CPU]\n"  );
    }else{
      if( cpu==ARG_MAIN ) z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
      else                z80_debug( &z80sub_cpu,  "[SUB CPU]\n"  );
    }
    return;
  }

  						/* レジスタ代入 */
  if( cpu==-1 ){
    if( cpu_timing >= 2 ){
      cpu = ARG_MAIN;
    }else{
      if( select_main_cpu ) cpu = ARG_MAIN;
      else                  cpu = ARG_SUB;
    }
  }

  if( cpu==ARG_MAIN ) z80 = &z80main_cpu;
  else                z80 = &z80sub_cpu;

  switch( reg ){
  case ARG_AF:	z80->AF.W = value;	break;
  case ARG_BC:	z80->BC.W = value;	break;
  case ARG_DE:	z80->DE.W = value;	break;
  case ARG_HL:	z80->HL.W = value;	break;
  case ARG_IX:	z80->IX.W = value;	break;
  case ARG_IY:	z80->IY.W = value;	break;
  case ARG_SP:	z80->SP.W = value;	break;
  case ARG_PC:	z80->PC.W = value;	break;
  case ARG_AF1:	z80->AF1.W = value;	break;
  case ARG_BC1:	z80->BC1.W = value;	break;
  case ARG_DE1:	z80->DE1.W = value;	break;
  case ARG_HL1:	z80->HL1.W = value;	break;
  case ARG_I:	value &= 0xff;	z80->I = value;		break;
  case ARG_R:	value &= 0xff;	z80->R = value;		break;
  case ARG_IFF:	if(value)  value=1;	z80->IFF  = value;	break;
  case ARG_IM:  if(value>3)value=2;	z80->IM   = value;	break;
  case ARG_HALT:if(value)  value=1;	z80->HALT = value;	break;
  }

  printf("[%s] reg %s <- %04X\n",argv2str(cpu),argv2str(reg),value);
}



/*--------------------------------------------------------------*/
/* disasm [[<cpu>][<start-addr>][#<steps>]]			*/
/*	逆アセンブル						*/
/*--------------------------------------------------------------*/
static	int	save_disasm_cpu     = -1;
static	int	save_disasm_addr[2] = { -1, -1 };
static	void	monitor_disasm( void )
{
  int	i, pc;
  int	addr = -1;
  int	cpu  = -1;
  int	step = 16;
  z80arch	*z80;


  if( exist_argv() ){

    if( argv_is( ARGV_CPU )){				/* [<cpu>] */
      cpu = argv.val;
      shift();
    }

    if( argv_is( ARGV_ADDR )){				/* [<addr>] */
      addr = argv.val;
      shift();
    }

    if( argv_is( ARGV_SIZE )){				/* [#<step>] */
      step = argv.val;
      shift();

    }else if( argv_is( ARGV_STR ) && strcmp( argv.str, "#" )==0 ){
      if( cpu == -1 ){
	save_disasm_cpu     = -1;
	save_disasm_addr[0] = -1;
	save_disasm_addr[1] = -1;
	printf( "disasm all reset\n" );
      }else{
	save_disasm_addr[cpu] = -1;
	printf( "disasm addr reset\n" );
      }
      return;
    }

  }
  if( exist_argv() ) error();


	/*================*/

  if( cpu == -1 ){					/* CPU 未指定時 */
    cpu = save_disasm_cpu;
    if( cpu == -1 ){
      if( cpu_timing >= 2 ){
	cpu = ARG_MAIN;
      }else{
	if( select_main_cpu ) cpu = ARG_MAIN;
	else                  cpu = ARG_SUB;
      }
    }
  }

  if( cpu==ARG_MAIN ) z80 = &z80main_cpu;
  else                z80 = &z80sub_cpu;

  if( addr == -1 ){					/* ADDR 未指定時 */
    addr = save_disasm_addr[ cpu ];
    if( addr == -1 ) addr = z80->PC.W;
  }


  pc = 0;
  for( i=0; i<step; i++ ){
    pc += z80_line_disasm( z80, addr+pc );
    printf("\n");
  }

  save_disasm_cpu       = cpu;
  save_disasm_addr[cpu] = ( addr + pc ) & 0xffff;
}


/*--------------------------------------------------------------*/
/* go								*/
/*	実行							*/
/*--------------------------------------------------------------*/
static	void	monitor_go( void )
{
  if( exist_argv() ) error();

  quasi88_exec();
}



/*--------------------------------------------------------------*/
/* trace <step>							*/
/* trace #<step>						*/
/* trace change							*/
/*	指定したステップ分またはCPU処理が変わるまで、実行	*/
/*--------------------------------------------------------------*/
static	int	save_trace_change = FALSE;
static	void	monitor_trace( void )
{
  int	change = FALSE, step = trace_counter;

  if( exist_argv() ){

    if     ( argv_is( ARGV_CHANGE )) change = TRUE;		/* [change]  */
    else if( argv_is( ARGV_SIZE )  ) step = argv.val;		/* [<step>]  */
    else if( argv_is( ARGV_NUM )   ) step = argv.val;		/* [#<step>] */
    else                             error();
    shift();

  }else{

    if( save_trace_change ) change = TRUE;
    else                    change = FALSE;

  }

  if( exist_argv() ) error();


	/*================*/

  if( change ){
    save_trace_change = TRUE;
    quasi88_exec_trace_change();

  }else {
    save_trace_change = FALSE;
    trace_counter = step;
    quasi88_exec_trace();
  }
}




/*--------------------------------------------------------------*/
/* step								*/
/* step [call] [jp] [rep] [all]					*/
/*	1ステップ、実行						*/
/*	CALL、DJNZ、LDIR etc のスキップが指定可能		*/
/*--------------------------------------------------------------*/
static	void	monitor_step( void )
{
  int	call = FALSE, jp = FALSE, rep = FALSE;
  int	flag = FALSE;
  int	cpu;
  byte	code;
  word	addr;

  while( exist_argv() ){
    if( argv_is( ARGV_STEP )){
      if( argv.val==ARG_CALL ) call = TRUE;
      if( argv.val==ARG_JP )   jp   = TRUE;
      if( argv.val==ARG_REP )  rep  = TRUE;
      if( argv.val==ARG_ALL ){ call = jp = rep = TRUE; }
      shift();
    }else{
      error();
    }
  }

  if( exist_argv() ) error();


	/*================*/

  if( cpu_timing >= 2 ){

    quasi88_exec_step();
    return;

  }else{

    if( select_main_cpu ){
      cpu  = BP_MAIN;
      addr = z80main_cpu.PC.W;
      code = main_mem_read( addr );
    }else{
      cpu  = BP_SUB;
      addr = z80sub_cpu.PC.W;
      code = sub_mem_read( addr );
    }

    if( call ){
      if(   code      == 0xcd ||		/* CALL nn    = 11001101B */
	  (code&0xc7) == 0xc4 ){		/* CALL cc,nn = 11ccc100B */
	addr += 3;
	flag = TRUE;
      }
    }
    if( jp ){
      if( code == 0x10 ){			/* DJNZ e     = 00010000B */
	addr += 2;
	flag = TRUE;
      }
    }
    if( rep ){
      if( code == 0xed ){			/* LDIR/LDDR/CPIR/CPDR etc */
	if( select_main_cpu ) code = main_mem_read( addr+1 );
	else                  code = sub_mem_read( addr+1  );
	if( (code&0xf4) == 0xb0 ){
	  addr += 2;
	  flag = TRUE;
	}
      }
    }

    if( flag ){
      break_point[cpu][BP_NUM_FOR_SYSTEM].type = BP_PC;
      break_point[cpu][BP_NUM_FOR_SYSTEM].addr = addr;
      quasi88_exec();
    }else{
      break_point[cpu][BP_NUM_FOR_SYSTEM].type = BP_NONE;
      quasi88_exec_step();
    }
  }
}



/*--------------------------------------------------------------*/
/* S								*/
/*	step all に同じ						*/
/*--------------------------------------------------------------*/
static	void	monitor_stepall( void )
{
  int	flag = FALSE;
  int	cpu;
  byte	code;
  word	addr;

  if( exist_argv() ) error();


	/*================*/

  if( cpu_timing >= 2 ){
    quasi88_exec_step();
    return;
  }else{
    if( select_main_cpu ){
      cpu  = BP_MAIN;
      addr = z80main_cpu.PC.W;
      code = main_mem_read( addr );
    }else{
      cpu  = BP_SUB;
      addr = z80sub_cpu.PC.W;
      code = sub_mem_read( addr );
    }
    if(   code      == 0xcd ||		/* CALL nn    = 11001101B */
        (code&0xc7) == 0xc4 ){		/* CALL cc,nn = 11ccc100B */
      addr += 3;
      flag = TRUE;
    }
    if( code == 0x10 ){			/* DJNZ e     = 00010000B */
      addr += 2;
      flag = TRUE;
    }
    if( code == 0xed ){			/* LDIR/LDDR/CPIR/CPDR etc */
      if( select_main_cpu ) code = main_mem_read( addr+1 );
      else                  code = sub_mem_read( addr+1  );
      if( (code&0xf4) == 0xb0 ){
	addr += 2;
	flag = TRUE;
      }
    }
    if( code == 0x76 ||			/* HALT / RST XXH */
        code == 0xc7 || code == 0xcf || code == 0xd7 || code == 0xdf || 
	code == 0xe7 || code == 0xef || code == 0xf7 || code == 0xff ){
      addr += 1;
      flag = TRUE;
    }
    if( flag ){
      break_point[cpu][BP_NUM_FOR_SYSTEM].type = BP_PC;
      break_point[cpu][BP_NUM_FOR_SYSTEM].addr = addr;
      quasi88_exec();
    }else{
      break_point[cpu][BP_NUM_FOR_SYSTEM].type = BP_NONE;
      quasi88_exec_step();
    }
  }

}



/*--------------------------------------------------------------*/
/* break [<cpu>] [PC|READ|WRITE|IN|OUT] <addr|port> [#<No>]	*/
/* break [<cpu>] CLEAR [#<No>]					*/
/* break							*/
/*	ブレークポイントの設定／解除／表示			*/
/*--------------------------------------------------------------*/
static	void	monitor_break( void )
{
  int	show = FALSE, i, j;
  char	*s=NULL;
  int	cpu = BP_MAIN, action = ARG_PC, addr=0, number = 0;


  if( exist_argv() ){

    if( argv_is( ARGV_CPU )){				/* [<cpu>] */
      if( argv.val==ARG_MAIN ) cpu = BP_MAIN;
      else                     cpu = BP_SUB;
      shift();
    }

    if( argv_is( ARGV_BREAK ) ){			/* <action> */
      action = argv.val;
      shift();
    }

    switch( action ){					/* <addr|port> */
    case ARG_IN: case ARG_OUT:
      if( !argv_is( ARGV_PORT )) error();
      addr = argv.val;
      shift();
      break;
    case ARG_PC: case ARG_READ: case ARG_WRITE:
      if( !argv_is( ARGV_ADDR )) error();
      addr = argv.val;
      shift();
      break;
    }

    if( exist_argv() ){					/* [#<No>] */
      if( argv.val < 0 || argv.val > NR_BP ||
	  (action != ARG_CLEAR && argv.val < 1) ) error();
      number = argv.val -1;
      shift();
    }

  }else{

    show = TRUE;

  }

  if( exist_argv() ) error();


	/*================*/

  if( show ){
    for( j=0; j<2; j++ ){
      printf( "  %s:\n", (j==0)?"MAIN":"SUB" );
      for( i=0; i<NR_BP; i++ ){
	printf( "    #%d  ", i+1 );
	if (i < 9) printf(" ");			/* 見やすく by peach */
	addr = break_point[j][i].addr;
	switch( break_point[j][i].type ){
	case BP_NONE:	printf("-- none --\n");				break;
	case BP_PC:	printf("PC   reach %04XH\n",addr&0xffff);	break;
	case BP_READ:	printf("READ  from %04XH\n",addr&0xffff);	break;
	case BP_WRITE:	printf("WRITE   to %04XH\n",addr&0xffff);	break;
	case BP_IN:	printf("INPUT from %02XH\n",addr&0xff);		break;
	case BP_OUT:	printf("OUTPUT  to %04XH\n",addr&0xff);		break;
	}
      }
    }
  }else{
    if( action==ARG_CLEAR ){
      if ( number<0 ) {
	for ( i=0; i<9; i++ ) break_point[cpu][i].type = BP_NONE;
	printf( "clear break point %s - all\n",(cpu==0)?"MAIN":"SUB" );
      } else {
	break_point[cpu][number].type = BP_NONE;
	printf( "clear break point %s - #%d\n",(cpu==0)?"MAIN":"SUB",number+1 );
      }
    }else{
      switch( action ){
      case ARG_PC:
	break_point[cpu][number].type = BP_PC;
	s = "PC : %04XH";
	break;
      case ARG_READ:
	break_point[cpu][number].type = BP_READ;
	s = "READ : %04XH";
	break;
      case ARG_WRITE:
	break_point[cpu][number].type = BP_WRITE;
	s = "WRITE : %04XH";
	break;
      case ARG_IN:
	break_point[cpu][number].type = BP_IN;
	s = "IN : %02XH";
	break;
      case ARG_OUT:
	break_point[cpu][number].type = BP_OUT;
	s = "OUT : %02XH";
	break;
      }
      break_point[cpu][number].addr = addr;
      printf( "set break point %s - #%d [ ",(cpu==0)?"MAIN":"SUB",number+1 );
      printf( s, addr );
      printf( " ]\n" );
    }
    if( cpu==BP_MAIN ) pc88main_bus_setup();
    else               pc88sub_bus_setup();
  }

}




/*--------------------------------------------------------------*/
/* loadmem <filename> <bank> <start-addr> [<end-addr>]		*/
/* loadmem <filename> <bank> <start-addr> [#<size>]		*/
/*	ファイルからメモリにロード				*/
/*--------------------------------------------------------------*/
static	void	monitor_loadmem( void )
{
  int	addr, size, bank;
  char	*filename;
  FILE	*fp;
  int	c, i;

  if( !exist_argv() ) error();				/* <filename> */
  filename = argv.str;
  shift();

  if( !argv_is( ARGV_BANK ) ) error();			/* <bank> */
  bank = argv.val;
  shift();

  if( !argv_is( ARGV_ADDR ) ) error();			/* <start-addr> */
  addr = argv.val;
  shift();

  if     ( argv_is( ARGV_SIZE ) ){			/* #<size>|<end-addr>*/
    size = argv.val;
    shift();
  }else if( argv_is( ARGV_ADDR ) ){
    size = argv.val - addr +1;
    shift();
  }else{
    size = -1;
  }

  if( exist_argv() ) error();


	/*================*/

  if( (fp=fopen( filename,"rb")) ){
    if( size<0 ){
#if 0
      struct stat filestats;

      if(fstat(fileno(fp), &filestats)) size = 0;
      else                              size = filestats.st_size;
#else
      if( fseek( fp, 0, SEEK_END )==0 ){
	size = ftell( fp );
	if( size < 0 ) size = 0;
      }
      fseek( fp, 0, SEEK_SET );
#endif
    }
    for( i=0; i<size; i++ ){
      if( (c=getc(fp)) ==  EOF ){
	printf("Warning : loadmem : file size too short (<%d)\n",size);
	break;
      }
      poke_memory( bank, addr+i, c );
    }
    fclose(fp);
    printf("Load [%s] -> %s (size %d )\n",filename,argv2str(bank),i);
  }else{
    printf("file [%s] can't open\n",filename);
  }
}



/*--------------------------------------------------------------*/
/* savemem <filename> <bank> <start-addr> <end-addr>		*/
/* savemem <filename> <bank> <start-addr> #<size>		*/
/*	メモリをファイルにセーブ				*/
/*--------------------------------------------------------------*/
static	void	monitor_savemem( void )
{
  int	addr, size, bank;
  char	*filename;
  FILE	*fp;
  int	c, i;

  if( !exist_argv() ) error();				/* <filename> */
  filename = argv.str;
  shift();

  if( !argv_is( ARGV_BANK ) ) error();			/* <bank> */
  bank = argv.val;
  shift();

  if( !argv_is( ARGV_ADDR ) ) error();			/* <start-addr> */
  addr = argv.val;
  shift();

  if     ( argv_is( ARGV_SIZE ) ) size = argv.val;	/* #<size>|<end-addr>*/
  else if( argv_is( ARGV_ADDR ) ) size = argv.val - addr +1;
  else                            error();
  shift();

  if( exist_argv() ) error();


	/*================*/

  if( (fp=fopen( filename,"wb")) ){
    for( i=0; i<size; i++ ){
      c = peek_memory( bank, addr+i );
      if( putc(c, fp) == EOF ){
	printf("Warning : savemem : file write failed\n");
	break;
      }
    }
    fclose(fp);
    printf("Save [%s] -> %s (size %d )\n",filename,argv2str(bank),i);
  }else{
    printf("file [%s] can't open\n",filename);
  }
}



/*----------------------------------------------------------------------*/
/* fbreak [<cpu>] [READ|WRITE|DIAG] <drive> <track> [<sector>] [#<No>]	*/
/* fbreak [<cpu>] CLEAR [#<No>]						*/
/* fbreak								*/
/*	FDC ブレークポイントの設定／解除／表示				*/
/*				この機能は peach氏により実装されました	*/
/*----------------------------------------------------------------------*/
static	void	monitor_fbreak(void)
{
    int show = FALSE, i;
    char *s = NULL;
    int action = ARG_READ, number = 0;
    int	drive = -1, track = -1, sector = -1;

    if(exist_argv()){
	if (argv_is(ARGV_FBREAK)) {			/* <action> */
	    action = argv.val;
	    shift();
	}

	if (action == ARG_READ || action == ARG_WRITE || action == ARG_DIAG) {
	    if (! argv_is(ARGV_DRV)) error();
	    drive = argv.val;
	    shift();
	    if (! argv_is(ARGV_NUM) || argv.val < 0 || argv.val > 163) error();
	    track = argv.val;
	    shift();
	    if (exist_argv() && ! argv_is(ARGV_SIZE)) {
		if (! argv_is(ARGV_NUM) || argv.val < 0) error();
		sector = argv.val;
		shift();
	    }
	}

	if (exist_argv()) {					/* [#<No>] */
	    if (argv.val < 0 || argv.val > NR_BP ||
		(action != ARG_CLEAR && argv.val < 1)) error();
	    number = argv.val -1;
	    shift();
	}
    } else {
	show = TRUE;
    }

    if (exist_argv()) error();

    if (show) {
	printf("  %s:\n", "FDC");
	for (i=0; i<NR_BP; i++) {
	    /*if (break_point_fdc[i].type == BP_NONE) continue;*/
	    printf("    #%d  ", i+1);
	    if (i < 9) printf(" ");
	    drive = break_point_fdc[i].drive;
	    track = break_point_fdc[i].track;
	    sector = break_point_fdc[i].sector;
	    if (break_point_fdc[i].type == BP_NONE) {
		printf("-- none --\n");
	    } else {
		switch (break_point_fdc[i].type) {
		case BP_READ:	printf("FDC READ from "); break;
		case BP_WRITE:	printf("FDC WRITE  to "); break;
		case BP_DIAG:	printf("FDC DIAG   in "); break;
		}
		printf("D:%d T:%d", drive, track);
		if (sector >= 0) printf(" S:%d", sector);
		printf("\n");
	    }
	}
    } else {
	if (action == ARG_CLEAR) {
	    if (number < 0) {
		for (i=0; i<9; i++) break_point_fdc[i].type = BP_NONE;
		printf("clear break point %s - all\n", "FDC");
	    } else {
		break_point_fdc[number].type = BP_NONE;
		printf("clear break point %s - #%d\n", "FDC", number + 1);
	    }
	} else {
	    switch (action) {
	    case ARG_READ:
		break_point_fdc[number].type = BP_READ;
		s = "READ";
		break;
	    case ARG_WRITE:
		break_point_fdc[number].type = BP_WRITE;
		s = "WRITE";
		break;
	    case ARG_DIAG:
		break_point_fdc[number].type = BP_DIAG;
		s = "DIAG";
		break;
	    }
	    break_point_fdc[number].drive = drive;
	    break_point_fdc[number].track = track;
	    break_point_fdc[number].sector = sector;
	    printf("set break point - #%d [ %s : D:%d T:%d ",
		   number + 1, s, drive, track);
	    if (sector >= 0) printf("S:%d ", sector);
	    printf("]\n");
	}
    }

    pc88fdc_break_point();
}

/*----------------------------------------------------------------------*/
/* textscr								*/
/*	テキスト画面をコンソールに表示					*/
/*				この機能は peach氏により実装されました	*/
/*----------------------------------------------------------------------*/
static	void	print_text_screen(void)
{
    /*FILE *fp;*/
    int i, j;
    int line;
    int width;
    int end;
    Uchar text_buf[82];		/* 80文字 + '\n' + '\0' */
    
    if (grph_ctrl & 0x20) line = 25;
    else                  line = 20;

    if (sys_ctrl & 0x01) width = 80;
    else                 width = 40;

    for (i = 0, end = 0; i < line; i++) {
	for (j = 0; j < width; j++) {
	    if (width == 80) {
		text_buf[j] =
		    text_attr_buf[ text_attr_flipflop ][i * 80 + j] >> 8;
	    } else {
		text_buf[j] =
		    text_attr_buf[ text_attr_flipflop ][i * 80 + j * 2] >> 8;
	    }
	    if (text_buf[j] == 0) text_buf[j] = ' ';
	    else end = j;
	}
	/* 終端までの空白は入れない */
	text_buf[end + 1] = '\n';
	text_buf[end + 2] = '\0';
	print_hankaku(stdout, text_buf, alt_char);
    }
}

static	void	monitor_textscr(void)
{
    if (exist_argv()) {					/* <char> */
	alt_char = argv.str[0];
	shift();
    }

    print_text_screen();
}

/*--------------------------------------------------------------*/
/* loadbas <filename> [<type>]					*/
/*	BASIC LIST を読み込む					*/
/*--------------------------------------------------------------*/
#if 1					/* experimental by peach */
static	void	monitor_loadbas(void)
{
    char *filename;
    int size;
    int type = ARG_ASCII;
    FILE *fp;

    if (! exist_argv()) error();			/* <filename> */
    filename = argv.str;
    shift();

    if (exist_argv()) {					/* <type> */
	if (! argv_is(ARGV_BASIC)) error();
	type = argv.val;
	shift();
    }

    if (exist_argv()) error();

    if ((fp = fopen(filename, "r"))) {
	if (type == ARG_ASCII) {
	    size = basic_encode_list(fp);
	} else {
	    size = basic_load_intermediate_code(fp);	    
	}
	fclose(fp);
	if (size > 0) printf("Load [%s] (size %d)\n", filename, size);
    } else {
	printf("file [%s] can't open\n",filename);
    }
}
#else
static void monitor_loadbas(void) { printf("sorry, not support\n"); }
#endif
/*--------------------------------------------------------------*/
/* savebas [<filename> [<type>]]				*/
/*	BASIC LIST を出力					*/
/*--------------------------------------------------------------*/
#if 1					/* experimental by peach */
static	void	monitor_savebas(void)
{
    char *filename;
    int size;
    int type = ARG_ASCII;
    FILE *fp;

    if (exist_argv()) {				/* <filename> */
	filename = argv.str;
	shift();
	if (exist_argv()) {			/* <type> */
	    if (! argv_is(ARGV_BASIC)) error();
	    type = argv.val;
	    shift();
	}
    } else {
	filename = NULL;
    }

    if (exist_argv()) error();

    if (filename == NULL) {
	basic_decode_list(stdout);
    } else if ((fp = fopen(filename, "w"))) {
	if (type == ARG_ASCII) {
	    size = basic_decode_list(fp);
	} else {
	    size = basic_save_intermediate_code(fp);
	}
	fclose(fp);
	if (size > 0) printf("Save [%s] (size %d)\n", filename, size);
    } else {
	printf("file [%s] can't open\n",filename);
    }
}
#else
static void monitor_savebas(void) { printf("sorry, not support\n"); }
#endif

/*--------------------------------------------------------------*/
/* set [<variable> [<value>] ]					*/
/* show [<variable> ]						*/
/*	内部変数を表示／変更					*/
/*--------------------------------------------------------------*/
static	void	monitor_set_mem_printf(void)		/*** set mem ***/
{
    const char *r0000, *r6000, *r8000, *r8400, *rC000, *rF000;
    const char *w0000, *w6000, *w8000, *w8400, *wC000, *wF000;

    if (grph_ctrl & GRPH_CTRL_64RAM) {		/* 64KB RAM mode */
	r0000 =
	r6000 = "MAIN RAM";
    } else {					/* ROM/RAM mode */
	if (grph_ctrl & GRPH_CTRL_N) {			/* N BASIC */
	    r0000 =
	    r6000 = "N ROM";
	} else {					/*N88 BASIC*/
	    r0000 = "MAIN ROM";
	    if (ext_rom_bank & EXT_ROM_NOT) {		/* 通常ROM */
		r6000 = "MAIN ROM";
	    } else {					/* 拡張ROM */
		r6000 = "EXT ROM";	/*misc_ctrl & MISC_CTRL_EBANK*/
	    }
	}
    }
    w0000 =
    w6000 = "MAIN_RAM";
    if (ext_ram_ctrl & 0x01) {				/* 拡張RAM R可 */
	if (ext_ram_bank < use_extram * 4) {
	    r0000 =
	    r6000 = "EXT RAM"; 		/*ext_ram_bank*/
	}
    }
    if (ext_ram_ctrl & 0x10) {				/* 拡張RAM W可 */
	if (ext_ram_bank < use_extram * 4) {
	    w0000 =
	    w6000 = "EXT RAM";		/*ext_ram_bank*/
	}
    }

    if (grph_ctrl & (GRPH_CTRL_64RAM | GRPH_CTRL_N)) {
	r8000 =
	w8000 = "MAIN RAM";
    } else {
	r8000 =
	w8000 = "WINDOW";		/*window_offset*/
    }

    r8400 =
    w8400 = "MAIN RAM";

    if ((misc_ctrl & MISC_CTRL_EVRAM)   &&	/* 拡張アクセスモード */
	(ALU2_ctrl & ALU2_CTRL_VACCESS)) {	/* VRAM拡張アクセス */
	rC000 =
	rF000 =
	wC000 =
	wF000 = "VRAM Ext-Acc.";
    } else if (! (misc_ctrl & MISC_CTRL_EVRAM) &&	/* 独立アクセスモード*/
	       (memory_bank != MEMORY_BANK_MAIN)) {	/* メインBANKでない  */
	rC000 =
	rF000 =
	wC000 =
	wF000 = (memory_bank == 0) ? "VRAM B"
				   : ((memory_bank == 1) ?"VRAM R" :"VRAM G");
    } else {
	rC000 =
	wC000 = "MAIN RAM";
	if (high_mode &&
	    (misc_ctrl & MISC_CTRL_TEXT_MAIN) == 0) {		/*高速RAM*/
	    rF000 =
	    wF000 = "HIGH RAM";
	} else {
	    rF000 =
	    wF000 = "MAIN RAM";
	}
	if (jisho_rom_ctrl == FALSE) {				/*辞書ROM*/
	    rC000 =
	    rF000 = "JISHO ROM";	/*jisho_rom_bank*/
	}
    }

    printf("  Memory mapping\n");

    printf("    %-12s%-12s%-12s        ", "Addr.",   "Read","Write");
    printf("Bank-Status\n");

    printf("    %-12s%-12s%-12s        ", "0000-5FFF", r0000, w0000);
    printf("Ext. Ram Bank(0-%d)  = %d\n", use_extram * 4, ext_ram_bank);

    printf("    %-12s%-12s%-12s        ", "6000-7FFF", r6000, w6000);
    printf("Ext. Rom Bank(0-3)  = %d\n", misc_ctrl & MISC_CTRL_EBANK);

    printf("    %-12s%-12s%-12s        ", "8000-83FF", r8000, w8000);
    printf("Window offset       = %04XH\n", window_offset);

    printf("    %-12s%-12s%-12s        ", "8400-BFFF", r8400, w8400);
    printf("\n");

    printf("    %-12s%-12s%-12s        ", "C000-EFFF", rC000, wC000);
    printf("JishoRom Bank(0-31) = %d\n", jisho_rom_bank);

    printf("    %-12s%-12s%-12s        ", "F000-FFFF", rF000, wF000);
    printf("\n");
}
static	void	monitor_set_key_printf(void)		/*** set key ***/
{
    int j;
    printf("  %-23s %-15s","key_scan[0]-[15]","(IN:00..0F)");
    for (j=0; j<0x8; j++) printf("%02X ", key_scan[j]);
    printf("\n");
    printf("  %-23s %-15s","","");
    for (   ; j<0x10; j++) printf("%02X ", key_scan[j]);
    printf("\n");
}
static	void	monitor_set_palette_printf(void)	/*** set palette ***/
{
    int j;
    const char *now = "    [Pal-mode is   ]";
    const char *pal = (misc_ctrl & MISC_CTRL_ANALOG)
		    ? "    [      *Analog*]"
		    : "    [     *Digital*]";

    printf("  %-23s %-15sG:R:B = %01X:%01X:%01X\n",
	   "vram_bg_palette",
	   "(OUT:52/54)",
	   vram_bg_palette.green,
	   vram_bg_palette.red  ,
	   vram_bg_palette.blue);

    for (j=0; j<8; j++) {
	printf("  %-23s (OUT:%02X)       G:R:B = %01X:%01X:%01X\n",
	       (j == 0) ? "vram_palette"
			: ((j == 2) ? now
				    : ((j == 3) ? pal : "")),
	       j + 0x54,
	       vram_palette[j].green,
	       vram_palette[j].red  ,
	       vram_palette[j].blue);
    }
}
static	void	monitor_set_crtc_printf(void)		/*** set crtc ***/
{
    int j;
    const char *dmamode[] = { "(burst)", "(character)" };
    const char *blinktime[] = { "(x3)", "(x1.5)", "(x1)", "(x0.75)" };
    const char *skipline[] = { "(normal)", "(skip)" };
    const char *cursortype[] = { "(noblink, underline)", "(blink, underline)",
				 "(noblink, block)",     "(blink, block)" };
    const char *lineschar[] = { "(forbid)","(forbid)","","","","","",
			"(200line, 25row)","","(200line, 20row)","","","",
			"","","(400line, 25row)","","","","(400line, 20row)",
			"","","","","","","","","","","","",};
    const char *vwide[] = { "", "(400line, 20row)", "(400line, 25row)", "",
			    "", "(200line, 20row)", "(200line, 25row)", "" };
    const char *hwide[] = { "(forbid)", "(forbid)", "(forbid)", "(forbid)","",
			"","","","","","","","","","","","","","","","","","",
			"","(400line)","","","","","","(200line)","",};
    const char *attr[] = { "(mono, sepatare, o)","(no-attr, x)",
			"(color, sepatare, o)","(forbid)","(mono, mixed, o)",
			"(mono, mixed, x)","(forbid)","(forbid)" };

    printf("  CRTC & DMAC internal variable\n");
  
    printf("    %-38s%s\n",
	   "CRTC active",    (crtc_active) ? "Yes" : "No");
    printf("    %-38s%02x\n",
	   "Interrupt mask", crtc_intr_mask);
    printf("    %-38s%s\n",
	   "Reverse",        (crtc_reverse_display) ? "Yes" : "No");
    printf("    %-38s%s\n",
	   "Line Skip",      (crtc_skip_line) ? "Yes" : "No");
    printf("    %-38s%02XH(%d) , %02XH(%d)\n",
	   "Cursor position[X,Y]",
	   crtc_cursor[0], crtc_cursor[0], crtc_cursor[1], crtc_cursor[1]);


    printf("    Format[0] %02XH  ", crtc_format[0]);
    j = crtc_format[0] >> 7;
    printf("C-------:DMA mode      %2d %s\n", j, dmamode[ j ]);
    printf("                   ");
    j = (crtc_format[0] & 0x7f) + 2;
    printf("-HHHHHHH:chars/line    %2d\n", j);

    printf("    Format[1] %02XH  ", crtc_format[1]);
    j = crtc_format[1] >> 6;
    printf("BB------:blink time    %2d %s\n", j, blinktime[ j ]);
    printf("                   ");
    j = (crtc_format[1] & 0x3f) + 1;
    printf("--LLLLLL:lines/screen  %2d\n", j);

    printf("    Format[2] %02XH  ", crtc_format[2]);
    j = crtc_format[2] >> 7;
    printf("S-------:skip line     %2d %s\n", j, skipline[ j ]);
    printf("                   ");
    j = (crtc_format[2] >> 5) & 0x03;
    printf("-CC-----:cursor type   %2d %s\n", j, cursortype[ j ]);
    printf("                   ");
    j = (crtc_format[2] & 0x1f) + 1;
    printf("---RRRRR:lines/char    %2d\n", j);

    printf("    Format[3] %02XH  ", crtc_format[3]);
    j = (crtc_format[3] >> 5);
    printf("VVV-----:v wide(line)  %2d %s\n", j+1, vwide[ j ]);
    printf("                   ");
    j = (crtc_format[3] & 0x1f);
    printf("---ZZZZZ:h wide(char)  %2d %s\n", j+2, hwide[ j ]);

    printf("    Format[4] %02XH  ", crtc_format[4]);
    j = (crtc_format[4] >> 5);
    printf("TTT-----:attr type     %2d %s\n", j, attr[ j ]);
    printf("                   ");
    j = (crtc_format[4] & 0x1f) + 1;
    printf("---AAAAA:attr size     %2d\n", j);

    for (j=0; j<4; j++) {
	printf("  %-23s Ch.%d%-11s %04XH .. +%04XH, %s\n",
	       (j==0)?"  DMAC addr/cntr/mode":"", j,"",
	       dmac_address[j].W, dmac_counter[j].W & 0x3fff,
	       ((dmac_counter[j].W & 0xc000) == 0x0000)   ? "Verify":
	       (((dmac_counter[j].W & 0xc000) == 0x4000)  ? "Write":
		(((dmac_counter[j].W & 0xc000) == 0x8000) ? "Read":"BAD"))
	       );
    }
    printf("  %-23s %-15s %02X (%X%X%X%X%X%X%X%X)\n",
	   "", "Mode",
	   dmac_mode,
	   (dmac_mode >> 7) & 0x01, (dmac_mode >> 6) & 0x01,
	   (dmac_mode >> 5) & 0x01, (dmac_mode >> 4) & 0x01,
	   (dmac_mode >> 3) & 0x01, (dmac_mode >> 2) & 0x01,
	   (dmac_mode >> 1) & 0x01, (dmac_mode >> 0) & 0x01
	   );
    printf("  %-23s %-15s %s\n",
	   "  text_display",    "",
	   (text_display == TEXT_DISABLE)  ? "TEXT_DISABLE"
	   :((text_display == TEXT_ENABLE) ? "TEXT_ENABLE"
					   :"TEXT_ATTR_ONLY"));
}
static	void	monitor_set_pio_printf(void)		/*** set pio ***/
{
    printf(
     "  pio_AB[0][0].type     %s    ___    ___  %s   pio_AB[1][0].type\n",
	 (pio_AB[0][0].type == PIO_READ) ? "R" : "W",
	 (pio_AB[1][0].type == PIO_READ) ? "R" : "W");

    printf(
     "              .exist    %s       \\  /     %s               .exist\n",
	 (pio_AB[0][0].exist == PIO_EXIST) ? "*" : "-",
	 (pio_AB[1][0].exist == PIO_EXIST) ? "*" : "-");

    printf(
     "              .data     %02XH      \\/      %02XH             .data\n",
	 (pio_AB[0][0].data),
	 (pio_AB[1][0].data));

    printf("                                 /\\\n");

    printf(
     "  pio_AB[0][1].type     %s    ___/  \\___  %s   pio_AB[1][1].type\n",
	 (pio_AB[0][1].type == PIO_READ) ? "R" : "W",
	 (pio_AB[1][1].type == PIO_READ) ? "R" : "W");

    printf(
     "              .exist    %s                %s               .exist\n",
	 (pio_AB[0][1].exist == PIO_EXIST) ? "*" : "-",
	 (pio_AB[1][1].exist == PIO_EXIST) ? "*" : "-");

    printf(
     "              .data     %02XH              %02XH             .data\n",
	 (pio_AB[0][1].data),
	 (pio_AB[1][1].data));

    printf(
     "                                                               \n");

    printf("  \n");

    printf(
     "  pio_C[0][0].type      %s    ___    ___  %s   pio_C[1][0].type\n",
	 (pio_C[0][0].type == PIO_READ) ? "R" : "W",
	 (pio_C[1][0].type == PIO_READ) ? "R" : "W");

    printf(
     "             .data      %02XH     \\  /     %02XH            .data\n",
	 (pio_C[0][0].data),
	 (pio_C[1][0].data));

    printf(
     "             .cont_f    % 2d       \\/      % 2d             .cont_f\n",
	 (pio_C[0][0].cont_f),
	 (pio_C[1][0].cont_f));

    printf("                                 /\\\n");

    printf(
     "  pio_C[0][1].type      %s    ___/  \\___  %s   pio_C[1][1].type\n",
	 (pio_C[0][1].type == PIO_READ) ? "R" : "W",
	 (pio_C[1][1].type == PIO_READ) ? "R" : "W");

    printf(
     "             .data      %02XH              %02XH            .data\n",
	 (pio_C[0][1].data),
	 (pio_C[1][1].data));

    printf(
     "             .cont_f    % 2d               % 2d             .cont_f\n",
	 (pio_C[0][1].cont_f),
	 (pio_C[1][1].cont_f));
}


#ifdef	USE_SOUND
static	void	monitor_set_volume_printf(int index)
{
    if (xmame_has_mastervolume()) {
	printf("  %-23s %-15s %d\n",
	       monitor_variable[index].var_name,
	       monitor_variable[index].port_mes,
	       xmame_cfg_get_mastervolume());
    } else {
	printf("  %-23s %-15s --\n",
	       monitor_variable[index].var_name, "");
    }
}
static	void	monitor_set_fmmixer_printf(int index)
{
    printf("  %-23s %-15s %d\n",
	   monitor_variable[index].var_name,
	   monitor_variable[index].port_mes,
	   xmame_cfg_get_mixer_volume(XMAME_MIXER_FM));
}
static	void	monitor_set_psgmixer_printf(int index)
{
    printf("  %-23s %-15s %d\n",
	   monitor_variable[index].var_name,
	   monitor_variable[index].port_mes,
	   xmame_cfg_get_mixer_volume(XMAME_MIXER_PSG));
}
static	void	monitor_set_beepmixer_printf(int index)
{
    printf("  %-23s %-15s %d\n",
	   monitor_variable[index].var_name,
	   monitor_variable[index].port_mes,
	   xmame_cfg_get_mixer_volume(XMAME_MIXER_BEEP));
}
static	void	monitor_set_rhythmmixer_printf(int index)
{
    printf("  %-23s %-15s %d\n",
	   monitor_variable[index].var_name,
	   monitor_variable[index].port_mes,
	   xmame_cfg_get_mixer_volume(XMAME_MIXER_RHYTHM));
}
static	void	monitor_set_adpcmmixer_printf(int index)
{
    printf("  %-23s %-15s %d\n",
	   monitor_variable[index].var_name,
	   monitor_variable[index].port_mes,
	   xmame_cfg_get_mixer_volume(XMAME_MIXER_ADPCM));
}
#ifdef	USE_FMGEN
static	void	monitor_set_fmgenmixer_printf(int index)
{
    printf("  %-23s %-15s %d\n",
	   monitor_variable[index].var_name,
	   monitor_variable[index].port_mes,
	   xmame_cfg_get_mixer_volume(XMAME_MIXER_FMGEN));
}
#endif
static	void	monitor_set_samplemixer_printf(int index)
{
    printf("  %-23s %-15s %d\n",
	   monitor_variable[index].var_name,
	   monitor_variable[index].port_mes,
	   xmame_cfg_get_mixer_volume(XMAME_MIXER_SAMPLE));
}
static	void	monitor_set_mixer_printf(void)
{
    if (xmame_has_sound()) {
	printf("\n  Following is mixing level of xmame-sound-driver.\n");
	xmame_cfg_set_mixer_volume(-1, -1);
    }
}

static	void	monitor_set_volume(int vol)
{
    xmame_cfg_set_mastervolume(vol);
}
static	void	monitor_set_fmmixer(int vol)
{
    xmame_cfg_set_mixer_volume(XMAME_MIXER_FM, vol);
}
static	void	monitor_set_psgmixer(int vol)
{
    xmame_cfg_set_mixer_volume(XMAME_MIXER_PSG, vol);
}
static	void	monitor_set_beepmixer(int vol)
{
    xmame_cfg_set_mixer_volume(XMAME_MIXER_BEEP, vol);
}
static	void	monitor_set_rhythmmixer(int vol)
{
    xmame_cfg_set_mixer_volume(XMAME_MIXER_RHYTHM, vol);
}
static	void	monitor_set_adpcmmixer(int vol)
{
    xmame_cfg_set_mixer_volume(XMAME_MIXER_ADPCM, vol);
}
#ifdef	USE_FMGEN
static	void	monitor_set_fmgenmixer(int vol)
{
    xmame_cfg_set_mixer_volume(XMAME_MIXER_FMGEN, vol);
}
#endif
static	void	monitor_set_samplemixer(int vol)
{
    xmame_cfg_set_mixer_volume(XMAME_MIXER_SAMPLE, vol);
}
#endif	/* USE_SOUND */

static	void	monitor_set_show_printf(int index)	/*** set (print) ***/
{
    int val;

    switch (monitor_variable[index].var_type) {

    case MTYPE_INT:
    case MTYPE_INT_C:
    case MTYPE_FONT:
    case MTYPE_FRAMESKIP:
    case MTYPE_INTERLACE:
    case MTYPE_INTERP:
    case MTYPE_BEEP:
	val = *((int *)monitor_variable[index].var_ptr);
	goto MTYPE_numeric_variable;

    case MTYPE_BYTE:
    case MTYPE_BYTE_C:
	val = *((byte *)monitor_variable[index].var_ptr);
	goto MTYPE_numeric_variable;

    case MTYPE_WORD:
    case MTYPE_WORD_C:
	val = *((word *)monitor_variable[index].var_ptr);
	goto MTYPE_numeric_variable;

    MTYPE_numeric_variable:;
	printf("  %-23s %-15s %08XH (%d)\n",
	       monitor_variable[index].var_name,
	       monitor_variable[index].port_mes, val, val);
	break;

    case MTYPE_DOUBLE:
    case MTYPE_DOUBLE_C:
    case MTYPE_CLOCK:
	printf("  %-23s %-15s %8.4f\n",
	       monitor_variable[index].var_name,
	       monitor_variable[index].port_mes,
	       *((double *)monitor_variable[index].var_ptr));
	break;

    case MTYPE_MEM:
	monitor_set_mem_printf();
	break;
    case MTYPE_KEY:
	monitor_set_key_printf();
	break;
    case MTYPE_PALETTE:
	monitor_set_palette_printf();
	break;
    case MTYPE_CRTC:
	monitor_set_crtc_printf();
	break;
    case MTYPE_PIO:
	monitor_set_pio_printf();
	break;

#ifdef	USE_SOUND
    case MTYPE_VOLUME:
	monitor_set_volume_printf(index);
	break;
    case MTYPE_FMMIXER:
	monitor_set_fmmixer_printf(index);
	break;
    case MTYPE_PSGMIXER:
	monitor_set_psgmixer_printf(index);
	break;
    case MTYPE_BEEPMIXER:
	monitor_set_beepmixer_printf(index);
	break;
    case MTYPE_RHYTHMMIXER:
	monitor_set_rhythmmixer_printf(index);
	break;
    case MTYPE_ADPCMMIXER:
	monitor_set_adpcmmixer_printf(index);
	break;
#ifdef	USE_FMGEN
    case MTYPE_FMGENMIXER:
	monitor_set_fmgenmixer_printf(index);
	break;
#endif
    case MTYPE_SAMPLEMIXER:
	monitor_set_samplemixer_printf(index);
	break;
    case MTYPE_MIXER:
	monitor_set_mixer_printf();
	break;
#endif

    case MTYPE_NEWLINE:
	printf("\n");
	break;
    }
}






static	void	monitor_set(void)
{
    void *var_ptr;
    int set_type, index=0, value=0, i, block = 0;
    double dvalue = 0.0;
    int key_flag = 0;

    set_type = 0;
    if (exist_argv()) {

	for (index = 0; index < COUNTOF(monitor_variable_block); index++) {
	    if (strcmp(argv.str, monitor_variable_block[index].block_name)==0)
		break;
	}
	if (index < COUNTOF(monitor_variable_block)) {
	    block = TRUE;
	    set_type = index;
	    shift();
	} else {

	    for (index = 0; index < COUNTOF(monitor_variable); index++) {
		if (strcmp(argv.str, monitor_variable[index].var_name) == 0)
		    break;
	    }
	    if (index == COUNTOF(monitor_variable)) error();
	    shift();
	    set_type = 1;
	    if (exist_argv()) {
		if        (argv_is(ARGV_INT)) {
		    value  = argv.val;
		    dvalue = (double)argv.val;
		} else if (argv_is(ARGV_STR)) {
		    if (monitor_variable[index].var_type != MTYPE_KEY) {
			char *chk;
			dvalue = strtod(argv.str, &chk);
			if (*chk != '\0') error();
		    } else {
			if (argv.str[0] != '~') {
			    error();
			}
			key_flag = 1;
		    }
		} else {
		    error();
		}
		shift();
		set_type = 2;
	    }
	}
    }

    if (exist_argv()) error();


	/*================*/

    if (block) {
	index = 0;
	block = 0;
	for ( ; index < COUNTOF(monitor_variable); index++) {
	    if (monitor_variable_block[set_type].start <= block &&
		block <= monitor_variable_block[set_type].end) {
		monitor_set_show_printf(index);
	    }
	    if (monitor_variable[index].var_type == MTYPE_NEWLINE) {
		block ++;
	    }
	}
	return;
    }

    switch (set_type) {
    case 0:
	for (index = 0; index < COUNTOF(monitor_variable); index++) {
	    monitor_set_show_printf(index);
	}
	break;

    case 1:
	monitor_set_show_printf(index);
	break;

    case 2:
	var_ptr = monitor_variable[index].var_ptr;
	switch (monitor_variable[index].var_type) {

	case MTYPE_INT_C:
	case MTYPE_BYTE_C:
	case MTYPE_WORD_C:
	case MTYPE_DOUBLE_C:
	    printf("Sorry! This variable can't set value. \n");
	    break;

	case MTYPE_INT: 	*((int *)var_ptr)  = value;	break;
	case MTYPE_BYTE:	*((byte *)var_ptr) = value;	break;
	case MTYPE_WORD:	*((word *)var_ptr) = value;	break;
	case MTYPE_DOUBLE:	*((double *)var_ptr) = dvalue;	break;

	case MTYPE_FONT:
	    *((int *)var_ptr) = value;
	    memory_set_font();
	    screen_update_immidiate();
	    break;
	case MTYPE_FRAMESKIP:
	    quasi88_cfg_set_frameskip_rate(value);
	    break;
	case MTYPE_INTERLACE:
	    quasi88_cfg_set_interlace(value);
	    screen_update_immidiate();
	    break;
	case MTYPE_INTERP:
	    quasi88_cfg_set_interp(value);
	    screen_update_immidiate();
	    break;

	case MTYPE_CLOCK:
	    *((double *)var_ptr) = dvalue;
	    interval_work_init_all();
	    break;

	case MTYPE_BEEP:
	    *((int *)var_ptr) = value;
#ifdef	USE_SOUND
	    xmame_dev_beep_cmd_sing((byte) use_cmdsing);
#endif
	    break;


	case MTYPE_KEY:
	    if (key_flag == 0) {
		for (i = 0; i < 0x10; i++) key_scan[i] = value;
		printf("     key_scan[0..15] = %d\n",value);
	    } else {
		char c_new[16];
		char *p = &argv.str[1];
		int i, j, x;
		for (i = 0; i < (int)sizeof(c_new) && *p; i++) {
		    for (j = 0; j < 2; j++) {
			x = -1;
			if      ('0' <= *p && *p <= '9') x = *p - '0';
			else if ('a' <= *p && *p <= 'f') x = *p - 'a' + 10;
			else if ('A' <= *p && *p <= 'F') x = *p - 'A' + 10;
			p++;
			if ((*p == '\0' && j == 0) || x < 0) {
			    goto set_key_break;
			}
			if (j == 0) c_new[i] = x * 16;
			else        c_new[i] = (c_new[i] + x);
		    }
		}
		if (*p) goto set_key_break;

		for (j = 0; j < i; j++) {
		    key_scan[j] = ~c_new[j];
		}
		monitor_set_show_printf(index);
	    }
	    break;

	set_key_break:
	    printf("Invalid parameter %s.\n", argv.str);
	    break;

	case MTYPE_MEM:
	case MTYPE_PALETTE:
	case MTYPE_CRTC:
	case MTYPE_PIO:
	    printf("Sorry! This variable can't set value. "
		   "(palette,crtc,pio, and so on)\n");
	    break;

#ifdef	USE_SOUND
	case MTYPE_VOLUME:
	    monitor_set_volume(value);
	    break;
	case MTYPE_FMMIXER:
	    monitor_set_fmmixer(value);
	    break;
	case MTYPE_PSGMIXER:
	    monitor_set_psgmixer(value);
	    break;
	case MTYPE_BEEPMIXER:
	    monitor_set_beepmixer(value);
	    break;
	case MTYPE_RHYTHMMIXER:
	    monitor_set_rhythmmixer(value);
	    break;
	case MTYPE_ADPCMMIXER:
	    monitor_set_adpcmmixer(value);
	    break;
#ifdef	USE_FMGEN
	case MTYPE_FMGENMIXER:
	    monitor_set_fmgenmixer(value);
	    break;
#endif
	case MTYPE_SAMPLEMIXER:
	    monitor_set_samplemixer(value);
	    break;
	case MTYPE_MIXER:
	    break;
#endif

	}
	break;

    }
}

static	void	monitor_show(void)
{
    int set_type, index=0;

    set_type = 0;
    if (exist_argv()) {
	for (index = 0; index < COUNTOF(monitor_variable); index++) {
	    if (strcmp(argv.str, monitor_variable[index].var_name) == 0) break;
	}
	if (index == COUNTOF(monitor_variable)) error();
	shift();
	set_type = 1;
    }

    if (exist_argv()) error();


    switch (set_type) {
    case 0:
	for (index = 0; index < COUNTOF(monitor_variable); index++) {
	    monitor_set_show_printf(index);
	}
	break;

    case 1:
	monitor_set_show_printf(index);
	break;
    }
}





/*--------------------------------------------------------------*/
/* resize <screen_size>						*/
/*	画面サイズを変更					*/
/*--------------------------------------------------------------*/
static	void	monitor_resize(void)
{
    int command = -1;

    if (exist_argv()) {
	if (! argv_is(ARGV_RESIZE)) error();
	command = argv.val;
	shift();
    }

    if (exist_argv()) error();


	/*================*/

    switch (command) {
    case -1:					/* resize */
	quasi88_cfg_set_size_large();
	break;
    case ARG_FULL:				/* resize full */
	quasi88_cfg_set_size(SCREEN_SIZE_FULL);
	break;
    case ARG_HALF:				/* resize half */
	quasi88_cfg_set_size(SCREEN_SIZE_HALF);
	break;
    case ARG_DOUBLE:				/* resize double */
#ifdef	SUPPORT_DOUBLE
	quasi88_cfg_set_size(SCREEN_SIZE_DOUBLE);
#endif
	break;
    case ARG_FULLSCREEN:			/* resize fullscreen */
	if (quasi88_cfg_can_fullscreen()) {
	    quasi88_cfg_set_fullscreen(TRUE);
	}
	break;
    case ARG_WINDOW:				/* resize window */
	if (quasi88_cfg_now_fullscreen()) {
	    quasi88_cfg_set_fullscreen(FALSE);
	}
	break;
    }

    screen_update_immidiate();

    return;
}





/*--------------------------------------------------------------*/
/* drive [show]							*/
/* drive eject [<drive_no>]					*/
/* drive empty <drive_no>					*/
/* drive set <drive_no> <filename >				*/
/*	ドライブ関連処理					*/
/*		ドライブに設定されたファイルの状態を見る	*/
/*		ドライブを空にする				*/
/*		ドライブを一時的に空にする			*/
/*		ドライブにファイルをセット(交換)		*/
/*--------------------------------------------------------------*/
static	void	monitor_drive(void)
{
    int i, j, command, drv = -1, img = 0;
    char *filename = NULL;

    if (! exist_argv()) {
	command = ARG_SHOW;
    } else {

	if (! argv_is(ARGV_DRIVE)) error();		/* <command> */
	command = argv.val;
	shift();

	switch (command) {
	case ARG_SHOW:					/*   show */
	    break;
	case ARG_EJECT:					/*   eject */
	    if (exist_argv()) {
		if (! argv_is(ARGV_DRV)) error();	/*	[<drive_no>] */
		drv = argv.val-1;
		shift();
	    }
	    break;
	case ARG_EMPTY:					/*   empty */
	    if (! argv_is(ARGV_DRV)) error();		/*	<drive_no> */
	    drv = argv.val-1;
	    shift();
	    break;
	case ARG_SET:					/*   set */
	    if (! argv_is(ARGV_DRV)) error();		/*	<drive_no> */
	    drv = argv.val-1;
	    shift();
	    if (! exist_argv()) error();		/*	<filename> */
	    filename = argv.str;
	    shift();
	    if (exist_argv()) {
		if (! argv_is(ARGV_IMG)) error();	/*	[<image_no>] */
		img = argv.val-1;
		shift();
	    }
	    break;
	}

    }
    if (exist_argv()) error();


	/*================*/

    switch (command) {
    case ARG_SHOW:				/* drive show */
	for (i = 0; i < NR_DRIVE; i++) {
	    printf("DRIVE %d: lamp[ %s ]%s\n",
		   i + 1, get_drive_ready(i) ? " " : "#",
		   (drive[i].fp && drive[i].empty) ? "  ..... *** Empty ***"
						   : "");
	    if (disk_image_exist(i)) {
	      /*printf("    filename = %s\n", drive[i].filename);*/
		printf("    filename = %s\n", filename_get_disk(i));
		printf("    FILE *fp = %p : read only? = %s\n",
		       (void *)drive[i].fp,
		       (drive[i].read_only) ? "Read Only" : "Read Write");
		printf("    Selected image No = %d/%d%s : protect = %s : media = %s\n",
		       drive[i].selected_image + 1,
		       drive[i].image_nr,
		       (drive[i].detect_broken_image) ? " + broken"
			     : ((drive[i].over_image) ? " + alpha "
						      : "         "),
		       (drive[i].protect == DISK_PROTECT_TRUE) ? "RO" : "RW",
		       (drive[i].type == DISK_TYPE_2HD)    ? "2HD"
		       : ((drive[i].type == DISK_TYPE_2DD) ? "2DD"
							   : "2D"));
		printf("    ------------------------------------------\n");
		for (j = 0; j < drive[i].image_nr; j++) {
		    printf("    %s% 3d %-17s  %s  %s  %ld\n",
			   (j == drive[i].selected_image) ? "*" : " ",
			   j + 1,
			   drive[i].image[j].name,
			   (drive[i].image[j].protect
						== DISK_PROTECT_TRUE)  ? "RO"
			   : ((drive[i].image[j].protect
						== DISK_PROTECT_FALSE) ? "RW"
								       : "??"),
			   (drive[i].image[j].type
						== DISK_TYPE_2D)  ? "2D "
			   : ((drive[i].image[j].type
						== DISK_TYPE_2DD) ? "2DD"
			     : ((drive[i].image[j].type
						== DISK_TYPE_2HD) ? "2HD"
								  : "???")),
			   drive[i].image[j].size);
		}
	    }
	    printf("\n");
	}
	break;
    case ARG_EMPTY:				/* drive empty n */
	if (disk_image_exist(drv)) {
	    drive_change_empty(drv);
	    if (drive_check_empty(drv))
		printf("** Set DRIVE %d: Empty **\n", drv + 1);
	    else
		printf("** Unset DRIVE %d: Empty **\n", drv + 1);
	}
	break;
    case ARG_EJECT:
	switch (drv) {
	case 0:					/* drive eject 0 */
	case 1:					/* drive eject 1 */
	    if (disk_image_exist(drv)) {
		quasi88_disk_eject(drv);
		printf("-- Disk Eject from DRIVE %d: --\n", drv + 1);
	    }
	    break;
	default:				/* drive eject	*/
	    for (i = 0; i < 2; i++) {
		if (disk_image_exist(i)) {
		    quasi88_disk_eject(i);
		    printf("-- Disk Eject from DRIVE %d: --\n", i + 1);
		}
	    }
	    break;
	}
	break;
    case ARG_SET:				/* drive set x yyy z */
	if (strcmp(filename, "-") == 0) {
	    switch (disk_change_image(drv, img)) {
	    case 0:					/* OK */
		printf("== Image change in DRIVE %d: ==\n", drv + 1);
		printf("   image number ->[%d]\n", img + 1);
		break;
	    case 1:					/* no disk */
		printf("** Disk not exist in DRIVE:%d **\n", drv + 1);
		break;
	    case 2:					/* no image */
		printf("** Image number %d is not exist in DRIVE:%d **\n",
		       img + 1, drv + 1);
		break;
	    }
	} else {
	    if (disk_image_exist(drv)) {
		quasi88_disk_eject(drv);
		printf("-- Disk Eject from DRIVE %d: --\n", drv + 1);
	    }
	    if (quasi88_disk_insert(drv, filename, img, FALSE)) { /* Success */
		printf("== Disk insert to DRIVE %d: ==\n", drv + 1);
		printf("   file ->[%s] image number ->[%d]\n",
		       filename, img + 1);
	    } else {						  /* Failed */
		printf("** Disk %s can't insert **\n", filename);
	    }
	}
	break;
    }


    return;
}



/*--------------------------------------------------------------*/
/* file show <filename>						*/
/* file create <filename>					*/
/* file protect <filename> <image_no>				*/
/* file unprotect <filename> <image_no>				*/
/* file format <filename> <image_no>				*/
/* file unformat <filename> <image_no>				*/
/* file rename <filename> <image_no> <image_name>		*/
/*	ファイル関連処理					*/
/*		ファイルを見る					*/
/*		ブランクイメージをファイルに追加(作成)		*/
/*		イメージのライトプロテクトを設定		*/
/*		イメージのライトプロテクトを解除		*/
/*		イメージをフォーマット				*/
/*		イメージをアンフォーマット			*/
/*		イメージ名を変更				*/
/*--------------------------------------------------------------*/
static	void	monitor_file(void)
{
    int command, num, drv, img = 0, result = -1, ro = FALSE;
    char *filename, *imagename = NULL;
    long offset;
    OSD_FILE *fp;
    Uchar c[32];
    char *s = NULL;



    if (! argv_is(ARGV_FILE)) error();			/* <command> */
    command = argv.val;
    shift();

    if (! exist_argv()) error();			/* <filename> */
    filename = argv.str;
    shift();

    switch (command) {
    case ARG_SHOW:					/*   show      */
    case ARG_CREATE:					/*   create    */
	break;
    case ARG_PROTECT:					/*   protect   */
    case ARG_UNPROTECT:					/*   unprotect */
    case ARG_FORMAT:					/*   format    */
    case ARG_UNFORMAT:					/*   unformat  */
	if (! argv_is(ARGV_IMG)) error();		/*	<image_no> */
	img = argv.val - 1;
	shift();
	break;
    case ARG_RENAME:					/*   rename    */
	if (! argv_is(ARGV_IMG)) error();		/*	<image_no> */
	img = argv.val - 1;
	shift();
	if (! exist_argv()) error();
	imagename = argv.str;				/*	<imagename> */
	shift();
	break;
    }

    if (exist_argv()) error();


	/*================*/

    switch (command) {
    case ARG_SHOW:
	if ((fp = osd_fopen(FTYPE_DISK, filename, "rb")) == NULL) {
	    if ((fp = osd_fopen(FTYPE_DISK, filename, "r+b")) == NULL) {
		printf("Open error! %s\n", filename);
		break;
	    }
	}

	offset = -1;
	if (osd_fseek(fp, 0,  SEEK_END) == 0) {
	    offset = osd_ftell(fp);
	}

	printf("filename = %s   size = %ld\n", filename, offset);
	printf("  -No------Name-----------R/W-Type---Size--\n");
	offset = 0;
	num = 0;
	while ((result = d88_read_header(fp, offset, c)) == 0) {
	    c[16] = '\0';
	    printf("  % 3d   %-17s  %s  %s  %ld\n",
		   num + 1,
		   c,
		   (c[DISK_PROTECT] == DISK_PROTECT_TRUE)     ? "RO"
		   : ((c[DISK_PROTECT] == DISK_PROTECT_FALSE) ? "RW"
							      : "??"),
		   (c[DISK_TYPE] == DISK_TYPE_2D)       ? "2D "
		   : ((c[DISK_TYPE] == DISK_TYPE_2DD)   ? "2DD"
		     : ((c[DISK_TYPE] == DISK_TYPE_2HD) ? "2HD"
							: "???")),
		   READ_SIZE_IN_HEADER(c));
	    offset += READ_SIZE_IN_HEADER(c);
	    num++;
	    if (num > 255) { result = -1; break; }
	}
	printf("\n");
	switch (result) {
	case -1:	   printf("Image number too many (over 255)\n");break;
	case D88_SUCCESS:						break;
	case D88_NO_IMAGE:						break;
	case D88_BAD_IMAGE:printf("Image No. %d is broken\n", num + 1);	break;
	case D88_ERR_SEEK: printf("Seek Error\n");			break;
	case D88_ERR_READ: printf("Read Error\n");			break;
	default:	   printf("Internal Error\n");
	}
	if (drive[0].fp != fp && drive[1].fp != fp) osd_fclose(fp);
	break;


    case ARG_CREATE:
    case ARG_PROTECT:
    case ARG_UNPROTECT:
    case ARG_FORMAT:
    case ARG_UNFORMAT:
    case ARG_RENAME:

				/* ファイルを開く */
	fp = osd_fopen(FTYPE_DISK, c, "r+b");		/* "r+b" でオープン */
	if (fp == NULL) {
	    fp = osd_fopen(FTYPE_DISK, c, "rb");	/* "rb" でオープン */
	    ro = TRUE;
	}

	if (fp) {					/* オープンできたら */
	    if      (fp == drive[ 0 ].fp) drv = 0;	/* すでにドライブに */
	    else if (fp == drive[ 1 ].fp) drv = 1;	/* 開いてないかを   */
	    else                          drv = -1;	/* チェックする     */
	}


	if (fp == NULL) {		/* オープン失敗 */
	    printf("Open error! %s\n", filename);
	    break;
	}
	else if (ro) {			/* リードオンリーなので処理不可 */
	    if (drv < 0) osd_fclose(fp);
	    printf("File %s is read only", filename);
	    if (drv < 0) printf("\n");
	    else         printf("(in DRIVE %d:)\n", drv + 1);
	    break;
	}
	else if (drv >= 0 &&		/* 壊れたイメージが含まれるので不可 */
		 drive[ drv ].detect_broken_image) {
	    printf("Warning! File %s maybe be broken!"
		   " ..... continued, but not update drive status.\n",
		   filename);
	}


				/* コマンド別処理 */
	switch (command) {
	case ARG_CREATE:
	    result = d88_append_blank(fp, drv);
	    s = "Create blank image";
	    break;
	case ARG_PROTECT:
	    c[0] = DISK_PROTECT_TRUE;
	    result = d88_write_protect(fp, drv, img, (char*)c);
	    s = "Set Protect";
	    break;
	case ARG_UNPROTECT:
	    c[0] = DISK_PROTECT_FALSE;
	    result = d88_write_protect(fp, drv, img, (char*)c);
	    s = "Unset Protect";
	    break;
	case ARG_FORMAT:
	    result = d88_write_format(fp, drv, img);
	    s = "Format";
	    break;
	case ARG_UNFORMAT:
	    result = d88_write_unformat(fp, drv, img);
	    s = "Unformat";
	    break;
	case ARG_RENAME:
	    strncpy((char *)c, imagename, 17);
	    result = d88_write_name(fp, drv, img, (char*)c);
	    s = "Rename image";
	    break;
	}
				/* エラー表示 */
	switch (result) {
	case D88_SUCCESS:   printf("%s complete.\n", s);		 break;
	case D88_NO_IMAGE:  printf("Image No. %d not exist.\n", img + 1);break;
	case D88_BAD_IMAGE: printf("Image No. %d is broken.\n", img + 1);break;
	case D88_MANY_IMAGE:printf("Image number over\n");		 break;
	case D88_ERR_SEEK:  printf("Seek Error\n");			 break;
	case D88_ERR_WRITE: printf("Write error\n");			 break;
	case D88_ERR_READ:  printf("Read error\n");			 break;
	case D88_ERR:	    printf("Internal error\n");			 break;
	}

				/* 終了処理 */
	if (drv < 0) {
	    osd_fclose(fp);
	} else {
	    if (result != D88_SUCCESS) {
		printf("Fatal error in File %s ( in DRIVE %d: )\n",
		       filename, drv + 1);
		printf("File %s maybe be broken.\n", filename);
	    }
	}
	break;
    }

    return;
}



/*--------------------------------------------------------------*/
/* statesave [<filename>]					*/
/*	ステートセーブ						*/
/*--------------------------------------------------------------*/
static	void	monitor_statesave(void)
{
    char *filename = NULL;

    if (exist_argv()) {
	filename = argv.str;
	shift();
	if (exist_argv()) error();
    } else {
	filename = NULL;
    }

    if (filename) {
	if (strlen(filename) >= QUASI88_MAX_FILENAME) {
	    printf("filename too long\n");
	    return;
	}
	filename_set_state(filename);
    }

    if (quasi88_statesave(-1)) {
	printf("statesave succsess\n"
	       "  state-file = %s\n", filename_get_state());
    } else {
	printf("statesave failed\n");
    }
    return;
}



/*--------------------------------------------------------------*/
/* stateload [<filename>]					*/
/*	ステートロード						*/
/*--------------------------------------------------------------*/
static	void	monitor_stateload(void)
{
    char *filename = NULL;

    if (exist_argv()) {
	filename = argv.str;
	shift();
	if (exist_argv()) error();
    } else {
	filename = NULL;
    }

    if (filename) {
	if (strlen(filename) >= QUASI88_MAX_FILENAME) {
	    printf("filename too long\n");
	    return;
	}
	filename_set_state(filename);
    }

    if (stateload_check_file_exist() == FALSE) {
	printf("state-file \"%s\" not exist or broken\n",
	       filename_get_state());
    } else {
	if (quasi88_stateload(-1)) {
	    printf("stateload succsess\n"
		   "  state-file = %s\n", filename_get_state());
	    quasi88_exec();
	} else {
	    printf("stateload failed\n");
	}
    }
    return;
}



/*--------------------------------------------------------------*/
/* snapshot							*/
/*	スクリーンスナップショットの保存			*/
/*--------------------------------------------------------------*/
static	void	monitor_snapshot(void)
{
    int format = -1;

    if (exist_argv()) {
	if (! argv_is(ARGV_SNAPSHOT)) error();
	format = argv.val;
	shift();
	if (exist_argv()) error();
    } else {
	format = -1;
    }

    switch (format) {
    case ARG_BMP:	snapshot_format = 0;	break;
    case ARG_PPM:	snapshot_format = 1;	break;
    case ARG_RAW:	snapshot_format = 2;	break;
    }

    if (screen_snapshot_save() == 0) {
	printf("save-snapshot failed\n");
    }
    return;
}



/*--------------------------------------------------------------*/
/* loadfont <filename> <format> <type>				*/
/*	フォントファイルのロード				*/
/*--------------------------------------------------------------*/
static	void	monitor_loadfont(void)
{
    char *filename = NULL;
    int format = 0;
    int type   = 0;

    if (! exist_argv()) error();			/* <filename> */
    filename = argv.str;
    shift();

    if (! exist_argv() ||				/* <format> */
	! argv_is(ARGV_INT)) error();
    format = argv.val;
    if (! BETWEEN(0, format, 2)) error();
    shift();

    if (! exist_argv() ||				/* <type> */
	! argv_is(ARGV_INT)) error();
    type = argv.val;
    if (! BETWEEN(0, type, 3)) error();
    shift();

    if (exist_argv()) error();


    {
	static const int rev[] = { 0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
				   0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf };
	int c, i, j, k, l, n;
	int result;
	FILE *fp;
	bit8 *font;

	if      (type == 1) font = font_mem;
	else if (type == 2) font = font_mem2;
	else if (type == 3) font = font_mem3;
	else                font = font_pcg;

	fp = fopen(filename, (format == 0) ? "rb" : "r");

	if (fp) {

	    if (format == 0) {

		if (fread(font, sizeof(byte), 8*256, fp) != 8 * 256) {
		    printf("file [%s] read error\n", filename);
		}

	    } else {
						/* #define font_width 128 */
		result = fscanf(fp, "#define %*s %d\n", &c);
		if (result != 1 || c != 128) goto ERR;
						/* #define font_height 128 */
		result = fscanf(fp, "#define %*s %d\n", &c);
		if (result != 1 || c != 128) goto ERR;
				     /* static unsigned char font_bits[] = { */
		result = fscanf(fp, "%*[^]]%*[^=]%*[^{]{");
		if (result != 0) goto ERR;

		l = 0;
		for (k = 0; k < 16; k++) {
		    for (j = 0; j < 8; j++) {
			for (i = 0; i < 16; i++) {

			    result = fscanf(fp, "%i", &c);	/* 0xnn */
			    if (result != 1) goto ERR;

			    c = ((rev[ c & 0xf ]) << 4) | (rev[ c >> 4 ]);
			    if (format == 1) n = (k * 16 + i) * 8 + j;
			    else             n = (i * 16 + k) * 8 + j;
			    font[ n ] = c & 0xff;

			    if (l < 8 * 256) {
				result = fscanf(fp, "%*[^1234567890]"); /* , */
				if (result != 0) goto ERR;
			    }
			    l++;
			}
		    }
		}
	    }

	    fclose(fp);
	    screen_update_immidiate();

	} else {
	    printf("file [%s] can't open\n", filename);
	}

	return;

    ERR:
	printf("file [%s] read error\n", filename);
	fclose(fp);
	screen_update_immidiate();
    }

    return;
}



/*--------------------------------------------------------------*/
/* savefont <filename> <format> <type>				*/
/*	フォントファイルのセーブ				*/
/*--------------------------------------------------------------*/
static	void	monitor_savefont(void)
{
    char *filename = NULL;
    int format = 0;
    int type   = 0;

    if (! exist_argv()) error();			/* <filename> */
    filename = argv.str;
    shift();

    if (! exist_argv() ||				/* <format> */
	! argv_is(ARGV_INT)) error();
    format = argv.val;
    if (! BETWEEN(0, format, 2)) error();
    shift();

    if (! exist_argv() ||				/* <type> */
	! argv_is(ARGV_INT)) error();
    type = argv.val;
    if (! BETWEEN(0, type, 3)) error();
    shift();

    if (exist_argv()) error();


    {
	static const int rev[] = { 0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
				   0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf };
	int c, i, j, k, l, n;
	FILE *fp;
	bit8 *font;

	if      (type == 1) font = font_mem;
	else if (type == 2) font = font_mem2;
	else if (type == 3) font = font_mem3;
	else                font = font_pcg;

	fp = fopen(filename, (format == 0) ? "wb" : "w");

	if (fp) {

	    if (format == 0) {

		if (fwrite(font, sizeof(byte), 8 * 256, fp) != 8 * 256) {
		    printf("file [%s] write error\n", filename);
		}

	    } else {

		fprintf(fp, "#define font_width 128\n");
		fprintf(fp, "#define font_height 128\n");
		fprintf(fp, "static unsigned char font_bits[] = {\n");

		l = 0;
		for (k = 0; k < 16; k++) {
		    for (j = 0; j < 8; j++) {
			for (i = 0; i < 16; i++) {

			    if (format == 1) n = (k * 16 + i) * 8 + j;
			    else             n = (i * 16 + k) * 8 + j;
			    c = (font[ n ]) & 0xff;
			    c = ((rev[ c & 0xf ]) << 4) | (rev[ c >> 4 ]);

			    if ((l % 12) == 0) fprintf(fp, "  ");
			    fprintf(fp, " 0x%02x", c & 0xff);
			    if      (l == 8 * 256 - 1) fprintf(fp, "};\n");
			    else if ((l % 12) == 11)   fprintf(fp, ",\n");
			    else                       fprintf(fp, ",");
			    l++;
			}
		    }
		}

	    }

	    fclose(fp);

	} else {
	    printf("file [%s] can't open\n", filename);
	}
    }

    return;
}



/*--------------------------------------------------------------*/
/* tapeload [<filename>]					*/
/*	ロード用テープイメージファイルのセット			*/
/*--------------------------------------------------------------*/
static	void	monitor_tapeload(void)
{
    char *filename = NULL;

    if (exist_argv()) {
	filename = argv.str;
	shift();
	if (exist_argv()) error();
    } else {
	const char *filename = filename_get_tape(CLOAD);
	if (filename) 
	    printf("  Tape load image is -> %s\n", filename);
	else
	    printf("  Tape load image is NOT set\n");
	return;
    }

    if (strcmp(filename, "-") == 0) {
	quasi88_load_tape_eject();
	printf("  Tape load image is NOT set\n");
    } else {
	if (quasi88_load_tape_insert(filename)) {
	    printf("-- Tape set as load --\n");
	    printf("   file ->[%s] \n", filename);
	} else {
	    printf("** Tape %s can't set **\n", filename);
	}
    }
    return;
}



/*--------------------------------------------------------------*/
/* tapesave [<filename>]					*/
/*	セーブ用テープイメージファイルのセット			*/
/*--------------------------------------------------------------*/
static	void	monitor_tapesave(void)
{
    char *filename = NULL;

    if (exist_argv()) {
	filename = argv.str;
	shift();
	if (exist_argv()) error();
    } else {
	const char *filename = filename_get_tape(CSAVE);
	if (filename) 
	    printf("  Tape save image is -> %s\n", filename);
	else
	    printf("  Tape save image is NOT set\n");
	return;
    }

    if (strcmp(filename, "-") == 0) {
	quasi88_save_tape_eject();
	printf("  Tape save image is NOT set\n");
    } else {
	if (quasi88_save_tape_insert(filename)) {
	    printf("-- Tape set as save --\n");
	    printf("   file ->[%s] \n", filename);
	} else {
	    printf("** Tape %s can't set **\n", filename);
	}
    }
    return;
}



/*--------------------------------------------------------------*/
/* printer [<filename>]						*/
/*	プリントアウト用イメージファイルのセット		*/
/*--------------------------------------------------------------*/
static	void	monitor_printer(void)
{
    char *filename = NULL;

    if (exist_argv()) {
	filename = argv.str;
	shift();
	if (exist_argv()) error();
    } else {
	const char *filename = filename_get_prn();
	if (filename) printf("  Printout image is -> %s\n", filename);
	else          printf("  Printout image is NOT set\n");
	return;
    }

    if (strcmp(filename, "-") == 0) {
	quasi88_printer_remove();
	printf("  Printout image is NOT set\n");
    } else {
	if (quasi88_printer_connect(filename)) {
	    printf("-- Printout image set --\n");
	    printf("   file ->[%s] \n", filename);
	} else {
	    printf("** Printout image %s can't set **\n", filename);
	}
    }
    return;
}



/*--------------------------------------------------------------*/
/* serialin [<filename>]					*/
/*	シリアル入力用イメージファイルのセット			*/
/*--------------------------------------------------------------*/
static	void	monitor_serialin(void)
{
    char *filename = NULL;

    if (exist_argv()) {
	filename = argv.str;
	shift();
	if (exist_argv()) error();
    } else {

	char buf[16];
	long cur, end;
	const char *filename = filename_get_sin();
	buf[0] = '\0';
	if (filename) {
	    if (sio_com_pos(&cur, &end)) {
		if (end == 0) sprintf(buf, " (END)");
		else          sprintf(buf, " (%3ld%%)", cur * 100 / end);
	    }
	}

	if (filename) printf("  Serial-In image is -> %s%s\n", filename, buf);
	else          printf("  Serial-In image is NOT set\n");
	return;
    }

    if (strcmp(filename, "-") == 0) {
	quasi88_serial_in_remove();
	printf("  Serial-In image is NOT set\n");
    } else {
	if (quasi88_serial_in_connect(filename)) {
	    printf("-- Serial-In image set --\n");
	    printf("   file ->[%s] \n", filename);
	} else {
	    printf("** Serial-In image %s can't set **\n", filename);
	}
    }
    return;
}



/*--------------------------------------------------------------*/
/* serialout <filename>						*/
/*	シリアル出力用イメージファイルのセット			*/
/*--------------------------------------------------------------*/
static	void	monitor_serialout(void)
{
    char *filename = NULL;

    if (exist_argv()) {
	filename = argv.str;
	shift();
	if (exist_argv()) error();
    } else {
	const char *filename = filename_get_sout();
	if (filename) printf("  Serial-Out image is -> %s\n", filename);
	else          printf("  Serial-Out image is NOT set\n");
	return;
    }

    if (strcmp(filename, "-") == 0) {
	quasi88_serial_out_remove();
	printf("  Serial-Out image is NOT set\n");
    } else {
	if (quasi88_serial_out_connect(filename)) {
	    printf("-- Serial-Out image set --\n");
	    printf("   file ->[%s] \n", filename);
	} else {
	    printf("** Serial-Out image %s can't set **\n", filename);
	}
    }
    return;
}





/*----------------------------------*/
static	void    monitor_misc(void)
{
/*
  int ch;
  extern const char *mixer_get_name(int ch);
  extern mixer_get_mixing_level(int ch);
    for( ch=0; ch<16 ; ch++ ){
      const char *name = mixer_get_name(ch);
      if(name) printf( "%d[ch] %s\t:%d\n", ch,name,mixer_get_mixing_level(ch));
    }
*/
#if 0
  FILE *fp;
  fp = fopen( "log.main","wb");
  fwrite( main_ram,            sizeof(byte),  0x0f000,  fp );
  fwrite( main_high_ram,       sizeof(byte),  0x01000,  fp );
  fclose(fp);

  fp = fopen( "log.high","wb");
  fwrite( &main_ram[0xf000],  sizeof(byte),   0x1000,  fp );
  fclose(fp);

  fp = fopen( "log.sub","wb");
  fwrite( &sub_romram[0x4000], sizeof(byte),   0x4000,  fp );
  fclose(fp);

  fp = fopen( "log.vram","wb");
  fwrite( main_vram,           sizeof(byte), 4*0x4000,  fp );
  fclose(fp);
#endif

/*
  extern void monitor_fdc(void);
  monitor_fdc();
*/
/*
  int line = 0;
  if( exist_argv() ){
    line = argv.val;
    shift();
  }
  printf( "line=%d\n",line );
  attr_misc(line);
*/

    if (quasi88_cfg_can_showstatus()) {
	int now  = quasi88_cfg_now_showstatus();
	int next = (now) ? FALSE : TRUE;
	quasi88_cfg_set_showstatus(next);
    }
}





#ifdef USE_GNU_READLINE

/*--------------------------------------------------------------*/
/* readline を使ってみよう。					*/
/*--------------------------------------------------------------*/

char *command_generator( char *text, int state );
char *set_arg_generator( char *text, int state );
char **fileman_completion( char *text, int start, int end );

static void initialize_readline( void )
{
  rl_readline_name = "QUASI88";	 /*よくわからんが ~/.inputrc に関係あるらしい*/
  rl_attempted_completion_function = (CPPFunction *)fileman_completion;
}

char **fileman_completion( char *text, int start, int end )
{
  char **matches = NULL;

  int i=0;
  char c;					/* "set " と入力された場合 */
  while( (c = rl_line_buffer[i]) ){
    if( c==' ' || c=='\t' ){ i++; continue; }
    else                   break;
  }
  if (strncmp( &rl_line_buffer[i], "set", 3) == 0  &&  start > (i+3) )
#ifdef RL_READLINE_VERSION	/* ? */
    matches = rl_completion_matches( text, (rl_compentry_func_t *)set_arg_generator );
#else
    matches = completion_matches( text, set_arg_generator );
#endif
  else

  if (start == 0)				/* 行頭での入力の場合 */
#ifdef RL_READLINE_VERSION	/* ? */
    matches = rl_completion_matches( text, (rl_compentry_func_t *)command_generator );
#else
    matches = completion_matches( text, command_generator );
#endif

  return matches;
}

char *command_generator( char *text, int state )
{
  static int count, len;
  char *name;

  if( state == 0 ){		/* この関数、最初は state=0 で呼び出される */
    count = 0;			/* らしいので、その時に変数を初期化する。  */
    len = strlen (text);
  }

  while( count < COUNTOF(monitor_cmd) ){	/* コマンド名を検索 */

    name = monitor_cmd[count].cmd;
    count ++;

    if( strncmp( name, text, len ) == 0 ){
      char *p = malloc( strlen(name) + 1 );
      if (p){
	strcpy( p, name );
      }
      return p;
    }
  }

  return NULL;
}

char *set_arg_generator( char *text, int state )
{
  static int count, len;	/* set コマンドが入力済みの場合 */
  char *name;

  if( state == 0 ){
    count = 0;
    len = strlen (text);
  }

  while( count < COUNTOF(monitor_variable) ){	/* 変数名を検索 */

    name = monitor_variable[count].var_name;
    count ++;

    if( strcmp( name, "" )==0 ) continue;

    if( strncmp( name, text, len ) == 0 ){
      char *p = malloc( strlen(name) + 1 );
      if (p){
	strcpy( p, name );
      }
      return p;
    }
  }

  return NULL;
}
#endif



/****************************************************************/
/* デバッグ メイン処理						*/
/****************************************************************/

static	int	monitor_job = 0;

void	monitor_init( void )
{
  monitor_job = 0;

  save_dump_addr = -1;
  save_dump_bank = ARG_MAIN;
  save_dumpext_addr = -1;
  save_dumpext_bank = ARG_EXT0;
  save_dumpext_board = 1;
  save_disasm_cpu     = -1;
  save_disasm_addr[0] = -1;
  save_disasm_addr[1] = -1;


  {
	/* 一番最初にモニターモードに入った時は、メッセージを表示 */

    static int enter_monitor_mode_first = TRUE;
    if( enter_monitor_mode_first ){
      printf("\n"
	     "*******************************************************************************\n"
	     "* QUASI88   - monitor mode -                                                  *\n"
	     "*                                                                             *\n"
	     "*    Enter  go   or g  : Return to emulator                                   *\n"
	     "*    Enter  menu or m  : Enter menu mode                                      *\n"
	     "*    Enter  quit or q  : Quit QUASI88                                         *\n"
	     "*                                                                             *\n"
	     "*    Enter  help or ?  : Display help                                         *\n"
	     "*******************************************************************************\n"
	     "\n" );

      enter_monitor_mode_first = FALSE;

#ifdef USE_GNU_READLINE
      initialize_readline();
#endif
    }
  }

  fflush(NULL);

  if (quasi88_event_flags & EVENT_DEBUG) {
    quasi88_event_flags &= ~EVENT_DEBUG;

    /* ブレークした場合は、レジスタ表示 */
    switch( cpu_timing ){
    case 0:
      if( select_main_cpu ) z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
      else                  z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
      break;

    case 1:
                           z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
      if( dual_cpu_count ) z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
      break;

    case 2:
      z80_debug( &z80main_cpu, "[MAIN CPU]\n" );
      z80_debug( &z80sub_cpu,  "[SUB CPU]\n" );
      break;
    }
  }

  status_message_default(0, " MONITOR ");
  status_message_default(1, NULL);
  status_message_default(2, NULL);

  screen_update_immidiate();
}



void	monitor_main( void )
{
  int	i;


  /* モードが切り替わるまで、処理を続行 */
  while ((quasi88_event_flags & EVENT_MODE_CHANGED) == 0) {

    switch( monitor_job ){

    case MONITOR_LINE_INPUT:

#ifndef USE_GNU_READLINE

      printf("QUASI88> ");
      if( fgets( buf, MAX_CHRS, stdin ) == NULL ){	/* ^D が入力されたら */
#ifndef IGNORE_CTRL_D					/* 強制的に終了。    */
	quasi88_quit();					/* 回避方法がわからん*/
#else
	quasi88_monitor();				/* IRIX/AIXは大丈夫? */
#endif
	break;
      }

#else
      {
	char *p, *chk;
	HIST_ENTRY *ph;
	p = readline( "QUASI88> " );			/* GNU readline の  */
	if( p==NULL ){					/* 仕様がいまいち   */
	  printf( "\n" );				/* わからん。       */
	  break;					/* man で斜め読み   */
	}else{						/* してみたが、英語 */
							/* は理解しがたい。 */
	  ph = previous_history();
	  if ( *p=='\0' && ph != NULL) {	/* リターンキーで直前の */
	    strncpy( buf, ph->line, MAX_CHRS-1 );	/*コマンドを実行 */
	  } else strncpy( buf, p, MAX_CHRS-1 );
	  buf[ MAX_CHRS-1 ] = '\0';

	  chk = p;	/* 空行じゃなければ履歴に残す */
	  while( *chk ){
	    if( *chk==' ' || *chk=='\t' ){ chk++;		continue; }
	    /* 同じコマンドは履歴に残さない */
	    else if (ph != NULL && strcmp(chk, ph->line) == 0)	break;
	    else			 { add_history( chk );  break;    }
	  }
	}
	free( p );
	/* このあたりの処理は、peachさんにより改良されました */
      }
#endif

      getarg();						/* 引数を分解 */

      if( d_argc==0 ){					/* 空行の場合 */
	monitor_job = 0;
      }else{
	for( i=0; i<COUNTOF(monitor_cmd); i++ ){
	  if( strcmp( d_argv[0], monitor_cmd[i].cmd )==0 ) break;
	}
	if( i==COUNTOF(monitor_cmd) ){			/* 無効命令の場合 */
	  printf("Invalid command : %s\n",d_argv[0]);
	  monitor_job = 0;
	}else{						/* 引数が ? の場合 */
	  if( d_argc==2 && strcmp( d_argv[1], "?" )==0 ){
	    (monitor_cmd[i].help)();
	    monitor_job = 0;
	  }else{					/* 通常の命令の場合 */
	    monitor_job = monitor_cmd[i].job;
	    shift();
	    fflush(NULL);
	  }
	}
      }
      break;
      


    case MONITOR_HELP:
      monitor_job=0;
      monitor_help();
      break;
    case MONITOR_MENU:
      quasi88_menu();
      break;
    case MONITOR_QUIT:
      quasi88_quit();
      break;
      
    case MONITOR_GO:
      monitor_job=0;
      monitor_go();
      break;
    case MONITOR_TRACE:
      monitor_job=0;
      monitor_trace();
      break;
    case MONITOR_STEP:
      monitor_job=0;
      monitor_step();
      break;
    case MONITOR_STEPALL:
      monitor_job=0;
      monitor_stepall();
      break;
    case MONITOR_BREAK:
      monitor_job=0;
      monitor_break();
      break;
      
      
    case MONITOR_READ:
      monitor_job=0;
      monitor_read();
      break;
    case MONITOR_WRITE:
      monitor_job=0;
      monitor_write();
      break;
    case MONITOR_DUMP:
      monitor_job=0;
      monitor_dump();
      break;
    case MONITOR_DUMPEXT:
      monitor_job=0;
      monitor_dumpext();
      break;
    case MONITOR_FILL:
      monitor_job=0;
      monitor_fill();
      break;
    case MONITOR_MOVE:
      monitor_job=0;
      monitor_move();
      break;
    case MONITOR_SEARCH:
      monitor_job=0;
      monitor_search();
      break;
    case MONITOR_IN:
      monitor_job=0;
      monitor_in();
      break;
    case MONITOR_OUT:
      monitor_job=0;
      monitor_out();
      break;
    case MONITOR_LOADMEM:
      monitor_job=0;
      monitor_loadmem();
      break;
    case MONITOR_SAVEMEM:
      monitor_job=0;
      monitor_savemem();
      break;
      
    case MONITOR_RESET:
      monitor_job=0;
      monitor_reset();
      break;
    case MONITOR_REG:
      monitor_job=0;
      monitor_reg();
      break;
    case MONITOR_DISASM:
      monitor_job=0;
      monitor_disasm();
      break;
      
    case MONITOR_SET:
      monitor_job=0;
      monitor_set();
      break;
    case MONITOR_SHOW:
      monitor_job=0;
      monitor_show();
      break;
    case MONITOR_REDRAW:
      monitor_job=0;
      screen_update_immidiate();
      break;
    case MONITOR_RESIZE:
      monitor_job=0;
      monitor_resize();
      break;
    case MONITOR_DRIVE:
      monitor_job=0;
      monitor_drive();
      break;
    case MONITOR_FILE:
      monitor_job=0;
      monitor_file();
      break;
    case MONITOR_STATESAVE:
      monitor_job=0;
      monitor_statesave();
      break;
    case MONITOR_STATELOAD:
      monitor_job=0;
      monitor_stateload();
      break;
    case MONITOR_SNAPSHOT:
      monitor_job=0;
      monitor_snapshot();
      break;
    case MONITOR_LOADFONT:
      monitor_job=0;
      monitor_loadfont();
      break;
    case MONITOR_SAVEFONT:
      monitor_job=0;
      monitor_savefont();
      break;
    case MONITOR_TAPELOAD:
      monitor_job=0;
      monitor_tapeload();
      break;
    case MONITOR_TAPESAVE:
      monitor_job=0;
      monitor_tapesave();
      break;
    case MONITOR_PRINTER:
      monitor_job=0;
      monitor_printer();
      break;
    case MONITOR_SERIALIN:
      monitor_job=0;
      monitor_serialin();
      break;
    case MONITOR_SERIALOUT:
      monitor_job=0;
      monitor_serialout();
      break;

    case MONITOR_MISC:
      monitor_job=0;
      monitor_misc();
      break;

    case MONITOR_FBREAK:
      monitor_job=0;
      monitor_fbreak();
      break;
    case MONITOR_TEXTSCR:
      monitor_job=0;
      monitor_textscr();
      break;
    case MONITOR_LOADBAS:
      monitor_job=0;
      monitor_loadbas();
      break;
    case MONITOR_SAVEBAS:
      monitor_job=0;
      monitor_savebas();
      break;

      
    default:
      monitor_job=0;
      printf("Internal Error\n");
      break;
    }

  }

}



#endif	/* USE_MONITOR */
