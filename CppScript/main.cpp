#include "CppScript.h"
#include <windows.h>
#include "Communal.h"
#include <assert.h>

///对应script里面的MyClass
// 需要在这里声明所有 script里面的MyClass的虚函数
// 注意：虚函数的位置顺序要求一致
class MyClass
{
public:
	//顺位1
	virtual ~MyClass() = 0;
	//顺位2
	virtual double foo(double d) = 0;
	//顺位3
	virtual void printTest() = 0;
	
};

int extFoo(int n)
{
	return n * 1000;
}

void main(int argc, char** argv)
{
	CppScript cs;

	//设置工作目录(临时的目录，里面的内容会被清理掉)
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
	
	//添加宏定义
	cs.addDefinition("MY_DEF=1");

#ifdef _DEBUG
	cs.addCompileOption("/Od /Zi");//Od禁用优化，Zi启用调试信息
	cs.addCompileOption("/showIncludes");//打印所有include files
	cs.addLinkOption("/DEBUG");
	cs.addLinkOption("/VERBOSE:Lib");
#endif


	//compile
	printf("==================COMPILE===============\n");
	bool res = false;
	std::string sCompileResult;
	if (argc == 2)
	{
		std::string sCpp = Communal::ReadText(argv[1]);
		if (!sCpp.empty())
		{
			res = cs.compile(sCpp, &sCompileResult);			
		}
		else
		{
			//测试纯脚本代码
			res = cs.compileInClosure("MessageBoxA(NULL, \"compileInClosure\", SCRIPT_ID, 0);", &sCompileResult);
		}
	}
	else
	{
		//测试文件
		std::vector<std::string> cppFiles;
		cppFiles.push_back("..\\testScript.cpp");
		cppFiles.push_back("..\\subScript.cpp");
		res = cs.compile(cppFiles, &sCompileResult);
	}

	printf(sCompileResult.c_str());

	if (!res)
	{
		printf("COMPILE出错\n");
		system("pause");
		cs.clean();
		return;
	}


	//link
	printf("\n===================LINK=================\n");
	std::string sLinkResult;
	res = cs.link(&sLinkResult);
	printf(sLinkResult.c_str());
	if (!res)
	{
		system("pause");
		cs.clean();
		return;
	}
	


	std::string fileName;
	{//使用
		CppScript::Context ct = cs.eval();
		fileName = ct.getFileName();
		ct.markClean(false);//标志结束后 不 清理文件夹
		
		printf("================列出脚本中所有的导出===============\n");
		std::vector<std::string> vctNames;
		ct.getNames(vctNames);
		for (int i = 0; i < vctNames.size(); ++i)
		{
			void* p = ct.getAddress(vctNames[i]);
			printf("%s:0x%p\n", vctNames[i].c_str(), p);
			assert(p);
		}

		printf("================测试读取脚本中的变量===============\n");
		//取script的数据
		int* pMyData = (int *)ct.getAddress("myData");
		DWORD err = GetLastError();
		if (pMyData)
			printf("Data in script: %d\n", *pMyData);

		printf("=================把C中的函数传给脚本===============\n");
		//把函数给script使用
		typedef int(*PFN_extFoo)(int n);
		PFN_extFoo* pPFN = (PFN_extFoo*)ct.getAddress("pfnExtFoo");
		if (pPFN)
			*pPFN = extFoo;

		printf("================== 调用脚本中的函数 ===============\n");
		//调用script的函数
		typedef void(*PFN_printTest)();
		PFN_printTest pfnPrintTest = (PFN_printTest)ct.getAddress("printTest");
		if (pfnPrintTest)
			pfnPrintTest();

		printf("================== 调用脚本中的类 =================\n");
		typedef MyClass* (*PFN_create_MyClass)(/*构造函数参数*/);
		PFN_create_MyClass create_MyClass = (PFN_create_MyClass)ct.getAddress("create_MyClass");
		if (create_MyClass)
		{
			MyClass* pMyClass = create_MyClass();
			if (pMyClass)
			{
				pMyClass->printTest();
				double fooRes = pMyClass->foo(123.456);
			}
		}
	}

	//直接从文件加载
	{
		printf("\n\n================   直接从文件加载   ===============\n");
		CppScript::Context ct2(fileName);
		ct2.markClean(true);//标志结束后清理文件夹
		if (ct2.isValid())
		{
			printf("================列出脚本中所有的导出===============\n");
			std::vector<std::string> vctNames;
			ct2.getNames(vctNames);
			for (int i = 0; i < vctNames.size(); ++i)
			{
				void* p = ct2.getAddress(vctNames[i]);
				printf("%s:0x%p\n", vctNames[i].c_str(), p);
				assert(p);
			}

			printf("================测试读取脚本中的变量===============\n");
			//取script的数据
			int* pMyData = (int *)ct2.getAddress("myData");
			DWORD err = GetLastError();
			if (pMyData)
				printf("Data in script: %d\n", *pMyData);

			printf("=================把C中的函数传给脚本===============\n");
			//把函数给script使用
			typedef int(*PFN_extFoo)(int n);
			PFN_extFoo* pPFN = (PFN_extFoo*)ct2.getAddress("pfnExtFoo");
			if (pPFN)
				*pPFN = extFoo;

			printf("================== 调用脚本中的函数 ===============\n");
			//调用script的函数
			typedef void(*PFN_printTest)();
			PFN_printTest pfnPrintTest = (PFN_printTest)ct2.getAddress("printTest");
			if (pfnPrintTest)
				pfnPrintTest();

			printf("================== 调用脚本中的类 =================\n");
			typedef MyClass* (*PFN_create_MyClass)(/*构造函数参数*/);
			PFN_create_MyClass create_MyClass = (PFN_create_MyClass)ct2.getAddress("create_MyClass");
			if (create_MyClass)
			{
				MyClass* pMyClass = create_MyClass();
				if (pMyClass)
				{
					pMyClass->printTest();
					double fooRes = pMyClass->foo(123.456);
				}
			}
		}
	}	

	system("pause");
}