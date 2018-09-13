#include "RA_Implementation.h"
#include "RA_Interface.h"
#include "BuildVer.h"
#include <windows.h>

//#ifdef ARCH_WIN32
#include "projects/msvc/resource.h"
//#endif

#include "vmachine.h"

//Don't like using externs, but many of the corressponding header files don't agree well with inclusion here
extern HWND ConsoleHWND(void);						//message.c
extern void ConsolePrintf(const char *format, ...);	//message.c

#include "shared.h"
#include "machine.h"
#include "app_memview.h"
#include "app_cheatfinder.h"
#include "debugger.h"


//Required for new RA Memory Bank interface
//See: RA_MemManager.h _RAMByteReadFn _RAMByteWriteFn
unsigned char RAMeka_RAMByteReadFn(unsigned int Offset) {
	return static_cast<unsigned char>(RAM[Offset]);
}
void RAMeka_RAMByteWriteFn(unsigned int Offset, unsigned int nVal) {
	//why is nVal an int and not a char?
	RAM[Offset] = static_cast<unsigned char>(nVal);
}



//Needed as RA and Meka both use GetCurrentDirectory and SetCurrentDirectory seperately
#define RAMEKA_DIR_PATH_SIZE (2048)
static char RA_rootDir[RAMEKA_DIR_PATH_SIZE];
static char Meka_currDir[RAMEKA_DIR_PATH_SIZE];

void RAMeka_Stash_Meka_CurrentDirectory() {
	GetCurrentDirectory(RAMEKA_DIR_PATH_SIZE, Meka_currDir);
}
void RAMeka_Restore_Meka_CurrentDirectory() {
	SetCurrentDirectory(Meka_currDir);
}
void RAMeka_Stash_RA_RootDirectory() {
	GetCurrentDirectory(RAMEKA_DIR_PATH_SIZE, RA_rootDir);
}
void RAMeka_Restore_RA_RootDirectory() {
	SetCurrentDirectory(RA_rootDir);
}



// returns -1 if not found
int GetMenuItemIndex(HMENU hMenu, const char* ItemName)
{
	int index = 0;
	char buf[256];

	while (index < GetMenuItemCount(hMenu))
	{
		if (GetMenuString(hMenu, index, buf, sizeof(buf) - 1, MF_BYPOSITION))
		{
			if (!strcmp(ItemName, buf))
				return index;
		}
		index++;
	}
	return -1;
}


//	Return whether a game has been loaded. Should return FALSE if
//	 no ROM is loaded, or a ROM has been unloaded.
bool GameIsActive()
{

	//	bool check = (g_machine_flags ^ MACHINE_RUN) == MACHINE_RUN) &&
	//			((g_machine_flags & MACHINE_PAUSED ) == MACHINE_PAUSED)	
		//return (g_machine_flags ^ MACHINE_ROM_LOADED)== MACHINE_ROM_LOADED; // doesn't detect system
	return true;  //otherwise debugger won't work.
}

//	Perform whatever action is required to unpause emulation.
void CauseUnpause()
{
	Machine_UnPause();
	//Machine_Pause(); Don't do this. Running pause code from within Machin_Pause() now.
	//FCEUI_SetEmulationPaused(false);
}

//	Perform whatever action is required to Pause emulation.
void CausePause()
{
	Machine_Pause();
}


//	Perform whatever function in the case of needing to rebuild the menu.
void RebuildMenu()
{
	//Meka uses Allegro window (Not Win32)
	//So going to use the Console window from message.c to attach the RA Menu to because using SetMenu distorts the window
	//We actually need to create the entire menu in the first place to do this (Console doesn't have one)
	
	HMENU MainMenu = GetMenu(ConsoleHWND());
	if (!MainMenu) return;
	
	// get file menu index
	int index = GetMenuItemIndex(MainMenu, "&RetroAchievements");
	if (index >= 0)
		DeleteMenu(MainMenu, index, MF_BYPOSITION);

	//	##RA embed RA
	AppendMenu(MainMenu, MF_POPUP | MF_STRING, (UINT_PTR)RA_CreatePopupMenu(), TEXT("&RetroAchievements"));

	InvalidateRect(ConsoleHWND(), NULL, TRUE);

	DrawMenuBar(ConsoleHWND());

}

