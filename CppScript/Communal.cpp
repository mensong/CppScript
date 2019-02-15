#include "Communal.h"
#include <malloc.h>
#include <windows.h>
#include <io.h>

bool Communal::Execute(const char* szFile, const char* szParam, unsigned long& exitCode, std::string* sPrintText/* = NULL*/)
{
	if (!szFile || szFile[0] == '\0')
		return false;

	HANDLE hRead, hWrite;
	if (sPrintText != NULL)
	{
		//创建匿名管道
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
		if (!CreatePipe(&hRead, &hWrite, &sa, 0))
		{
			return false;
		}
	}

	int nCmdLen = (strlen(szFile) + strlen(szParam) + 4) * sizeof(char);
	char* szCmd = (char*)_alloca(nCmdLen);//_alloca在栈上申请的，会自动释放
	memset(szCmd, 0, nCmdLen);
	strcpy(szCmd, "\"");
	strcat(szCmd, szFile);
	strcat(szCmd, "\"");
	if (szParam)
	{
		strcat(szCmd, " ");
		strcat(szCmd, szParam);
	}

	//设置命令行进程启动信息(以隐藏方式启动命令并定位其输出到hWrite)
	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	GetStartupInfoA(&si);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	if (sPrintText != NULL)
	{
		si.hStdError = hWrite;
		si.hStdOutput = hWrite;
	}

	//启动命令行
	PROCESS_INFORMATION pi;
	if (!CreateProcessA(NULL, (char *)szCmd, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		if (sPrintText != NULL)
		{
			CloseHandle(hWrite);
			CloseHandle(hRead);
		}
		return false;
	}

	if (sPrintText != NULL)
	{
		//立即关闭hWrite
		CloseHandle(hWrite);

		//读取命令行返回值
		char buff[1024 + 1];
		DWORD dwRead = 0;
		while (ReadFile(hRead, buff, 1024, &dwRead, NULL))
		{
			buff[dwRead] = '\0';
			sPrintText->append(buff, dwRead);
		}
		CloseHandle(hRead);
	}
	else
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
	}

	//获得返回值
	GetExitCodeProcess(pi.hProcess, &exitCode);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return true;
}

size_t Communal::WriteFile(const char * path, const char * writeContent, size_t & in_outLen,
	int start/* = -1*/, bool bInsert/* = true*/, bool bDelTail/* = true*/)
{
	if (!path)
	{
		return -1;
	}

	FILE *f = NULL;
	if ((f = fopen(path, "rb+")) == NULL)
	{
		//文件不存在，则新建一个空的
		if ((f = fopen(path, "wb")) == NULL)
		{
			return -1;
		}
		fclose(f);

		if ((f = fopen(path, "rb+")) == NULL)
			return -1;
	}

	do
	{
		int nFileSize = 0;
		if (fseek(f, 0, SEEK_END) != 0)
		{
			nFileSize = 0;
		}
		else
		{
			nFileSize = ftell(f);
		}
		if (nFileSize < 0)
		{
			break;
		}

		char* pOldStart = NULL;
		char* pOldEnd = NULL;
		if (start > -1)
		{
			if (nFileSize < start)
			{//插入处比原有的文件大，则在原文件尾到插入处空白的这些位置填充NULL
				int nSpace = start - nFileSize + 1;
				char *pSpace = new char[nSpace];
				memset(pSpace, 0, nSpace);
				fwrite(pSpace, nSpace, 1, f);
				delete[] pSpace;
			}
			else if (bInsert && (nFileSize > start))
			{//在中间插入要写的内容
			 //读取插入处到原文件结尾处的内容，以回写
				int nRead = (nFileSize - start);
				fseek(f, start, SEEK_SET);
				pOldEnd = new char[nRead];
				fread(pOldEnd, 1, nRead, f);
			}
			else if (bDelTail && (nFileSize > start) && (in_outLen < nFileSize - start))
			{//回写插入处前面的内容，而丢弃插入处后面的内容
			 //读取插入处前面的内容
				if (start > 0)
				{
					fseek(f, 0, SEEK_SET);
					pOldStart = new char[start];
					fread(pOldStart, 1, start, f);
				}

				//删除原有的文件，新建一个空的
				fclose(f);
				remove(path);
				if ((f = fopen(path, "wb")) == NULL)
				{
					return -1;
				}
				//回写插入处前面的内容，而丢弃插入处后面的内容
				if (start > 0)
					fwrite(pOldStart, 1, start, f);
			}

			if (fseek(f, start, SEEK_SET) != 0)
			{
				if (pOldEnd)
				{
					delete[] pOldEnd;
				}
				break;
			}
		}

		in_outLen = fwrite(writeContent, 1, in_outLen, f);
		if (pOldEnd)
		{
			fwrite(pOldEnd, 1, (nFileSize - start), f);
			delete[] pOldEnd;
		}

	} while (0);

	int nFileSize = 0;
	if (fseek(f, 0, SEEK_END) != 0)
	{
		nFileSize = -1;
	}
	else
	{
		nFileSize = ftell(f);
	}

	fclose(f);
	return nFileSize;
}

