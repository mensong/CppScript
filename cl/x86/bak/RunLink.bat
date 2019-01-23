set libs=".\lib\x86\VC10\LIBCMT.lib" ".\lib\x86\VC10\OLDNAMES.lib" ".\lib\x86\SDK\kernel32.lib" ".\lib\x86\VC10\libcpmt.lib"
link /dll /OUT:".\MyDLL.dll" %libs% dllmain.obj MyDLL.obj stdafx.obj
pause