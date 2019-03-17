# eSCV

### What's this ?

This repository includes the source code for the eSCV Epoch Super Cassette Vision emulator by Toshiya Takeda.

It corresponds to the [Common Source Code Project](http://takeda-toshiya.my.coocan.jp/common/index.html)'s 2/19/2019 release.


### How to build

Build the projects with Microsoft Visual C++ 2008 with Service Pack 1,
Microsoft Visual C++ 2013 with Update 5, or Microsoft Visual C++ 2017.

The [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812) is required.
The installation should succeed even if you encounter the error S1023 after installation.

When you use Microsoft Visual C++ 2008 with Service Pack 1,
the Windows SDK for Windows 8.1 is also required to get the mt.exe utility.
The mt.exe is used to merge a manifest file for Windows Vista or later.

https://msdn.microsoft.com/en-us/windows/desktop/bg162891.aspx

When you use Microsoft Visual C++ 2013 with Update 5, the dir macros,
WindowsSDK_IncludePath, WindowsSDK_LibraryPath_x86, and DXSDK_DIR shoud be
defined and should specifies the install directories of the Windows SDK
and the DirectX SDK.
They are usually defined automatically when you install the SDKs.


### License

The copyright belongs to the author, but you can use the source code
under the GNU GENERAL PUBLIC LICENSE Version 2.

See also COPYING.txt for more details about the license.