//	sNameOut points to a 64 character buffer.
//	sNameOut should have copied into it the estimated game title 
//	 for the ROM, if one can be inferred from the ROM.
void GetEstimatedGameTitle(char* sNameOut)
{
	//if( emu && emu->get_NES_ROM() )
	//	strcpy_s( sNameOut, 49, emu->get_NES_ROM()->GetRomName() );
}


extern void Machine_Reset(); //machine.c
void ResetEmulation()
{
	Machine_Reset(); // FCEUI_ResetNES();
}

void LoadROMFromEmu(const char* sFullPath)
{
	//FCEUI_LoadGame(sFullPath, 0);
}

//	Installs these shared functions into the DLL
void RA_InitShared()
{
    RA_InstallSharedFunctions( &GameIsActive, &CauseUnpause, &CausePause, &RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadROMFromEmu );
}


//extras
//RAMeka specific helper/refactoring code
//Called from within the emulator

//Handle initial setup of RA integration, including attaching RA Menu to Consolw prompt
void RAMeka_RA_Setup() {

	RAMeka_Stash_Meka_CurrentDirectory();

	RAMeka_MakePlaceholderRAMenu();
	RAMeka_InstallRA(); //after install, CurrentDirectory should be whatever RA uses by default

	//Make a record of RA_rootDir and restore Meka working directory
	RAMeka_Stash_RA_RootDirectory();  
	RAMeka_Restore_Meka_CurrentDirectory();

	ConsolePrintf("%s\n--\n", "RA Init Completed");
}


//Wrapper around RA Call. For neatness.
void RAMeka_RA_InvokeDialog(WPARAM wParam) {

	RA_InvokeDialog(LOWORD(wParam));
}

