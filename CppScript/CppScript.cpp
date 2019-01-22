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
	_WorkingDirMan _auto_dir(this);

	++m_compileCount;

	_generateIncludeFile();
	m_vctTmpCompiledNames = _generateCFile(sScript);

	std::string sCPath = _getSrcFileCmdLine(m_vctTmpCompiledNames);

	unsigned long exitCode = 0;
	std::string sResult;
	std::string sOption = "/nologo /c /EHsc " + sCPath + " " + getIncDirs();
	if (!Communal::Execute(m_CompilePath.c_str(), sOption.c_str(), exitCode, sResult))
		return false;
	if (result)
		*result = sResult;
	
	std::string sResultFile = _getCurTempFileName().c_str();
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
	_WorkingDirMan _auto_dir(this);

	std::string sObjPath = _getObjFileCmdLine(m_vctTmpCompiledNames);
	std::string sOut = "/OUT:\"" + _getOutFile() + "\"";

	std::string sMachine =
#ifdef _WIN64
		"/MACHINE:X64"
#else
		"/MACHINE:X86"
#endif
		;

	unsigned long exitCode = 0;
	std::string sResult;
	std::string sOption = "/nologo /DLL " + sMachine + ' ' + sOut + ' ' + sObjPath + ' ' + getLibrarys();
	if (!Communal::Execute(m_LinkPath.c_str(), sOption.c_str(), exitCode, sResult))
		return false;
	if (result)
		*result = sResult;

	std::string sResultFile = _getCurTempFileName().c_str();
	sResultFile += ".dll";	
	return Communal::IsPathExist(sResultFile.c_str());
}

CppScript::Context CppScript::eval()
{
	_WorkingDirMan _auto_dir(this);
	return CppScript::Context(Communal::LoadLib(_getOutFile().c_str()));
}

void CppScript::addIncDir(const std::string& sDir)
{
	if (sDir.size() == 0)
		return;
	std::string _sDir = sDir;
	int nLen = (int)strlen(_sDir.c_str());
	if (_sDir[nLen - 1] == '\\')
		_sDir[nLen - 1] = '\0';
	m_vctIncDirs.push_back(_sDir);
}

void CppScript::addLibrary(const std::string& sLibPath)
{
	m_vctLibs.push_back(sLibPath);
}

std::string CppScript::getIncDirs()
{
	std::string sIncDirs;
	for (int i = 0; i < (int)m_vctIncDirs.size(); ++i)
	{
		if (!sIncDirs.empty())
			sIncDirs += ' ';

		sIncDirs += "/I \"";
		sIncDirs += m_vctIncDirs[i].c_str();
		sIncDirs += "\"";
	}

	return sIncDirs;
}

std::string CppScript::getLibrarys()
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
	_WorkingDirMan _auto_dir(this);

	std::string sTempName = _getMainTempName();
	std::string sWorkinDir = getWorkingDir() ;

	for (int i = 1; i <= m_compileCount; ++i)
	{
		std::vector<std::string> vctDirs;
		std::vector<std::string> vctFiles;
		Communal::ListFilesA(sWorkinDir.c_str(), vctDirs, vctFiles, 
			(sTempName + '-' + std::to_string(i) + ".*").c_str(), false, true);
		for (int i = 0; i < (int)vctFiles.size(); ++i)
		{
			Communal::DelFile(vctFiles[i].c_str());
		}
	}
}

std::string CppScript::_getMainTempName()
{
	return std::to_string((unsigned long long)this);
}

std::string CppScript::_getCurTempFileName()
{
	return _getMainTempName() + '-' + std::to_string(m_compileCount);
}

std::string CppScript::_generateIncludeFile()
{
	const char* szHFile =
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
		"#include <windows.h>\n\n"
		"#define DECLARE_ENTRY(_EntryName) \\\n"
		"	BOOL _EntryName(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved); \\\n"
		"	class __has_declare_entry { public: __has_declare_entry(){ g_entry = _EntryName; } } g_has_declare_entry;\n";
	size_t nLen = strlen(szHFile);
	std::string sHFile = _getCurTempFileName() + ".h";
	Communal::WriteFile(sHFile.c_str(), szHFile, nLen, 0);

	return sHFile;
}

//不带后缀
std::vector<std::string> CppScript::_generateCFile(const std::string& sCode)
{
	std::string sTempFileName = _getCurTempFileName();

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
	return _getCurTempFileName() + ".dll";
}


CppScript::_WorkingDirMan::_WorkingDirMan(CppScript* THIS)
{
	m_sOriginalDir = Communal::GetWorkingDir();
	Communal::SetWorkingDir(THIS->getWorkingDir().c_str());
}

CppScript::_WorkingDirMan::~_WorkingDirMan()
{

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