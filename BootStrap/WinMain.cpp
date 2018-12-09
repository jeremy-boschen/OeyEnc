/* BootStrap - A very generic MSI file bootstrapper.
 *
 * Copyright (C) 2004-2008 Jeremy Boschen. All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software. 
 *
 * Permission is granted to anyone to use this software for any purpose,
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

/* WinMain.cpp
 *    Everything...
 */

#include "Stdafx.h"

typedef const BYTE* LPCBYTE;

/**
 * __C_IMAGE_SIZE
 *    This is the size of the final bootstrap.exe image. It has to be set
 *    manually after a successful build. The default is the size generated
 *    by the release build
 */
#ifdef _DEBUG
   #define __C_IMAGE_SIZE 129536
#else /* _DEBUG */
   #define __C_IMAGE_SIZE 18944
#endif /* _DEBUG */

/**
 * MessageBoxf
 *    MessageBox wrapper with printf style support for the message text.
 */
int __cdecl MessageBoxf( HWND hWnd, LPCTSTR lpTextFormat, LPCTSTR lpCaption, UINT uType, ... ) throw()
{
   int      iRet;
   va_list  args;
   TCHAR    chBuf[1024];

   /* Initialize locals */
   iRet = -1;

   va_start(args,
            uType);

   if ( SUCCEEDED(StringCchVPrintf(chBuf,
                                   _countof(chBuf),
                                   lpTextFormat,
                                   args)) )
   {
      iRet = MessageBox(hWnd,
                        chBuf,
                        lpCaption,
                        uType);
   }

   va_end(args);

   /* Success / Failure */
   return ( iRet );
}

/**
 * InstallPackage
 *    Installs/uninstalls a package
 *
 * Parameters
 *    pszPackage
 *       [in] Path of the package to install
 *    bInstall
 *       [in] TRUE to install the package, FALSE to uninstall it
 *
 * Return Value
 *    Returns ERROR_SUCCESS if successful, otherwise an error value
 *    
 */
