/************************************************************************/
/*									*/
/* ヘルプメッセージ (OS依存)						*/
/*	Q_TITLE, Q_VERSION は コンパイル時に(Makefileで)定義		*/
/*	それ以外のマクロは、全て initval.h に				*/
/*									*/
/*		getconf.c ファイル内から、include されている。		*/
/************************************************************************/

static	void	help_msg_common(void)
{
  fprintf
  (
   stdout,
   Q_TITLE " ver " Q_VERSION " (" Q_COMMENT ")   ---  PC-8801 emulator\n"
   "\n"
   "Usage: %s [-option1 [-option2...]] [ filename1 [n][filename2][m] ]\n"
   "  [filename1] = filename of disk image set in DRIVE 1:\n"
   "  [filename2] = filename of disk image set in DRIVE 2:\n"
   "  [n]         = image number of DRIVE 1:\n"
   "  [m]         = image number of DRIVE 2:\n"
   "  [-option]   =\n"
   "  ** PC-8801 CONFIG **\n"
   "    -v2/-v1h/-v1s/-n        Select BASIC mode [%s]\n"
   "    -4mhz/-8mhz             Select CPU clock mode [%s]\n"
   "    -sd/-sd2                Select soundboard type I/II [%s]\n"
   "    -dipsw <num>            Dip switch setting [0x%04x]\n"
   "    -baudrate <bps>         Baud rate setting [%d]\n"
   "    -romboot/-diskboot      Select boot from ROM or DISK [%s]\n"
   "    -extram <cards>         Number of extend ram card (0..16, 128KB/card) [%d]\n"
   "    -noextram               Not use extram ( mean -extram 0 )\n"
   "    -jisho/-nojisho         Use/Not use JISHO-ROM [%s]\n"
   "    -nomouse/-mouse/-joymouse/-joystick\n"
   "                            Select mouse/joystick port setting [-nomouse]\n"
   "    -analog/-digital        Set monitor RGB analog/digital [-analog]\n"
   "    -24k/-15k               Set monitor frequency(dummy) [-24k]\n"
   "    -pcg/-nopcg             Use/Not use PCG-8100 [-nopcg]\n"
   "    -tapeload <filename>    Set tape image for load (T88,CMT)\n"
   "    -tapesave <filename>    Set tape image for save (CMT)\n"
   "    -serialmouse            Use serial-mouse\n"
   "  ** EMULATION **\n"
   "    -cpu <0/1/2>            Main-Sub CPU control timing [%d]\n"
   "    -fdc_wait/-fdc_nowait   Enable/Disable FDC wait [-fdc_nowait]\n"
   "    -clock <rate>           CPU clock MHz (0.1..999.9) [%6.4f]\n"
   "    -speed <rate>           Set speed rate (5..5000%%) [100]\n"
   "    -nowait                 No wait ( ignore option '-speed' )\n"
   "    -boost <rate>           Set boost n times (1..100) [1]\n"
   "    -cmt_intr/-cmt_poll     Use/Not use interrupt for tape-loading [-cmt_intr]\n"
   "    -cmt_speed <bps>        Set tape-Baudrate [AUTO]\n"
   "    -hsbasic                High-speed basic mode\n"
   "    -mem_wait/-mem_nowait   Enable/Disable memory wait [-mem_nowait]\n"
   "    -setver <num>           Set V1 mode version as <num>\n"
   "                             0..2=88/3=mkII/4=SR/5..7=FR/8=FH/9=FA..\n"
   "    -exchange               Send a fake signal when exchange disks\n"
   "  ** GRAPHIC **\n"
   "    -frameskip <period>     Period of frame skip [%d]\n"
   "    -autoskip/-noautoskip   Use/Not use auto frame skip [-autoskip]\n"
#ifdef	SUPPORT_DOUBLE
   "    -full/-half/-double     Screen size is full, half, or double [-full]\n"
#else
   "    -full/-half             Screen size is full or half [-full]\n"
#endif
   "    -fullscreen/-window     Try to start in Full-screen/Windowed [-window]\n"
   "    -aspect <ratio>         Set aspect of fullscreen (ex 1.33) (experimental)\n"
   "    -width <x>/-height <y>  Set window size <x>*<y>\n"
   "    -interp/-nointerp       Use/Not use reduce interpolation [-interp]\n"
   "    -skipline/-interlace/-nointerlace\n"
   "                            Select interlace graphic-effect [-nointerlace]\n"
   "    -hide_mouse/-show_mouse/-auto_mouse\n"
   "                            Hide/Show or Auto-Hide mouse cursor [-show_mouse]\n"
   "    -grab_mouse/-ungrab_mouse/-auto_grab\n"
   "                            Grab/Not grab or Auto-Grab mouse [-ungrab_mouse]\n"
   "    -nostatus               Hide status\n"
   "    -status_fg <RGB>        Set status foreground color [0x000000]\n"
   "    -status_bg <RGB>        Set status background color [0xd6d6d6]\n"
   "    -statusimage            Display image-name on status\n"
   "  ** INPUT **\n"
   "    -tenkey                 Convert from full-key 0-9 to ten-key 0-9\n"
   "    -numlock                Set software NumLock to ON\n"
   "    -cursor                 Assign cursor-key to ten-key 2,4,6,8\n"
   "    -mousekey               Assign mouse-move to ten-key 2,4,6,8\n"
   "    -joykey                 Assign joypad-key to ten-key 2,4,6,8\n"
   "    -f6 <func>/-f7 <func>/-f8 <func>/-f9 <func>/-f10 <func>\n"
   "                            Assign F6..F10 key to function of <func>\n"
   "                             ( FRATE-UP,FRATE-DOWN,VOLUME-UP,VOLUME-DOWN,\n"
   "                               PAUSE,RESIZE,NOWAIT,SPEED-UP,SPEED-DOWN,\n"
   "                               FULLSCREEN,SNAPSHOT,MAX-CLOCK,MAX-BOOST\n"
   "                               IMAGE-NEXT1,IMAGE-PREV1,IMAGE-NEXT2,IMAGE-PREV2,\n"
   "                               NUMLOCK,RESET,KANA,ROMAJI,CAPS,STATUS,MENU )\n"
   "    -romaji <type>          Set ROMAJI-HENKAN type (0:egg/1:MS-IME/2:ATOK) [0]\n"
   "    -kanjikey               Assign F6-F10 Key for KANJI-input\n"
   "    -joyswap                Swap Joystick Button A<-->B\n"
   "    -mouseswap              Swap Mouse Button Left<-->Right\n"
   "    -mousespeed <rate>      Set mouse speed rate (5..400%%) [100]\n"
   "  ** MENU **\n"
   "    -menu                   start in menu mode\n"
   "    -japanese/-english      Menu mode language is Japanese/English\n"
   "    -euc/-sjis/-utf8        filename decoding is EUC-japan/Shift JIS/UTF-8\n"
   "    -bmp/-ppm/-raw          Screen snapshot file format [-bmp]\n"
   "    -swapdrv                Change display position of Menu-Disk-Tab\n"
   "    -menucursor             Display mouse cursor in menu mode\n"
   "  ** MISC **\n"
   "    -romdir <path>          Set directory of ROM image file\n"
   "    -diskdir <path>         Set directory of DISK image file\n"
   "    -tapedir <path>         Set directory of TAPE image file\n"
   "    -snapdir <path>         Set directory of SNAPSHOT file\n"
   "    -statedir <path>        Set directory of STATE file\n"
   "    -noconfig               Not load config file\n"
   "    -compatrom <filename>   Specify ROM image file of P88SR.EXE\n"
   "    -resume                 stateload in start\n"
   "    -resumefile <filename>  stateload in start (state file is <filename>)\n"
   "    -focus                  Running quasi88 only in window focus\n"
   "    -sleep/-nosleep         Sleep/Not sleep during idle [-sleep]\n"
   "    -ro/-rw                 Open disk image file as read-only/read-write [-rw]\n"
   "    -ignore_ro              Treat RO disk image file as RW\n"
   "  ** DEBUG **\n"
   "    -help                   Print this help page\n"
   "    -verbose <level>        Select debugging messages [0x%02x]\n"
   "                                0 - Silent          1 - Startup messages\n"
   "                                2 - Undef Z80 op    4 - Undecorded I/O\n" 
   "                                8 - PIO warning    16 - FDC disk error\n"
   "                               32 - wait error     64 - statesave info\n"
   "                              128 - sound info     ...\n"
   "    -printer <filename>     Set printer output to file (append)\n"
   "    -serialout <filename>   Set serial output to file (append)\n"
   "    -serialin <filename>    Set serial input from file\n"
   "    -record <filename>      Record all key inputs to the file <filename>\n"
   "    -playback <filename>    Play back all key inputs from the file <filename>\n"
   "    -timestop               Freeze real-time-clock\n"
   "    -vsync <hz>             Set VSYNC frequency [55.4]\n"
#ifdef	USE_MONITOR
   "    -debug                  enable to go to monitor mode\n"
   "    -monitor                start in monitor mode\n"
   "    -fdcdebug               print FDC status\n"
#endif
   "\n"
   ,
   command,
   (DEFAULT_BASIC==BASIC_AUTO)
     ? "AUTO"
     : (DEFAULT_BASIC==BASIC_N)
       ? "-n"
       : (DEFAULT_BASIC==BASIC_V1S)
         ? "-v1s"
         : (DEFAULT_BASIC==BASIC_V1H)
            ? "-v1h"
            : "-v2",
   (DEFAULT_CLOCK==CLOCK_8MHZ) ? "-8mhz" : "-4mhz",
   (DEFAULT_SOUND==SOUND_I) ? "-sd" : "-sd2",
   DEFAULT_DIPSW,
   (DEFAULT_BAUDRATE==BAUDRATE_75 )
     ? 75
     : (DEFAULT_BAUDRATE==BAUDRATE_150 )
       ? 150
       : (DEFAULT_BAUDRATE==BAUDRATE_300 )
         ? 300
         : (DEFAULT_BAUDRATE==BAUDRATE_600 )
           ? 600
           : (DEFAULT_BAUDRATE==BAUDRATE_1200 )
             ? 1200
             : (DEFAULT_BAUDRATE==BAUDRATE_2400 )
               ? 2400
               : (DEFAULT_BAUDRATE==BAUDRATE_4800 )
                 ? 4800
                 : (DEFAULT_BAUDRATE==BAUDRATE_9600 )
                   ? 9600
                   : 19200,
   (DEFAULT_BOOT==BOOT_AUTO)
     ? "AUTO"
     : (DEFAULT_BOOT==BOOT_DISK)
       ? "-diskboot"
       : "-romboot",
   DEFAULT_EXTRAM,
   (DEFAULT_JISHO) ? "-jisho" : "-nojisho",

   DEFAULT_CPU,
   DEFAULT_CPU_CLOCK_MHZ,

   DEFAULT_FRAMESKIP, 

   DEFAULT_VERBOSE
  );
}


