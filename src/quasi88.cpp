/************************************************************************/
/* QUASI88 --- PC-8801 emulator                     */
/*  Copyright (c) 1998-2012 Showzoh Fukunaga            */
/*  All rights reserved.                        */
/*                                  */
/*    このソフトは、UNIX + X Window System の環境で動作する、   */
/*  PC-8801 のエミュレータです。                  */
/*                                  */
/*    このソフトの作成にあたり、Marat Fayzullin氏作の fMSX、   */
/*  Nicola Salmoria氏 (MAME/XMAME project) 作の MAME/XMAME、    */
/*  ゆみたろ氏作の PC6001V のソースを参考にさせてもらいました。    */
/*                                  */
/*  ＊注意＊                            */
/*    サウンドドライバは、MAME/XMAME のソースを流用しています。  */
/*  この部分のソースの著作権は、MAME/XMAME チームあるいはソースに  */
/*  記載してある著作者にあります。                   */
/*    FM音源ジェネレータは、fmgen のソースを流用しています。 */
/*  この部分のソースの著作権は、 cisc氏 にあります。       */
/*                                  */
/************************************************************************/

extern "C"
{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <ctype.h>

    #include "quasi88.h"
    #include "initval.h"

    #include "pc88main.h"
    #include "pc88sub.h"
    #include "graph.h"
    #include "memory.h"
    #include "file-op.h"

    #include "emu.h"
    #include "drive.h"
    #include "keyboard.h"
    #include "monitor.h"
    #include "snddrv.h"
    #include "wait.h"
    #include "status.h"
    #include "suspend.h"
    #include "snapshot.h"
    #include "soundbd.h"
    #include "screen.h"
    #include "menu.h"
    #include "pause.h"
    #include "z80.h"
    #include "intr.h"
}

#if USE_RETROACHIEVEMENTS
#include "retroachievements.h"
#endif


int verbose_level   = DEFAULT_VERBOSE;  /* 冗長レベル      */
int verbose_proc    = FALSE;        /* 処理の進行状況の表示   */
int verbose_z80 = FALSE;        /* Z80処理エラーを表示  */
int verbose_io  = FALSE;        /* 未実装I/Oアクセス表示*/
int verbose_pio = FALSE;        /* PIO の不正使用を表示 */
int verbose_fdc = FALSE;        /* FDイメージ異常を報告    */
int verbose_wait    = FALSE;        /* ウエイト時の異常を報告 */
int verbose_suspend = FALSE;        /* サスペンド時の異常を報告 */
int verbose_snd = FALSE;        /* サウンドのメッセージ   */

static  void    imagefile_all_open(int stateload);
static  void    imagefile_all_close(void);
static  void    status_override(void);

/***********************************************************************
 *
 *          QUASI88 メイン関数
 *
 ************************************************************************/
void    quasi88(void)
{
    quasi88_start();
    quasi88_main();
    quasi88_stop(TRUE);
}

/* =========================== メイン処理の初期化 ========================== */

#define SET_PROC(n) proc = n; if (verbose_proc) printf("\n"); fflush(NULL);
static  int proc = 0;

void    quasi88_start(void)
{
    stateload_init();           /* ステートロード関連初期化 */
    drive_init();           /* ディスク制御のワーク初期化  */
    /* ↑ これらは、ステートロード開始までに初期化しておくこと       */

    SET_PROC(1);

                    /* エミュレート用メモリの確保  */
    if (memory_allocate() == FALSE) { quasi88_exit(-1); }

    if (resume_flag) {          /* ステートロード        */
    SET_PROC(2);
    if (stateload() == FALSE) {
        fprintf(stderr, "stateload: Failed ! (filename = %s)\n",
            filename_get_state());
        quasi88_exit(-1);
    }
    if (verbose_proc) printf("Stateload...OK\n"); fflush(NULL);
    }
    SET_PROC(3);

                    /* グラフィックシステム初期化  */
    if (screen_init() == FALSE) { quasi88_exit(-1); }
    SET_PROC(4);

                    /* システムイベント初期化    */
    event_init();           /* (screen_init の後で！)   */

#if USE_RETROACHIEVEMENTS
    RA_InitUI();    /* 実績システム初期化 */

    if (quasi88_cfg_now_wait_rate() < 100 && RA_HardcoreModeIsActive())
    {
        quasi88_cfg_set_wait_rate(100);
    }
#endif                      /* (ここもscreen_initの後で) */

                        /* サウンドドライバ初期化    */
    if (xmame_sound_start() == FALSE) { quasi88_exit(-1); }
    SET_PROC(5);

                    /* ウエイト用タイマー初期化 */
    if (wait_vsync_init() == FALSE) { quasi88_exit(-1); }
    SET_PROC(6);


    set_signal();           /* INTシグナルの処理を設定    */

    imagefile_all_open(resume_flag);    /* イメージファイルを全て開く  */

                        /* エミュ用ワークを順次初期化  */
    pc88main_init((resume_flag) ? INIT_STATELOAD : INIT_POWERON);
    pc88sub_init ((resume_flag) ? INIT_STATELOAD : INIT_POWERON);

    key_record_playback_init();     /* キー入力記録/再生 初期化  */

    screen_snapshot_init();     /* スナップショット関連初期化   */


    debuglog_init();
    profiler_init();

    emu_breakpoint_init();

    if (verbose_proc) printf("Running QUASI88kai...\n");
}

