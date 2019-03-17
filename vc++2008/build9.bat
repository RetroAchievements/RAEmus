echo off

if exist "%ProgramFiles(x86)%" goto is_x64
set path="%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE";%PATH%
set path="%ProgramFiles%\Windows Kits\8.1\bin\x86";%PATH%
goto start

:is_x64
set path="%ProgramFiles(x86)%\Microsoft Visual Studio 9.0\Common7\IDE";%PATH%
set path="%ProgramFiles(x86)%\Windows Kits\8.1\bin\x86";%PATH%

:start
rmdir /s /q build_xp
mkdir build_xp
ren Release Release_tmp
rmdir /s /q Release_tmp

devenv.com scv.vcproj /Rebuild Release
mkdir build_xp\scv
copy Release\scv.exe build_xp\scv\.

rmdir /s /q binary_xp
mkdir binary_xp
copy Release\*.exe binary_xp\.
rmdir /s /q binary_vista
mkdir binary_vista
copy Release\*.exe binary_vista\.
rmdir /s /q build_vista
xcopy /e /y build_xp build_vista\
rmdir /s /q Release

pushd binary_vista
for /r %%i in (*.exe) do mt.exe /manifest ..\..\src\res\vista.manifest -outputresource:%%i;1
popd
pushd build_vista
for /r %%i in (*.exe) do mt.exe /manifest ..\..\src\res\vista.manifest -outputresource:%%i;1
popd

pause
echo on
