#include "CppScript.h"
#include <windows.h>
#include "Communal.h"
#include <assert.h>

int extFoo(int n)
{
	return n * 1000;
}

void main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("CppScript.exe scriptfile\n");
		return;
	}

	CppScript cs;
	
	//设置工作目录
	cs.setWorkingDir("..\\..\\tmp");

	//添加包含路径
	cs.addIncDir("..\\cl\\include\\SDK");
	cs.addIncDir("..\\cl\\include\\VC10");
		
#ifdef _WIN64
	//设置cl.exe及link.exe的路径
	cs.setCompilePath("..\\cl\\x64\\cl.exe");
	cs.setLinkPath("..\\cl\\x64\\link.exe");

	//添加lib目录
	cs.addLibDir("..\\cl\\x64\\lib\\VC10\\");
	cs.addLibDir("..\\cl\\x64\\lib\\SDK");

	//添加lib
	//没有指定路径的lib将从lib dirs里面找
	cs.addLibrary("LIBCMT.lib");
	cs.addLibrary("OLDNAMES.lib");
	cs.addLibrary("libcpmt.lib");
	cs.addLibrary("..\\cl\\x64\\lib\\SDK\\kernel32.lib");
	cs.addLibrary("..\\cl\\x64\\lib\\SDK\\uuid.lib");
	cs.addLibrary("..\\cl\\x64\\lib\\SDK\\User32.lib");
#else
	//设置cl.exe及link.exe的路径
	cs.setCompilePath("..\\cl\\x86\\cl.exe");
	cs.setLinkPath("..\\cl\\x86\\link.exe");

	//添加lib目录
	cs.addLibDir("..\\cl\\x86\\lib\\VC10\\");
	cs.addLibDir("..\\cl\\x86\\lib\\SDK");

	//添加lib
	//没有指定路径的lib将从lib dirs里面找
	cs.addLibrary("LIBCMT.lib");
	cs.addLibrary("OLDNAMES.lib");
	cs.addLibrary("libcpmt.lib");
	cs.addLibrary("..\\cl\\x86\\lib\\SDK\\kernel32.lib");
	cs.addLibrary("..\\cl\\x86\\lib\\SDK\\uuid.lib");
	cs.addLibrary("..\\cl\\x86\\lib\\SDK\\User32.lib");
#endif
	
#ifdef _DEBUG
	cs.addCompileOption("/Od /ZI");
	cs.addCompileOption("/showIncludes");
	cs.addLinkOption("/DEBUG");
	cs.addLinkOption("/VERBOSE:Lib");
#endif

	//compile
	std::string sResult;
	std::string sCpp = Communal::ReadText(argv[1]);
	printf("==================%s===============\n", "COMPILE");
	bool res = cs.compile(sCpp, &sResult);
	printf(sResult.c_str());
	if (!res)
	{
		cs.clean();
		return;
	}

	//link
	printf("==================%s===============\n", "LINK");
	res = cs.link(&sResult);
	printf(sResult.c_str());
	if (!res)
	{
		cs.clean();
		return;
	}

	//使用
	printf("==================%s===============\n", "USE");
	{//使用
		CppScript::Context ct = cs.eval();

		printf("==================%d===============\n", 1);

		std::vector<std::string> vctNames;
		ct.getNames(vctNames);
		for (int i = 0; i < vctNames.size(); ++i)
		{
			void* p = ct.getAddress(vctNames[i]);
			printf("%s:%llu\n", vctNames[i].c_str(), (unsigned long long)p);
			assert(p);
		}

		printf("==================%d===============\n", 2);

		//取script的数据
		int* pMyData = (int *)ct.getAddress("myData");
		DWORD err = GetLastError();
		if (pMyData)
			printf("%d\n", *pMyData);

		printf("==================%d===============\n", 3);

		//把函数给script使用
		typedef int(*PFN_extFoo)(int n);
		PFN_extFoo* pPFN = (PFN_extFoo*)ct.getAddress("pfnExtFoo");
		if (pPFN)
			*pPFN = extFoo;

		printf("==================%d===============\n", 4);

		//调用script的函数
		typedef void(*PFN_printTest)();
		PFN_printTest pfnPrintTest = (PFN_printTest)ct.getAddress("printTest");
		if (pfnPrintTest)
			pfnPrintTest();

		printf("==================%d===============\n", 5);
	}

	system("pause");

	//清理
	printf("==================%s===============\n", "CLEAN");
	//清理临时文件
	cs.clean();
}