/* ======================== メイン処理のメインループ ======================= */

void    quasi88_main(void)
{
    for (;;) {

    /* 終了の応答があるまで、繰り返し呼び続ける */

    int result = quasi88_loop();
    if (result == QUASI88_LOOP_EXIT) {
        break;
    }
    if (result == QUASI88_LOOP_ONE) {
#if USE_RETROACHIEVEMENTS
        RA_HandleHTTPResults();

        if (RA_GameIsActive())
            RA_DoAchievementsFrame();
#endif
    }

    }

    /* quasi88_loop() は、 1フレーム (VSYNC 1周期分≒1/60秒) おきに、
       QUASI88_LOOP_ONE を返してくる。
       この戻り値を判断して、なんらかの処理を加えてもよい。

       また、内部処理の事情で QUASI88_LOOP_BUSY を返してくることもあるが、
       この場合は気にせずに、繰り返し呼び出しを続行すること */

}

/* ========================== メイン処理の後片付け ========================= */

void    quasi88_stop(int normal_exit)
{
    if (normal_exit) {
    if (verbose_proc) printf("Shutting down.....\n");
    }

    /* 初期化途中の場合、verbose による詳細表示がなければ、エラー表示する */
#define ERR_DISP(n) ((proc == (n)) && (verbose_proc == 0))

    switch (proc) {
    case 6:         /* 初期化 正常に終わっている */
    profiler_exit();
    debuglog_exit();
    screen_snapshot_exit();
    key_record_playback_exit();
    pc88main_term();
    pc88sub_term();
    imagefile_all_close();
    wait_vsync_exit();
    /* FALLTHROUGH */

    case 5:         /* タイマーの初期化でNG */
    if (ERR_DISP(5)) printf("timer initialize failed!\n");
    xmame_sound_stop();
    /* FALLTHROUGH */

    case 4:         /* サウンドの初期化でNG */
    if (ERR_DISP(4)) printf("sound system initialize failed!\n");
    event_exit();
    screen_exit();
    /* FALLTHROUGH */

    case 3:         /* グラフィックの初期化でNG */
    if (ERR_DISP(3)) printf("graphic system initialize failed!\n");
    /* FALLTHROUGH */

    case 2:         /* ステートロードでNG */
    /* FALLTHROUGH */

    case 1:         /* メモリの初期化でNG */
    if (ERR_DISP(2)) printf("memory allocate failed!\n");
    memory_free();
    /* FALLTHROUGH */

    case 0:         /* 終了処理 すでに完了 */
    break;
    }

#if USE_RETROACHIEVEMENTS
    RA_Shutdown();
#endif

    proc = 0;   /* この関数を続けて呼んでも問題無いように、クリアしておく */
}


/***********************************************************************
 * QUASI88 途中終了処理関数
 *  exit() の代わりに呼ぼう。
 ************************************************************************/

#define MAX_ATEXIT  (32)
static  void (*exit_function[MAX_ATEXIT])(void);

/*
 * 関数を最大 MAX_ATEXIT 個、登録できる。ここで登録した関数は、
 * quasi88_exit() を呼び出した時に、登録した順と逆順で、呼び出される。
 */
void    quasi88_atexit(void (*function)(void))
{
    int i;
    for (i=0; i<MAX_ATEXIT; i++) {
    if (exit_function[i] == NULL) {
        exit_function[i] = function;
        return;
    }
    }
    printf("quasi88_atexit: out of array\n");
    quasi88_exit(-1);
}

/*
 * quasi88 を強制終了する。
 * quasi88_atexit() で登録した関数を呼び出した後に、 exit() する
 */
void    quasi88_exit(int status)
{
    int i;

    quasi88_stop(FALSE);

    for (i=MAX_ATEXIT-1; i>=0; i--) {
    if (exit_function[i]) {
        (*exit_function[i])();
        exit_function[i] = NULL;
    }
    }

    exit(status);
}





/***********************************************************************
 * QUASI88メインループ制御
 *  QUASI88_LOOP_EXIT が返るまで、無限に呼び出すこと。
 * 戻り値
 *  QUASI88_LOOP_EXIT … 終了時
 *  QUASI88_LOOP_ONE  … 1フレーム経過時 (ウェイトが正確ならば約1/60秒周期)
 *  QUASI88_LOOP_BUSY … 上記以外の、なんらかのタイミング
 ************************************************************************/
int quasi88_event_flags = EVENT_MODE_CHANGED;
static  int mode    = EXEC;     /* 現在のモード */
static  int next_mode   = EXEC;     /* モード切替要求時の、次モード */

