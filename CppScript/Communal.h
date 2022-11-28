#pragma once
#include <string>
#include <vector>
#include <wtypes.h>

class Communal
{
public:
	static bool Execute(const char* szFile, const char* szParam, unsigned long& exitCode, std::string* sPrintText = NULL, unsigned long timeout = 0);
	static size_t WriteFile(const char * path, const char * writeContent, size_t & in_outLen, int start = -1,
		bool bInsert = true, bool bDelTail = true);
	static std::string ReadText(const char * path);
	static std::string GetWorkingDir();
	static bool SetWorkingDir(const char* dir);
	static unsigned long long LoadLib(const char* libPath);
	static void* GetAddressByExportName(unsigned long long hHanle, const char* sExportName);
	static bool FreeLib(unsigned long long hHandle);
	static std::string GetModulePath(unsigned long long hHandle);
	static void ListFilesA(const char * lpPath, std::vector<std::string>& vctDir, std::vector<std::string>& vctFiles,
		const char * filter, bool bSubDir, bool bAppendPath);
	static bool DelFile(const char* pFilePath);
	static bool DelFloder(const char* pDirPath);
	static bool IsPathExist(const char* path);
	static bool GetExportNames(const char* dllPath, std::vector<std::string>& outExportNames);
	static bool CpyFile(const char* srcPath, const char* dstPath);
	static bool CpyFloder(const char* srcDir, const char* dstDir);
	static bool Rename(const char* srcFileName, const char* dstFileName);
	static std::string GetRootDirFromPath(std::string path);
	static bool CleanFloder(const char* dir);
	static bool MakeFloder(const char* dir);
	static void RemoveDir(const char* dirPath);
	static bool StringReplace(std::string& strBase, const std::string& strSrc, const std::string& strDes);
	static std::string MakeGUID();
	static HMODULE GetSelfModuleHandle();
	static std::string GetSelfPath();
	static std::string GetSelfDir();
};

