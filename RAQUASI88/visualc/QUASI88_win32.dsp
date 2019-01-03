# Microsoft Developer Studio Project File - Name="QUASI88_win32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=QUASI88_win32 - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "QUASI88_win32.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "QUASI88_win32.mak" CFG="QUASI88_win32 - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "QUASI88_win32 - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE "QUASI88_win32 - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "QUASI88_win32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "win32/Debug"
# PROP Intermediate_Dir "win32/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\src" /I "..\src\FWIN" /I "..\src\WIN32" /I "..\src\snddrv" /I "..\src\snddrv\quasi88" /I "..\src\snddrv\src" /I "..\src\snddrv\src\sound" /I "..\src\fmgen" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USE_SOUND" /D "USE_FMGEN" /D "DEBUGPRINTF" /Fp"win32/Debug/QUASI88win32.pch" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG" /d "USE_SOUND" /d "USE_FMGEN"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"win32/Debug/QUASI88win32.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib imm32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /out:"win32/Debug/QUASI88win32.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "QUASI88_win32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "win32/Release"
# PROP Intermediate_Dir "win32/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\src" /I "..\src\FWIN" /I "..\src\WIN32" /I "..\src\snddrv" /I "..\src\snddrv\quasi88" /I "..\src\snddrv\src" /I "..\src\snddrv\src\sound" /I "..\src\fmgen" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USE_SOUND" /D "USE_FMGEN" /Fp"win32/Release/QUASI88win32.pch" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG" /d "USE_SOUND" /d "USE_FMGEN"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"win32/Release/QUASI88win32.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib imm32.lib winmm.lib /nologo /subsystem:windows /machine:I386 /out:"win32/Release/QUASI88win32.exe"

!ENDIF 

# Begin Target

# Name "QUASI88_win32 - Win32 Debug"
# Name "QUASI88_win32 - Win32 Release"
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

SOURCE=..\src\WIN32\audio.c
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

SOURCE=..\src\WIN32\event.c
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

SOURCE=..\src\WIN32\graph.c
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

SOURCE=..\src\WIN32\main.c
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

SOURCE=..\src\WIN32\menubar.c
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

SOURCE=..\src\WIN32\wait.c
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
# Begin Source File

SOURCE=..\src\WIN32\quasi88.rc
# End Source File
# End Group
# End Target
# End Project
