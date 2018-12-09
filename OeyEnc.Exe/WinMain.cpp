/* OeyEnc - Outlook Express plugin for decoding messages encoded with
 * the yEnc format. 
 *
 * Copyright (C) 2004-2008 Jeremy Boschen. All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software. 
 * 
 *  Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software in
 * a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 
 * 2. Altered source versions must be plainly marked as such, and must not
 * be misrepresented as being the original software.
 * 
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */
 
/*  WinMain.cpp
 *     Application entry point.
 */

#include "Stdafx.h"

#define INITGUID
#include <guiddef.h>
#include <msident.h>
#include <msoeapi.h>

#pragma warning( disable : 4127 )

#define SZ_APPLICATION_TITLE  L"OeyEnc"
#define SZ_APPLICATION_EXE    L"OEYENC.EXE"
#define SZ_APPLICATION_DLL    L"OEYENC.DLL"

/* Command Line Options:
      /username:
      /identity:
 */

/**********************************************************************

    Global Data

 **********************************************************************/
LPWSTR  g_szIdentity = NULL;
LPWSTR  g_szUserName = NULL;
LPWSTR  g_szPassword = NULL;
BOOL    g_bShellOpen = FALSE;
HMODULE g_hMuiModule = NULL;

bool _IsUserNameSpecified( ) throw()
{
   return ( (NULL != g_szUserName) && (L'\0' != *g_szUserName) );
}

bool _IsIdentitySpecified( ) throw()
{
   return ( (NULL != g_szIdentity) && (L'\0' != *g_szIdentity) );
}

static int __cdecl MessageBoxIdf( HWND hWnd, LPCWSTR pwszFormat, LPCWSTR pwszCaption, UINT uType, ... ) throw()
{
   int         iRet;
   va_list     args;
   DWORD       dwErr;
   MCharT<128> chCap;
   MCharT<512> chFmt;
   WCHAR       chBuf[1024];
   
   iRet   = -1;
   *chBuf = L'\0';

   chCap[0] = L'1';

   if ( IS_INTRESOURCE(pwszCaption) )
   {
      if ( 0 == MuiLoadString(g_hMuiModule,
                              (UINT)(ULONG_PTR)pwszCaption,
                              &chCap) )
      {
         _ASSERTE(FALSE);
         /* Failure */
         return ( -1 );
      }

      pwszCaption = chCap;
   }

   if ( IS_INTRESOURCE(pwszFormat) )
   {
      if ( 0 == MuiLoadString(g_hMuiModule,
                              (UINT)(ULONG_PTR)pwszFormat,
                              &chFmt) )
      {
         _ASSERTE(FALSE);
         /* Failure */
         return ( -1 );
      }

      pwszFormat = chFmt;
   }

   va_start(args, uType);

   dwErr = FormatMessage(FORMAT_MESSAGE_FROM_STRING,
                         pwszFormat,
                         0,
                         0,
                         chBuf,
                         _countof(chBuf),
                         &args);

   if ( dwErr > 0 )
   {
      iRet = MessageBox(hWnd, chBuf, pwszCaption, uType);
   }

   va_end(args);

   return ( iRet );
}

/**
 *  _GetOePath
 *     Retrieves the path for msimn.exe.
 */