int quasi88_loop(void)
{
    static enum {
    INIT,
    MAIN,
    WAIT,
    } step = INIT, step_after_wait = INIT;

    int stat;

    switch (step) {

    /* ======================== イニシャル処理 ======================== */
    case INIT:
    profiler_lapse( PROF_LAPSE_RESET );

    /* モード変更時は、必ずここに来る。モード変更フラグをクリア */
    quasi88_event_flags &= ~EVENT_MODE_CHANGED;
    mode = next_mode;

    /* 例外的なモード変更時の処理 */
    switch (mode) {
#ifndef USE_MONITOR
    case MONITOR:   /* ありえないけど、念のため */
        mode = PAUSE;
        break;
#endif
    case QUIT:  /* QUIT なら、メインループ終了 */
        return FALSE;
    }

    /* モード別イニシャル処理 */
    if (mode == EXEC) { xmame_sound_resume(); }
    else              { xmame_sound_suspend();}

    screen_switch();
    event_switch();
    keyboard_switch();

    switch (mode) {
    case EXEC:  emu_init();     break;

    case MENU:  menu_init();        break;
#ifdef  USE_MONITOR
    case MONITOR:   monitor_init();     break;
#endif
    case PAUSE: pause_init();       break;
    }

    status_override();

    wait_vsync_switch();


    /* イニシャル処理が完了したら、MAIN に遷移 */
    step = MAIN;

    /* 遷移するため、一旦関数を抜ける (FALLTHROUGHでもいいけど) */
    return QUASI88_LOOP_BUSY;


    /* ======================== メイン処理 ======================== */
    case MAIN:
    switch (mode) {

    case EXEC:  profiler_lapse( PROF_LAPSE_RESET );
            emu_main();     break;
#ifdef  USE_MONITOR
    case MONITOR:   monitor_main();     break;
#endif
    case MENU:  menu_main();        break;

    case PAUSE: pause_main();       break;
    }

    /* モード変更が発生していたら、(WAIT後に) INIT へ遷移する */
    /* そうでなければ、            (WAIT後に) MAIN へ遷移する */
    if (quasi88_event_flags & EVENT_MODE_CHANGED) {
        step_after_wait = INIT;
    } else {
        step_after_wait = MAIN;
    }

    /* 描画タイミングならばここで描画。その後 WAIT へ  */
    /* そうでなければ、                WAIT せずに遷移 */
    if (quasi88_event_flags & EVENT_FRAME_UPDATE) {
        quasi88_event_flags &= ~EVENT_FRAME_UPDATE;
        screen_update();
        step = WAIT;
    } else {
        step = step_after_wait;
    }

    /* モニター遷移時や終了時は、 WAIT せずに即ちに INIT へ */
    if (quasi88_event_flags & (EVENT_DEBUG | EVENT_QUIT)) {
        step = INIT;
    }

    /* 遷移するため、一旦関数を抜ける (WAIT時はFALLTHROUGHでもいいけど) */
    return QUASI88_LOOP_BUSY;


    /* ======================== ウェイト処理 ======================== */
    case WAIT:
    stat = WAIT_JUST;

    switch (mode) {
    case EXEC:
        profiler_lapse( PROF_LAPSE_IDLE );
        if (! no_wait) { stat = wait_vsync_update(); }
        break;

    case MENU:
    case PAUSE:
        /* Esound の場合、 MENU/PAUSE でも stream を流しておかないと
           複数起動時に、 MENU/PAUSE してないほうが音がでなくなる。
           が、このままだと、現在の音が流れっぱなしになるので、
           無音を流すようにしないと。 */
        xmame_sound_update();       /* サウンド出力 */
        xmame_update_video_and_audio(); /* サウンド出力 その2 */
        stat = wait_vsync_update();
        break;
    }

    if (stat == WAIT_YET) { return QUASI88_LOOP_BUSY; }


    /* ウェイト時間を元に、フレームスキップの有無を決定 */
    if (mode == EXEC) {
        frameskip_check((stat == WAIT_JUST) ? TRUE : FALSE);
    }

    /* ウェイト処理が完了したら、次 (INIT か MAIN) に遷移 */
    step = step_after_wait;
    return QUASI88_LOOP_ONE;
    }

    /* ここには来ない ! */
    return QUASI88_LOOP_EXIT;
}



/*======================================================================
 * QUASI88 のモード制御
 *  モードとは QUASI88 の状態のことで、 EXEC (実行)、PAUSE (一時停止)、
 *  MENU (メニュー画面)、 MONITOR (対話型デバッガ)、 QUIT(終了) がある。
 *======================================================================*/
/* QUASI88のモードを設定する */
static  void    set_mode(int newmode)
{
    if (mode != newmode) {

    if (mode == MENU) {     /* メニューから他モードの切替は */
        q8tk_event_quit();      /* Q8TK の終了が必須            */
    }

    next_mode = newmode;
    quasi88_event_flags |= EVENT_MODE_CHANGED;
    CPU_BREAKOFF();
    }
}

/* QUASI88のモードを切り替える */
void    quasi88_exec(void)
{
    set_mode(EXEC);
    set_emu_exec_mode(GO);

#if USE_RETROACHIEVEMENTS
    RA_SetPaused(false);
#endif
}

