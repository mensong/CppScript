#pragma once
#include <string>
#include <vector>

class Communal
{
public:
	static bool Execute(const char* szFile, const char* szParam, unsigned long& exitCode, std::string& sPrintText);
	static size_t WriteFile(const char * path, const char * writeContent, size_t & in_outLen, int start = -1,
		bool bInsert = true, bool bDelTail = true);
	static std::string ReadText(const char * path);
	static std::string GetWorkingDir();
	static bool SetWorkingDir(const char* dir);
	static unsigned long long LoadLib(const char* libPath);
	static void* GetAddressByExportName(unsigned long long hHanle, const char* sExportName);
	static bool FreeLib(unsigned long long hHandle);
	static std::vector<std::string> GetDllExportNames(const char* pFile);
	static std::string GetModulePath(unsigned long long hHandle);
	static void ListFilesA(const char * lpPath, std::vector<std::string>& vctDir, std::vector<std::string>& vctFiles,
		const char * filter, bool bSubDir, bool bAppendPath);
	static bool DelFile(const char* pFilePath);
};