std::string Communal::ReadText(const char * path)
{
	FILE *f = NULL;
	long sz;

	if (!path)
	{
		return "";
	}

	std::string sRet;

	f = fopen(path, "rb");
	if (!f)
	{
		return "";
	}

	do
	{
		if (fseek(f, 0, SEEK_END) < 0)
		{
			break;
		}

		sz = ftell(f);
		if (sz < 0)
		{
			break;
		}

		if (fseek(f, 0, SEEK_SET) < 0)
		{
			break;
		}

		sRet.resize((size_t)sz + 1, '\0');

		if ((size_t)fread(const_cast<char*>(sRet.c_str()), 1, (size_t)sz, f) != (size_t)sz)
		{
			sRet = "";
			break;
		}
	} while (0);

	fclose(f);

	return sRet;
}

std::string Communal::GetWorkingDir()
{
	char path[MAX_PATH] = { 0 };
	if (0 == GetCurrentDirectoryA(MAX_PATH, path))
		return ".";
	return path;
}

bool Communal::SetWorkingDir(const char* dir)
{
	return (TRUE == SetCurrentDirectoryA(dir));
}

unsigned long long Communal::LoadLib(const char* libPath)
{
	unsigned long long h = (unsigned long long)::LoadLibraryA(libPath);
	return h;
}

void* Communal::GetAddressByExportName(unsigned long long hHanle, const char* sExportName)
{
	return GetProcAddress((HMODULE)hHanle, sExportName);
}

bool Communal::FreeLib(unsigned long long hHandle)
{
	return (TRUE == FreeLibrary((HMODULE)hHandle));
}

std::string Communal::GetModulePath(unsigned long long hHandle)
{
	char path[MAX_PATH] = { 0 };
	GetModuleFileNameA((HMODULE)hHandle, path, MAX_PATH);
	return path;
}

void Communal::ListFilesA(const char * lpPath, std::vector<std::string>& vctDir, std::vector<std::string>& vctFiles,
	const char * filter, bool bSubDir, bool bAppendPath)
{
	char szFind[MAX_PATH] = { 0 };
	lstrcpyA(szFind, lpPath);
	lstrcatA(szFind, "/");
	lstrcatA(szFind, filter);
	
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = ::FindFirstFileA(szFind, &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		//int n = GetLastError();
		return;
	}

	char szFile[MAX_PATH] = { 0 };
	while (TRUE)
	{
		szFile[0] = '\0';
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				if (bAppendPath)
				{
					lstrcpyA(szFile, lpPath);
					lstrcatA(szFile, "\\");
					lstrcatA(szFile, FindFileData.cFileName);
				}
				else
				{
					lstrcpyA(szFile, FindFileData.cFileName);
				}
				vctDir.push_back(szFile);

				if (bSubDir)
				{
					ListFilesA(szFile, vctDir, vctFiles, filter, bSubDir, bAppendPath);
				}
			}
		}
		else
		{
			if (bAppendPath)
			{
				lstrcpyA(szFile, lpPath);
				lstrcatA(szFile, "\\");
				lstrcatA(szFile, FindFileData.cFileName);
			}
			else
			{
				lstrcpyA(szFile, FindFileData.cFileName);
			}
			vctFiles.push_back(szFile);
		}

		if (!FindNextFileA(hFind, &FindFileData))
			break;
	}

	FindClose(hFind);
}

bool Communal::DelFile(const char* pFilePath)
{
	BOOL b = DeleteFileA(pFilePath);
	DWORD err = GetLastError();
	return (b == TRUE);
}

