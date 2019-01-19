rem cl /LD dllmain.cpp MyDLL.cpp /Yu"stdafx.h" /Fp".\MyDLL.pch" /I "." /link /OUT:".\MyDLL.dll" "D:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\lib\libcmt.lib" "D:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\lib\oldnames.lib" "C:\Program Files (x86)\Windows Kits\8.1\lib\winv6.3\um\x86\Kernel32.Lib" "D:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\lib\libvcruntime.lib" "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.10240.0\ucrt\x86\libucrt.lib" "C:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x86\uuid.lib" dllmain.obj MyDLL.obj stdafx.obj

cl /EHsc dllmain.cpp MyDLL.cpp /Yu"stdafx.h" /Fp".\MyDLL.pch" /I "." /I ".\include\SDK" /I ".\include\VC10" /c
echo %errorlevel%
pause