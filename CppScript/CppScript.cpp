#include "CppScript.h"
#include "Communal.h"
#include <io.h>

#define COMMON_H_FILE "CPP_SCRIPT.H"

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

void CppScript::addCompileOption(const std::string& sOption)
{
	m_vctCompileOption.push_back(sOption);
}

std::vector<std::string> CppScript::getCompileOptions()
{
	return m_vctCompileOption;
}

std::string CppScript::getCompileOptionCmdLine()
{
	std::string sCmdLine;
	for (int i = 0; i < (int)m_vctCompileOption.size(); ++i)
	{
		if (!sCmdLine.empty())
			sCmdLine += ' ';

		sCmdLine += m_vctCompileOption[i].c_str();
	}

	return sCmdLine;
}


void CppScript::addLinkOption(const std::string& sOption)
{
	m_vctLinkOption.push_back(sOption);
}

std::vector<std::string> CppScript::getLinkOptions()
{
	return m_vctLinkOption;
}

std::string CppScript::getLinkOptionCmdLine()
{
	std::string sCmdLine;
	for (int i = 0; i < (int)m_vctLinkOption.size(); ++i)
	{
		if (!sCmdLine.empty())
			sCmdLine += ' ';

		sCmdLine += m_vctLinkOption[i].c_str();
	}

	return sCmdLine;
}

bool CppScript::compile(const std::string& sScript, std::string* result /*= NULL*/)
{
	WorkingDirScope _auto_dir(this);

	++m_compileCount;
	m_vctTmpDefinition.clear();
	m_vctTmpIncDirs.clear();

	_generateEnvironment();
	m_vctTmpCompiledNames = _generateCFile(sScript);

	std::string sCPath = _getSrcFileCmdLine(m_vctTmpCompiledNames);

	int exitCode = 0;
	std::string sResult;
	std::string sOption = "/nologo /c /EHsc ";

	sOption += sCPath.c_str();
	sOption += " ";
	sOption += getCompileOptionCmdLine().c_str();
	sOption += " ";
	sOption += getIncDirsCmdLine().c_str();
	sOption += " ";
	sOption += getTmpIncDirsCmdLine().c_str();
	sOption += " ";
	sOption += getDefinitionCmdLine().c_str();
	sOption += " ";
	sOption += getTmpDefinitionCmdLine().c_str();

	if (!Communal::Execute(m_CompilePath.c_str(), sOption.c_str(), &exitCode, &sResult))
		return false;
	if (result)
		*result = sResult;
	
	std::string sResultFile = _getCurScriptId().c_str();
	sResultFile += ".obj";
	return Communal::IsPathExist(sResultFile.c_str());
}

bool CppScript::compile(const std::vector<std::string>& sCppFiles, std::string* result /*= NULL*/)
{
	WorkingDirScope _auto_dir(this);

	++m_compileCount;
	m_vctTmpDefinition.clear();
	m_vctTmpIncDirs.clear();

	_generateEnvironment();

	m_vctTmpCompiledNames = sCppFiles;

	std::string sCPath = _getSrcFileCmdLine(m_vctTmpCompiledNames);

	int exitCode = 0;
	std::string sResult;
	std::string sOption = "/nologo /c /EHsc ";

	sOption += sCPath.c_str();
	sOption += " ";
	sOption += getCompileOptionCmdLine().c_str();
	sOption += " ";
	sOption += getIncDirsCmdLine().c_str();
	sOption += " ";
	sOption += getTmpIncDirsCmdLine().c_str();
	sOption += " ";
	sOption += getDefinitionCmdLine().c_str();
	sOption += " ";
	sOption += getTmpDefinitionCmdLine().c_str();

	if (!Communal::Execute(m_CompilePath.c_str(), sOption.c_str(), &exitCode, &sResult))
		return false;
	if (result)
		*result = sResult;

	return true;
}

