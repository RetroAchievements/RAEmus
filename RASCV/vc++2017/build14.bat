echo off

if exist "%ProgramFiles(x86)%" goto is_x64
set path="%ProgramFiles%\Microsoft Visual Studio 14.0\Common7\IDE";%PATH%
goto start

:is_x64
set path="%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\Common7\IDE";%PATH%

:start
rmdir /s /q binary_vc14
mkdir binary_vc14
rmdir /s /q build_vc14
mkdir build_vc14
ren Release Release_tmp
rmdir /s /q Release_tmp


devenv.com scv.vcxproj /Rebuild Release
call :clean
mkdir build_vc14\scv
copy binary_vc14\scv.exe build_vc14\scv\.


pause
echo on
exit /b

:clean
copy Release\*.exe binary_vc14\.
ren Release Release_tmp
rmdir /s /q Release_tmp
exit /b
