#pragma once
#include <string>
#include <vector>

class CppScript
{
public:
	///eval�Ľ������smart ptr��dll handle���й���
	class Context
	{
	public:
		Context();
		Context(unsigned long long hMod);
		Context(const std::string& sModFile);
		~Context();
		Context(const Context& o);
		Context& operator=(const Context& o);
		
		///�Ƿ���Ч
		bool isValid();

		///��õ����ĵ�ַ
		void* getAddress(const std::string& name);

		///������е���������
		bool getNames(std::vector<std::string>& outNames);

		///�����Զ�����
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

	///���ù���Ŀ¼
	// Ĭ��Ϊ��ǰĿ¼
	std::string getWorkingDir();
	bool setWorkingDir(const std::string& sDir);

	///����compiler��·������cl.exe���ļ�·��
	// ���Ϊ���·�����������WorkingDir
	void setCompilePath(const std::string& sPath);
	std::string getCompilePath();

	///����linker��·������link.exe���ļ�·��
	// ���Ϊ���·�����������WorkingDir
	void setLinkPath(const std::string& sPath);
	std::string getLinkPath();

	///����compile������ѡ��
	// ���磺Ԥ����ȡ��� cl.exe /?
	void addCompileOption(const std::string& sOption);
	std::vector<std::string> getCompileOptions();
	std::string getCompileOptionCmdLine();

	///����link������ѡ��
	// ���磺�Ż��ȡ��� link.exe /?
	void addLinkOption(const std::string& sOption);
	std::vector<std::string> getLinkOptions();
	std::string getLinkOptionCmdLine();
	
	///��Ӱ���Ŀ¼
	// ���Ϊ���·�����������WorkingDir
	void addIncDir(const std::string& sDir);
	///
	std::vector<std::string> getIncDirs();
	///����include dirs��������
	std::string getIncDirsCmdLine();

	///���lib�ļ�
	// ���Ϊ���·�����������WorkingDir
	void addLibrary(const std::string& sLibPath);
	///
	std::vector<std::string> getLibraries();
	///����libraries��������
	std::string getLibrariesCmdLine();
	///���libĿ¼
	// ���Ϊ���·�����������WorkingDir
	void addLibDir(const std::string& sDir);
	///
	std::vector<std::string> getLibDirs();
	///����libĿ¼������
	std::string getLibDirsCmdLine();

	///��Ӻ궨��
	void addDefinition(const std::string& sDefinition);
	///
	std::vector<std::string> getDefinitions();
	///����libĿ¼������
	std::string getDefinitionCmdLine();

	///compile
	// sScript����Ϊ���պϵĴ���
	bool compile(const std::string& sScript, std::string* result = NULL);

	///compile
	// sCppFiles���Ϊ���·���������WorkingDir
	bool compile(const std::vector<std::string>& sCppFiles, std::string* result = NULL);

	///compile in closure
	// sScript����Ϊ���պϵĴ��룬��:int n = 0; ++n;
	bool compileInClosure(const std::string& sScript, std::string* result = NULL, 
		const std::vector<std::string>* vctIncludedFiles = NULL);

	///link����compiled������
	bool link(std::string* result = NULL);

	///���ز�����linked�Ĵ���
	CppScript::Context eval();

	///��õ�ǰSCRIPT_ID
	std::string GetScriptId();

	///�����ʱ�ļ�
	void clean();
		
protected:
	///��Ӻ궨��
	void addTmpDefinition(const std::string& sDefinition);
	///���غ궨��
	std::vector<std::string> getTmpDefinitions();
	///���غ궨��������
	std::string getTmpDefinitionCmdLine();

	///��Ӱ���Ŀ¼
	// ���Ϊ���·�����������WorkingDir
	void addTmpIncDir(const std::string& sDir);
	///���include dirs
	std::vector<std::string> getTmpIncDirs();
	///����include dirs��������
	std::string getTmpIncDirsCmdLine();

protected:
	std::string _getMainID();//ID
	std::string _getCurScriptId();//��ʱ����������׺��_getMainID() + '-' + m_compileCount
	std::string _generateEnvironment();//����ͷ�ļ�
	std::vector<std::string> _generateCFile(const std::string& sCode);//����Դ�ļ������ش�cpp��׺�����б�
	std::string _getSrcFileCmdLine(const std::vector<std::string>& vctNames);//���Դ�ļ�������
	std::vector<std::string> _getObjFiles(const std::vector<std::string>& vctNames);
	std::string _getObjFileCmdLine(const std::vector<std::string>& vctNames);//���obj�ļ�������
	std::string _getOutFile();//�������ļ���

private:
	//ȥ��·����׺��
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
	int m_compileCount;//ÿcompileһ�ξ͵���1

	std::vector<std::string> m_vctTmpDefinition;
	std::vector<std::string> m_vctTmpCompiledNames;
	std::vector<std::string> m_vctTmpIncDirs;
};