bool CppScript::compileInClosure(const std::string& sScript, std::string* result /*= NULL*/,
	const std::vector<std::string>* vctIncludedFiles /*= NULL*/)
{
	std::string sCppCode;

	sCppCode += "#include <";
	sCppCode += COMMON_H_FILE;
	sCppCode += ">\n";

	if (vctIncludedFiles != NULL)
	{
		for (int i = 0; i < (int)vctIncludedFiles->size(); ++i)
		{
			sCppCode += "#include \"" + vctIncludedFiles->at(i) + "\"\n";
		}
	}

	sCppCode +=
		"extern \"C\" BOOL WINAPI DllMain( \n"
		"								 HMODULE hModule, \n"
		"								 DWORD ul_reason_for_call, \n"
		"								 LPVOID lpReserved) \n"
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

	std::vector<std::string> vctObjFiles = _getObjFiles(m_vctTmpCompiledNames);
	std::string sObjPath = _getObjFileCmdLine(vctObjFiles);
	std::string sOut = "/OUT:\"";
	sOut += _getOutFile().c_str();
	sOut += "\"";

	std::string sMachine =
#ifdef _WIN64
		"/MACHINE:X64";
#else
		"/MACHINE:X86";
#endif

	int exitCode = 0;
	std::string sResult;
	std::string sOption = "/nologo /DLL ";
	sOption += sMachine.c_str();
	sOption += ' ';
	sOption += getLinkOptionCmdLine().c_str();
	sOption += ' ';
	sOption += sOut.c_str();
	sOption += ' ';
	sOption += sObjPath.c_str();	
	sOption += ' ';
	sOption += getLibDirsCmdLine().c_str();
	sOption += ' ';
	sOption += getLibrariesCmdLine();
	
	if (!Communal::Execute(m_LinkPath.c_str(), sOption.c_str(), &exitCode, &sResult))
		return false;
	if (result)
		*result = sResult;

	std::string sResultFile = _getCurScriptId().c_str();
	sResultFile += ".dll";	
	return Communal::IsPathExist(sResultFile.c_str());
}

CppScript::Context CppScript::eval()
{
	WorkingDirScope _auto_dir(this);
	return CppScript::Context(Communal::LoadLib(_getOutFile().c_str()));
}

