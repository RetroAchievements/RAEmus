@echo off

git describe --tags --long > LiveTag.txt
@set /p ACTIVE_TAG=<LiveTag.txt

git diff HEAD > Diffs.txt
@set /p RAW_DIFFS_FOUND=<Diffs.txt

setlocal
REM set file=
@for /F "usebackq" %%A in ('"Diffs.txt"') do set DIFF_FILE_SIZE=%%~zA
@if %DIFF_FILE_SIZE% GTR 0 set ACTIVE_TAG=Unstaged Changes

@echo Tag: %ACTIVE_TAG%
@echo #define RAGENS_VERSION "%ACTIVE_TAG%" > ../../BuildVer.h