DWORD InstallPackage( LPCTSTR pszPackage, BOOL bInstall ) throw()
{
   DWORD          dwResult;
   
   INSTALLUILEVEL eInstallLevel;
   MSIHANDLE      hProduct;

   DWORD          cchBuf;
   TCHAR          chProductCode[128];
   TCHAR          chProductVersion[128];
   TCHAR          chProductName[260];
   LPCTSTR        pszName;
   LPCTSTR        pszVersion;

   INSTALLSTATE   eState;

   BOOL           bInstalled;
   BOOL           bDoUninstall;    
 
   TCHAR          chQuery[256];
   int            iQueryRet;
               

   /* Initialize locals */
   hProduct     = NULL;
   bInstalled   = FALSE;
   bDoUninstall = TRUE;

   /* 
    * First get the product code and version of the package being installed 
    */
   eInstallLevel = MsiSetInternalUI(INSTALLUILEVEL_NONE, 
                                    NULL);

   dwResult = MsiOpenPackageEx(pszPackage, 
                               MSIOPENPACKAGEFLAGS_IGNOREMACHINESTATE, 
                               &hProduct);

   if ( ERROR_SUCCESS != dwResult )
   {
      /* Failure */
      goto __CLEANUP;
   }

   /*
    * Check for whether the package wants previous versions removed or not
    */
   if ( MSICONDITION_TRUE == MsiEvaluateCondition(hProduct, 
                                                  _T("NOT BSPREMOVEPREVIOUSVERION")) )
   {
      /* We're done with the package */
      MsiCloseHandle(hProduct);
      hProduct = NULL;

      /* Move on to the install phase */
      goto __INSTALL;
   }

   ZeroMemory(chProductCode,
              sizeof(chProductCode));
   
   cchBuf   = _countof(chProductCode);
   dwResult = MsiGetProductProperty(hProduct, 
                                    _T("ProductCode"), 
                                    chProductCode, 
                                    &cchBuf);

   if ( ERROR_SUCCESS != dwResult )
   {
      /* Failure */
      goto __CLEANUP;
   }

   ZeroMemory(chProductVersion,
              sizeof(chProductVersion));

   pszVersion = chProductVersion;
   cchBuf     = _countof(chProductVersion);
   dwResult   = MsiGetProductProperty(hProduct, 
                                      _T("ProductVersion"), 
                                      chProductVersion, 
                                      &cchBuf);

   if ( ERROR_SUCCESS != dwResult )
   {
      /* Don't fail, but force an uninstall */
      pszVersion = NULL;
   }

   ZeroMemory(chProductName,
              sizeof(chProductName));

   pszName  = chProductName;
   cchBuf   = _countof(chProductName);
   dwResult = MsiGetProductProperty(hProduct,
                                    _T("ProductName"),
                                    chProductName,
                                    &cchBuf);

   if ( ERROR_SUCCESS != dwResult )
   {
      /* Don't fail, but force setup for a no-name installation */
      pszName = NULL;
   }

   /* We're done with the package */
   MsiCloseHandle(hProduct);
   hProduct = NULL;

   /*
    * Now check if the package is already installed, and if so
    * compare its version to the new one's. If they are different
    * then uninstall the existing one.
    */
   MsiSetInternalUI(INSTALLUILEVEL_DEFAULT, 
                    NULL);

   eState = MsiQueryProductState(chProductCode);

   if ( (INSTALLSTATE_ABSENT == eState)     ||
        (INSTALLSTATE_ADVERTISED == eState) ||
        (INSTALLSTATE_DEFAULT == eState) )
   {
      bInstalled = TRUE;
      iQueryRet  = 0;

      if ( NULL != pszVersion )
      {
         dwResult = MsiOpenProduct(chProductCode,
                                   &hProduct);

         if ( ERROR_SUCCESS != dwResult )
         {
            /* Failure */
            goto __CLEANUP;
         }

         ZeroMemory(chQuery,
                    sizeof(chQuery));

         if ( FAILED(StringCchPrintf(chQuery,
                                     _countof(chQuery),
                                     _T("ProductVersion=\"%s\""),
                                     pszVersion)) )
         {
            dwResult = ERROR_INSTALL_FAILURE;
            /* Failure */
            goto __CLEANUP;
         }

         if ( MSICONDITION_TRUE == MsiEvaluateCondition(hProduct, 
                                                        chQuery) )
         {
            bDoUninstall = FALSE;
         }
         else 
         {
            if ( FAILED(StringCchPrintf(chQuery,
                                        _countof(chQuery),
                                        _T("ProductVersion<\"%s\""), 
                                        pszVersion)) )
            {
               dwResult = ERROR_INSTALL_FAILURE;
               /* Failure */
               goto __CLEANUP;
            }

            if ( MSICONDITION_TRUE == MsiEvaluateCondition(hProduct, 
                                                           chQuery) )
            {
               iQueryRet = 1;
            }
            else
            {
               if ( FAILED(StringCchPrintf(chQuery,
                                           _countof(chQuery),
                                           _T("ProductVersion>\"%s\""), 
                                           pszVersion)) )
               {
                  dwResult = ERROR_INSTALL_FAILURE;
                  /* Failure */
                  goto __CLEANUP;
               }

               if ( MSICONDITION_TRUE == MsiEvaluateCondition(hProduct, 
                                                              chQuery) )
               {
                  iQueryRet = 2;
               }
            }
         }

         MsiCloseHandle(hProduct);
         hProduct = NULL;

         if ( 1 == iQueryRet )
         {
            if ( IDYES != MessageBoxf(NULL, 
                                      _T("A previous version of %s is installed and must be removed before continuing.\n")
                                      _T("\n")
                                      _T("Do you want to remove the previous version and continue?"),
                                      !pszName ? _T("Setup") : pszName, 
                                      MB_ICONQUESTION|MB_YESNO|MB_SETFOREGROUND, 
                                      !pszName ? _T("this application") : pszName) )
            {
               /* User canceled */
               goto __CLEANUP;
            }

            bDoUninstall = TRUE;
         }
         else if ( 2 == iQueryRet )
         {
            if ( IDYES != MessageBoxf(NULL, 
                                     _T("A newer version of %s is already installed and must be removed before continuing.\n")
                                     _T("\n")
                                     _T("Do you want to remove the newer version and continue?"), 
                                     !pszName ? _T("Setup") : pszName, 
                                     MB_ICONQUESTION|MB_YESNO|MB_SETFOREGROUND, 
                                     !pszName ? _T("this application") : pszName) )
            {
               /* User cancelled */
               goto __CLEANUP;
            }
            
            bDoUninstall = TRUE;
         }
      }
      
      if ( bDoUninstall )
      {
         dwResult = MsiConfigureProduct(chProductCode,
                                        INSTALLLEVEL_MAXIMUM, 
                                        INSTALLSTATE_ABSENT);
      
         if ( ERROR_SUCCESS != dwResult )
         {
            /* Failure */
            goto __CLEANUP;
         }
         
         bInstalled = FALSE;
      }
   }

__INSTALL:
   if ( bInstall )
   {
      MsiSetInternalUI(INSTALLUILEVEL_FULL, 
                       NULL);

      /* Install the product */
      dwResult = MsiInstallProduct(pszPackage, 
                                   bInstalled ? _T("REINSTALLMODE=vomus") : NULL);
   }

__CLEANUP:
   if ( NULL != hProduct )
   {
      MsiCloseHandle(hProduct);
   }

   if ( INSTALLUILEVEL_NOCHANGE != eInstallLevel )
   {
      MsiSetInternalUI(eInstallLevel,
                       NULL);
   }

   return ( dwResult );
}

