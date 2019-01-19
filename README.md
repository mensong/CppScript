# CppScript

把C++代码当脚本一样写。
在项目的过程中往往遇到一些这样的情况：有一些代码的业务逻辑比较复杂，而且业务比较容易发生变化。我们有一种方法就是把这部分的业务实现提出去，让脚本的形式运行，例如：lua，js等，但是还要求性能，例如一个数据转发器，不可能在数据转发的过程使用如lua或js吧，怎么办呢？于是就有这个项目了。


# 特性

 - 脚本化
 - 性能和原来的C++一样（因为就是DLL）

# 原理

把vc中的命令行编译工具提取出来。我们写好业务脚本后，保存一个文件，再使用这个命令行编译工具编译、连接后loadlibrary，运行。

# 使用

主程序里：

    char path[MAX_PATH] = { 0 };
	GetCurrentDirectoryA(MAX_PATH, path);
	std::string sCurDir = path;

	CppScript cs;
	cs.setWorkingDir(sCurDir + "\\..\\..\\tmp");

	cs.addIncDir(sCurDir + "\\..\\..\\cl\\include\\SDK");
	cs.addIncDir(sCurDir + "\\..\\..\\cl\\include\\VC10");

    #ifdef _WIN64
	cs.setCompilePath(sCurDir + "\\..\\..\\cl\\x86_amd64\\cl.exe");
	cs.setLinkPath(sCurDir + "\\..\\..\\cl\\x86_amd64\\link.exe");

	cs.addLibrary(sCurDir + "\\..\\..\\cl\\lib\\x64\\VC10\\LIBCMT.lib");
	cs.addLibrary(sCurDir + "\\..\\..\\cl\\lib\\x64\\VC10\\OLDNAMES.lib");
	cs.addLibrary(sCurDir + "\\..\\..\\cl\\lib\\x64\\VC10\\libcpmt.lib");
	cs.addLibrary(sCurDir + "\\..\\..\\cl\\lib\\x64\\SDK\\kernel32.lib");
	cs.addLibrary(sCurDir + "\\..\\..\\cl\\lib\\x64\\SDK\\uuid.lib");
	cs.addLibrary(sCurDir + "\\..\\..\\cl\\lib\\x64\\SDK\\User32.lib");
    #else
	cs.setCompilePath(sCurDir + "\\..\\..\\cl\\cl.exe");
	cs.setLinkPath(sCurDir + "\\..\\..\\cl\\link.exe");

	cs.addLibrary(sCurDir + "\\..\\..\\cl\\lib\\x86\\VC10\\LIBCMT.lib");
	cs.addLibrary(sCurDir + "\\..\\..\\cl\\lib\\x86\\VC10\\OLDNAMES.lib");
	cs.addLibrary(sCurDir + "\\..\\..\\cl\\lib\\x86\\VC10\\libcpmt.lib");
	cs.addLibrary(sCurDir + "\\..\\..\\cl\\lib\\x86\\SDK\\kernel32.lib");
	cs.addLibrary(sCurDir + "\\..\\..\\cl\\lib\\x86\\SDK\\uuid.lib");
	cs.addLibrary(sCurDir + "\\..\\..\\cl\\lib\\x86\\SDK\\User32.lib");
    #endif
	
	std::string sResult;
	std::string sCpp = Communal::ReadText("..\\..\\test.cpp");
	printf("==================%s===============\n", "COMPILE");
	int res = cs.compile(sCpp, &sResult);
	printf(sResult.c_str());
	if (res != 259)//cl成功则返回259，这个为什么我也不知道
	{
		cs.clean();
		return;
	}

	printf("==================%s===============\n", "LINK");
	res = cs.link(&sResult);
	printf(sResult.c_str());
	if (res != 259)//link成功则返回259，这个为什么我也不知道
	{
		cs.clean();
		return;
	}

	printf("==================%s===============\n", "USE");
	{//使用
		CppScript::Context ct = cs.eval();
		
		//取script的数据
		int* pMyData = (int *)ct.getAddress("myData");
		DWORD err = GetLastError();
		if (pMyData)
			printf("%d\n", *pMyData);

		//把函数给script使用
		typedef int(*PFN_extFoo)(int n);
		PFN_extFoo* pPFN = (PFN_extFoo*)ct.getAddress("pfnExtFoo");
		if (pPFN)
			*pPFN = extFoo;

		//调用script的函数
		typedef void(*PFN_printTest)();
		PFN_printTest pfnPrintTest = (PFN_printTest)ct.getAddress("printTest");
		if (pfnPrintTest)
			pfnPrintTest();
	}

	system("pause");

	printf("==================%s===============\n", "CLEAN");
	//清理临时文件
	cs.clean();

脚本：

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
