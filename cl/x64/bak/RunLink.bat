set libs="..\lib\x64\VC10\LIBCMT.lib" "..\lib\x64\VC10\OLDNAMES.lib" "..\lib\x64\SDK\kernel32.lib" "..\lib\x64\VC10\libcpmt.lib"
link /dll /OUT:".\MyDLL.dll" %libs% dllmain.obj MyDLL.obj stdafx.obj
pause