bool __stdcall _GetOePath( LPWSTR pszBuf, size_t cchBuf ) throw()
{
   /* Look in the App Paths key in the registry */

   HRESULT hr;
   BOOL    bRet;
   WCHAR   chKey[128];
   WCHAR   chBuf[MAX_PATH];
   LPCWSTR rgszAppName[] = {L"msimn.exe", L"WinMail.exe"};
   size_t  idx;

   for ( idx = 0; idx < _countof(rgszAppName); idx++ )
   {
      hr = StringCchCopyW(chKey,
                          _countof(chKey),
                          L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\");
      _ASSERTE(SUCCEEDED(hr));
      hr = StringCchCatW(chKey,
                         _countof(chKey),
                         rgszAppName[idx]);
      _ASSERTE(SUCCEEDED(hr));

      bRet = _GetRegSetting(HKEY_LOCAL_MACHINE,
                            chKey,
                            NULL,
                            chBuf,
                            sizeof(chBuf));

      if ( bRet )
      {         
         /* This needs to be cleared before we check it after calling ExpandEnvironmentStrings */
         SetLastError(NO_ERROR);

         bRet = ExpandEnvironmentStrings(chBuf, 
                                         pszBuf, 
                                         (DWORD)cchBuf);

         if ( bRet )
         {
            return ( true );
         }
         
         ZeroMemory(pszBuf, 
                    sizeof(WCHAR) * cchBuf);
      }
   }

   MessageBoxIdf(NULL, 
                 MAKEINTRESOURCE(IDS_CANTFINDMSIMN),
                 SZ_APPLICATION_TITLE, 
                 MB_ICONERROR|MB_OK|MB_SETFOREGROUND);

   /* Failure */
   return ( false );
}

/**
 * ParseCommandLine
 */
bool __stdcall ParseCommandLine( LPCTSTR pszCmdLine )
{
   if ( !pszCmdLine ) 
   {
      return ( FALSE );
   }

   typedef struct _CMDINFO
   {
      LPCWSTR Tag;
      size_t  TagLength;
      LPWSTR* Command;
   }CMDINFO;

   size_t   idx;
   LPCWSTR  pszCmd;
   LPCWSTR  pwszStart;
   WCHAR    chCheckFor;
   size_t   cchCmd;
   LPWSTR   pwszBuf;
   
   static const CMDINFO rgCmd[] =
   {
      /* The /identity and /username commands are the same, but /username will replace /identity */
      {L"/identity:", _charsof(L"/identity:"), &g_szIdentity},
      {L"/username:", _charsof(L"/username:"), &g_szUserName}
   };

   pwszBuf = NULL;
   
   for ( idx = 0; idx < (sizeof(rgCmd) / sizeof(rgCmd[0])); idx++ )
   {
      pszCmd = StrStrI(pszCmdLine, rgCmd[idx].Tag);
      if ( NULL != pszCmd )
      {
         /* Bump past the command token... */
         pszCmd += rgCmd[idx].TagLength;

         pwszStart   = pszCmd;
         chCheckFor = (_T('\"') != *pszCmd ? _T(' ') : _T('\"'));

         if ( _T('\"') == chCheckFor )
         {
            pwszStart++; 
            pszCmd++;
         }

         while ( (_T('\0') != *pszCmd) && (chCheckFor != *pszCmd) )
         {
            pszCmd++;
         }

         cchCmd = (size_t)(pszCmd - pwszStart);
         pwszBuf = new WCHAR[cchCmd + 1];
         if ( NULL != pwszBuf )
         {               
            if ( FAILED(StringCchCopyNW(pwszBuf, 
                                        cchCmd + 1, 
                                        pwszStart, 
                                        cchCmd)) )
            {
               delete [] pwszBuf;
               pwszBuf = NULL;
            }
            else
            {
               *(rgCmd[idx].Command) = pwszBuf;
               pwszBuf = NULL;
            }
         }
      }
   }

   /* The /username option is only for systems that support CredUI, which is XP+. So for
    * earlier systems, clear the username if it was specified */
   if ( !IsWinNT(5, 1, 0, 0) && (NULL != g_szUserName) )
   {
      delete [] g_szUserName;
      g_szUserName = NULL;
   }

   /* Check if either a /eml: or /nws: command is present, indicating that a user clicked on a file within
    * the shell */
   if ( (NULL != StrStrI(pszCmdLine, _T("/eml:"))) ||
        (NULL != StrStrI(pszCmdLine, _T("-eml:"))) ||
        (NULL != StrStrI(pszCmdLine, _T("/nws:"))) ||
        (NULL != StrStrI(pszCmdLine, _T("-nws:"))) )
   {
      g_bShellOpen = TRUE;
   }

   delete [] pwszBuf;

   return ( true );
}

HRESULT __stdcall _LogonIdentity( )
{
   HRESULT               hr;
   IUserIdentity*        pIdent;
   IUserIdentityManager* pIdentMgr;
   WCHAR                 chBuf[CCH_IDENTITY_NAME_MAX_LENGTH + 1];

   /* This function should not be called on Vista+ */
   _ASSERTE(!IsWinNT(6,0,0,0));

   hr = CoInitialize(NULL);
   if ( FAILED(hr) )
   {
      /* Failure */
      return ( hr );
   }

   pIdent    = NULL;
   pIdentMgr = NULL;

   __try
   {
      hr = CoCreateInstance(CLSID_UserIdentityManager, 
                            NULL, 
                            CLSCTX_INPROC_SERVER|CLSCTX_INPROC_HANDLER|CLSCTX_LOCAL_SERVER, 
                            __uuidof(IUserIdentityManager), 
                            (LPVOID*)&pIdentMgr);

      if ( FAILED(hr) )
      {
         __leave;
      }
     
      /* Get the currently logged on identity name, if any */
      hr = pIdentMgr->GetIdentityByCookie(const_cast<GUID*>(&UID_GIBC_CURRENT_USER), 
                                          &pIdent);

      if ( false && SUCCEEDED(hr) )
      {
         hr = pIdent->GetName(chBuf, 
                              _countof(chBuf));

         if ( FAILED(hr) )
         {
            __leave;
         }

         if ( 0 == lstrcmpiW(chBuf, 
                              g_szIdentity) )
         {
            /* The currently logged on identity is the one specified by the user */
            hr = S_OK;
            __leave;
         }

         pIdent->Release();
         pIdent = NULL;
      }

      /* Check if the user specified a blank identity. For this case, we throw up the
       * login UI and we don't need to do anything else */
      hr = pIdentMgr->Logon(GetDesktopWindow(),
                            UIL_FORCE_UI,
                            &pIdent);
   }
   __finally
   {
      if ( NULL != pIdent )
      {
         pIdent->Release();
      }

      if ( NULL != pIdentMgr )
      {
         pIdentMgr->Release();
      }

      CoUninitialize();
   }

   /* Success / Failure */
   return ( hr );
}

HRESULT __stdcall _ExpandModulePath( LPCTSTR pszLibrary, LPTSTR pszBuf, size_t cchBuf ) throw()
{
   DWORD  cchModule;
   LPTSTR pszTmp;

   _ASSERTE(cchBuf <= ULONG_MAX);

   /* Use the executable file's module path as the base */
   cchModule = GetModuleFileName(NULL, 
                                 pszBuf, 
                                 static_cast<DWORD>(cchBuf));

   if ( (0 == cchModule) || (cchBuf == cchModule) )
   {
      /* Failure */
      return ( STRSAFE_E_INSUFFICIENT_BUFFER );
   }

   /* Find the file name portion of the module path */
   pszTmp = pszBuf;
   while ( _T('\0') != *pszTmp )
   {
      pszTmp++;
   }

   while ( pszTmp > pszBuf )
   {
      if ( _T('\\') == *pszTmp )
      {
         pszTmp++;
         break;
      }

      pszTmp--;
   }

   /* Append the library name */
   cchBuf -= (DWORD_PTR)(pszTmp - pszBuf);

   return ( StringCchCopy(pszTmp, 
                          cchBuf, 
                          pszLibrary) );
}

/**
 * FreeUserCredentials
 *    Wipes out memory allocated by various routines
 */
void __stdcall FreeUserCredentials( )
{
   if ( NULL != g_szIdentity )
   {
      SecureZeroMemory(g_szIdentity, 
                       sizeof(TCHAR) * lstrlen(g_szIdentity));
      delete [] g_szIdentity;
   }   

   if ( NULL != g_szUserName )
   {
      SecureZeroMemory(g_szUserName, 
                       sizeof(TCHAR) * lstrlen(g_szUserName));
      delete [] g_szUserName;
   }

   if ( NULL != g_szPassword )
   {
      SecureZeroMemory(g_szPassword, 
                       sizeof(TCHAR) * lstrlen(g_szPassword));
      delete [] g_szPassword;
   }
}

DWORD __cdecl FormatMessagef( DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPTSTR lpBuffer, DWORD nSize, ... ) throw()
{
   DWORD   dwRet;
   va_list args;

   va_start(args,
            nSize);

   dwRet = FormatMessage(dwFlags,
                         lpSource,
                         dwMessageId,
                         dwLanguageId,
                         lpBuffer,
                         nSize,
                         &args);

   va_end(args);

   return ( dwRet );
}

DWORD _PromptForWindowsCredentials( HANDLE* phToken ) throw()
{

   DWORD       dwErr;
   CREDUI_INFO UiInfo;
   WCHAR       chUserName[CREDUI_MAX_USERNAME_LENGTH+1];
   WCHAR       chPassword[CREDUI_MAX_PASSWORD_LENGTH+1];
   BOOL        bSave;
   DWORD       dwFlags;
   LPWSTR      pwszBuf;
   WCHAR       chBuffer[1024];   
   MCharT<128> chClient;
   MCharT<128> chMessage;
   BOOL        bRet;
   DWORD       dwAuthErr;

   /* Initialize outputs */
   (*phToken) = NULL;

   /* Initialize locals */
   ZeroMemory(&UiInfo, 
              sizeof(UiInfo));

   ZeroMemory(chUserName, 
              sizeof(chUserName));

   ZeroMemory(chPassword, 
              sizeof(chPassword));

   if ( _IsUserNameSpecified() )
   {
      StringCchCopy(chUserName, 
                    _countof(chUserName), 
                    g_szUserName);
   }
   
   if ( 0 == MuiLoadString(g_hMuiModule, 
                           IDS_CREDENTIALSPROMPT,
                           &chMessage) )
   {
      dwErr = ERROR_RESOURCE_NOT_FOUND;
      /* Failure */
      goto __CLEANUP;
   }

   if ( 0 == MuiLoadString(g_hMuiModule,
                           IsWinNT(6,0,0,0) ? IDS_WINDOWSMAIL : IDS_OUTLOOKEXPRESS,
                           &chClient) )
   {
      dwErr = ERROR_RESOURCE_NOT_FOUND;
      /* Failure */
      goto __CLEANUP;
   }
   
   dwErr = FormatMessagef(FORMAT_MESSAGE_FROM_STRING,
                          static_cast<LPCWSTR>(chMessage),
                          0,
                          0,
                          chBuffer,
                          _countof(chBuffer),
                          static_cast<LPCWSTR>(chClient));

   if ( 0 == dwErr )
   {
      dwErr = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   UiInfo.cbSize         = sizeof(UiInfo);
   UiInfo.pszCaptionText = SZ_APPLICATION_TITLE;
   UiInfo.pszMessageText = chBuffer;
   bSave                 = TRUE;
   dwFlags               = CREDUI_FLAGS_DO_NOT_PERSIST|CREDUI_FLAGS_GENERIC_CREDENTIALS;
   dwAuthErr             = 0;

   while ( true )
   {
      dwErr = CredUIPromptForCredentialsW(&UiInfo,
                                          SZ_APPLICATION_TITLE,
                                          NULL,
                                          dwAuthErr,
                                          chUserName,
                                          _countof(chUserName),
                                          chPassword,
                                          _countof(chPassword),
                                          &bSave,
                                          dwFlags);

      if ( NO_ERROR != dwErr )
      {
         /* Failure */
         break;
      }
      
      /* Make sure the account is valid */
      bRet = LogonUser(chUserName,
                       L".",
                       chPassword,
                       LOGON32_LOGON_INTERACTIVE,
                       LOGON32_PROVIDER_DEFAULT,
                       phToken);

      if ( bRet )
      {
         /* Success */
         break;
      }

      dwAuthErr = GetLastError();
      if ( !CREDUI_IS_AUTHENTICATION_ERROR(dwAuthErr) && !CREDUI_NO_PROMPT_AUTHENTICATION_ERROR(dwAuthErr) )
      {
         /* Failure */
         break;
      }

      dwFlags |= CREDUI_FLAGS_INCORRECT_PASSWORD;

      SecureZeroMemory(chPassword,
                       sizeof(chPassword));
   }

   if ( NO_ERROR == dwErr )
   {
      pwszBuf = new WCHAR[_countof(chUserName)];
      if ( NULL != pwszBuf )
      {               
         if ( FAILED(StringCchCopyW(pwszBuf, 
                                    _countof(chUserName), 
                                    chUserName)) )
         {
            delete [] pwszBuf;
            pwszBuf = NULL;
         }
         else
         {
            delete [] g_szUserName;
            g_szUserName = pwszBuf;
            pwszBuf = NULL;
         }
      }

      pwszBuf = new WCHAR[_countof(chPassword)];
      if ( NULL != pwszBuf )
      {               
         if ( FAILED(StringCchCopyW(pwszBuf, 
                                    _countof(chPassword), 
                                    chPassword)) )
         {
            SecureZeroMemory(pwszBuf, 
                             _countof(chPassword));

            delete [] pwszBuf;
            pwszBuf = NULL;
         }
         else
         {
            if ( NULL != g_szPassword )
            {
               SecureZeroMemory(g_szPassword,
                                sizeof(TCHAR) * lstrlen(g_szPassword));
            }

            delete [] g_szPassword;
            g_szPassword = pwszBuf;
            pwszBuf = NULL;
         }
      }
   }

__CLEANUP:
   SecureZeroMemory(chPassword, 
                    _countof(chPassword));

   return ( dwErr );
}

BOOL _CreateOeProcess( LPCWSTR pwszPath, LPWSTR pwszCommandLine, LPCWSTR /*pwszDirectory*/, DWORD dwCreateFlags, LPSTARTUPINFOW /*lpStartupInfo*/, LPPROCESS_INFORMATION lpProcessInfo ) throw()
{
   BOOL        bRet;
   STARTUPINFO StartupInfo;   
   size_t      cchBuff;
   LPWSTR      pwszBuff;
   DWORD       cchCompName;
   TCHAR       chCompName[MAX_COMPUTERNAME_LENGTH+1];
   
   ZeroMemory(&StartupInfo, 
              sizeof(StartupInfo));

   ZeroMemory(chCompName,
              sizeof(chCompName));

   cchCompName    = _countof(chCompName);
   StartupInfo.cb = sizeof(StartupInfo);

   /* Build up the full command line */
   cchBuff  = lstrlenW(pwszPath);
   cchBuff += lstrlenW(pwszCommandLine);
   cchBuff += _charsof("\"\" \0");

   pwszBuff = new WCHAR[cchBuff];
   if ( NULL != pwszBuff )
   {
      if ( FAILED(StringCchPrintfW(pwszBuff,
                                   cchBuff,
                                   L"\"%s\" %s",
                                   pwszPath,
                                   pwszCommandLine)) )
      {
         StringCchCopy(pwszBuff,
                       cchBuff,
                       pwszCommandLine);
      }
   }

   if ( !g_bShellOpen && _IsUserNameSpecified() )
   {
      /* CreateProcessWithLogonW() seems to require a valid computer name on some platforms */
      bRet = GetComputerNameEx(ComputerNameNetBIOS,
                               chCompName,
                               &cchCompName);

      if ( bRet )
      {
         bRet = CreateProcessWithLogonW(g_szUserName, 
                                        chCompName,
                                        g_szPassword, 
                                        LOGON_WITH_PROFILE, 
                                        pwszPath,
                                        pwszBuff ? pwszBuff : pwszCommandLine,
                                        dwCreateFlags,
                                        NULL,
                                        NULL,
                                        NULL,
                                        lpProcessInfo);
      }
   }
   else
   {
      /* Launch as the current user */
      bRet = CreateProcess(pwszPath, 
                           pwszBuff ? pwszBuff : pwszCommandLine, 
                           NULL, 
                           NULL, 
                           FALSE, 
                           dwCreateFlags, 
                           NULL, 
                           NULL,
                           &StartupInfo, 
                           lpProcessInfo);
   }

   delete [] pwszBuff;

   /* Success / Failure */
   return ( bRet );
}

/**
 * _tWinMain
 */ 
int WINAPI _tWinMain( HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPTSTR pszCmdLine, int /*nCmdShow*/ ) throw()
{
   DWORD               dwErr;
   HRESULT             hr;
   BOOL                bRet;
   HWND                hwndOE;
   DWORD               dwPid;
   DWORD               dwTid;
   PROCESS_INFORMATION ProcessInfo;
   TCHAR               chModule[MAX_PATH];
   WCHAR               chOePath[MAX_PATH];
   HANDLE              hToken;
   TOKEN_PRIVILEGES    TokenPriv;
   MCharT<128>         szClient;

   dwErr                = NO_ERROR;
   hToken               = NULL;
   ProcessInfo.hProcess = NULL;
   ProcessInfo.hThread  = NULL;

#ifdef _DEBUG
   if ( NULL != _tcsstr(pszCmdLine,
                        _T("/debug")) )
   {
      while ( !IsDebuggerPresent() )
      {
         Sleep(1000);
      }

      __debugbreak();
   }
#endif /* _DEBUG */

#ifdef _MUI_ENABLE
   MuiInitializeLibrary(SZ_APPLICATION_EXE,
                        &g_hMuiModule);
#else /* _MUI_ENABLE */
   g_hMuiModule = GetModuleHandle(NULL);
#endif /* _MUI_ENABLE */

   ParseCommandLine(pszCmdLine);

   if ( FAILED(_ExpandModulePath(SZ_APPLICATION_DLL, 
                                 chModule, 
                                 _countof(chModule))) )
   {
      /* Failure */
      goto __CLEANUP;
   }      
   
   /* Check if OE is already running... */
   hwndOE = FindWindow(STR_MSOEAPI_BROWSERCLASS, 
                       NULL);

   if ( NULL != hwndOE )
   {               
      dwTid = GetWindowThreadProcessId(hwndOE, 
                                       &dwPid);

      if ( !dwTid )
      {
         dwErr = GetLastError();
         /* Failure */
         goto __CLEANUP;
      }

      ZeroMemory(&TokenPriv,
                 sizeof(TokenPriv));
      
      bRet = SetProcessTokenPrivilege(GetCurrentProcess(), 
                                      SE_DEBUG_NAME, 
                                      TRUE, 
                                      &TokenPriv);

      if ( !bRet )
      {
         dwErr = GetLastError();
         /* Failure */
         goto __CLEANUP;
      }

      ProcessInfo.hProcess = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_QUERY_INFORMATION|SYNCHRONIZE, 
                                         FALSE, 
                                         dwPid);

      if ( !ProcessInfo.hProcess )
      {
         dwErr = GetLastError();
         /* Failure */
         goto __CLEANUP;
      }
      
      if ( !IsTokenPrivilegeEnabled(&TokenPriv) )
      {
         bRet = SetProcessTokenPrivilege(GetCurrentProcess(), 
                                         SE_DEBUG_NAME, 
                                         FALSE, 
                                         NULL);

         if ( !bRet )
         {
            dwErr = GetLastError();
            /* Failure */
            goto __CLEANUP;
         }
      }

      /* Insert and load OeyEnc.dll in OE's process */
      dwErr = LoadLibraryInProcess(ProcessInfo.hProcess, 
                                   chModule);

      if ( NO_ERROR != dwErr )
      {
         /* Failure */
         goto __CLEANUP;
      }

      if ( g_bShellOpen )
      {
         goto __CONTINUESHELLOPEN;
      }

      goto __ACTIVATEWINDOW;
   }

   if ( !g_bShellOpen && _IsUserNameSpecified() )
   {
      dwErr = _PromptForWindowsCredentials(&hToken);
      if ( NO_ERROR != dwErr )
      {
         /* Failure */
         goto __CLEANUP;
      }
   }

   /* Identities don't work when logging on as another user */
   if ( !g_bShellOpen && !_IsUserNameSpecified() && _IsIdentitySpecified() )
   {
      /* Log on or off the specified identity */
      hr = _LogonIdentity();

      if ( FAILED(hr) )
      {
         dwErr = HRESULT_CODE(hr);
         /* Failure */
         goto __CLEANUP;
      }
   }
   
__CONTINUESHELLOPEN:
   if ( !_GetOePath(chOePath, 
                    _countof(chOePath)) )
   {
      /* Failure */
      goto __CLEANUP;
   }

   /* Load OE in a suspended state. This is always done to support the OE command line options for
    * opening .nws/.eml files */
   if ( !_CreateOeProcess(chOePath,
                          pszCmdLine,
                          NULL,
                          CREATE_SUSPENDED,
                          NULL,
                          &ProcessInfo) )
   {
      dwErr = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   dwErr = ResumeProcessWithModule(chOePath,
                                   &ProcessInfo,
                                   chModule);

   if ( NO_ERROR != dwErr )
   {
      /* Kill the process */
      TerminateProcess(ProcessInfo.hProcess,
                       dwErr);

      /* Failure */
      goto __CLEANUP;
   }

__ACTIVATEWINDOW:         
   hwndOE = FindWindow(STR_MSOEAPI_BROWSERCLASS, 
                       NULL);

   if ( NULL != hwndOE )
   {
      if ( IsIconic(hwndOE) )
      {
         ShowWindow(hwndOE, 
                    SW_RESTORE);
      }

      SetActiveWindow(hwndOE);
      SetForegroundWindow(hwndOE);
      SetFocus(hwndOE);
   }

__CLEANUP:
   if ( NULL != ProcessInfo.hThread )
   {
      CloseHandle(ProcessInfo.hThread);
   }

   if ( NULL != ProcessInfo.hProcess )
   {
      CloseHandle(ProcessInfo.hProcess);
   }

   if ( NULL != hToken )
   {
      CloseHandle(hToken);
   }

   FreeUserCredentials();

#ifdef _DEBUG
   if ( !IsDebuggerPresent() )
#endif /* _DEBUG */
   {
      if ( (NO_ERROR != dwErr) && (ERROR_CANCELLED != dwErr) )
      {
         if ( 0 != MuiLoadString(g_hMuiModule, 
                                 IsWinNT(6,0,0,0) ? IDS_WINDOWSMAIL : IDS_OUTLOOKEXPRESS,
                                 &szClient) )
         {
            MessageBoxIdf(NULL,
                          MAKEINTRESOURCE(IDS_CANTATTACHMSIMN),
                          SZ_APPLICATION_TITLE,
                          MB_ICONERROR|MB_OK|MB_SETFOREGROUND,
                          static_cast<LPCWSTR>(szClient),
                          dwErr);
         }
      }
   }

#ifdef _MUI_ENABLE
   MuiUninitializeLibrary(SZ_APPLICATION_EXE,
                          g_hMuiModule);
#endif /* _MUI_ENABLE */

   return ( (int)dwErr );
}
