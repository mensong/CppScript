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
		Context();
		Context(unsigned long long hMod);
		Context(const std::string& sModFile);
		~Context();
		Context(const Context& o);
		Context& operator=(const Context& o);
		
		///是否有效
		bool isValid();

		///获得导出的地址
		void* getAddress(const std::string& name);

		///获得所有导出的名称
		bool getNames(std::vector<std::string>& outNames);

		///设置自动清理
		void markClean(bool bClean);

		std::string getFileName();

	protected:
		void _deRef();
		void _final();

	private:
		int* m_pRefCount;
		unsigned long long m_hMod;
		bool m_cleanAfter;
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
	void addCompileOption(const std::string& sOption);
	std::vector<std::string> getCompileOptions();
	std::string getCompileOptionCmdLine();

	///设置link的其它选项
	// 例如：优化等。见 link.exe /?
	void addLinkOption(const std::string& sOption);
	std::vector<std::string> getLinkOptions();
	std::string getLinkOptionCmdLine();
	
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

	///添加宏定义
	void addDefinition(const std::string& sDefinition);
	///
	std::vector<std::string> getDefinitions();
	///返回lib目录命令行
	std::string getDefinitionCmdLine();

	///compile
	// sScript不能为不闭合的代码
	bool compile(const std::string& sScript, std::string* result = NULL);

	///compile
	// sCppFiles如果为相对路径则相对于WorkingDir
	bool compile(const std::vector<std::string>& sCppFiles, std::string* result = NULL);

	///compile in closure
	// sScript可以为不闭合的代码，如:int n = 0; ++n;
	bool compileInClosure(const std::string& sScript, std::string* result = NULL, 
		const std::vector<std::string>* vctIncludedFiles = NULL);

	///link以上compiled的内容
	bool link(std::string* result = NULL);

	///加载并运行linked的代码
	CppScript::Context eval();

	///获得当前SCRIPT_ID
	std::string GetScriptId();

	///清楚临时文件
	void clean();
		
protected:
	///添加宏定义
	void addTmpDefinition(const std::string& sDefinition);
	///返回宏定义
	std::vector<std::string> getTmpDefinitions();
	///返回宏定义命令行
	std::string getTmpDefinitionCmdLine();

	///添加包含目录
	// 如果为相对路径，则相对于WorkingDir
	void addTmpIncDir(const std::string& sDir);
	///获得include dirs
	std::vector<std::string> getTmpIncDirs();
	///返回include dirs的命令行
	std::string getTmpIncDirsCmdLine();

protected:
	std::string _getMainID();//ID
	std::string _getCurScriptId();//临时名，不带后缀：_getMainID() + '-' + m_compileCount
	std::string _generateEnvironment();//创建头文件
	std::vector<std::string> _generateCFile(const std::string& sCode);//创建源文件，返回带cpp后缀名的列表
	std::string _getSrcFileCmdLine(const std::vector<std::string>& vctNames);//获得源文件命令行
	std::vector<std::string> _getObjFiles(const std::vector<std::string>& vctNames);
	std::string _getObjFileCmdLine(const std::vector<std::string>& vctNames);//获得obj文件命令行
	std::string _getOutFile();//获得输出文件名

private:
	//去掉路径后缀名
	std::string _getFileNameWithoutExt(const std::string& sFilePath);

private:
	std::string m_CompilePath;
	std::string m_LinkPath;	
	std::vector<std::string> m_vctCompileOption;
	std::vector<std::string> m_vctLinkOption;
	std::vector<std::string> m_vctIncDirs;
	std::vector<std::string> m_vctLibs;
	std::vector<std::string> m_vctLibDirs;
	std::vector<std::string> m_vctDefinition;
	std::string m_workingDir;
	std::string m_ID;
	int m_compileCount;//每compile一次就递增1

	std::vector<std::string> m_vctTmpDefinition;
	std::vector<std::string> m_vctTmpCompiledNames;
	std::vector<std::string> m_vctTmpIncDirs;
};

