del MyDLL.pch
del stdafx.obj
rem cl stdafx.cpp /Yc"stdafx.h" /Fp".\MyDLL.pch" /I "D:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include" /I "D:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\atlmfc\include" /I "C:\Program Files (x86)\Windows Kits\10\Include\10.0.10240.0\ucrt" /I "C:\Program Files (x86)\Windows Kits\8.1\Include\um" /I "C:\Program Files (x86)\Windows Kits\8.1\Include\shared" /I "C:\Program Files (x86)\Windows Kits\8.1\Include\winrt" /c

cl stdafx.cpp /Yc"stdafx.h" /Fp".\MyDLL.pch" /I "..\include\SDK" /I "..\include\VC10" /c
pause