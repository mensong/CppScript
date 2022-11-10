#ifndef FUNC_API
#define FUNC_API extern "C" __declspec(dllexport)
#endif
#ifndef C_DATA
#define C_DATA extern "C" __declspec(dllexport)
#endif
#ifndef CLASS_API
#define CLASS_API __declspec(dllexport)
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