void    quasi88_exec_step(void)
{
    set_mode(EXEC);
    set_emu_exec_mode(STEP);
}

void    quasi88_exec_trace(void)
{
    set_mode(EXEC);
    set_emu_exec_mode(TRACE);
}

void    quasi88_exec_trace_change(void)
{
    set_mode(EXEC);
    set_emu_exec_mode(TRACE_CHANGE);
}

void    quasi88_menu(void)
{
    set_mode(MENU);
}

void    quasi88_pause(void)
{
    set_mode(PAUSE);
#if USE_RETROACHIEVEMENTS
    RA_SetPaused(true);
#endif
}

void    quasi88_monitor(void)
{
#ifdef  USE_MONITOR
    set_mode(MONITOR);
#else
    set_mode(PAUSE);
#endif
}

void    quasi88_debug(void)
{
#ifdef  USE_MONITOR
    set_mode(MONITOR);
    quasi88_event_flags |= EVENT_DEBUG;
#else
    set_mode(PAUSE);
#endif
}

void    quasi88_quit(void)
{
#if USE_RETROACHIEVEMENTS
    if (mode == QUIT || !RA_ConfirmLoadNewRom(true))
        return;
#endif

    set_mode(QUIT);
    quasi88_event_flags |= EVENT_QUIT;
}

/* QUASI88のモードを取得する */
int quasi88_is_exec(void)
{
  return (mode == EXEC) ? TRUE : FALSE;
}
int quasi88_is_menu(void)
{
  return (mode == MENU) ? TRUE : FALSE;
}
int quasi88_is_pause(void)
{
  return (mode == PAUSE) ? TRUE : FALSE;
}
int quasi88_is_monitor(void)
{
  return (mode == MONITOR) ? TRUE : FALSE;
}





/***********************************************************************
 *  適切な位置に移動せよ
 ************************************************************************/
void    wait_vsync_switch(void)
{
    long dt;

    /* dt < 1000000us (1sec) でないとダメ */
    if (quasi88_is_exec()) {
    dt = (long)((1000000.0 / (CONST_VSYNC_FREQ * wait_rate/100)));
    wait_vsync_setup(dt, wait_by_sleep);
    } else {
    dt = (long)(1000000.0 / CONST_VSYNC_FREQ);
    wait_vsync_setup(dt, TRUE);
    }
}


static  void    status_override(void)
{
    static int first_fime = TRUE;

    if (first_fime) {

    /* EMUモードで起動した場合のみ、ステータスの表示を変える */
    if (mode == EXEC) {

        status_message(0, STATUS_INFO_TIME, Q_TITLE " " Q_VERSION);

        if (resume_flag == 0) {
        if (status_imagename == FALSE) {
            status_message_default(1, "<F12> key to MENU");
        }
        } else {
        status_message(1, STATUS_INFO_TIME, "State-Load Successful");
        }
    }
    first_fime = FALSE;
    }
}



/***********************************************************************
 *  デバッグ用
 ************************************************************************/
#include "debug.c"



/***********************************************************************
 *  雑多な関数
 ************************************************************************/
#include "utility.c"



/***********************************************************************
 *          ファイル名制御／管理
 ************************************************************************/
#include "fname.c"




/***********************************************************************
 * 各種動作パラメータの変更
 *  これらの関数は、ショートカットキー処理や、機種依存部のイベント
 *  処理などから呼び出されることを *一応* 想定している。
 *
 *  メニュー画面の表示中に呼び出すと、メニュー表示と食い違いが生じる
 *  ので、メニュー中は呼び出さないように。エミュ実行中に呼び出すのが
 *  一番安全。うーん、いまいち。
 *
 *  if( mode == EXEC ){
 *      quasi88_disk_insert_and_reset( file, FALSE );
 *  }
 *
 ************************************************************************/

/***********************************************************************
 * QUASI88 起動中のリセット処理関数
 ************************************************************************/
void    quasi88_get_reset_cfg(T_RESET_CFG *cfg)
{
    cfg->boot_basic = boot_basic;
    cfg->boot_dipsw = boot_dipsw;
    cfg->boot_from_rom  = boot_from_rom;
    cfg->boot_clock_4mhz= boot_clock_4mhz;
    cfg->set_version    = set_version;
    cfg->baudrate_sw    = baudrate_sw;
    cfg->use_extram = use_extram;
    cfg->use_jisho_rom  = use_jisho_rom;
    cfg->sound_board    = sound_board;
}

