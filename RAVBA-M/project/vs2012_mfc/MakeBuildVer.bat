@echo off

git describe --tags --match "RAVBA-M.*" > Temp.txt
set /p ACTIVE_TAG=<Temp.txt
set VERSION_NUM=%ACTIVE_TAG:~8,3%
set VERSION_REVISION=%ACTIVE_TAG:~12,-9%
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
@echo #define RAVBA_VERSION "0.%VERSION_NUM%.%VERSION_REVISION%.%VERSION_MODIFIED%" > BuildVer2.h

if not exist ..\..\src\BuildVer.h goto nonexistant
fc ..\..\src\BuildVer.h BuildVer2.h > nul
if errorlevel 1 goto different
del BuildVer2.h
goto done
:different
del ..\..\src\BuildVer.h
:nonexistant
move BuildVer2.h ..\..\src\BuildVer.h > nul
:done

del Temp.txt
