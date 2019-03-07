@echo off

git describe --tags --match "RAppleWin.*" > Temp.txt
set /p ACTIVE_TAG=<Temp.txt
set VERSION_NUM=%ACTIVE_TAG:~10,4%
set VERSION_REVISION=%ACTIVE_TAG:~15,-9%
if "%VERSION_REVISION%"=="" set VERSION_REVISION=0

setlocal
git diff HEAD > Temp.txt
for /F "usebackq" %%A in ('"Temp.txt"') do set DIFF_FILE_SIZE=%%~zA
if %DIFF_FILE_SIZE% GTR 0 (
    set ACTIVE_TAG=Unstaged changes
    set VERSION_MODIFIED=1
) else (
    set VERSION_MODIFIED=0
)

@echo Tag: %ACTIVE_TAG% (%VERSION_NUM%)
@echo #define RAPPLEWIN_VERSION "%VERSION_NUM%.%VERSION_REVISION%.%VERSION_MODIFIED%" > BuildVer2.h
@echo #define RAPPLEWIN_VERSION_SHORT "%VERSION_NUM%" >> BuildVer2.h

if not exist source\BuildVer.h goto nonexistant
fc source\BuildVer.h BuildVer2.h > nul
if errorlevel 1 goto different
del BuildVer2.h
goto done
:different
del source\BuildVer.h
:nonexistant
move BuildVer2.h source\BuildVer.h > nul
:done

del Temp.txt