DWORD __stdcall GetPackageFileName( LPCTSTR pszPackage, LPTSTR pszBuf, DWORD cchBuf ) throw()
{   
   DWORD          dwResult;
   INSTALLUILEVEL eInstallLevel;
   MSIHANDLE      hProduct;
   DWORD          cchTmp;

   /* Initialize locals */
   hProduct = NULL;

   /* Check for the BSPIMAGEFILENAME property, and if that's not there fallback to the
    * ProductName property */
   eInstallLevel = MsiSetInternalUI(INSTALLUILEVEL_NONE, 
                                    NULL);

   dwResult = MsiOpenPackageEx(pszPackage, 
                               MSIOPENPACKAGEFLAGS_IGNOREMACHINESTATE, 
                               &hProduct);

   if ( ERROR_SUCCESS != dwResult )
   {
      /* Failure */
      goto __CLEANUP;
   }

   cchTmp   = cchBuf;
   dwResult = MsiGetProductProperty(hProduct,
                                    _T("BSPIMAGEFILENAME"),
                                    pszBuf,
                                    &cchTmp);

   if ( (ERROR_SUCCESS != dwResult) || (cchTmp == 0) )
   {
      cchTmp   = cchBuf;
      dwResult = MsiGetProductProperty(hProduct, 
                                       _T("ProductName"), 
                                       pszBuf, 
                                       &cchTmp);
   }

__CLEANUP:
   if ( NULL != hProduct )
   {
      MsiCloseHandle(hProduct);
   }

   if ( INSTALLUILEVEL_NOCHANGE != eInstallLevel )
   {
      MsiSetInternalUI(eInstallLevel,
                       NULL);
   }

   /* Success / Failure */
   return ( dwResult );
}

