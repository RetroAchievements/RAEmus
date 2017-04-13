!include Makefile.inc

!IF "$(RELEASE)"=="1"
OUTPATH=release
!ELSE
OUTPATH=debug
!ENDIF

TEMPPATH=$(OUTPATH)\temp
GENSPATH=$(OUTPATH)\gens
SYMPATH=$(OUTPATH)\symbols
TOOLSPATH=tools
SRCPATH=src
SRCCOMMONPATH=..\common\src
SRC_RA=..\..\RA_Integration
DX_LIBS="..\..\DXSDK_March09_x86"
DX_SDK="..\..\DXSDK_March09_x86"

PREBUILD=MakeBuildVer.bat

!IF "$(RELEASE)"=="1"
POSTBUILD=copy $(SRC_RA)\RA_Integration.dll . && copy $(SRC_RA)\*.pdb .
!ELSE
POSTBUILD=copy $(SRC_RA)\RA_Integration_d.dll . && copy $(SRC_RA)\*.pdb .
!ENDIF

!IF [if not exist $(GENSPATH) mkdir $(GENSPATH)]
!ERROR Could not create path "$(GENSPATH)".
!ENDIF

!IF [if not exist $(TEMPPATH) mkdir $(TEMPPATH)]
!ERROR Could not create path "$(TEMPPATH)".
!ENDIF

!IF [if not exist $(SYMPATH) mkdir $(SYMPATH)]
!ERROR Could not create path "$(SYMPATH)".
!ENDIF

!IF "$(RELEASE)"=="1" && "$(RELEASEDEBUG)"!="1"
LINK_FLAGS=/RELEASE
!ELSE
LINK_FLAGS=/DEBUG
!ENDIF
LINK=link.exe /INCREMENTAL:NO /NOLOGO /MACHINE:X86 /LIBPATH:$(SRCPATH) /LIBPATH:$(SRC_RA) /LIBPATH:$(DX_LIBS) $(LINK_FLAGS)

NASM=$(TOOLSPATH)\nasmw.exe -f win32 -i$(SRCCOMMONPATH) -i$(SRCPATH)

!IF "$(RELEASE)"=="1"
RC_FLAGS=/d "NDEBUG"
!ELSE
RC_FLAGS=
!ENDIF
RC=rc.exe /l 0x40c $(RC_FLAGS)

!IF "$(RELEASE)"=="1"
CC_FLAGS=/Ox /Zi /D "NDEBUG" /MT
!ELSE
CC_FLAGS=/Od /Z7 /D "_DEBUG" /MTd
!ENDIF
CC=cl.exe /c /nologo /EHsc /W3 /D "WIN32" /D "_MBCS" /D "RA_GENS" /D "_CRT_SECURE_NO_WARNINGS" /D _USING_V110_SDK71_ /wd4101 /wd4244 /wd4018 /wd4305 /I$(SRCCOMMONPATH) /I$(SRC_RA) /I$(SRCPATH) /I$(DX_SDK) $(CC_FLAGS)

OBJS = \
	$(TEMPPATH)\blit.obj \
	$(TEMPPATH)\mem_m68k.obj \
	$(TEMPPATH)\mem_s68k.obj \
	$(TEMPPATH)\mem_sh2.obj \
	$(TEMPPATH)\mem_z80.obj \
	$(TEMPPATH)\misc.obj \
	$(TEMPPATH)\sh2a.obj \
	$(TEMPPATH)\gfx_cd.obj \
	$(TEMPPATH)\io.obj \
	$(TEMPPATH)\pwm.obj \
	$(TEMPPATH)\vdp_32x.obj \
	$(TEMPPATH)\vdp_io.obj \
	$(TEMPPATH)\vdp_rend.obj \
	$(TEMPPATH)\z80.obj \
	$(TEMPPATH)\ccnet.obj \
	$(TEMPPATH)\cpu_68k.obj \
	$(TEMPPATH)\cpu_sh2.obj \
	$(TEMPPATH)\cpu_z80.obj \
	$(TEMPPATH)\lc89510.obj \
	$(TEMPPATH)\m68kd.obj \
	$(TEMPPATH)\sh2.obj \
	$(TEMPPATH)\sh2d.obj \
	$(TEMPPATH)\cd_file.obj \
	$(TEMPPATH)\cd_sys.obj \
	$(TEMPPATH)\cdda_mp3.obj \
	$(TEMPPATH)\unzip.obj \
	$(TEMPPATH)\wave.obj \
	$(TEMPPATH)\ym2612.obj \
	$(TEMPPATH)\z80dis.obj \
	$(TEMPPATH)\common.obj \
	$(TEMPPATH)\tabinit.obj \
	$(TEMPPATH)\dct64_i386.obj \
	$(TEMPPATH)\decode_i386.obj \
	$(TEMPPATH)\interface.obj \
	$(TEMPPATH)\layer3.obj \
	$(TEMPPATH)\scrshot.obj \
	$(TEMPPATH)\save.obj \
	$(TEMPPATH)\net.obj \
	$(TEMPPATH)\ggenie.obj \
	$(TEMPPATH)\rom.obj \
	$(TEMPPATH)\cd_aspi.obj \
	$(TEMPPATH)\debug.obj \
	$(TEMPPATH)\g_ddraw.obj \
	$(TEMPPATH)\g_dinput.obj \
	$(TEMPPATH)\g_dsound.obj \
	$(TEMPPATH)\g_main.obj \
	$(TEMPPATH)\gens.obj \
	$(TEMPPATH)\gens.res \
	$(TEMPPATH)\sub68k.obj \
	$(TEMPPATH)\main68k.obj \
	$(TEMPPATH)\psg.obj \
	$(TEMPPATH)\pcm.obj \
	$(TEMPPATH)\DialogProc.obj \
	$(TEMPPATH)\RA_Interface.obj \
	$(TEMPPATH)\RA_Implementation.obj


