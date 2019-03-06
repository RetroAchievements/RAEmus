@echo off

rem === Get the most recent tag matching our prefix ===
git describe --tags --match "RAProject64.*" > Temp.txt
set /p ACTIVE_TAG=<Temp.txt
for /f "tokens=1,2 delims=-" %%a in ("%ACTIVE_TAG:~12%") do set VERSION_TAG=%%a&set VERSION_REVISION=%%b
if "%VERSION_REVISION%" == "" set VERSION_REVISION=0

rem === Extract the major/minor/patch version from the tag (append 0s if necessary) ===
for /f "tokens=1,2,3 delims=." %%a in ("%VERSION_TAG%.0.0") do set VERSION_NUM=%%a.%%b.%%c

rem === If there are any local modifications, increment revision ===
setlocal
git diff HEAD > Temp.txt
for /F "usebackq" %%A in ('"Temp.txt"') do set DIFF_FILE_SIZE=%%~zA
if %DIFF_FILE_SIZE% GTR 0 (
    set ACTIVE_TAG=Unstaged changes
    set /A VERSION_REVISION=VERSION_REVISION+1
)

rem === Generate a new version file ===
@echo Tag: %ACTIVE_TAG% (%VERSION_TAG%)
@echo #define RAPROJECT64_VERSION "%VERSION_NUM%.%VERSION_REVISION%" > BuildVer2.h

rem === Update the existing file only if the new file differs ===
if not exist BuildVer.h goto nonexistant
fc BuildVer.h BuildVer2.h > nul
if errorlevel 1 goto different
del BuildVer2.h
goto done
:different
del BuildVer.h
:nonexistant
move BuildVer2.h BuildVer.h > nul
:done

rem === Clean up after ourselves ===
del Temp.txt