DWORD __stdcall ExportPackageData( LPTSTR pszBuf, size_t cchBuf, HMODULE hModule ) throw()
{
   DWORD          dwRet;

   DWORD          cchModule;
   TCHAR          chModule[MAX_PATH];
   HANDLE         hModFile;
   HANDLE         hModMap;
   LPVOID         pModMap;
   LARGE_INTEGER  cbModFile;

   DWORD          cchTmpPath;
   TCHAR          chTmpPath[MAX_PATH];
   HANDLE         hMsiFile;
   DWORD          cchMsiFile;
   TCHAR          chMsiFile[MAX_PATH];

   PUCHAR         pbPackage;
   ULONG          cbPackage;
   DWORD          cbWritten;

   /* Initialize locals */
   hModFile = INVALID_HANDLE_VALUE;
   hModMap  = NULL;
   pModMap  = NULL;
   hMsiFile = INVALID_HANDLE_VALUE;
   
   ZeroMemory(chModule,
              sizeof(chModule));

   cchModule = GetModuleFileName(hModule, 
                                 chModule, 
                                 _countof(chModule));

   if ( (0 == cchModule) || ((_countof(chModule) == cchModule) && (ERROR_INSUFFICIENT_BUFFER == GetLastError())) )
   {
      dwRet = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   hModFile = CreateFile(chModule, 
                         GENERIC_READ, 
                         FILE_SHARE_READ|FILE_SHARE_WRITE, 
                         NULL, 
                         OPEN_EXISTING, 
                         0, 
                         NULL);

   if ( INVALID_HANDLE_VALUE == hModFile )
   {
      dwRet = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   /* Create a mapping for the file that will be used to probe the headers,
    * validate the image, and provide data for the msi file */
   hModMap = CreateFileMapping(hModFile, 
                               NULL, 
                               PAGE_READONLY|SEC_COMMIT, 
                               0, 
                               0, 
                               NULL);

   if ( !hModMap )
   {
      dwRet = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   pModMap = MapViewOfFile(hModMap, 
                           FILE_MAP_READ, 
                           0, 
                           0, 
                           0);

   if ( !pModMap )
   {
      dwRet = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   if ( !GetFileSizeEx(hModFile,
                       &cbModFile) )
   {
      dwRet = ERROR_BAD_EXE_FORMAT;
      /* Failure */
      goto __CLEANUP;
   }

   if ( cbModFile.QuadPart < (sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS)) )
   {
      dwRet = ERROR_BAD_EXE_FORMAT;
      /* Failure */
      goto __CLEANUP;
   }

   if ( (__C_IMAGE_SIZE >= cbModFile.QuadPart) || (cbModFile.QuadPart > ULONG_MAX) )
   {
      dwRet = ERROR_BAD_EXE_FORMAT;
      /* Failure */
      goto __CLEANUP;
   }

   ZeroMemory(chTmpPath,
              sizeof(chTmpPath));

   /* Create a temporary file name for the output msi file */
   cchTmpPath = GetTempPath(_countof(chTmpPath),
                            chTmpPath);

   if ( 0 == cchTmpPath )
   {
      dwRet = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   ZeroMemory(chMsiFile,
              sizeof(chMsiFile));

   cchMsiFile = GetTempFileName(chTmpPath,
                                _T("pkg"),
                                GetCurrentThreadId(),
                                chMsiFile);

   if ( 0 == cchMsiFile )
   {
      dwRet = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   hMsiFile = CreateFile(chMsiFile, 
                         GENERIC_WRITE, 
                         0, 
                         NULL, 
                         CREATE_ALWAYS, 
                         FILE_ATTRIBUTE_TEMPORARY, 
                         NULL);

   if ( INVALID_HANDLE_VALUE == hMsiFile )
   {
      dwRet = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   pbPackage = reinterpret_cast<PUCHAR>(pModMap) + __C_IMAGE_SIZE;
   if ( pbPackage < reinterpret_cast<PUCHAR>(pModMap) )
   {
      dwRet = ERROR_BAD_EXE_FORMAT;
      /* Failure */
      goto __CLEANUP;
   }

   cbPackage = static_cast<ULONG>(cbModFile.LowPart) - __C_IMAGE_SIZE;
   cbWritten = 0;
   if ( !WriteFile(hMsiFile, 
                   pbPackage, 
                   cbPackage, 
                   &cbWritten, 
                   NULL) || (cbWritten < cbPackage) )
   {
      dwRet = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }
   
   if ( FAILED(StringCchCopy(pszBuf,
                             cchBuf,
                             chMsiFile)) )
   {
      *pszBuf = _T('\0');
      dwRet   = ERROR_INSUFFICIENT_BUFFER;
   }
   else
   {
      dwRet = ERROR_SUCCESS;
   }

__CLEANUP:
   if ( INVALID_HANDLE_VALUE != hMsiFile )
   {
      CloseHandle(hMsiFile);
   }

   if ( NULL != pModMap )
   {
      UnmapViewOfFile(pModMap);
   }

   if ( NULL != hModMap )
   {
      CloseHandle(hModMap);
   }

   if ( INVALID_HANDLE_VALUE != hModFile )
   {
      CloseHandle(hModFile);
   }   

   /* Success / Failure */
   return ( dwRet );
}

DWORD PathRemoveFileSpec( LPTSTR pszPath )
{
   LPTSTR pszTmp;

   pszTmp = pszPath;

   while ( _T('\0') != *pszTmp )
   {
      pszTmp++;
   }

   while ( pszTmp > pszPath )
   {
      if ( _T('\\') == *pszTmp )
      {
         *pszTmp = _T('\0');
         /* Success */
         return ( ERROR_SUCCESS );
      }

      pszTmp--;
   }

   /* Failure */
   return ( ERROR_BAD_PATHNAME );
}

int _tWinMain( HINSTANCE, HINSTANCE, LPTSTR, int )
{
   HRESULT hr;
   DWORD   dwRet;

   TCHAR   chPackage[MAX_PATH];
   TCHAR   chPackageName[MAX_PATH];
   TCHAR   chPackagePath[MAX_PATH];

   /* Initialize locals */
   *chPackage     = _T('\0');
   *chPackagePath = _T('\0');

   hr = CoInitialize(NULL);
   if ( FAILED(hr) )
   {
      /* Failure */
      return ( static_cast<int>(hr) );
   }

   ZeroMemory(chPackage,
              sizeof(chPackage));

   dwRet = ExportPackageData(chPackage,
                             _countof(chPackage),
                             NULL);

   if ( ERROR_SUCCESS != dwRet )
   {
      *chPackage = _T('\0');
      /* Failure */
      goto __CLEANUP;
   }
   
   dwRet = MsiVerifyPackage(chPackage);

   if ( ERROR_SUCCESS != dwRet )
   {
      MessageBoxf(GetActiveWindow(),
                  _T("Setup cannot continue because the installation file is corrupt.\n")
                  _T("\n")
                  _T("Error Code: %08x"),
                  _T("Setup"),
                  MB_ICONERROR|MB_OK|MB_SETFOREGROUND,
                  dwRet);

      /* Failure */
      goto __CLEANUP;
   }

   /* MSI requires that packages have the same filename, so get it from the 
    * package just extracted and rename the file */
   ZeroMemory(chPackageName,
              sizeof(chPackageName));

   *chPackageName = _T('\\');

   dwRet = GetPackageFileName(chPackage,
                              chPackageName + 1,
                              _countof(chPackageName) - 1);

   if ( ERROR_SUCCESS != dwRet )
   {
      /* Failure */
      goto __CLEANUP;
   }

   ZeroMemory(chPackagePath,
              sizeof(chPackagePath));

   hr = StringCchCopy(chPackagePath,
                      _countof(chPackagePath),
                      chPackage);

   if ( FAILED(hr) )
   {
      dwRet = ERROR_BAD_PATHNAME;
      /* Failure */
      goto __CLEANUP;
   }

   /* Build the new path for the extracted package */
   dwRet = PathRemoveFileSpec(chPackagePath);

   if ( ERROR_SUCCESS == dwRet )
   {
      hr = StringCchCat(chPackagePath,
                        _countof(chPackagePath),
                        chPackageName);

      if ( SUCCEEDED(hr) )
      {
         hr = StringCchCat(chPackagePath,
                           _countof(chPackagePath),
                           _T(".msi"));
      }

      if ( FAILED(hr) )
      {
         dwRet = ERROR_INSUFFICIENT_BUFFER;
      }
   }
      

   if ( ERROR_SUCCESS != dwRet )
   {
      dwRet = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   /* Delete the file first, in case it exists */
   DeleteFile(chPackagePath);

   if ( !MoveFile(chPackage,
                  chPackagePath) )
   {
      dwRet = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   *chPackage = _T('\0');

   dwRet = InstallPackage(chPackagePath,
                          TRUE);

__CLEANUP:
   CoUninitialize();

   if ( _T('\0') != *chPackage )
   {
      DeleteFile(chPackage);
   }

   if ( _T('\0') != *chPackagePath )
   {
      DeleteFile(chPackagePath);
   }

   return ( static_cast<int>(dwRet) );
}
