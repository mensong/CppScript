#include "CppScriptPack.h"
#include "CppScript.h"
#include "Communal.h"



CppScriptPack::CppScriptPack(CppScript* cs)
	: m_pCppScript(cs)
{
}


CppScriptPack::~CppScriptPack()
{
}

bool CppScriptPack::init(const std::string& sWorkingDir)
{
	m_sWorkingDir = sWorkingDir;
	m_sCompileExeDir = sWorkingDir.c_str();
	m_sCompileExeDir += "\\cl";
	m_sIncludeBaseDir = m_sCompileExeDir.c_str();
	m_sIncludeBaseDir += "\\include";
	m_sLibBaseDir = m_sCompileExeDir.c_str();
	m_sLibBaseDir += "\\lib";

	if (!m_pCppScript)
		return false;
	
	{
		//切换到cppscript的工作目录下
		CppScript::WorkingDirScope wds(m_pCppScript);

		//copy the floder where the cl.exe is:
		std::string sCompileExeDir = m_pCppScript->getCompilePath();
		sCompileExeDir = Communal::GetDirFromPath(sCompileExeDir);
#ifdef _WIN64
		sCompileExeDir = Communal::GetDirFromPath(sCompileExeDir);

#endif
		Communal::CpyFloder(sCompileExeDir.c_str(), m_sCompileExeDir.c_str());
	}

	return true;
}

std::vector<std::string> CppScriptPack::testIncludes()
{
	std::vector<std::string> vctNeedHeaders;
	std::vector<std::string> vctFiles;
	std::vector<std::string> vctDirs;
	Communal::ListFilesA(m_sIncludeBaseDir.c_str(), vctFiles, vctDirs, "*.*", true, true);
	for (int i = 0; i < vctFiles.size(); ++i)
	{

	}
	return vctNeedHeaders;
}
