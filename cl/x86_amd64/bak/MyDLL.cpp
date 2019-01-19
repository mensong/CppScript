// MyDLL.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "MyDLL.h"
#include <vector>


// 这是导出变量的一个示例
MYDLL_API int nMyDLL=0;

// 这是导出函数的一个示例。
MYDLL_API int fnMyDLL(void)
{
	std::vector<int> vct;
	vct.push_back(0);
	vct.push_back(1);
	vct.push_back(2);
	vct.push_back(3);
    return vct[0];
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 MyDLL.h
CMyDLL::CMyDLL()
{
    return;
}
