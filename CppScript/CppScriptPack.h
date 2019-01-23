#pragma once
#include <vector>
#include <string>

class CppScriptPack
{
public:
	CppScriptPack(class CppScript* cs);
	~CppScriptPack();

	bool init(const std::string& sWorkingDir);
	
	std::vector<std::string> testIncludes();

private:
	std::string m_sWorkingDir;
	std::string m_sCompileExeDir;
	std::string m_sIncludeBaseDir;
	std::string m_sLibBaseDir;
	class CppScript* m_pCppScript;
};

