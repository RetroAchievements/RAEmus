# Microsoft Developer Studio Project File - Name="QUASI88_sdl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=QUASI88_sdl - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "QUASI88_sdl.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "QUASI88_sdl.mak" CFG="QUASI88_sdl - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "QUASI88_sdl - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE "QUASI88_sdl - Win32 Monitor" ("Win32 (x86) Application" 用)
!MESSAGE "QUASI88_sdl - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "QUASI88_sdl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "sdl/Debug"
# PROP Intermediate_Dir "sdl/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "..\sdl-devel\include" /I "..\src" /I "..\src\FWIN" /I "..\src\SDL" /I "..\src\snddrv" /I "..\src\snddrv\quasi88" /I "..\src\snddrv\quasi88\SDL" /I "..\src\snddrv\src" /I "..\src\snddrv\src\sound" /I "..\src\fmgen" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "QUASI88_SDL" /D "USE_SOUND" /D "USE_FMGEN" /D "SUPPORT_DOUBLE" /D "USE_MONITOR" /Fp"sdl/Debug/QUASI88.pch" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"sdl/Debug/QUASI88.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"sdl/Debug/QUASI88.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "QUASI88_sdl - Win32 Monitor"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Monitor"
# PROP BASE Intermediate_Dir "Monitor"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "sdl/Monitor"
# PROP Intermediate_Dir "sdl/Monitor"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\sdl-devel\include" /I "..\src" /I "..\src\FWIN" /I "..\src\SDL" /I "..\src\snddrv" /I "..\src\snddrv\quasi88" /I "..\src\snddrv\quasi88\SDL" /I "..\src\snddrv\src" /I "..\src\snddrv\src\sound" /I "..\src\fmgen" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "QUASI88_SDL" /D "USE_SOUND" /D "USE_FMGEN" /D "SUPPORT_DOUBLE" /D "USE_MONITOR" /Fp"sdl/Monitor/QUASI88mon.pch" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"sdl/Monitor/QUASI88mon.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"sdl/Monitor/QUASI88mon.exe"

!ELSEIF  "$(CFG)" == "QUASI88_sdl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "sdl/Release"
# PROP Intermediate_Dir "sdl/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\sdl-devel\include" /I "..\src" /I "..\src\FWIN" /I "..\src\SDL" /I "..\src\snddrv" /I "..\src\snddrv\quasi88" /I "..\src\snddrv\quasi88\SDL" /I "..\src\snddrv\src" /I "..\src\snddrv\src\sound" /I "..\src\fmgen" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QUASI88_SDL" /D "USE_SOUND" /D "USE_FMGEN" /D "SUPPORT_DOUBLE" /Fp"sdl/Release/QUASI88.pch" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"sdl/Release/QUASI88.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"sdl/Release/QUASI88.exe"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "QUASI88_sdl - Win32 Debug"
# Name "QUASI88_sdl - Win32 Monitor"
# Name "QUASI88_sdl - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\snddrv\quasi88\2203fmgen.cpp
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\sound\2203intf.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\quasi88\2608fmgen.cpp
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\sound\2608intf.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\quasi88\SDL\audio.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\sound\ay8910.c
# End Source File
# Begin Source File

SOURCE=..\src\basic.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\quasi88\beep.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\quasi88\beepintf.c
# End Source File
# Begin Source File

SOURCE=..\src\crtcdmac.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\driver.c
# End Source File
# Begin Source File

SOURCE=..\src\emu.c
# End Source File
# Begin Source File

SOURCE=..\src\SDL\event.c
# End Source File
# Begin Source File

SOURCE=..\src\fdc.c
# End Source File
# Begin Source File

SOURCE="..\src\FWIN\file-op.c"
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\sound\flt_rc.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\sound\flt_vol.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\sound\fm.c
# End Source File
# Begin Source File

SOURCE=..\src\fmgen\fmgen.cpp
# End Source File
# Begin Source File

SOURCE=..\src\fmgen\fmtimer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\getconf.c
# End Source File
# Begin Source File

SOURCE=..\src\SDL\graph.c
# End Source File
# Begin Source File

SOURCE=..\src\image.c
# End Source File
# Begin Source File

SOURCE=..\src\intr.c
# End Source File
# Begin Source File

SOURCE=..\src\keyboard.c
# End Source File
# Begin Source File

SOURCE=..\src\SDL\main.c
# End Source File
# Begin Source File

SOURCE="..\src\snddrv\quasi88\mame-quasi88.c"
# End Source File
# Begin Source File

SOURCE=..\src\memory.c
# End Source File
# Begin Source File

SOURCE="..\src\menu-screen.c"
# End Source File
# Begin Source File

SOURCE=..\src\menu.c
# End Source File
# Begin Source File

SOURCE=..\src\monitor.c
# End Source File
# Begin Source File

SOURCE=..\src\fmgen\opna.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pause.c
# End Source File
# Begin Source File

SOURCE=..\src\pc88main.c
# End Source File
# Begin Source File

SOURCE=..\src\pc88sub.c
# End Source File
# Begin Source File

SOURCE=..\src\pio.c
# End Source File
# Begin Source File

SOURCE=..\src\fmgen\psg.cpp
# End Source File
# Begin Source File

SOURCE="..\src\q8tk-glib.c"
# End Source File
# Begin Source File

SOURCE=..\src\q8tk.c
# End Source File
# Begin Source File

SOURCE=..\src\quasi88.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\restrack.c
# End Source File
# Begin Source File

SOURCE=..\src\romaji.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\sound\samples.c
# End Source File
# Begin Source File

SOURCE="..\src\screen-16bpp.c"
# End Source File
# Begin Source File

SOURCE="..\src\screen-32bpp.c"
# End Source File
# Begin Source File

SOURCE="..\src\screen-8bpp.c"
# End Source File
# Begin Source File

SOURCE="..\src\screen-snapshot.c"
# End Source File
# Begin Source File

SOURCE=..\src\screen.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\quasi88\SDL\sdl.c
# End Source File
# Begin Source File

SOURCE=..\src\snapshot.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\sndintrf.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\sound.c
# End Source File
# Begin Source File

SOURCE=..\src\soundbd.c
# End Source File
# Begin Source File

SOURCE=..\src\status.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\streams.c
# End Source File
# Begin Source File

SOURCE=..\src\suspend.c
# End Source File
# Begin Source File

SOURCE=..\src\SDL\wait.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\sound\wavwrite.c
# End Source File
# Begin Source File

SOURCE=..\src\snddrv\src\sound\ymdeltat.c
# End Source File
# Begin Source File

SOURCE="..\src\z80-debug.c"
# End Source File
# Begin Source File

SOURCE=..\src\z80.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE="..\sdl-devel\lib\x86\SDL.lib"
# End Source File
# Begin Source File

SOURCE="..\sdl-devel\lib\x86\SDLmain.lib"
# End Source File
# End Target
# End Project