void    quasi88_reset(const T_RESET_CFG *cfg)
{
    int sb_changed = FALSE;
    int empty[2];

    if (verbose_proc) printf("Reset QUASI88kai...start\n");

    pc88main_term();
    pc88sub_term();

    if (cfg) {
    if (sound_board != cfg->sound_board) {
        sb_changed = TRUE;
    }

    boot_basic  = cfg->boot_basic;
    boot_dipsw  = cfg->boot_dipsw;
    boot_from_rom   = cfg->boot_from_rom;
    boot_clock_4mhz = cfg->boot_clock_4mhz;
    set_version = cfg->set_version;
    baudrate_sw = cfg->baudrate_sw;
    use_extram  = cfg->use_extram;
    use_jisho_rom   = cfg->use_jisho_rom;
    sound_board = cfg->sound_board;
    }

    /* メモリの再確保が必要なら、処理する */
    if (memory_allocate_additional() == FALSE) {
    quasi88_exit(-1);   /* 失敗！ */
    }

    /* サウンド出力のリセット */
    if (sb_changed == FALSE) {
    xmame_sound_reset();
    } else {
    menu_sound_restart(FALSE);  /* サウンドドライバの再初期化 */
    }

    /* ワークの初期化 */
    pc88main_init(INIT_RESET);
    pc88sub_init(INIT_RESET);

    /* FDCの初期化 */
    empty[0] = drive_check_empty(0);
    empty[1] = drive_check_empty(1);
    drive_reset();
    if (empty[0]) drive_set_empty(0);
    if (empty[1]) drive_set_empty(1);

    /*if (xmame_has_sound()) xmame_sound_reset();*/

#if USE_RETROACHIEVEMENTS
    if (RA_HardcoreModeIsActive())
    {
        if (loaded_disk.data_len > 0 && loaded_tape.data_len > 0)
        {
            if (loaded_title != NULL)
            {
                switch (loaded_title->file_type)
                {
                case FTYPE_DISK:
                    quasi88_load_tape_eject();
                    break;
                case FTYPE_TAPE_LOAD:
                    quasi88_disk_eject(DRIVE_1);
                    break;
                default:
                    /* ディスクイメージを優先して、テープを取り外す */
                    quasi88_load_tape_eject();
                }
            }
        }

        if (quasi88_cfg_now_wait_rate() < 100)
        {
            quasi88_cfg_set_wait_rate(100);
        }

        event_switch();
    }

    if (loaded_title == NULL)
    {
        if (loaded_disk.data_len > 0)
            loaded_title = &loaded_disk;
        else if (loaded_tape.data_len > 0)
            loaded_title = &loaded_tape;

        if (loaded_title != NULL)
        {
            RA_UpdateAppTitle(loaded_title->name);
            RA_ActivateGame(loaded_title->title_id);
        }
    }

    RA_OnReset();
#endif

    emu_reset();

    if (verbose_proc) printf("Reset QUASI88kai...done\n");
}



/***********************************************************************
 * QUASI88 起動中のステートロード処理関数
 *  TODO 引数で、ファイル名指定？
 ************************************************************************/
int quasi88_stateload(int serial)
{
    int now_board, success = 0;

    if (serial >= 0) {          /* 連番指定あり (>=0) なら */
    filename_set_state_serial(serial);  /* 連番を設定する */
    }

    if (verbose_proc) printf("Stateload...start (%s)\n",filename_get_state());

    if (stateload_check_file_exist() == FALSE) {    /* ファイルなし */
    if (quasi88_is_exec()) {
        status_message(1, STATUS_INFO_TIME, "State-Load file not found !");
    } /* メニューではダイアログ表示するので、ステータス表示は無しにする */

    if (verbose_proc) printf("State-file not found\n");
    return FALSE;
    }

#if USE_RETROACHIEVEMENTS
    if (!RA_WarnDisableHardcore("load a state"))
    {
        if (verbose_proc) printf("State-Load cancelled (RA)\n");
        return FALSE;
    }
#endif

    pc88main_term();            /* 念のため、ワークを終了状態に */
    pc88sub_term();
    imagefile_all_close();      /* イメージファイルを全て閉じる */

    /*xmame_sound_reset();*/        /* 念のため、サウンドリセット */
    /*quasi88_reset();*/        /* 念のため、全ワークリセット */


    now_board = sound_board;

    success = stateload();      /* ステートロード実行 */

    if (now_board != sound_board) {     /* サウンドボードが変わったら */
    menu_sound_restart(FALSE);  /* サウンドドライバの再初期化 */
    }

    if (verbose_proc) {
        if (success)
        {
            printf("Stateload...done\n");
        }
    else         printf("Stateload...Failed, Reset start\n");
    }


    if (success) {          /* ステートロード成功したら・・・ */

    imagefile_all_open(TRUE);       /* イメージファイルを全て開く*/

    pc88main_init(INIT_STATELOAD);
    pc88sub_init(INIT_STATELOAD);

#if USE_RETROACHIEVEMENTS
    RA_OnLoadState(filename_get_state());
#endif

    } else {                /* ステートロード失敗したら・・・ */

    quasi88_reset(NULL);            /* とりあえずリセット */
    }


    if (quasi88_is_exec()) {
    if (success) {
        status_message(1, STATUS_INFO_TIME, "State-Load Successful");
    } else {
        status_message(1, STATUS_INFO_TIME, "State-Load Failed !  Reset done ...");
    }

    /* quasi88_loop の内部状態を INIT にするため、モード変更扱いとする */
    quasi88_event_flags |= EVENT_MODE_CHANGED;
    }
    /* メニューではダイアログ表示するので、ステータス表示は無しにする */

    return success;
}



/***********************************************************************
 * QUASI88 起動中のステートセーブ処理関数
 *  TODO 引数で、ファイル名指定？
 ************************************************************************/
