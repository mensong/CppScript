#pragma once
#include <string>
#include <vector>

class CppScript
{
public:
	///eval的结果，仿smart ptr对dll handle进行管理
	class Context
	{
	public:
		Context(unsigned long long hMod = NULL);
		~Context();
		Context(const Context& o);
		Context& operator=(const Context& o);
		
		///是否有效
		bool isValid();

		///获得导出的地址
		void* getAddress(const std::string& name);

		///获得所有导出的名称
		bool getNames(std::vector<std::string>& outNames);

	protected:
		void _deRef();

	private:
		int* m_pRefCount;
		unsigned long long m_hMod;
		std::string m_name;
	};

	class WorkingDirScope
	{
	public:
		WorkingDirScope(CppScript* cs);
		WorkingDirScope(const char* dirSwitchTo);
		~WorkingDirScope();
		std::string m_sOriginalDir;
	};

public:
	CppScript();
	~CppScript();

	///设置工作目录
	// 默认为当前目录
	std::string getWorkingDir();
	bool setWorkingDir(const std::string& sDir);

	///设置compiler的路径，既cl.exe的文件路径
	// 如果为相对路径，则相对于WorkingDir
	void setCompilePath(const std::string& sPath);
	std::string getCompilePath();

	///设置linker的路径，既link.exe的文件路径
	// 如果为相对路径，则相对于WorkingDir
	void setLinkPath(const std::string& sPath);
	std::string getLinkPath();

	///设置compile的其它选项
	// 例如：预编译等。见 cl.exe /?
	void setCompileOption(const std::string& sOption);
	std::string getCompileOption();

	///设置link的其它选项
	// 例如：优化等。见 link.exe /?
	void setLinkOption(const std::string& sOption);
	std::string getLinkOption();
	
	///添加包含目录
	// 如果为相对路径，则相对于WorkingDir
	void addIncDir(const std::string& sDir);
	///
	std::vector<std::string> getIncDirs();
	///返回include dirs的命令行
	std::string getIncDirsCmdLine();

	///添加lib文件
	// 如果为相对路径，则相对于WorkingDir
	void addLibrary(const std::string& sLibPath);
	///
	std::vector<std::string> getLibraries();
	///返回libraries的命令行
	std::string getLibrariesCmdLine();
	///添加lib目录
	// 如果为相对路径，则相对于WorkingDir
	void addLibDir(const std::string& sDir);
	///
	std::vector<std::string> getLibDirs();
	///返回lib目录命令行
	std::string getLibDirsCmdLine();

	///compile
	// sScript不能为不闭合的代码
	bool compile(const std::string& sScript, std::string* result = NULL);

	///compile in closure
	// sScript可以为不闭合的代码，如:int n = 0; ++n;
	bool compileInClosure(const std::string& sScript, std::string* result = NULL, 
		const std::vector<std::string>* vctIncludedFiles = NULL);

	///link以上compiled的内容
	bool link(std::string* result = NULL);

	///加载并运行linked的代码
	CppScript::Context eval();

	///清楚临时文件
	void clean();
		
protected:
	std::string _getMainTempName();//临时名字，为this的字符串
	std::string _getCurTempFileName();//临时文件名，不带后缀：_getMainTempName() + '-' + m_compileCount
	std::string _generateIncludeFile();//创建头文件
	std::vector<std::string> _generateCFile(const std::string& sCode);//创建源文件，返回不带cpp后缀名的列表
	std::string _getSrcFileCmdLine(const std::vector<std::string>& vctNames);//获得源文件命令行
	std::string _getObjFileCmdLine(const std::vector<std::string>& vctNames);//获得obj文件命令行
	std::string _getOutFile();//获得输出文件名

private:
	std::string m_CompilePath;
	std::string m_CompileOption;
	std::string m_LinkPath;	
	std::string m_LinkOption;
	std::vector<std::string> m_vctIncDirs;
	std::vector<std::string> m_vctLibs;
	std::vector<std::string> m_vctLibDirs;
	std::string m_workingDir;
	int m_compileCount;//每compile一次就递增1

private:
	std::vector<std::string> m_vctTmpCompiledNames;
};

