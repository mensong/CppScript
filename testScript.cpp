
#include "CPP_SCRIPT.H"
#include <iostream>
#include <string>
#include "subScript.h"

//用于接收 宿主给script使用的函数
typedef int(*PFN_extFoo)(int n);
C_DATA PFN_extFoo pfnExtFoo = NULL;

//导出一个函数给宿主使用
FUNC_API void printTest()
{
	if (pfnExtFoo)
	{
		int n = pfnExtFoo(123);
		std::cout << "compute use C function():" << std::to_string((long long)n) << std::endl;
	}
	subfoo1("test sub script foo.");
}

//导出一个数据给宿主使用
C_DATA int myData = 123456;

class MyClass
{
public:
	MyClass()
	{

	}

	//顺位1
	virtual ~MyClass()
	{

	}

	//顺位2
	virtual double foo(double d)
	{
		std::cout << "MyClass::foo()" << std::endl;
		return d * 1000;
	}

	//顺位3
	virtual void printTest()
	{
		std::cout << "MyClass::printTest()" << std::endl;
	}
	
};
//导出一个类型给宿主使用，这里只有虚函数才能C调用
FUNC_API MyClass* create_MyClass(/*构造函数参数*/)
{
	return new MyClass(/*构造函数参数*/);
}

extern "C" BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		printf("================DLL_PROCESS_ATTACH================\n");

		printf("脚本唯一标识:%s\n", SCRIPT_ID);
#if (defined MY_DEF) && (MY_DEF == 1)
		//MessageBoxA(NULL, "MY_DEF", SCRIPT_ID, 0);
		printf("MY_DEF=%d\n", MY_DEF);
#endif
		break;
	case DLL_PROCESS_DETACH:
		printf("================DLL_PROCESS_DETACH================\n");
		break;
	case DLL_THREAD_ATTACH:
		printf("================DLL_THREAD_ATTACH================\n");
		break;
	case DLL_THREAD_DETACH:
		printf("================DLL_THREAD_DETACH================\n");
		break;
	default:
		break;
	}
	
	return TRUE;
}