bool Communal::IsPathExist(const char* path)
{
	int nRet = _access(path, 0);
	return 0 == nRet || EACCES == nRet;
}

bool Communal::GetExportNames(const char* dllPath, std::vector<std::string>& outExportNames)
{
	HANDLE hFile, hFileMap;//文件句柄和内存映射文件句柄
	DWORD fileAttrib = 0;//存储文件属性用，在createfile中用到。
	void* mod_base;//内存映射文件的起始地址，也是模块的起始地址
	typedef PVOID(CALLBACK* PFNEXPORTFUNC)(PIMAGE_NT_HEADERS, PVOID, ULONG, PIMAGE_SECTION_HEADER*);
	//首先取得ImageRvaToVa函数本来只要#include <Dbghelp.h>就可以使用这个函数，但是可能没有这个头文件
	PFNEXPORTFUNC ImageRvaToVax = NULL;
	HMODULE hModule = ::LoadLibraryA("DbgHelp.dll");
	if (hModule != NULL)
	{
		ImageRvaToVax = (PFNEXPORTFUNC)::GetProcAddress(hModule, "ImageRvaToVa");
		if (ImageRvaToVax == NULL)
		{
			::FreeLibrary(hModule);
			return false;
		}
	}
	else
	{
		return false;
	}

	if (!IsPathExist(dllPath))
	{//返回值为NULL，则文件不存在，退出
		::FreeLibrary(hModule);
		return false;
	}

	hFile = CreateFileA(dllPath, GENERIC_READ, 0, 0, OPEN_EXISTING, fileAttrib, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		::FreeLibrary(hModule);
		return false;
	}
	hFileMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
	if (hFileMap == NULL)
	{
		CloseHandle(hFile);
		::FreeLibrary(hModule);
		return false;
	}
	mod_base = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
	if (mod_base == NULL)
	{
		CloseHandle(hFileMap);
		CloseHandle(hFile);
		::FreeLibrary(hModule);
		return false;
	}
	IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)mod_base;
	IMAGE_NT_HEADERS * pNtHeader =
		(IMAGE_NT_HEADERS *)((BYTE*)mod_base + pDosHeader->e_lfanew);//得到NT头首址
	IMAGE_OPTIONAL_HEADER * pOptHeader =
		(IMAGE_OPTIONAL_HEADER *)((BYTE*)mod_base + pDosHeader->e_lfanew + 24);//optional头首址
	IMAGE_EXPORT_DIRECTORY* pExportDesc = (IMAGE_EXPORT_DIRECTORY*)
		ImageRvaToVax(pNtHeader, mod_base, pOptHeader->DataDirectory[0].VirtualAddress, 0);
	//导出表首址。函数名称表首地址每个DWORD代表一个函数名字字符串的地址
	PDWORD nameAddr = (PDWORD)ImageRvaToVax(pNtHeader, mod_base, pExportDesc->AddressOfNames, 0);
	char* func_name = (char*)ImageRvaToVax(pNtHeader, mod_base, (DWORD)nameAddr[0], 0);
	DWORD i = 0;
	DWORD unti = pExportDesc->NumberOfNames;
	for (i = 0; i < unti; i++)
	{
		outExportNames.push_back(func_name);
		func_name = (char*)ImageRvaToVax(pNtHeader, mod_base, (DWORD)nameAddr[i], 0);
	}
	::FreeLibrary(hModule);
	UnmapViewOfFile(mod_base);
	CloseHandle(hFileMap);
	CloseHandle(hFile);

	return true;
}

bool Communal::CpyFile(const char* srcPath, const char* dstPath)
{
	return (CopyFileA(srcPath, dstPath, FALSE) == TRUE);
}