int quasi88_statesave(int serial)
{
    int success = 0;

    if (serial >= 0) {          /* 連番指定あり (>=0) なら */
    filename_set_state_serial(serial);  /* 連番を設定する */
    }

    if (verbose_proc) printf("Statesave...start (%s)\n",filename_get_state());

    success = statesave();      /* ステートセーブ実行 */

    if (verbose_proc) {
        if (success)
        {
            printf("Statesave...done\n");
        }
    else         printf("Statesave...Failed, Reset done\n");
    }


    if (quasi88_is_exec()) {
    if (success) {
#if USE_RETROACHIEVEMENTS
        RA_OnSaveState(filename_get_state());
#endif
        status_message(1, STATUS_INFO_TIME, "State-Save Successful");
    } else {
        status_message(1, STATUS_INFO_TIME, "State-Save Failed !");
    }
    }   /* メニューではダイアログ表示するので、ステータス表示は無しにする */

    return success;
}



/***********************************************************************
 * 画面スナップショット保存
 *  TODO 引数で、ファイル名指定？
 ************************************************************************/
int quasi88_screen_snapshot(void)
{
    int success;

    success = screen_snapshot_save();


    if (success) {
    status_message(1, STATUS_INFO_TIME, "Screen Capture Saved");
    } else {
    status_message(1, STATUS_INFO_TIME, "Screen Capture Failed !");
    }

    return success;
}



/***********************************************************************
 * サウンドデータのファイル出力
 *  TODO 引数で、ファイル名指定？
 ************************************************************************/
int quasi88_waveout(int start)
{
    int success;

    if (start) {
    success = waveout_save_start();

    if (success) {
        status_message(1, STATUS_INFO_TIME, "Sound Record Start ...");
    } else {
        status_message(1, STATUS_INFO_TIME, "Sound Record Failed !");
    }

    } else {

    success = TRUE;

    waveout_save_stop();
    status_message(1, STATUS_INFO_TIME, "Sound Record Stopped");
    }

    return success;
}



/***********************************************************************
 * ドラッグアンドドロップ
 ************************************************************************/
int quasi88_drag_and_drop(const char *filename)
{
    if (!quasi88_is_exec() && !quasi88_is_pause()) {
        return FALSE;
    }

    int success;

    if (quasi88_disk_insert_all(filename, FALSE)) {
        status_message(1, STATUS_INFO_TIME, "Disk Image Set and Reset");
        success = TRUE;
    } else if (quasi88_load_tape_insert(filename)) {
        status_message(1, STATUS_INFO_TIME, "Tape Image Set and Reset");
        success = TRUE;
    } else {
        status_message(1, STATUS_WARN_TIME, "D&D Failed !  Disk Unloaded ...");
        success = FALSE;
    }

    if (success)
    {
        quasi88_reset(NULL);

        if (quasi88_is_pause()) {
            quasi88_exec();
        }
    }

    return success;
}



/***********************************************************************
 * ウェイトの比率設定
 * ウェイトの有無設定
 ************************************************************************/
int quasi88_cfg_now_wait_rate(void)
{
    return wait_rate;
}
void    quasi88_cfg_set_wait_rate(int rate)
{
    int time = STATUS_INFO_TIME;
    char str[32];
    long dt;

#if USE_RETROACHIEVEMENTS
    if (rate < 100)
    {
        if (!RA_WarnDisableHardcore("set the speed below 100%"))
            return;
    }
#endif
    if (rate < 5)    rate = 5;
    if (rate > 5000) rate = 5000;

    if (wait_rate != rate) {
    wait_rate = rate;

    if (quasi88_is_exec()) {

        sprintf(str, "WAIT  %4d[%%]", wait_rate);

        status_message(1, time, str);
        /* ↑ ウェイト変更したので、表示時間はウェイト倍になる */

        dt = (long)((1000000.0 / (CONST_VSYNC_FREQ * wait_rate / 100)));
        wait_vsync_setup(dt, wait_by_sleep);
    }
    }
}
int quasi88_cfg_now_no_wait(void)
{
    return no_wait;
}
void    quasi88_cfg_set_no_wait(int enable)
{
    int time = STATUS_INFO_TIME;
    char str[32];
    long dt;

    if (no_wait != enable) {
    no_wait = enable;

    if (quasi88_is_exec()) {

        if (no_wait) { sprintf(str, "WAIT  OFF");    time *= 10; }
        else           sprintf(str, "WAIT  ON");

        status_message(1, time, str);
        /* ↑ ウェイトなしなので、表示時間は実際のところ不定 */

        dt = (long)((1000000.0 / (CONST_VSYNC_FREQ * wait_rate / 100)));
        wait_vsync_setup(dt, wait_by_sleep);
    }
    }
}



/***********************************************************************
 * ディスクイメージファイル設定
 *  ・両ドライブに挿入
 *  ・指定ドライブに挿入
 *  ・反対ドライブのイメージファイルを、挿入
 *  ・両ドライブ取り出し
 *  ・指定ドライブ取り出し
 ************************************************************************/