GENS_LIBS = \
	wsock32.lib \
	ddraw.lib \
	dsound.lib \
	dinput8.lib \
	dxguid.lib \
	winmm.lib \
	comdlg32.lib \
	user32.lib \
	gdi32.lib \
	advapi32.lib \
	htmlhelp.lib \
	zlib.lib \
	windowscodecs.lib \
	Winhttp.lib \
	comctl32.lib \
	shell32.lib \
	kernel32.lib \
	winspool.lib \
	comdlg32.lib \
	advapi32.lib \
	shell32.lib \
	ole32.lib \
	oleaut32.lib \
	uuid.lib \
	odbc32.lib \
	odbccp32.lib

STARSCREAM_LIBS = \
	odbc32.lib \
	odbccp32.lib \
	kernel32.lib \
	user32.lib \
	gdi32.lib \
	winspool.lib \
	comdlg32.lib \
	advapi32.lib \
	shell32.lib \
	ole32.lib \
	oleaut32.lib \
	uuid.lib \
	odbc32.lib \
	odbccp32.lib

all: pre-build gens

pre-build:
	$(PREBUILD)

gens: $(GENSPATH)\RAGens.exe
	copy /Y $(SRCCOMMONPATH)\license.txt $(GENSPATH)\LICENSE.txt
	$(POSTBUILD)

$(GENSPATH)\RAGens.exe: $(OBJS)
	$(LINK) /SUBSYSTEM:WINDOWS /NODEFAULTLIB:libc.lib /OUT:"$@" /PDB:"$(SYMPATH)\$(@B).pdb" $(GENS_LIBS) $**
    $(_VC_MANIFEST_EMBED_EXE)

	
clean:
	if exist $(OUTPATH) rd /s /q $(OUTPATH)
	if exist $(TEMPPATH) rd /s /q $(TEMPPATH)
    $(_VC_MANIFEST_CLEAN)

$(TEMPPATH)\main68k.obj: $(TEMPPATH)\main68k.asm
	$(NASM) $** -o $@

$(TEMPPATH)\sub68k.obj: $(TEMPPATH)\sub68k.asm
	$(NASM) $** -o $@

$(TEMPPATH)\main68k.asm: $(TEMPPATH)\main68kgen.exe
	$** $@ -hog -name main68k_

$(TEMPPATH)\sub68k.asm: $(TEMPPATH)\sub68kgen.exe
	$** $@ -hog -name sub68k_

$(TEMPPATH)\main68kgen.exe: $(TEMPPATH)\main68kgen.obj
	$(LINK) /SUBSYSTEM:CONSOLE /OUT:"$@" /PDB:"$(SYMPATH)\$(@B).pdb" $(STARSCREAM_LIBS) $**
	
$(TEMPPATH)\main68kgen.obj: $(SRCCOMMONPATH)\main68kgen.c
	$(CC) /TP /D "_CONSOLE" /Fo"$@" /TP $**
	
$(TEMPPATH)\sub68kgen.exe: $(TEMPPATH)\sub68kgen.obj
	$(LINK) /SUBSYSTEM:CONSOLE /OUT:"$@" /PDB:"$(SYMPATH)\$(@B).pdb" $(STARSCREAM_LIBS) $**
	
$(TEMPPATH)\sub68kgen.obj: $(SRCCOMMONPATH)\sub68kgen.c
	$(CC) /TP /D "_CONSOLE" /Fo"$@" /TP $**
	
{$(SRCPATH)}.rc{$(TEMPPATH)}.res:
	$(RC) /Fo"$@" $< 

{$(SRC_RA)}.rc{$(TEMPPATH)}.res:
	$(RC) /Fo"$@" $< 
	
{$(SRC_RA)}.cpp{$(TEMPPATH)}.obj:
	$(CC) /TP /Fo"$@" $<
	
{$(SRCCOMMONPATH)}.c{$(TEMPPATH)}.obj:
	$(CC) /TC /Fo"$@" $<
	
{$(SRCCOMMONPATH)}.cpp{$(TEMPPATH)}.obj:
	$(CC) /TP /Fo"$@" $<
	
{$(SRCCOMMONPATH)}.asm{$(TEMPPATH)}.obj:
	$(NASM) $< -o $@
	
!include Makefile.targ.inc