std::string CppScript::GetScriptId()
{
	return _getCurScriptId();
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

void CppScript::addTmpIncDir(const std::string& sDir)
{
	if (sDir.size() == 0)
		return;
	//目录不能以\或/结尾
	std::string _sDir = sDir;
	int nLen = (int)strlen(_sDir.c_str());
	if (_sDir[nLen - 1] == '\\' || _sDir[nLen - 1] == '/')
		_sDir[nLen - 1] = '\0';
	m_vctTmpIncDirs.push_back(_sDir);
}

std::vector<std::string> CppScript::getTmpIncDirs()
{
	return m_vctTmpIncDirs;
}

std::string CppScript::getTmpIncDirsCmdLine()
{
	std::string sCmdLine;
	for (int i = 0; i < (int)m_vctTmpIncDirs.size(); ++i)
	{
		if (!sCmdLine.empty())
			sCmdLine += ' ';

		sCmdLine += "/I \"";
		sCmdLine += m_vctTmpIncDirs[i].c_str();
		sCmdLine += "\"";
	}

	return sCmdLine;
}

void CppScript::addLibrary(const std::string& sLibPath)
{
	m_vctLibs.push_back(sLibPath);
}

std::vector<std::string> CppScript::getLibraries()
{
	return m_vctLibs;
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

void CppScript::addDefinition(const std::string& sDefinition)
{
	m_vctDefinition.push_back(sDefinition);
}

std::vector<std::string> CppScript::getDefinitions()
{
	return m_vctDefinition;
}

std::string CppScript::getDefinitionCmdLine()
{
	std::string sCmdLine;
	for (int i = 0; i < (int)m_vctDefinition.size(); ++i)
	{
		if (!sCmdLine.empty())
			sCmdLine += ' ';

		sCmdLine += "/D \"";
		sCmdLine += m_vctDefinition[i].c_str();
		sCmdLine += "\"";
	}

	return sCmdLine;
}

void CppScript::addTmpDefinition(const std::string& sDefinition)
{
	m_vctTmpDefinition.push_back(sDefinition);
}

std::vector<std::string> CppScript::getTmpDefinitions()
{
	return m_vctTmpDefinition;
}

std::string CppScript::getTmpDefinitionCmdLine()
{
	std::string sCmdLine;
	for (int i = 0; i < (int)m_vctTmpDefinition.size(); ++i)
	{
		if (!sCmdLine.empty())
			sCmdLine += ' ';

		sCmdLine += "/D \"";
		sCmdLine += m_vctTmpDefinition[i].c_str();
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

	Communal::CleanFloder(".");
}

std::string CppScript::_getMainID()
{
	if (m_ID.empty())
		m_ID = Communal::MakeGUID();

	return m_ID;
}

std::string CppScript::_getCurScriptId()
{
	return _getMainID() + '-' + std::to_string(m_compileCount);
}

std::string CppScript::_generateEnvironment()
{
	std::string sScriptIdDef = "SCRIPT_ID=\\\"";
	sScriptIdDef += _getCurScriptId().c_str();
	sScriptIdDef += "\\\"";
	addTmpDefinition(sScriptIdDef);

	addTmpIncDir(Communal::GetWorkingDir());

	if (_access(COMMON_H_FILE, 0) != -1)
		return COMMON_H_FILE;

	const char* szHFileText =
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
		;

	size_t nLen = strlen(szHFileText);
	Communal::WriteFile(COMMON_H_FILE, szHFileText, nLen, 0);

	return COMMON_H_FILE;
}

//不带后缀
std::vector<std::string> CppScript::_generateCFile(const std::string& sCode)
{
	std::string sTempFileName = _getCurScriptId();

	std::string sCppContent;
	
	sCppContent += sCode.c_str();
	sCppContent += '\n';

	size_t nLen = strlen(sCppContent.c_str());
	std::string sCFile = sTempFileName + ".cpp";
	Communal::WriteFile(sCFile.c_str(), sCppContent.c_str(), nLen, 0);

	return { sCFile };
}

std::string CppScript::_getSrcFileCmdLine(const std::vector<std::string>& vctNames)
{
	std::string sCmdLine;
	for (int i = 0; i < (int)vctNames.size(); ++i)
	{
		if (!sCmdLine.empty())
			sCmdLine += " ";
		sCmdLine += '\"';
		sCmdLine += vctNames[i];
		sCmdLine += '\"';
	}

	return sCmdLine;
}

std::vector<std::string> CppScript::_getObjFiles(const std::vector<std::string>& vctNames)
{
	std::vector<std::string> vctObjFiles;
	for (int i = 0; i < (int)vctNames.size(); ++i)
	{
		vctObjFiles.push_back(_getFileNameWithoutExt(vctNames[i]) + ".obj");
	}

	return vctObjFiles;
}

std::string CppScript::_getFileNameWithoutExt(const std::string& sFilePath)
{
	std::string sFileName;
	size_t idxDir = sFilePath.find_last_of("/\\");
	if (idxDir != std::string::npos)
	{
		sFileName = sFilePath.substr(idxDir + 1);
	}
	else
	{
		sFileName = sFilePath;
	}

	size_t nDotIdx = sFileName.find_last_of('.');
	if (nDotIdx == std::string::npos)
		return sFileName;
	return sFileName.substr(0, nDotIdx);
}

std::string CppScript::_getObjFileCmdLine(const std::vector<std::string>& vctNames)
{
	std::string sCmdLine;
	for (int i = 0; i < (int)vctNames.size(); ++i)
	{
		if (!sCmdLine.empty())
			sCmdLine += " ";
		sCmdLine += '\"';
		sCmdLine += vctNames[i];
		sCmdLine += '\"';
	}

	return sCmdLine;
}

std::string CppScript::_getOutFile()
{
	return _getCurScriptId() + ".dll";
}

CppScript::WorkingDirScope::WorkingDirScope(CppScript* cs)
{
	m_sOriginalDir = Communal::GetWorkingDir();

	std::string sWorkingDir = cs->getWorkingDir().c_str();
	Communal::SetWorkingDir(sWorkingDir.c_str());
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

CppScript::Context::Context()
	: m_pRefCount(NULL)
	, m_hMod(NULL)
	, m_cleanAfter(true)
{

}

CppScript::Context::Context(unsigned long long hMod)
	: m_hMod(hMod)
	, m_cleanAfter(true)
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

CppScript::Context::Context(const std::string& sModFile)
{
	m_hMod = Communal::LoadLib(sModFile.c_str());
	if (m_hMod)
	{
		m_pRefCount = new int;
		*m_pRefCount = 1;
	}
	else
	{
		m_pRefCount = NULL;
	}

	m_cleanAfter = false;
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

void CppScript::Context::markClean(bool bClean)
{
	m_cleanAfter = bClean;
}

std::string CppScript::Context::getFileName()
{
	if (!m_hMod)
		return "";

	return Communal::GetModulePath(m_hMod);
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
			_final();
		}
	}
}

void CppScript::Context::_final()
{
	if (m_hMod)
	{
		//free hmodule
		Communal::FreeLib(m_hMod);
		m_hMod = NULL;

		//清理目录
		if (m_cleanAfter)
		{
			std::string sModPath = Communal::GetModulePath(m_hMod);
			std::string sDir = Communal::GetRootDirFromPath(sModPath);
			if (!sDir.empty())
				Communal::CleanFloder(sDir.c_str());
		}
	}
}

CppScript::Context& CppScript::Context::operator=(const Context& o)
{
	_deRef();
	
	m_pRefCount = o.m_pRefCount;
	if (m_pRefCount)
		++(*m_pRefCount);

	m_hMod = o.m_hMod;
	m_cleanAfter = o.m_cleanAfter;

	return *this;
}

CppScript::Context::~Context()
{
	_deRef();
}