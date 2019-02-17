/***********************************************************************
 * グラフィック処理 (システム依存)
 *
 *  詳細は、 graph.h 参照
 ************************************************************************/

extern "C"
{
    #include <stdio.h>
    #include <stdlib.h>

    #include "quasi88.h"
    #include "graph.h"
    #include "device.h"
}

#include "retroachievements.h"



HINSTANCE   g_hInstance;
HWND        g_hWnd;
HMENU       g_hMenu;
int     g_keyrepeat;



static  T_GRAPH_SPEC    graph_spec;     /* 基本情報     */

static  int     graph_exist;        /* 真で、画面生成済み  */
static  T_GRAPH_INFO    graph_info;     /* その時の、画面情報  */
static  T_GRAPH_INFO    graph_info_windowed; /* ウィンドウモード時の情報 */


/************************************************************************
 *  グラフィック処理の初期化
 *  グラフィック処理の動作
 *  グラフィック処理の終了
 ************************************************************************/

const T_GRAPH_SPEC  *graph_init(void)
{
    if (verbose_proc) {
    printf("Initializing Graphic System ... ");
    }

#ifdef SUPPORT_DOUBLE
    graph_spec.window_max_width = 1280;
    graph_spec.window_max_height = 960;
#else
    graph_spec.window_max_width      = 640;
    graph_spec.window_max_height     = 480;
#endif
    graph_spec.fullscreen_max_width  = GetSystemMetrics(SM_CXSCREEN);
    graph_spec.fullscreen_max_height = GetSystemMetrics(SM_CYSCREEN);
    graph_spec.forbid_status         = FALSE;
    graph_spec.forbid_half           = FALSE;

    if (verbose_proc)
    printf("OK\n");

    return &graph_spec;
}

/************************************************************************/

static DWORD winStyle;
static int create_window(int width, int height);
static void calc_window_size(int *width, int *height);

static unsigned char *buffer = NULL;
static BITMAPINFO bmpInfo;