bool Communal::CpyFloder(const char* srcDir, const char* dstDir)
{
	char szSrcPath[MAX_PATH];//源文件路径
	int nLen = strlen(srcDir);
	strcpy(szSrcPath, srcDir);
	szSrcPath[nLen] = '\0';//必须要以“\0\0”结尾，不然删除不了
	szSrcPath[nLen + 1] = '\0';

	char szDstPath[MAX_PATH];
	nLen = strlen(dstDir);
	strcpy(szDstPath, dstDir);
	szDstPath[nLen] = '\0';
	szDstPath[nLen + 1] = '\0';

	SHFILEOPSTRUCTA FileOp;
	SecureZeroMemory((void*)&FileOp, sizeof(SHFILEOPSTRUCTA));//secureZeroMemory和ZeroMerory的区别
	//根据MSDN上，ZeryMerory在当缓冲区的字符串超出生命周期的时候，
	//会被编译器优化，从而缓冲区的内容会被恶意软件捕捉到。
	//引起软件安全问题，特别是对于密码这些比较敏感的信息而说。
	//而SecureZeroMemory则不会引发此问题，保证缓冲区的内容会被正确的清零。
	//如果涉及到比较敏感的内容，尽量使用SecureZeroMemory函数。
	FileOp.fFlags = FOF_NOCONFIRMATION; //操作与确认标志 
	FileOp.hNameMappings = NULL; //文件映射
	FileOp.hwnd = NULL; //消息发送的窗口句柄；
	FileOp.lpszProgressTitle = NULL; //文件操作进度窗口标题 
	FileOp.pFrom = szSrcPath; //源文件及路径 
	FileOp.pTo = szDstPath; //目标文件及路径 
	FileOp.wFunc = FO_COPY; //操作类型 
	return SHFileOperationA(&FileOp) == 0;
}

bool Communal::Rename(const char* srcFileName, const char* dstFileName)
{
	return 0 == rename(srcFileName, dstFileName);
}

std::string Communal::GetDirFromPath(std::string path)
{
	int nLen = strlen(path.c_str());
	while (path.size() > 0 && (path[nLen - 1] == '\\' || path[nLen - 1] == '/'))
	{
		path[nLen - 1] = '\0';
		nLen = strlen(path.c_str());
	}

	size_t idx1 = path.rfind('\\');
	if (idx1 == std::string::npos)
		idx1 = path.rfind('/');
	else
	{
		size_t idx2 = path.rfind('/');
		if (idx2 != std::string::npos && idx2 > idx1)
		{
			idx1 = idx2;
		}
	}

	if (idx1 == std::string::npos)
		return "";

	return path.substr(0, idx1);
}

bool Communal::CleanFloder(const char* dir)
{
	std::vector<std::string> vctDirs;
	std::vector<std::string> vctFiles;
	ListFilesA(dir, vctDirs, vctFiles, "*.*", false, true);
	for (int i = 0; i < vctFiles.size(); ++i)
	{
		DelFile(vctFiles[i].c_str());
	}

	return true;
}

bool Communal::StringReplace(std::string& strBase, const std::string& strSrc, const std::string& strDes)
{
	bool b = false;

	std::string::size_type pos = 0;
	std::string::size_type srcLen = strSrc.size();
	std::string::size_type desLen = strDes.size();
	pos = strBase.find(strSrc, pos);
	while ((pos != std::string::npos))
	{
		strBase.replace(pos, srcLen, strDes);
		pos = strBase.find(strSrc, (pos + desLen));
		b = true;
	}

	return b;
}

std::string Communal::MakeGUID()
{
	return std::to_string(GetTickCount64());
}

bool Communal::MakeFloder(const char* sDir)
{
	std::string strDir(sDir);//存放要创建的目录字符串
	StringReplace(strDir, "/", "\\");//把“/”转为“\”

	//确保以'\'结尾以创建最后一个目录
	if (strDir[strDir.length() - 1] != '\\')
	{
		strDir += '\\';
	}
	std::vector<std::string> vPath;//存放每一层目录字符串
	std::string strTemp;//一个临时变量,存放目录字符串
	bool bSuccess = false;//成功标志
	//遍历要创建的字符串
	for (int i = 0; i < strDir.length(); ++i)
	{
		if (strDir[i] != '\\')
		{//如果当前字符不是'\\'
			strTemp += strDir[i];
		}
		else
		{//如果当前字符是'\\'
			vPath.push_back(strTemp);//将当前层的字符串添加到数组中
			strTemp += '\\';
		}
	}

	//遍历存放目录的数组,创建每层目录
	std::vector<std::string>::const_iterator vIter;
	for (vIter = vPath.begin(); vIter != vPath.end(); vIter++)
	{
		//如果CreateDirectory执行成功,返回true,否则返回false
		bSuccess = CreateDirectoryA(vIter->c_str(), NULL) ? true : false;
	}

	return bSuccess;
}
