#pragma once
#include <stdio.h> 
#include <string>
#if (defined WIN32 || defined _WIN32 || defined WINCE || defined __CYGWIN__)
#include "windows.h"
#else
#include <dlfcn.h>
#include <errno.h>
#endif

// 动态库辅助类
class CDllHelper
{
private:
	char m_szFileName[_MAX_DIR];                  // 文件名

#if defined WINDOWS || _WIN32
    HMODULE	m_hLibrary;                    // 模块句柄
#else
	void* m_hLibrary;
#endif

protected:
	 // 加载动态库
	void dll_Load()
	{
		if (m_szFileName == nullptr)
		{
			return;
		}

#if defined WINDOWS || _WIN32
		m_hLibrary =  LoadLibraryExA(m_szFileName, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
		return dlopen(m_szFileName, RTLD_NOW);
#endif		
	}

	//卸载dll
	void dll_FreeLib()
	{
		if (m_hLibrary == NULL)
		{
			return;
		}

#if defined WINDOWS || _WIN32
		FreeLibrary(m_hLibrary);
#else
		dlclose(m_hLibrary);
#endif
	}

public:
	CDllHelper(const char *lpszFileName) :
		m_hLibrary(NULL)
	{
		if (lpszFileName)
		{
			sprintf_s(m_szFileName, _MAX_DIR, lpszFileName);
			dll_Load();
		}		
	}

	~CDllHelper()
	{
		dll_FreeLib();
		m_hLibrary = NULL;
	}

public:
    // 获取Proc
	template<typename Proc>
	Proc GetProcedure(char* lpszProc)
	{
		if (m_hLibrary == NULL)
		{
			return nullptr;
		}		

		Proc pfnProc = NULL;

#if defined WINDOWS || _WIN32
		
		pfnProc = (Proc)GetProcAddress(m_hLibrary, lpszProc);
		return pfnProc;
#else
		pfnProc = (Proc)dlsym(m_hLibrary, lpszProc)
		return pfnProc;
#endif
	}

	const char* dll_GetLastError()
	{
#if defined WINDOWS || _WIN32
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();
		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&lpMsgBuf,
			0, NULL);
		return (const char*)lpMsgBuf;
#else
		return dlerror();
#endif
	}
};