int quasi88_disk_insert_all(const char *filename, int ro)
{
    int success = FALSE;

    if (!quasi88_disk_eject_all())
        return FALSE;

    success = quasi88_disk_insert(DRIVE_1, filename, 0, ro);

    if (success) {

    if (disk_image_num(DRIVE_1) > 1) {
        quasi88_disk_insert_A_to_B(DRIVE_1, DRIVE_2, 1);
    }
    }

    if (quasi88_is_exec()) {
    status_message_default(1, NULL);
    }
    return success;
}
int quasi88_disk_insert(int drv, const char *filename, int image, int ro)
{
    int success = FALSE;

#if USE_RETROACHIEVEMENTS
    if (drv == DRIVE_1)
    {
        if (!RA_PrepareLoadNewRom(filename, FTYPE_DISK))
            return FALSE;
    }
#endif

    if (!quasi88_disk_eject(drv))
        return FALSE;

    if (strlen(filename) < QUASI88_MAX_FILENAME) {

    if (disk_insert(drv, filename, image, ro) == 0) success = TRUE;
    else                                            success = FALSE;

    if (success) {

        if (drv == DRIVE_1) boot_from_rom = FALSE;

        strcpy(file_disk[ drv ], filename);
        readonly_disk[ drv ] = ro;

        if (filename_synchronize) {
        filename_init_state(TRUE);
        filename_init_snap(TRUE);
        filename_init_wav(TRUE);
        }

#if USE_RETROACHIEVEMENTS
        RA_CommitLoadNewRom();
#endif
    }
    }

    if (quasi88_is_exec()) {
    status_message_default(1, NULL);
    }
    return success;
}
int quasi88_disk_insert_A_to_B(int src, int dst, int img)
{
    int success;

    if (!quasi88_disk_eject(dst))
        return FALSE;

    if (disk_insert_A_to_B(src, dst, img) == 0) success = TRUE;
    else                                        success = FALSE;

    if (success) {
    strcpy(file_disk[ dst ], file_disk[ src ]);
    readonly_disk[ dst ] = readonly_disk[ src ];

    if (filename_synchronize) {
        filename_init_state(TRUE);
        filename_init_snap(TRUE);
        filename_init_wav(TRUE);
    }
    }

    if (quasi88_is_exec()) {
    status_message_default(1, NULL);
    }
    return success;
}
int quasi88_disk_eject_all(void)
{
    int drv;

    for (drv = 0; drv<2; drv++) {
    if (!quasi88_disk_eject(drv))
        return FALSE;
    }

    boot_from_rom = TRUE;

    if (quasi88_is_exec()) {
    status_message_default(1, NULL);
    }

    return TRUE;
}
int quasi88_disk_eject(int drv)
{
    if (disk_image_exist(drv)) {
#if USE_RETROACHIEVEMENTS
    if (drv == DRIVE_1 && loaded_title != NULL &&
        loaded_title->file_type == FTYPE_DISK)
    {
        if (!RA_ConfirmLoadNewRom(false))
            return FALSE;
    }
#endif
    disk_eject(drv);
    memset(file_disk[ drv ], 0, QUASI88_MAX_FILENAME);

    if (filename_synchronize) {
        filename_init_state(TRUE);
        filename_init_snap(TRUE);
        filename_init_wav(TRUE);
    }

#if USE_RETROACHIEVEMENTS
    if (drv == DRIVE_1)
    {
#if !RA_RELOAD_MULTI_DISK
        if (loaded_title != NULL && loaded_title->title_id != loading_file.title_id)
#endif
        RA_OnGameClose(FTYPE_DISK);
    }
#endif
    }

    if (quasi88_is_exec()) {
    status_message_default(1, NULL);
    }

    return TRUE;
}

/***********************************************************************
 * ディスクイメージファイル設定
 *  ・ドライブを一時的に空の状態にする
 *  ・ドライブのイメージを変更する
 *  ・ドライブのイメージを前のイメージに変更する
 *  ・ドライブのイメージを次のイメージに変更する
 ************************************************************************/
enum { TYPE_SELECT, TYPE_EMPTY, TYPE_NEXT, TYPE_PREV };

static void disk_image_sub(int drv, int type, int img)
{
    int d;
    char str[48];

    if (disk_image_exist(drv)) {
    switch (type) {

    case TYPE_EMPTY:
        drive_set_empty(drv);
        sprintf(str, "DRIVE %d:  <<<< Eject >>>>         ", drv + 1);
        break;

    case TYPE_NEXT:
    case TYPE_PREV:
        if (type == TYPE_NEXT) d = +1;
        else                   d = -1;

        img = disk_image_selected(drv) + d;
        /* FALLTHROUGH */

    default:
        if (img < 0) img = disk_image_num(drv)-1;
        if (img >= disk_image_num(drv)) img = 0;

        drive_unset_empty(drv);
        disk_change_image(drv, img);

        sprintf(str, "DRIVE %d:  %-16s   %s  ",
            drv + 1,
            drive[drv].image[ disk_image_selected(drv) ].name,
            (drive[drv].image[ disk_image_selected(drv) ].protect)
                            ? "(p)" : "   ");
        break;
    }
    } else {
    sprintf(str, "DRIVE %d:   --  No Disk  --        ", drv + 1);
    }

    if (quasi88_is_exec()) {
    status_message_default(1, NULL);
    }
    status_message(1, STATUS_INFO_TIME, str);
}
void    quasi88_disk_image_select(int drv, int img)
{
    disk_image_sub(drv, TYPE_SELECT, img);
}
void    quasi88_disk_image_empty(int drv)
{
    disk_image_sub(drv, TYPE_EMPTY, 0);
}
void    quasi88_disk_image_next(int drv)
{
    disk_image_sub(drv, TYPE_NEXT, 0);
}
void    quasi88_disk_image_prev(int drv)
{
    disk_image_sub(drv, TYPE_PREV, 0);
}






