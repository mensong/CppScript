#include "CppScript.h"
#include "Communal.h"

CppScript::CppScript()
	: m_compileCount(0)
{
}

CppScript::~CppScript()
{
}

void CppScript::setCompilePath(const std::string& sPath)
{
	m_CompilePath = sPath;
}

std::string CppScript::getCompilePath()
{
	return m_CompilePath;
}

void CppScript::setLinkPath(const std::string& sPath)
{
	m_LinkPath = sPath;
}

std::string CppScript::getLinkPath()
{
	return m_LinkPath;
}

void CppScript::setCompileOption(const std::string& sOption)
{
	m_CompileOption = sOption;
}

std::string CppScript::getCompileOption()
{
	return m_CompileOption;
}

void CppScript::setLinkOption(const std::string& sOption)
{
	m_LinkOption = sOption;
}

std::string CppScript::getLinkOption()
{
	return m_LinkOption;
}


bool CppScript::compile(const std::string& sScript, std::string* result /*= NULL*/)
{
	WorkingDirScope _auto_dir(this);

	++m_compileCount;

	_generateIncludeFile();
	m_vctTmpCompiledNames = _generateCFile(sScript);

	std::string sCPath = _getSrcFileCmdLine(m_vctTmpCompiledNames);

	unsigned long exitCode = 0;
	std::string sResult;
	std::string sOption = "/nologo /c /EHsc ";
	sOption += sCPath.c_str();
	sOption += " ";
	sOption += getCompileOption().c_str();
	sOption += " ";
	sOption += getIncDirsCmdLine().c_str();

	if (!Communal::Execute(m_CompilePath.c_str(), sOption.c_str(), exitCode, sResult))
		return false;
	if (result)
		*result = sResult;
	
	std::string sResultFile = _getCurFileName().c_str();
	sResultFile += ".obj";
	return Communal::IsPathExist(sResultFile.c_str());
}

bool CppScript::compileInClosure(const std::string& sScript, std::string* result /*= NULL*/,
	const std::vector<std::string>* vctIncludedFiles /*= NULL*/)
{
	std::string sCppCode;

	if (vctIncludedFiles != NULL)
	{
		for (int i = 0; i < (int)vctIncludedFiles->size(); ++i)
		{
			sCppCode += "#include \"" + vctIncludedFiles->at(i) + "\"\n";
		}
	}

	sCppCode +=
		"DECLARE_ENTRY(__innerEntry)\n"
		"BOOL __innerEntry(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)\n"
		"{\n"
		"	if (ul_reason_for_call == DLL_PROCESS_ATTACH)\n"
		"	{\n";
	sCppCode += sScript;
	sCppCode += '\n';
	sCppCode +=
		"	}\n"
		"	return TRUE;\n"
		"}\n";

	return compile(sCppCode, result);
}

bool CppScript::link(std::string* result /*= NULL*/)
{
	WorkingDirScope _auto_dir(this);

	std::string sObjPath = _getObjFileCmdLine(m_vctTmpCompiledNames);
	std::string sOut = "/OUT:\"";
	sOut += _getOutFile().c_str();
	sOut += "\"";

	std::string sMachine =
#ifdef _WIN64
		"/MACHINE:X64"
#else
		"/MACHINE:X86"
#endif
		;

	unsigned long exitCode = 0;
	std::string sResult;
	std::string sOption = "/nologo /DLL ";
	sOption += sMachine.c_str();
	sOption += ' ';
	sOption += getLinkOption().c_str();
	sOption += ' ';
	sOption += sOut.c_str();
	sOption += ' ';
	sOption += sObjPath.c_str();	
	sOption += ' ';
	sOption += getLibDirsCmdLine().c_str();
	sOption += ' ';
	sOption += getLibrariesCmdLine();
	
	if (!Communal::Execute(m_LinkPath.c_str(), sOption.c_str(), exitCode, sResult))
		return false;
	if (result)
		*result = sResult;

	std::string sResultFile = _getCurFileName().c_str();
	sResultFile += ".dll";	
	return Communal::IsPathExist(sResultFile.c_str());
}

CppScript::Context CppScript::eval()
{
	WorkingDirScope _auto_dir(this);
	return CppScript::Context(Communal::LoadLib(_getOutFile().c_str()));
}

void CppScript::addIncDir(const std::string& sDir)
{
	if (sDir.size() == 0)
		return;
	//目录不能以\或/结尾
	std::string _sDir = sDir;
	int nLen = (int)strlen(_sDir.c_str());
	if (_sDir[nLen - 1] == '\\' || _sDir[nLen - 1] == '/')
		_sDir[nLen - 1] = '\0';
	m_vctIncDirs.push_back(_sDir);
}