//Attaches the placeholder RA_Menu to the Meka Win32 startup console window (ConsoleHWND() returns handle to this)
//RA_Integration will use the placeholder to attach its menu items later on
//This is essentially the easierest way for me to do this right now because RA_Integration naturally attaches itself to windows menus, not to Allegro menus.
void RAMeka_MakePlaceholderRAMenu() {

	//See Meka.rc for Console window display parameters.
	//disable close window for console because I am lazy.
	EnableMenuItem(GetSystemMenu(ConsoleHWND(), FALSE), SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	//Make new placeholder menu for console (We will put the RA menu on the console because this is the easiest way for me right now)
	HMENU MainMenu = CreateMenu();

	//Created placeholder menu called "RetroAchievements". RA_Integration will populate this later.
	HMENU RAMenu = CreatePopupMenu();  
	AppendMenu(RAMenu, MF_POPUP | MF_STRING, 100001, "(RA Not Yet Loaded)");
	AppendMenu(MainMenu, MF_STRING | MF_POPUP, (UINT)RAMenu, "&RetroAchievements");
	SetMenu(ConsoleHWND(), MainMenu);
	InvalidateRect(ConsoleHWND(), NULL, TRUE);
	DrawMenuBar(ConsoleHWND());

}

//Various calls to initialise RA, login, install menus, etc
void RAMeka_InstallRA() {

	RA_Init(ConsoleHWND(), RA_Meka, RAMEKA_VERSION);

	RA_InitShared();
	RA_UpdateAppTitle("RAMEKA");

	RebuildMenu();
	RA_HandleHTTPResults();
	RA_AttemptLogin(true);
	RebuildMenu();
}

//Performs needed procedures for shutting down achievement system
void RAMeka_RA_Shutdown() {

	RA_HandleHTTPResults();
	RA_Shutdown();
}

//Call to RA made once every frame
void RAMeka_RA_AchievementsFrameCheck() {
	RA_DoAchievementsFrame();
	RA_HandleHTTPResults();
}

//wrapper around RA_SetPaused
void RAMeka_RA_SetPaused(bool bIsPaused) {

	{
		RAMeka_Stash_Meka_CurrentDirectory();

		RA_SetPaused(bIsPaused);

		RAMeka_Restore_Meka_CurrentDirectory();
	}
}

//Wrapper and directory handling around .rap file load call
void RAMeka_RA_OnSaveStateLoad(char* filename) {

	RAMeka_Stash_Meka_CurrentDirectory();
	RAMeka_Restore_RA_RootDirectory();

	RA_OnLoadState(filename);

	RAMeka_Restore_Meka_CurrentDirectory();

}

//Wrapper and directory handling around .rap file creation call
void RAMeka_RA_OnSaveStateSave(char* filename) {

	RAMeka_Stash_Meka_CurrentDirectory();
	RAMeka_Restore_RA_RootDirectory();

	RA_OnSaveState(filename);

	RAMeka_Restore_Meka_CurrentDirectory();

}

void RAMeka_RA_MountROM( ConsoleID consoleID ) {
    RAMeka_Stash_Meka_CurrentDirectory();
    RAMeka_Restore_RA_RootDirectory();

    RA_ClearMemoryBanks();
    RA_SetConsoleID( consoleID );

    switch ( consoleID )
    {
        case MasterSystem:
            RA_InstallMemoryBank( 0, RAMeka_RAMByteReadFn, RAMeka_RAMByteWriteFn, 0x2000 ); //8KB
            break;
        case GameGear:
            RA_InstallMemoryBank( 0, RAMeka_RAMByteReadFn, RAMeka_RAMByteWriteFn, 0x2000 ); //8KB
            break;
        case Colecovision:
            RA_InstallMemoryBank( 0, RAMeka_RAMByteReadFn, RAMeka_RAMByteWriteFn, 0x400 ); //1KB
            break;
        case SG1000:
            RA_InstallMemoryBank( 0, RAMeka_RAMByteReadFn, RAMeka_RAMByteWriteFn, 0x400 ); //1KB
            break;
    }
    
    RA_OnLoadNewRom( ROM, tsms.Size_ROM );

    RAMeka_Restore_Meka_CurrentDirectory();
    chdir( Meka_currDir );
}

//Code to handle reading RAMeka specific configuration variables
void RAMeka_Config_Load_Line(char *var, char *value) {

	if (!strcmp(var, "overlay_render_method")) {
		if (strcmp(value, "win_layer") == 0) {
			overlay_render_method = OVERLAY_RENDER_WIN_LAYER;
		}
		else if (strcmp(value, "allegro") == 0) {
			overlay_render_method = OVERLAY_RENDER_ALLEGRO;
		}
		else {
			overlay_render_method = OVERLAY_RENDER_ALLEGRO; // default to allegro
		}
		return;
	}

	int input;
	if (!strcmp(var, "disable_ra_overlay")) {  //will be "disable_RA_overlay" in config file
		disable_RA_overlay = (atoi(value) != 0); return;
	}
	if (!strcmp(var, "overlay_frame_skip")) {
		input = atoi(value); input *= (input > 0);
		overlay_frame_skip = input; return;
	}
	if (!strcmp(var, "overlay_alternate_render_blit")) { overlay_alternate_render_blit = (atoi(value) != 0); return; }
	if (!strcmp(var, "overlay_allegro_bg_splits")) {
		input = atoi(value); input *= (input > 0);
		overlay_bg_splits = input; return;
	}

}

//Code to handle saving RAMeka specific configuration variables (without Meka code needing to know about this)
void RAMeka_Configs_Save(CFG_Write_Line_Func CFG_Write_Line, CFG_Write_Int_Func CFG_Write_Int, CFG_Write_Str_Func CFG_Write_Str){

	//See original  (static) definitions of these functions in config.c

	CFG_Write_Line("-----<RETROACHIEVEMENTS SETTINGS -------------------------------------------");

	if (overlay_render_method == OVERLAY_RENDER_WIN_LAYER) {
		CFG_Write_Str("overlay_render_method", "win_layer");
	}
	else {
		CFG_Write_Str("overlay_render_method", "allegro");
	}

	CFG_Write_Int("disable_RA_overlay", (int)disable_RA_overlay);							//
	CFG_Write_Int("overlay_frame_skip", overlay_frame_skip);								//
	CFG_Write_Int("overlay_alternate_render_blit", (int)overlay_alternate_render_blit);	//
	CFG_Write_Int("overlay_allegro_bg_splits", overlay_bg_splits);							//

}

//-----------------------------------------------------------------------
//                RA Overlay Video Functions                            | 
//-----------------------------------------------------------------------

//Functions used to render the RA Spefcific overlay visuals inside the emulator
//See blit.c and video.c for calls
//A lot of code here, but again goal is to make Meka as unaware of RAMeka as possible.

#include "inputs_t.h"
//All constants below are preset defaults, set here, but read in by config.c from mekaw.cfg
static int overlay_render_method = OVERLAY_RENDER_ALLEGRO;
static bool disable_RA_overlay = false;
static int overlay_frame_skip = 0;
static bool overlay_alternate_render_blit = false;
static int overlay_bg_splits = 0; //overridden by mekaw.cfg

static HWND layeredWnd;
static HWND MekaWND;

//Clear up parts of screen which RA overlay draws on but Meka doesn't clear
void RAMeka_Overlay_ClearBackbuffer() {

	//This code is for cosmetic purposes and arguably could be much more efficient
	//Really on the sides of the screen need to be blanked.
	//The idea of only blanking stripes during each frame of is questionable benefit

	//FIX-ME: Change code to ONLY redraw the portions of the screen which we know Meka is not updating

	if (overlay_render_method == OVERLAY_RENDER_ALLEGRO && g_env.state == MEKA_STATE_GAME) {

		//This is far simpler, but we don't really need to blank the entire baackbuffer on every frame.
		//al_set_target_bitmap(al_get_backbuffer(g_display));
		//al_clear_to_color(BORDER_COLOR);

		//Instead, give the user the option of specificying the number of "stripes" the backbuffer
		//is split into and we will blank only one of these stripes every frame.

		//FIX-ME: Currently setting width and height on first run. Will cause issues if Meka changes this
		static int w = al_get_display_width(g_display);
		static int h = al_get_display_height(g_display);
		static int strip_top = h + 1;
		static int strip_bottom = h + 2;

		int dh = h / (overlay_bg_splits + 1); if (dh == 0) dh = 1;


		if (strip_bottom >= h) {
			strip_top = 0;
			strip_bottom = dh;
		}
		else {
			strip_top += dh;
			strip_bottom += dh;
			if (strip_bottom > h) strip_bottom = h;
		}



		//Arguably we don't need to clear the whole screen here as the overlay code doesn't require this much attention
		//Could clear fractions of the display backbuffer instead, or only do this every so many frames
		//We're emulating retro (60Hz) for millisecond matter here. May need ot look into this again (When git and VS aren't eating my code)
		al_set_target_bitmap(al_get_backbuffer(g_display));
		//al_set_target_bitmap(al_get_backbuffer(fs_out));

		//al_clear_to_color(BORDER_COLOR);
		al_draw_filled_rectangle(0, strip_top, w, strip_bottom, BORDER_COLOR);
	}
}


void RAMeka_Overlay_RenderAchievementOverlays() {

	if (disable_RA_overlay) { al_flip_display(); return; }

	if (g_env.state == MEKA_STATE_SHUTDOWN) { al_flip_display(); return; } // should be here

	if (overlay_render_method == OVERLAY_RENDER_WIN_LAYER) {
		al_flip_display();
		RAMeka_Overlay_RenderAchievementOverlays_WIN_LAYER();
	}
	else { //render allegro by default
		RAMeka_Overlay_RenderAchievementOverlays_ALLEGRO_OVERLAY();
		al_flip_display();
		//PROFILE_STEP("al_flip_display");
	}

}


void RAMeka_Overlay_RenderAchievementOverlays_WIN_LAYER() {
	//#RA:
	//So alegro is finished flipping screenbuffers?
	//So we can draw the overlays now, right?
	//WARNING: Ugly Hack
	MekaWND = al_get_win_window_handle(g_display);

	RECT rect;
	GetClientRect(MekaWND, &rect);

	char meka_currDir[2048];
	GetCurrentDirectory(2048, meka_currDir); // "where'd you get the multithreaded code, Ted?"

											 // Initialize layered window
	if (layeredWnd == NULL)
	{
		//Set up window class
		WNDCLASSEX wndEx;
		memset(&wndEx, 0, sizeof(wndEx));
		wndEx.cbSize = sizeof(wndEx);
		wndEx.lpszClassName = "RA_WND_CLASS";
		wndEx.lpfnWndProc = RAMeka_RAWndProc;
		wndEx.hInstance = (HINSTANCE)GetWindowLong(MekaWND, GWL_HINSTANCE);
		int result = RegisterClassEx(&wndEx);

		// Create Window. WS_POPUP style is important so window displays without any borders or toolbar;
		layeredWnd = CreateWindowEx(
			(WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | WS_EX_LAYERED),
			wndEx.lpszClassName,
			"RAWnd",
			(WS_POPUP),
			CW_USEDEFAULT, CW_USEDEFAULT, rect.right, rect.bottom,
			MekaWND, NULL, wndEx.hInstance, NULL);

		//SetParent(MekaWND, layeredWnd);

		ShowWindow(layeredWnd, SW_SHOWNOACTIVATE);
	}
	else
	{
		// Set up buffer and back buffer
		HDC hdc = GetDC(MekaWND);

		static HDC hdcMem = NULL;
		static HBITMAP hBmp = NULL;
		static HBITMAP hBmpOld = NULL;
		if (!hdcMem) {
			hdcMem = CreateCompatibleDC(hdc);
			hBmp = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
			hBmpOld = (HBITMAP)SelectObject(hdcMem, hBmp);
		}

		// Blits the MekaWND to the back buffer.
		BitBlt(hdcMem, 0, 0, rect.right, rect.bottom, hdc, 0, 0, SRCCOPY);

		// Update RA stuff
		RAMeka_Overlay_UpdateOverlay(hdcMem, rect);

		// Actually draw to the back buffer
		// Not familiar with BLENDFUNCTION, may not be needed.
		BLENDFUNCTION blend = { 0 };
		blend.BlendOp = AC_SRC_OVER;
		blend.SourceConstantAlpha = 255;
		blend.AlphaFormat = AC_SRC_OVER;
		POINT ptSrc = { 0, 0 };
		SIZE sizeWnd = { rect.right, rect.bottom };
		UpdateLayeredWindow(layeredWnd, hdc, NULL, &sizeWnd, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

		// Get position of the client rect. (NOT the window rect)
		ClientToScreen(MekaWND, reinterpret_cast<POINT*>(&rect.left));
		ClientToScreen(MekaWND, reinterpret_cast<POINT*>(&rect.right));

		// Move layered window over MekaWND.
		SetWindowPos(layeredWnd, 0, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
		SetWindowPos(MekaWND, layeredWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE); // Don't think this line is necessary on most OS, but just safety net.

																								 //SelectObject(hdcMem, hBmpOld);
																								 //DeleteObject(hBmp);
																								 //DeleteDC(hdcMem);
		ReleaseDC(MekaWND, hdc);
	}

	SetCurrentDirectory(meka_currDir); // "Cowboys Ted! They're a bunch of cowboys!"
}


LRESULT CALLBACK RAMeka_RAWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_SIZE:
		return SIZE_RESTORED;
	case WM_NCHITTEST:
		return HTNOWHERE;

	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
}

void RAMeka_Overlay_RenderAchievementOverlays_ALLEGRO_OVERLAY() {
	static int frames_skipped = 0;

	static int display_width = -1;
	static int display_height = -1;
	static int bpp = 32; // Bits per pixel. Don't change this. The hack won't work without it
	static int stride = -1;

	HWND MekaWND = al_get_win_window_handle(g_display);
	HDC hDC = GetDC(MekaWND);
	RECT rcSize;
	GetClientRect(MekaWND, &rcSize);

	static HDC			hdcMem = NULL;
	static HBITMAP      hbmMem = NULL;
	static HANDLE		hbmOld = NULL;
	static HBRUSH		hbrBkGnd;

	static BITMAPV5HEADER bmh;
	static BYTE *pBits = NULL; // WINAPI pixel array. Needs to be initalised or debugger complains.
	static ALLEGRO_BITMAP *bitmap = NULL;
	static ALLEGRO_LOCKED_REGION *lock = NULL;

	//The color value which is set to transparent when we blit into allegro. All elements of the overlay which are this color will be made transparent.
	//Note: Currently set to modified magic magenta. 
	static DWORD mask = 0x00fe01fd;
	static DWORD win_mask = (mask << 16) & 0x00ff0000   //BYTE order shenanigans
		| (mask >> 16) & 0x000000ff | mask & 0x0000ff00;

	if (!hdcMem) {  //WARNING: Not currently checking or destroying this anywhere so if Meka goes to ACTUAL Alt+Enter Fullscreen mode we are hosed

		display_width = al_get_display_width(g_display);
		display_height = al_get_display_height(g_display);
		stride = (display_width * (bpp / 8));

		memset(&bmh, 0, sizeof(BITMAPV5HEADER));		bmh.bV5Size = sizeof(BITMAPV5HEADER);
		bmh.bV5Width = display_width;					bmh.bV5Height = display_height;
		bmh.bV5Planes = 1;								bmh.bV5BitCount = bpp;
		bmh.bV5Compression = BI_RGB;
		/*bmh.bV5AlphaMask = 0xFF000000;		bmh.bV5RedMask = 0x00FF0000;		bmh.bV5GreenMask = 0x0000FF00;		bmh.bV5BlueMask = 0x000000FF;*/ // Probably don't need these really

																																						// Create an off-screen WINAPI bitmap for RA_dll to draw to
		hdcMem = CreateCompatibleDC(hDC);
		hbmMem = CreateDIBSection(hDC, (BITMAPINFO *)&bmh, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
		hbrBkGnd = CreateSolidBrush((DWORD)win_mask); //magic magenta mask
		hbmOld = SelectObject(hdcMem, hbmMem);

		//Create ALLEGRO bitmap we must transfer the windows bit to and then draw in allegro
		//This format must be ARGB for this to work (Not unsetting here but meka seems to use XRGB internally anyway)
		al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ARGB_8888);
		bitmap = al_create_bitmap(display_width, display_height);

	} //End bitmaps creation code (Only run once at the moment)


	if (frames_skipped  < overlay_frame_skip) {
		frames_skipped++; //don't update the overlay buffers this frame
	}
	else { //update overlays
		frames_skipped = 0;


		static bool UPDATE_OVERLAY = false;
		static bool COPY_OVERLAY = true; //for now. Decide whether to alternate blits based on overlay_alternate_render_blit

										 //if overlay_alternate_render_blit ==0 then both updates are true. Otherwise each flips its truth value
		UPDATE_OVERLAY = !overlay_alternate_render_blit | !UPDATE_OVERLAY;
		COPY_OVERLAY = !overlay_alternate_render_blit | !COPY_OVERLAY;                           //a smart compiler should use !(a&b) instead of !a|!b but code must be more readable.

		if (UPDATE_OVERLAY)
		{
			//First let RA Draw the pixels to the WINAPI buffer
			FillRect(hdcMem, &rcSize, hbrBkGnd);
			//RA_UpdateRenderOverlay(hdcMem, &input, ((float)nDelta / 1000.0f), &rcSize, meka_fullscreen, meka_paused);
			RAMeka_Overlay_UpdateOverlay(hdcMem, rcSize);
		}


		/* Now we do something very stupid and hacky
		....................................................
		......................```...........................
		...............```...:////-`........................
		...............-:+o++syyhyy+-.`.....................
		..........-../syyhdhyhdhhhhhyys+/:-`................
		.........`./sshddddddhhhhhhhhhhhhyso/...............
		......``-+yyyhhhdddddhhyyhhhddhhdddhys-.............
		......-/yhhhhhdhddddhhhyhhhdhdddddddddh.............
		.....`/shhddhhddhhhhhhhhyhhhhdddmmdmddm+............
		....:-osyhhddhhhdhhhhddddhhdddddmddmmmmdo...........
		...../oosyhhhhhhhyyhddddddmmmmmmmmmmmmmmd-..........
		.....+ooosyhhhhhhhhhdddmmmmmddddmmNNNNmmmo..........
		...`:+oosyhdhhddddddddmdmmmmmdhddNNNmNNNNd-.........
		...`osyydmmddddhyyyhdmdmmmddmmhhdmNNNNNNNNd`.`......
		....````hNNNmmddhysshddhhhyshhshdmmNNNNNNNm:oNNdy/`.
		.........sdmdhhhyssyyyyyhhyyhyyhdmmmmNNmmmNNNNNMMMNo
		........./ssyhhysyyyyyhhhhhhyhdmddddmmddymNNNMMMMMMM
		.........+yyyyyyyhhhhhhhdddddhhhdddddhssmNNNMMMMMMNm
		.........+ssyyhhhhhhhhhddddhhddddhhdssmNNNNMMMMMNNNN
		.........:ydmmdhhhhhhhhddddhhdddhdhsdMNNNNMMMMNNNNNN
		...........```/syhhhhhhdddhhddddddmMNNNNNMMNNNNNNNNN
		.................:hdhhhhdddddmmNNMMNNNNNMMNNNNNNNNNN
		................./shhhhdddmmNNNNMMNNNNMMMNNNNNNNNNNN
		..................`/syhddmmdhmNNNNNNMMMNNNNNNNNNNNNN
		....................:ohs++:`.`+mNNNMMMNNNNNNNNNNNNNN
		............................`+hNNNMMMNNNNNNNNNNNNNNN
		"A funeral! You let Dougal do a funeral!!"
		*/
		if (COPY_OVERLAY)
		{
			//		int x, y;// , y;
			int pitch;
			int w = display_width;
			int h = display_height;
			uint8_t *dst;
			static DWORD *pPixels = (DWORD*) &(*pBits); //Represent BYTE array as array of 4 byte RGBA DWORD array

														//The alpha value of all the bits had better be zero after WINAPI is done or ... nothing will happen here.
			for (int pixel = 0; pixel < w*h; pixel++) {
				if (pPixels[pixel] != mask)   //there is probably some gosu way of doing this in one SIMD operation but I don't know.
					pPixels[pixel] += 0xff000000;
				//Is the system little endian or big endian? If colors are being inverted you know what is wrong now.
			}

			pitch = w * BYTES_PER_PIXEL(bpp);
			pitch = (pitch + 3) & ~3;  /* align on dword */

									   //must use ARGB format here. For the life of me I don't understand why this isn't RGBA. or ABGR. I think windows is internally returning 0xAARRGGBB corresponding to BGR. Never mind it works 
			lock = al_lock_bitmap(bitmap, ALLEGRO_PIXEL_FORMAT_ARGB_8888, ALLEGRO_LOCK_WRITEONLY);
			dst = (uint8_t *)lock->data;

			//Copy the WINAPI bitmap bits into the ALLEGRO bitmap's buffer.
			memcpy(dst, pBits, w*h*BYTES_PER_PIXEL(bpp));
			al_unlock_bitmap(bitmap);

		}							// "You have used three inches of sticky tape, God bless you"
	}


	//must still draw existing buffers even if update was skipped.

	//[02:26] <+SiegeLord> Allegro assumes pre - multiplied alpha channel
	//[02:27] <+SiegeLord> While what you have uses the non - pre multiplied alpha channel
	//So we need to switch blender types for drawing transparent bitmaps
	int old_op; int old_src; int old_dst;
	al_get_blender(&old_op, &old_src, &old_dst); // get current state of blender
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);

	//finally we draw to the display backbuffer (remember al_flip_display() still needs to be called
	al_draw_bitmap(bitmap, 0, 0, ALLEGRO_FLIP_VERTICAL);
	al_set_blender(old_op, old_src, old_dst); // put this back the way we found it or its white out time

	ReleaseDC(MekaWND, hDC);
}


void RAMeka_Overlay_UpdateOverlay(HDC hdc, RECT rect)
{


	static int nOldTime = GetTickCount(); //Time in ms I presume

	int nDelta;
	nDelta = GetTickCount() - nOldTime;
	nOldTime = GetTickCount();

	ControllerInput input; // This has to be passed to the overlay

						   //Just taking input from standard keyboard because Meka controller input is super wierd
						   //Note: Not eating allegro key inputs (FALSE)
	input.m_bUpPressed = Inputs_KeyPressed(ALLEGRO_KEY_UP, FALSE);
	input.m_bDownPressed = Inputs_KeyPressed(ALLEGRO_KEY_DOWN, FALSE);
	input.m_bLeftPressed = Inputs_KeyPressed(ALLEGRO_KEY_LEFT, FALSE);
	input.m_bRightPressed = Inputs_KeyPressed(ALLEGRO_KEY_RIGHT, FALSE);
	input.m_bCancelPressed = Inputs_KeyPressed(ALLEGRO_KEY_B, FALSE); //
	input.m_bConfirmPressed = Inputs_KeyPressed(ALLEGRO_KEY_A, FALSE); // I think these match the interface
	input.m_bQuitPressed = Inputs_KeyPressed(ALLEGRO_KEY_ENTER, FALSE);

	bool meka_paused; meka_paused = (g_machine_flags & MACHINE_PAUSED);
	bool meka_fullscreen; meka_fullscreen = FALSE; // just going to set this

	RA_UpdateRenderOverlay(hdc, &input, ((float)nDelta / 1000.0f), &rect, meka_fullscreen, meka_paused);
}

