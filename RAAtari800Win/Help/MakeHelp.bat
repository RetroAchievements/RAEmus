@echo off
REM Help ID bases
REM #define HID_BASE_COMMAND    0x00010000UL        // ID and IDM
REM #define HID_BASE_RESOURCE   0x00020000UL        // IDR and IDD
REM #define HID_BASE_PROMPT     0x00030000UL        // IDP
REM #define HID_BASE_NCAREAS    0x00040000UL
REM #define HID_BASE_CONTROL    0x00050000UL        // IDC
REM #define HID_BASE_DISPATCH   0x00060000UL        // IDispatch help codes

REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by Atari800Win.HPJ. >"Atari800Win.hm"
echo. >>"Atari800Win.hm"
echo // Commands (ID_* and IDM_*) >>"Atari800Win.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"Atari800Win.hm"
echo. >>"Atari800Win.hm"
echo // Prompts (IDP_*) >>"Atari800Win.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"Atari800Win.hm"
echo. >>"Atari800Win.hm"
echo // Resources (IDR_*) >>"Atari800Win.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"Atari800Win.hm"
echo. >>"Atari800Win.hm"
echo // Dialogs (IDD_*) >>"Atari800Win.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"Atari800Win.hm"
echo. >>"Atari800Win.hm"
echo // Frame Controls (IDW_*) >>"Atari800Win.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"Atari800Win.hm"
echo // Controls (IDC_*) >>"Atari800Win.hm"
makehm IDC_,HIDC_,0x50000 resource.h >>"Atari800Win.hm"
REM -- Make help for Project Atari800Win PLus


echo Building Win32 Help files
start /wait "C:\Program Files\Help Workshop\hcw" "Atari800Win.hpj"
if errorlevel 1 goto :Error
if not exist "Atari800Win.hlp" goto :Error
if not exist "Atari800Win.cnt" goto :Error
echo.
copy "Atari800Win.hlp" "..\Distribution\Atari800WinPLus"
copy "Atari800Win.cnt" "..\Distribution\Atari800WinPLus"
echo.
goto :done

:Error
echo Atari800Win.hpj(1) : error: Problem encountered creating help file

:done
echo.