std::vector<std::string> CppScript::getIncDirs()
{
	return m_vctIncDirs;
}

void CppScript::addLibrary(const std::string& sLibPath)
{
	m_vctLibs.push_back(sLibPath);
}

std::vector<std::string> CppScript::getLibraries()
{
	return m_vctLibs;
}

std::string CppScript::getIncDirsCmdLine()
{
	std::string sCmdLine;
	for (int i = 0; i < (int)m_vctIncDirs.size(); ++i)
	{
		if (!sCmdLine.empty())
			sCmdLine += ' ';

		sCmdLine += "/I \"";
		sCmdLine += m_vctIncDirs[i].c_str();
		sCmdLine += "\"";
	}

	return sCmdLine;
}

std::string CppScript::getLibrariesCmdLine()
{
	std::string sLibs;
	for (int i = 0; i < (int)m_vctLibs.size(); ++i)
	{
		if (!sLibs.empty())
			sLibs += ' ';
		sLibs += '\"';
		sLibs += m_vctLibs[i];
		sLibs += '\"';
	}

	return sLibs;
}

void CppScript::addLibDir(const std::string& sDir)
{
	if (sDir.size() == 0)
		return;
	//目录不能以\或/结尾
	std::string _sDir = sDir;
	int nLen = (int)strlen(_sDir.c_str());
	if (_sDir[nLen - 1] == '\\' || _sDir[nLen - 1] == '/')
		_sDir[nLen - 1] = '\0';
	m_vctLibDirs.push_back(_sDir);
}

std::vector<std::string> CppScript::getLibDirs()
{
	return m_vctLibDirs;
}

std::string CppScript::getLibDirsCmdLine()
{
	std::string sCmdLine;
	for (int i = 0; i < (int)m_vctLibDirs.size(); ++i)
	{
		if (!sCmdLine.empty())
			sCmdLine += ' ';

		sCmdLine += "/LIBPATH:\"";
		sCmdLine += m_vctLibDirs[i].c_str();
		sCmdLine += "\"";
	}

	return sCmdLine;
}

std::string CppScript::getWorkingDir()
{
	if (m_workingDir.empty())
	{
		setWorkingDir(Communal::GetWorkingDir());		
	}

	return m_workingDir;

}

bool CppScript::setWorkingDir(const std::string& sDir)
{
	std::string _dir = sDir;

	if (_dir.size() == 0)
	{
		return false;
	}
	char cEnd = _dir[strlen(_dir.c_str()) - 1];
	if (cEnd == '\\' || cEnd == '/')
		_dir[strlen(_dir.c_str()) - 1] = '\0';

	if (!Communal::IsPathExist(_dir.c_str()))
		Communal::MakeFloder(_dir.c_str());

	//test dir is valid
	std::string sOldDir = Communal::GetWorkingDir();
	if (Communal::SetWorkingDir(_dir.c_str()))
	{
		Communal::SetWorkingDir(sOldDir.c_str());
		m_workingDir = _dir.c_str();
		return true;
	}
	return false;
}

void CppScript::clean()
{
	WorkingDirScope _auto_dir(this);

	std::string sTempName = _getMainID();

	for (int i = 1; i <= m_compileCount; ++i)
	{
		std::vector<std::string> vctDirs;
		std::vector<std::string> vctFiles;
		Communal::ListFilesA(".", vctDirs, vctFiles, 
			(sTempName + '-' + std::to_string(i) + ".*").c_str(), false, true);
		for (int i = 0; i < (int)vctFiles.size(); ++i)
		{
			Communal::DelFile(vctFiles[i].c_str());
		}
	}
}

std::string CppScript::_getMainID()
{
	if (m_ID.empty())
		m_ID = Communal::MakeGUID();

	return m_ID;
}

std::string CppScript::_getCurFileName()
{
	return _getMainID() + '-' + std::to_string(m_compileCount);
}

std::string CppScript::_generateIncludeFile()
{
	const char* szHFileFormat =
		"#ifndef FUNC_API\n"
		"#define FUNC_API extern \"C\" __declspec(dllexport)\n"
		"#endif\n"
		"#ifndef C_DATA\n"
		"#define C_DATA extern \"C\" __declspec(dllexport)\n"
		"#endif\n"
		"#ifndef CLASS_API\n"
		"#define CLASS_API __declspec(dllexport)\n"
		"#endif\n\n"
		"#define WIN32_LEAN_AND_MEAN\n"
		"#include <windows.h>\n"
		"\n#define DECLARE_ENTRY(_EntryName) \\\n"
		"	BOOL _EntryName(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved); \\\n"
		"	class __has_declare_entry { public: __has_declare_entry(){ g_entry = _EntryName; } } g_has_declare_entry;\n"
		"\n#define SCRIPT_ID \"%s\"";
	char szHFileText[2048];
	sprintf(szHFileText, szHFileFormat, _getCurFileName().c_str());
	size_t nLen = strlen(szHFileText);
	std::string sHFile = _getCurFileName() + ".h";
	Communal::WriteFile(sHFile.c_str(), szHFileText, nLen, 0);

	return sHFile;
}