static	void	help_msg_config( void )
{
  fprintf
  (
   stdout,
   "\n"
   "Configuration:\n"
   "  <Configuration file>\n"
   "                        ... %s" CONFIG_FILENAME  CONFIG_SUFFIX "\n"
   "                            %s" "DISK-IMAGE-FILENAME" CONFIG_SUFFIX "\n"
   "  <Keyboard configuration file>\n"
   "                        ... -keyconf <filename>  or\n"
   "                        ... %s" KEYCONF_FILENAME  CONFIG_SUFFIX "\n"
   "  <State file>\n"
   "                        ... %s" STATE_FILENAME STATE_SUFFIX"\n"
   "                            %s" "DISK-IMAGE-FILENAME" STATE_SUFFIX"\n"
   "  <Directory of ROM IMAGE FILE>\n"
   "                        ... -romdir <path>       or\n"
   "                            ${QUASI88_ROM_DIR}   or\n"
   "                            \"%s\"\n"
   "  <Directory of DISK IMAGE FILE>\n"
   "                        ... -diskdir <path>      or\n"
   "                            ${QUASI88_DISK_DIR}  or\n"
   "                            \"%s\"\n"
   "  <Directory of TAPE IMAGE FILE>\n"
   "                        ... -tapedir <path>      or\n"
   "                            ${QUASI88_TAPE_DIR}  or\n"
   "                            \"%s\"\n"
   "  <Directory of SCREEN SNAPSHOT FILE>\n"
   "                        ... ${QUASI88_SNAP_DIR}  or\n"
   "                            \"%s\"\n"
   "  <Directory of STATE FILE>\n"
   "                        ... ${QUASI88_STATE_DIR} or\n"
   "                            \"%s\"\n"
   "  <Max number of images in one file>\n"
   "                        ... %d images\n"
   ,

#if	defined( QUASI88_FUNIX )
   "${HOME}/.quasi88/",
   "${HOME}/.quasi88/rc/",
   "${HOME}/.quasi88/",
   "${HOME}/.quasi88/state/",
   "${HOME}/.quasi88/state/",
   (strlen(ROM_DIR)==0) ? "current directory" : ROM_DIR,
   (strlen(DISK_DIR)==0)? "current directory" : DISK_DIR,
   (strlen(TAPE_DIR)==0)? "current directory" : TAPE_DIR,
   "current directory",
   "${HOME}/.quasi88/state/",
#elif	defined( QUASI88_FWIN )
   "${QUASI88_HOME}\\",
   "${QUASI88_INI_DIR}\\",
   "${QUASI88_HOME}\\",
   "${QUASI88_STATE_DIR}\\",
   "${QUASI88_STATE_DIR}\\",
   "ROM",
   "DISK",
   "TAPE",
   "SNAP",
   "STATE",
#elif	defined( QUASI88_FMAC )
   ":",
   ":",
   ":",
   ":",
   ":",
   ":ROM:",
   ":DISK:",
   ":TAPE:",
   ":SNAP:",
   ":STATE:",
#else
   "",
   "",
   "",
   "",
   "",
   "current directory",
   "current directory",
   "current directory",
   "current directory",
   "current directory",
#endif

   MAX_NR_IMAGE
  );
}
