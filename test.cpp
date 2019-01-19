#include <iostream>
#include <string>

//用于接收 宿主给script使用的函数
typedef int(*PFN_extFoo)(int n);
C_DATA PFN_extFoo pfnExtFoo = NULL;

//导出一个函数给宿主使用
FUNC_API void printTest()
{
	int n = pfnExtFoo(123);
	std::cout << "printTest()" << std::to_string((long long)n) << std::endl;
}

//导出一个数据给宿主使用
C_DATA int myData = 123456;

//导出一个类型给宿主使用，至于怎么用我也不知道
class CLASS_API MyClass
{
public:
	MyClass()
	{

	}
	~MyClass()
	{

	}
	void printTest()
	{
		std::cout << "MyClass::printTest()" << std::endl;
	}
};

//定义入口点，也可以不定义
DECLARE_ENTRY(MyEntry)
BOOL MyEntry(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		MessageBoxA(NULL, "DLL_PROCESS_ATTACH", "", 0);
		break;
	case DLL_PROCESS_DETACH:
		MessageBoxA(NULL, "DLL_PROCESS_DETACH", "", 0);
		break;
	case DLL_THREAD_ATTACH:
		MessageBoxA(NULL, "DLL_THREAD_ATTACH", "", 0);
		break;
	case DLL_THREAD_DETACH:
		MessageBoxA(NULL, "DLL_THREAD_DETACH", "", 0);
		break;
	default:
		break;
	}
	return TRUE;
}