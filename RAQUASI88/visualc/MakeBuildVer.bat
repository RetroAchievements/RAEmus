@echo off

git describe --tags --match "RAQUASI88.*" > Temp.txt
set /p ACTIVE_TAG=<Temp.txt
set VERSION_TAG=%ACTIVE_TAG:~10%
for /f "tokens=1,2 delims=-" %%a in ("%VERSION_TAG%") do set VERSION_NUM=%%a&set VERSION_REVISION=%%b
if "%VERSION_REVISION%" == "" set VERSION_REVISION=0

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
@echo #define RAQUASI88_VERSION "%VERSION_NUM%.%VERSION_REVISION%.%VERSION_MODIFIED%" > BuildVer2.h
@echo #define RAQUASI88_VERSION_SHORT "%VERSION_NUM%" >> BuildVer2.h

if not exist ..\src\WIN32\BuildVer.h goto nonexistant
fc ..\src\WIN32\BuildVer.h BuildVer2.h > nul
if errorlevel 1 goto different
del BuildVer2.h
goto done
:different
del ..\src\WIN32\BuildVer.h
:nonexistant
move BuildVer2.h ..\src\WIN32\BuildVer.h > nul
:done

del Temp.txt