const T_GRAPH_INFO  *graph_setup(int width, int height,
                     int fullscreen, double aspect)
{
    int win_width, win_height;
    int win_offx, win_offy;
    int scaled_width, scaled_height;
    int scaled_offx, scaled_offy;
    int prev_fullscreen = graph_info.fullscreen;

    win_offx = 0;
    win_offy = 0;
    scaled_width = width;
    scaled_height = height;
    scaled_offx = 0;
    scaled_offy = 0;

#if 0 /* aspectは現在未実装 */
    if (aspect) {
        scaled_width *= aspect;
    }
#endif

    /* オフスクリーンバッファを確保する */

    if (buffer) {
    free(buffer);
    }

    buffer = (unsigned char *)malloc(width * height * sizeof(unsigned long));
    if (buffer == FALSE) {
    return NULL;
    }

    memset(&bmpInfo, 0, sizeof(bmpInfo));

    bmpInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth       =   width;
    bmpInfo.bmiHeader.biHeight      = - height;
    bmpInfo.bmiHeader.biPlanes      = 1;
    bmpInfo.bmiHeader.biBitCount    = 32;
    bmpInfo.bmiHeader.biCompression = BI_RGB;


    /* ウインドウの生成、ないしリサイズ */

    if (graph_exist == FALSE) {     /* ウインドウが無ければ生成 */

    if (create_window(scaled_width, scaled_height) == FALSE) {
        free(buffer);
        buffer = NULL;
        return NULL;
    }

    }

    /* ウインドウが有ればリサイズ */

    DWORD style = GetWindowLong(g_hWnd, GWL_STYLE);
    RECT win_rect;
        
    if (fullscreen) {
        if (!prev_fullscreen) {
            GetWindowRect(g_hWnd, &win_rect);
            graph_info.window_offx = win_rect.left;
            graph_info.window_offy = win_rect.top;

            /* ウィンドウモードの画面情報を保存 */
            graph_info_windowed = graph_info;
        }

        MONITORINFO mi = { sizeof(MONITORINFO) };
            
        if (!GetMonitorInfo(MonitorFromWindow(g_hWnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            free(buffer);
            buffer = NULL;
            return NULL;
        }

        win_width = mi.rcMonitor.right - mi.rcMonitor.left;
        win_height = mi.rcMonitor.bottom - mi.rcMonitor.top;

        int scale_factor = MIN(win_width / width, win_height / (height - (graph_spec.forbid_status || !show_status ? STATUS_HEIGHT : 0)));
        scaled_width = width * scale_factor;
        scaled_height = height * scale_factor;
        scaled_offx = (win_width - scaled_width) / 2;
        scaled_offy = (win_height - scaled_height) / 2;

        SetWindowLong(g_hWnd, GWL_STYLE, style & ~winStyle);
        SetWindowPos(g_hWnd, HWND_TOP,
            mi.rcMonitor.left, mi.rcMonitor.top - GetSystemMetrics(SM_CYMENU),
            win_width, win_height + GetSystemMetrics(SM_CYMENU),
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

        /* 残像を消す為に画面を更新する */
        HDC hdc = GetDC(g_hWnd);
        GetWindowRect(g_hWnd, &win_rect);
        FillRect(hdc, &win_rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        ReleaseDC(g_hWnd, hdc);
    }
    else {
        win_width = scaled_width;
        win_height = scaled_height;

        if (prev_fullscreen) {
            win_offx = graph_info_windowed.window_offx;
            win_offy = graph_info_windowed.window_offy;
        }

        calc_window_size(&win_width, &win_height);
        SetWindowLong(g_hWnd, GWL_STYLE, style | winStyle);
        SetWindowPos(g_hWnd, HWND_TOP,
            win_offx, win_offy,       /* ウィンドウのオフセット */
            win_width, win_height,    /* ウィンドウの幅・高さ   */
            (prev_fullscreen ? 0 : SWP_NOMOVE) | SWP_NOZORDER | SWP_FRAMECHANGED);
    }

    /* graph_info に諸言をセットする */

    graph_info.fullscreen       = fullscreen;
    graph_info.width            = width;
    graph_info.height           = height;
    graph_info.scaled_width     = scaled_width;
    graph_info.scaled_height    = scaled_height;
    graph_info.scaled_offx      = scaled_offx;
    graph_info.scaled_offy      = scaled_offy;
    graph_info.window_offx      = win_offx;
    graph_info.window_offy      = win_offy;
    graph_info.byte_per_pixel   = 4;
    graph_info.byte_per_line    = width * 4;
    graph_info.buffer           = buffer;
    graph_info.nr_color         = 255;
    graph_info.write_only       = FALSE;
    graph_info.broken_mouse     = FALSE;
    graph_info.draw_start       = NULL;
    graph_info.draw_finish      = NULL;
    graph_info.dont_frameskip   = FALSE;

    graph_exist = TRUE;

    return &graph_info;
}



/*
 * ウインドウを生成する
 */
static int create_window(int width, int height)
{
    WNDCLASSEX wc;
    int win_width, win_height;

    /* ウィンドウクラスの情報を設定 */
    wc.cbSize = sizeof(wc);         /* 構造体サイズ */
    wc.style = 0;               /* ウインドウスタイル */
    wc.lpfnWndProc = WndProc;           /* ウィンドウプロシージャ */
    wc.cbClsExtra = 0;              /* 拡張情報 */
    wc.cbWndExtra = 0;              /* 拡張情報 */
    wc.hInstance = g_hInstance;         /* インスタンスハンドル */
    wc.hIcon = NULL;                /* アイコン */
/*
    wc.hIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(IDI_APPLICATION),
                IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
*/
    wc.hIconSm = wc.hIcon;          /* 小さいアイコン */
    wc.hCursor = NULL;              /* マウスカーソル */
/*
    wc.hCursor = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(IDC_ARROW),
                    IMAGE_CURSOR, 0, 0,
                    LR_DEFAULTSIZE | LR_SHARED);
*/
                        /* ウィンドウ背景 */
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
/*  wc.lpszMenuName = NULL;*/           /* メニュー名 */
    wc.lpszMenuName = "QUASI88";        /* メニュー名 → quasi88.rc */
    wc.lpszClassName = "Win32App";      /* ウィンドウクラス名 適当 */

    /* ウィンドウクラスを登録する */
    if (RegisterClassEx(&wc) == 0) { return FALSE; }

    /* ウィンドウスタイルはこれ */
    winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    /* ウィンドウサイズの計算 */
    win_width  = width;
    win_height = height;
    calc_window_size(&win_width, &win_height);

    /* ウィンドウを作成する */
    g_hWnd = CreateWindowEx(WS_EX_ACCEPTFILES,  /* 拡張ウィンドウスタイル */
                wc.lpszClassName,   /* ウィンドウクラス名    */
                "QUASI88 win32",    /* タイトルバー文字列    */
                winStyle,       /* ウィンドウスタイル    */
                CW_USEDEFAULT,  /* ウィンドウのx座標      */
                CW_USEDEFAULT,  /* ウィンドウのy座標      */
                win_width,      /* ウィンドウの幅      */
                win_height,     /* ウィンドウの高さ   */
                NULL,       /* 親ウィンドウのハンドル */
                NULL,       /* メニューハンドル   */
                g_hInstance,    /* インスタンスハンドル     */
                NULL);      /* 付加情報       */

    if (g_hWnd == NULL) { return FALSE; }

    /* ウィンドウを表示する */
    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);

    /* メニューハンドル */
    g_hMenu = GetMenu(g_hWnd);
    
    /* Drag & Drop の許可 */
#if 0
    /* ウインドウの作成時に、 WS_EX_ACCEPTFILES をつけているので、これは不要 */
    DragAcceptFiles(g_hWnd, TRUE);
#endif


#if 0
    if (verbose_proc) { /* ディスプレイ情報 */
    HDC hdc;
    hdc = GetDC(g_hWnd);
    fprintf(debugfp, "Info: H-pixel %d\n", GetDeviceCaps(hdc, HORZRES));
    fprintf(debugfp, "Info: V-pixel %d\n", GetDeviceCaps(hdc, VERTRES));
    fprintf(debugfp, "Info: Depth   %d\n", GetDeviceCaps(hdc, BITSPIXEL));
    ReleaseDC(g_hWnd, hdc);
    }
#endif
    return TRUE;
}



/*
 * 本当のウインドウサイズを計算する
 */
static void calc_window_size(int *width, int *height)
{
    RECT rect;

    rect.left = 0;  rect.right  = *width;
    rect.top  = 0;  rect.bottom = *height;

    AdjustWindowRectEx(&rect,           /* クライアント矩形       */
               winStyle,        /* ウィンドウスタイル     */
               TRUE,            /* メニューフラグ         */
               0);          /* 拡張ウィンドウスタイル */

    *width  = rect.right - rect.left;       /* 本当のウィンドウの幅   */
    *height = rect.bottom - rect.top;       /* 本当のウィンドウの高さ */
}



/************************************************************************/

void    graph_exit(void)
{
    if (buffer) {
    free(buffer);
    }
}

/************************************************************************
 *  色の確保
 *  色の解放
 ************************************************************************/

void    graph_add_color(const PC88_PALETTE_T color[],
            int nr_color, unsigned long pixel[])
{
    int i;
    for (i=0; i<nr_color; i++) {

    pixel[i] = ((((unsigned long) color[i].red)   << 16) |
                (((unsigned long) color[i].green) <<  8) |
                (((unsigned long) color[i].blue)));

    /* RGB()マクロは、順序が反転しているので使えない */
    }
}

/************************************************************************/

void    graph_remove_color(int nr_pixel, unsigned long pixel[])
{
    /* 色に関しては何も管理しないので、ここでもなにもしない */
}

/************************************************************************
 *  グラフィックの更新
 ************************************************************************/

static  int graph_update_counter = 0;

int graph_update_WM_PAINT(void)
{
    int drawn;
    HDC hdc;
    PAINTSTRUCT ps;

    hdc = BeginPaint(g_hWnd, &ps);

#if USE_RETROACHIEVEMENTS
    HDC hdc_main = hdc;
    HDC hdc_buffer = CreateCompatibleDC(hdc);
    HBITMAP hbm_buffer = CreateCompatibleBitmap(hdc,
        graph_info.width, graph_info.height);
    SelectObject(hdc_buffer, hbm_buffer);

    hdc = hdc_buffer;
#endif

    /* graph_update() により、 WM_PAINT イベントが発生した場合、描画する。
       OS が勝手に発生させた WM_PAINT イベントの場合は、なにもしない。
       (quasi88_expose() の処理により、 graph_update() が呼び出されるため) */

    if (graph_update_counter > 0) {
#if 1   /* どちらの API でもよさげ。速度は？ */
#if USE_RETROACHIEVEMENTS
        StretchDIBits(hdc,
            0, 0, graph_info.width, graph_info.height,
            0, 0, graph_info.width, graph_info.height,
            buffer, &bmpInfo, DIB_RGB_COLORS, SRCCOPY);
#else
    StretchDIBits(hdc,
              graph_info.scaled_offx, graph_info.scaled_offy,
              graph_info.scaled_width, graph_info.scaled_height,
              0, 0, graph_info.width, graph_info.height,
              buffer, &bmpInfo, DIB_RGB_COLORS, SRCCOPY);
#endif
#else   /* こっちは、転送先の高さしか指定できない */
    SetDIBitsToDevice(hdc,
              0, 0, graph_info.width, graph_info.scaled_height,
              0, 0, 0, graph_info.height,
              buffer, &bmpInfo, DIB_RGB_COLORS);
#endif
    graph_update_counter = 0;
    drawn = TRUE;
    } else {
    drawn = FALSE;
    }

#if USE_RETROACHIEVEMENTS
    RA_RenderOverlayFrame(hdc);
    StretchBlt(hdc_main,
        graph_info.scaled_offx, graph_info.scaled_offy,
        graph_info.scaled_width, graph_info.scaled_height,
        hdc, 0, 0, graph_info.width, graph_info.height, SRCCOPY);

    DeleteObject(hbm_buffer);
    DeleteDC(hdc_buffer);
#endif

/*
    fprintf(debugfp,
        "%s %d:(%3d,%3d)-(%3d,%3d)\n",
        (drawn) ? "update" : "EXPOSE", graph_update_counter, 
        ps.rcPaint.left,  ps.rcPaint.top,
        ps.rcPaint.right, ps.rcPaint.bottom);
*/
    EndPaint(g_hWnd, &ps);

    return drawn;
}

void    graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
    graph_update_counter = 1;

    InvalidateRect(g_hWnd, NULL, FALSE);
    UpdateWindow(g_hWnd);

    /* ここで、直接ウインドウ描画をしようとしたのだが、なんかうまくいかない。
       WndProc() の内部でしか、ウインドウ描画はできないのかな？

       とりあえず、 InvalidateRect() をすると、 WM_PAINT イベントが発生する
       ので、この後の WndProc() の WM_PAINT 処理にて描画させることにしよう。

       ちなみに、 InvalidateRect() の直後に、 UpdateWindow() を呼び出すと、
       この関数の内部で WndProc() が呼び出され、 WM_PAINT の処理が行われる
       らしい。つまり UpdateWindow() が終わった時点で描画は終わっている。

       本来ここは nr_rect 回、処理を繰り返すようにすべきなのだが、面倒なので
       全画面を1回だけ描画させている。(速いマシンなら気にならない ^^;) */
}


/************************************************************************
 *  タイトルの設定
 *  属性の設定
 ************************************************************************/

void    graph_set_window_title(const char *title)
{
}

/************************************************************************/

void    graph_set_attribute(int mouse_show, int grab, int keyrepeat_on)
{
    g_keyrepeat = keyrepeat_on;

    if (mouse_show) ShowCursor(TRUE);
    else            ShowCursor(FALSE);
}
