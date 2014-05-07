// dsxlua_app.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "lua_test_vs.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "C:\Program Files (x86)\Visual Leak Detector\include\vld.h"


// The one and only application object
CWinApp theApp;

using namespace std;

void RunScript( const char* in_pchPath )
{
   int result = 0;

   CString str;
   str.Format("\nExecuting script %s", in_pchPath );
   OutputDebugString(str.GetBuffer());

   lua_State* L = luaL_newstate();
   luaL_openlibs(L);

   // load the config.lua script 
   if((result = luaL_loadfile(L, in_pchPath)) == 0)
      result = lua_pcall(L, 0, 0, 0);
   if(result != 0)
   {
      char buf[255];
      _snprintf_s(buf, sizeof(buf), _TRUNCATE, "Cannot load main script: [%s]\n", lua_tostring(L, -1));
      OutputDebugString(buf);
   }
	else
      OutputDebugString("main script has completed.\n");

   lua_close(L);
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;
	
   HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL) 
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
            RunScript(argv[1]);
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}
   CoUninitialize();

#ifdef _DEBUG
//_CrtDumpMemoryLeaks();
#endif

	return nRetCode;
}