//不带后缀
std::vector<std::string> CppScript::_generateCFile(const std::string& sCode)
{
	std::string sTempFileName = _getCurFileName();

	std::string sCppContent;

	//写内部包含文件
	std::string sInnerIncludeText;
	sInnerIncludeText += "#include \"" + sTempFileName + ".h\"\n";
	sCppContent += sInnerIncludeText.c_str();
	sCppContent += '\n';

	sCppContent += "typedef BOOL (*PFN_Entry)(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);\n";
	sCppContent += "PFN_Entry g_entry = NULL;\n\n";

	sCppContent += sCode.c_str();
	sCppContent += '\n';

	sCppContent += 
		"extern \"C\" BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)\n"
		"{\n"
		"	if (g_entry)\n"
		"		return g_entry(hModule, ul_reason_for_call, lpReserved);\n"
		"	return TRUE;\n"
		"}\n";

	size_t nLen = strlen(sCppContent.c_str());
	std::string sCFile = sTempFileName + ".cpp";
	Communal::WriteFile(sCFile.c_str(), sCppContent.c_str(), nLen, 0);

	return { sTempFileName };
}

std::string CppScript::_getSrcFileCmdLine(const std::vector<std::string>& vctNames)
{
	std::string sCmdLine;
	for (int i = 0; i < (int)vctNames.size(); ++i)
	{
		if (!sCmdLine.empty())
			sCmdLine += " ";
		sCmdLine += '\"';
		sCmdLine += vctNames[i] + ".cpp";
		sCmdLine += '\"';
	}

	return sCmdLine;
}

std::string CppScript::_getObjFileCmdLine(const std::vector<std::string>& vctNames)
{
	std::string sCmdLine;
	for (int i = 0; i < (int)vctNames.size(); ++i)
	{
		if (!sCmdLine.empty())
			sCmdLine += " ";
		sCmdLine += '\"';
		sCmdLine += vctNames[i] + ".obj";
		sCmdLine += '\"';
	}

	return sCmdLine;
}

std::string CppScript::_getOutFile()
{
	return _getCurFileName() + ".dll";
}


CppScript::WorkingDirScope::WorkingDirScope(CppScript* cs)
{
	m_sOriginalDir = Communal::GetWorkingDir();
	Communal::SetWorkingDir(cs->getWorkingDir().c_str());
}

CppScript::WorkingDirScope::WorkingDirScope(const char* dirSwitchTo)
{
	m_sOriginalDir = Communal::GetWorkingDir();
	Communal::SetWorkingDir(dirSwitchTo);
}

CppScript::WorkingDirScope::~WorkingDirScope()
{
	Communal::SetWorkingDir(m_sOriginalDir.c_str());
}

CppScript::Context::Context(unsigned long long hMod/* = NULL*/)
	: m_hMod(hMod)
{
	m_pRefCount = new int;
	*m_pRefCount = 1;
}

CppScript::Context::Context(const Context& o)
	: m_pRefCount(NULL)
	, m_hMod(NULL)
{
	*this = o;
}

bool CppScript::Context::isValid()
{
	return m_hMod != NULL;
}

void* CppScript::Context::getAddress(const std::string& name)
{
	if (!m_hMod)
		return NULL;

	return Communal::GetAddressByExportName(m_hMod, name.c_str());
}

bool CppScript::Context::getNames(std::vector<std::string>& outNames)
{
	if (!m_hMod)
		return false;

	return Communal::GetExportNames(Communal::GetModulePath(m_hMod).c_str(), outNames);
}

void CppScript::Context::_deRef()
{
	if (m_pRefCount)
	{
		--(*m_pRefCount);

		if ((*m_pRefCount) <= 0)
		{
			*m_pRefCount = 0;
			delete m_pRefCount;
			m_pRefCount = NULL;
			if (m_hMod)
			{
				//free hmodule
				Communal::FreeLib(m_hMod);
				m_hMod = NULL;
			}
		}
	}
}

CppScript::Context& CppScript::Context::operator=(const Context& o)
{
	_deRef();

	m_pRefCount = o.m_pRefCount;
	++(*m_pRefCount);

	m_hMod = o.m_hMod;
	return *this;
}

CppScript::Context::~Context()
{
	_deRef();
}