/*======================================================================
 * テープイメージファイル設定
 *      ・ロード用テープイメージファイルセット
 *      ・ロード用テープイメージファイル巻き戻し
 *      ・ロード用テープイメージファイル取り外し
 *      ・セーブ用テープイメージファイルセット
 *      ・セーブ用テープイメージファイル取り外し
 *======================================================================*/
int quasi88_load_tape_insert(const char *filename)
{
#if USE_RETROACHIEVEMENTS
    if (!RA_PrepareLoadNewRom(filename, FTYPE_TAPE_LOAD))
        return FALSE;
#endif

    if (!quasi88_load_tape_eject())
        return FALSE;

    if (strlen(filename) < QUASI88_MAX_FILENAME &&
    sio_open_tapeload(filename)) {

    strcpy(file_tape[ CLOAD ], filename);

#if USE_RETROACHIEVEMENTS
    RA_CommitLoadNewRom();
#endif

    return TRUE;

    }
    return FALSE;
}
int quasi88_load_tape_rewind(void)
{
    if (sio_tape_rewind()) {

    return TRUE;

    }
    quasi88_load_tape_eject();
    return FALSE;
}
int quasi88_load_tape_eject(void)
{
#if USE_RETROACHIEVEMENTS
    if (loaded_title != NULL && loaded_title->file_type == FTYPE_TAPE_LOAD &&
        loaded_title->data_len > 0)
    {
        if (!RA_ConfirmLoadNewRom(false))
            return FALSE;
    }
#endif

    sio_close_tapeload();
    memset(file_tape[ CLOAD ], 0, QUASI88_MAX_FILENAME);

#if USE_RETROACHIEVEMENTS
    if (loaded_tape.data_len > 0)
    {
#if !RA_RELOAD_MULTI_DISK
        if (loaded_title != NULL && loaded_title->title_id != loading_file.title_id)
#endif
        RA_OnGameClose(FTYPE_TAPE_LOAD);
    }
#endif

    return TRUE;
}

int quasi88_save_tape_insert(const char *filename)
{
    quasi88_save_tape_eject();

    if (strlen(filename) < QUASI88_MAX_FILENAME &&
    sio_open_tapesave(filename)) {

    strcpy(file_tape[ CSAVE ], filename);
    return TRUE;

    }
    return FALSE;
}
int quasi88_save_tape_eject(void)
{
    sio_close_tapesave();
    memset(file_tape[ CSAVE ], 0, QUASI88_MAX_FILENAME);

    return TRUE;
}

/*======================================================================
 * シリアル・パラレルイメージファイル設定
 *      ・シリアル入力用ファイルセット
 *      ・シリアル入力用ファイル取り外し
 *      ・シリアル出力用ファイルセット
 *      ・シリアル出力用ファイル取り外し
 *      ・プリンタ出力用ファイルセット
 *      ・プリンタ入力用ファイルセット
 *======================================================================*/
int quasi88_serial_in_connect( const char *filename )
{
  quasi88_serial_in_remove();

  if( strlen( filename ) < QUASI88_MAX_FILENAME &&
      sio_open_serialin( filename ) ){

    strcpy( file_sin, filename );
    return TRUE;

  }
  return FALSE;
}
void    quasi88_serial_in_remove( void )
{
  sio_close_serialin();
  memset( file_sin, 0, QUASI88_MAX_FILENAME );
}
int quasi88_serial_out_connect( const char *filename )
{
  quasi88_serial_out_remove();

  if( strlen( filename ) < QUASI88_MAX_FILENAME &&
      sio_open_serialout( filename ) ){

    strcpy( file_sout, filename );
    return TRUE;

  }
  return FALSE;
}
void    quasi88_serial_out_remove( void )
{
  sio_close_serialout();
  memset( file_sout, 0, QUASI88_MAX_FILENAME );
}
int quasi88_printer_connect( const char *filename )
{
  quasi88_printer_remove();

  if( strlen( filename ) < QUASI88_MAX_FILENAME &&
      printer_open( filename ) ){

    strcpy( file_prn, filename );
    return TRUE;

  }
  return FALSE;
}
void    quasi88_printer_remove( void )
{
  printer_close();
  memset( file_prn, 0, QUASI88_MAX_FILENAME );
}
