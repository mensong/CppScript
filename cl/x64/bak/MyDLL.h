#define MYDLL_API __declspec(dllexport)

// 此类是从 MyDLL.dll 导出的
class MYDLL_API CMyDLL {
public:
	CMyDLL(void);
	// TODO:  在此添加您的方法。
};

extern MYDLL_API int nMyDLL;

MYDLL_API int fnMyDLL(void);
