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
 
/*  Utility.h
 *      Generic utility code declarations
 */

#pragma once

#ifndef __UTILITY_H__
#define __UTILITY_H__

#pragma warning( push )
#pragma warning( disable : 4127 )

#include <shlobj.h>

/**********************************************************************
   
   Compile-time Miscellany

 **********************************************************************/

/**
 * _countof
 *    Number of elements in a static array.  
 */
#ifndef _countof
   #define _countof(rg) ( sizeof( rg ) / sizeof( rg[0] ) )
#endif

/**
 * _charsof
 *    Number of characters in a static string, not including the null terminator. 
 */
#ifndef _charsof
   #define _charsof(sz) ( (sizeof( sz ) / sizeof(sz[0])) - 1 )
#endif

/**
 * _sizeofField
 *    The size of a type's field.
 */
#ifndef _sizeofField
   #define _sizeofField(t, f) ( sizeof(((t *)0)->f) )
#endif

/**
 * _nodefault
 *    default case check/optimization  
 */
#ifndef _nodefault
   #ifdef _DEBUG
      #define _nodefault  { BOOL Unhandled_Switch_Case = FALSE; _ASSERTE(Unhandled_Switch_Case); }
   #else
      #define _nodefault  __assume(0)
   #endif
#endif

/**
 * _makeptr
 *    Makes a pointer of type, from a base and offset.
 */
#define _makeptr(t, b, o) ((t)((DWORD_PTR)(b) + (DWORD_PTR)(o)))

/**
 * _tostring
 */
#ifndef _tostring
   #define __tostring( x ) #x
   #define _tostring( x ) __tostring(x)
#endif

/**
 * _ltext 
 */
#ifndef _ltext
   #define __ltext( x ) L ## x
   #define _ltext( x ) __ltext(x)
#endif

/**********************************************************************
 *  initializeString
 *      Meta-template for performing an unrolled-initialization of a
 *      string buffer.
 */
template < size_t _cch > class initializeString
{
public:
   static inline void initialize( TCHAR rg[], LPCTSTR psz ) throw()
   {
      rg[_cch] = psz[_cch];
      /* When _cch reaches 0, the specialization below will be used
       * to end the recursion */
      initializeString<_cch - 1>::initialize(rg, psz);
   }
};
/*  Specialization to end recursion  */
template <> class initializeString<0>
{
public:
   static inline void initialize( TCHAR rg[], LPCTSTR psz ) throw()
   {
      rg[0] = psz[0];
   }
};

/**********************************************************************
 *  _initializeString
 *      Wrapper for the initializeString meta-template.
 *
 *  Parameters
 *      buf
 *        [out] The buffer to initialize.
 *      sz
 *        [in] The string to initialize with.
 *
 *  Return Value
 *      None
 *
 *  Remarks
 *      buf must be large enough to accomidate sz, including the null-
 *      terminator.
 */
#define _initializeString(buf, sz) \
   _ASSERTE(sizeof(buf) >= sizeof(sz)); \
   initializeString<(sizeof(sz) / sizeof(TCHAR)) - 1>::initialize(buf, sz)

/**********************************************************************
 *  _AlignUp / _AlignDown
 *      Alignment macros
 *
 *  Usage
 *      char buf1[ _AlignUp<55, 16>::AlignSize ];
 *      char buf2[ _AlignDown<55, 16>::AlignedSize ];
 *
 */
template < size_t cb, size_t al = 8 > class _AlignUp
{
public:
   enum { RawSize = cb, AlignedSize = ((cb + (al - 1)) &~ (al - 1)) };
};

template < size_t cb, size_t al = 8 > class _AlignDown
{
public:
   enum { RawSize = cb, AlignedSize = (cb &~ (al - 1)) };
};

/**********************************************************************
   
   Type stuff

 **********************************************************************/
inline LARGE_INTEGER LongLongToLargeInt( LONGLONG ill )
{
   return ( *((PLARGE_INTEGER)&ill) );
}

inline ULARGE_INTEGER ULongLongToULargeInt( ULONGLONG ull ) 
{
   return ( *((PULARGE_INTEGER)&ull) );
}

/**********************************************************************
   
   Win32 Specific

 **********************************************************************/
__forceinline HRESULT CoStatusFromWin32( DWORD dwError )
{
   return ( NO_ERROR !=  dwError ? HRESULT_FROM_WIN32(dwError) : S_OK );
}

/**********************************************************************
   
   Debugging

 **********************************************************************/

/**********************************************************************
 *  _CSystemMsg
 *      Wrapper of the FormatMessage function.
 */
template < size_t _cchMsgBuf = 512 > class _CSystemMsg
{
   /* Interface */
public:
   _CSystemMsg( ) throw()
   {
      memset(_chMsgBuf, 0, _cchMsgBuf);
   }

   _CSystemMsg( LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), va_list* Arguments = NULL ) throw()
   {
      memset(_chMsgBuf, 0, _cchMsgBuf);

      Format(lpSource, dwMessageId, dwLanguageId, Arguments);
   }

   /**********************************************************************
    *  Format
    *      Wraps FormatMessage.
    */
   DWORD Format( LPCVOID lpSource = NULL, DWORD dwMessageId = GetLastError(), DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), va_list* Arguments = NULL ) throw()
   {      
      return ( FormatMessage(0|(NULL != Arguments ? 0 : FORMAT_MESSAGE_IGNORE_INSERTS)|(NULL != lpSource ? 0 : FORMAT_MESSAGE_FROM_SYSTEM), lpSource, dwMessageId, dwLanguageId, _chMsgBuf, (DWORD)_cchMsgBuf, Arguments) );
   }

   operator LPTSTR( ) throw()
   {
      return ( (LPTSTR)_chMsgBuf );
   }

   TCHAR _chMsgBuf[_cchMsgBuf];

   /* Restricted */
private:    
   /* Copy construction and copy-assignment are not supported */
   _CSystemMsg( const _CSystemMsg& );
   _CSystemMsg& operator =( const _CSystemMsg& );
};

/**********************************************************************
 *  _SysErrorMsg / _OleErrTxt / _OleAutoErrorMsg
 *      Generic wrappers of _CSystemMsg
 */
#define _SysErrorMsg(c, m) \
   ((LPTSTR)_CSystemMsg<>((LPCVOID)(m), (DWORD)(c)))

#define _OleErrorMsg(c) \
   ((LPTSTR)_CSystemMsg<>((LPCVOID)::GetModuleHandle(_T("ole32")), (DWORD)(c)))

#define _OleAutoErrorMsg(c) \
   ((LPTSTR)_CSystemMsg<>((LPCVOID)::GetModuleHandle(_T("oleaut32")), (DWORD)(c)))

/**
 *  OutputDebugStringf
 *      OutputDebugString wrapper with printf support.
 *
 *  Remarks
 *      Formatted output string cannot be longer than OdsfMaxLine.
 */
inline void __cdecl OutputDebugStringf( LPCTSTR pszFormat, ... )
{
   enum
   {
      OdsfMaxLine = 1024
   };

   va_list args = NULL;
   TCHAR chBuf[OdsfMaxLine] = {_T('\0')};

   va_start(args, pszFormat);

   if ( SUCCEEDED(StringCchVPrintf(chBuf, _countof(chBuf), pszFormat, args)) )
   {
      OutputDebugString(chBuf);
   }

   va_end(args);
}

/**
 *  _CEmitTrace
 *      _OutputDebugStringf wrapper that includes file, function and
 *      line information.
 */
class _CEmitTrace {
public:
   _CEmitTrace( LPCTSTR pszFile, LPCTSTR pszFunction, int iLine, BOOL bFileNameOnly, BOOL bNewLine )  throw() : _pszFile(pszFile), _pszFunction(pszFunction), _iLine(iLine), _bNewLine(bNewLine)
   {
      if ( bFileNameOnly && NULL != pszFile )
      {
         /* Scan to the end... */
         while ( _T('\0') != *_pszFile )
         {
            _pszFile++;
         }
         /* Back it up to the file spec... */
         while ( _T('\\') != *_pszFile && pszFile != _pszFile )
         {
            _pszFile--;
         }
         /* If there was a \, we backed up one too many */
         if ( pszFile != _pszFile )
         {
            _pszFile++;
         }
      }
   }

   void __cdecl operator ( )( LPCTSTR pszFormat, ... ) throw()
   {
      va_list args;
      
      va_start(args, pszFormat);
      _OutputDebugStringVf(pszFormat, args);
      va_end(args);
   }

   void operator ( )( LPCTSTR pszFormat, va_list args ) throw()
   {
      _OutputDebugStringVf(pszFormat, args);
   }

   void _OutputDebugStringVf( LPCTSTR pszFormat, va_list args ) throw()
   {
      TCHAR chBuf[1024] = {_T('\0')};

      if ( SUCCEEDED(StringCchVPrintf(chBuf, _countof(chBuf), pszFormat, args)) )
      {
         OutputDebugStringf(_bNewLine ? _T("%s:%s(%d)!%s\n") : _T("%s:%s(%d)!%s"), NULL != _pszFile ? _pszFile : _T(""), NULL != _pszFunction ? _pszFunction : _T(""), _iLine, chBuf);
      }
   }

private:
   LPCTSTR _pszFile;
   LPCTSTR _pszFunction;
   int     _iLine;
   BOOL    _bNewLine;

   /* Default construction, copy-construction and copy-assignment are 
    * not supported */
   _CEmitTrace( );
   _CEmitTrace( const _CEmitTrace& );
   _CEmitTrace& operator =( const _CEmitTrace& );
};

/**********************************************************************
 *  _Trace/_Traceln
 *      Emits output to the debug device using only the filename spec
 *      of the __FILE__. _Traceln adds a new line character. 
 */
#define _Trace \
   _CEmitTrace(_T(__FILE__), _T(__FUNCTION__), __LINE__, TRUE, FALSE)

#define _Traceln \
   _CEmitTrace(_T(__FILE__), _T(__FUNCTION__), __LINE__, TRUE, TRUE)

/**********************************************************************
 *  _TraceEx/_TracelnEx
 *      Emits output to the debug device using the full file path spec
 *      of the __FILE__. _TracelnEx adds a new line character. 
 */
#define _TraceEx \
   _CEmitTrace(_T(__FILE__), _T(__FUNCTION__), __LINE__, FALSE, FALSE)

#define _TracelnEx \
   _CEmitTrace(_T(__FILE__), _T(__FUNCTION__), __LINE__, FALSE, TRUE)

/**
 *  _DEBUG only versions of all the _TraceXXX macros. 
 */
#ifdef _DEBUG
   #define _dTrace            _Trace
   #define _dTraceln          _Traceln
   #define _dTraceEx          _TraceEx
   #define _dTracelnEx        _TracelnEx
   #define _dTraceException   _TraceException
#else
   #define _dTrace            __noop
   #define _dTraceln          __noop
   #define _dTraceEx          __noop
   #define _dTracelnEx        __noop
   #define _dTraceException   __noop
#endif

/**
 * DBG_EXCEPTION_EXECUTE_HANDLER
 */
#ifdef _DEBUG
   #define DBG_EXCEPTION_EXECUTE_HANDLER EXCEPTION_CONTINUE_SEARCH
#else /* _DEBUG */
   #define DBG_EXCEPTION_EXECUTE_HANDLER EXCEPTION_EXECUTE_HANDLER
#endif /* _DEBUG */

/**********************************************************************
   
   Runtime Miscellany

 **********************************************************************/

/**********************************************************************
 *  _GetModuleHandleFromAddress
 *      Gets the module handle which contains the specified virtual 
 *      address.
 */
inline HMODULE _GetModuleHandleFromAddress( void* pva ) 
{
   MEMORY_BASIC_INFORMATION mbi = {0};

   VirtualQuery(pva, &mbi, sizeof(mbi));    
   _ASSERTE(NULL != mbi.AllocationBase);

   return ( (HMODULE)mbi.AllocationBase );
}

/**********************************************************************
 *  _GetConversionACP
 *      Gets the conversion code-page for use with WideCharToMultiByte
 *      and MultiByteToWideChar.
 *
 *  Remarks
 *      If ATL's _GetConversionACP is defined, this function calls
 *      that, otherwise CP_THREAD_ACP is returned for builds targeted 
 *      to Win2000 and greater, and CP_ACP for any other OS.
 */
inline UINT _GetConversionACP( ) throw()
{
#ifdef _AtlGetConversionACP
   return ( _AtlGetConversionACP() );
#else
#if _WIN32_WINT >= 0x0500
   return ( CP_THREAD_ACP );
#else
   return ( CP_ACP );
#endif
#endif /* _AtlGetConversionACP */
}

/**********************************************************************
 *  _Wide2Multi / _Multi2Wide
 *      Generic string conversion classes which don't throw exceptions
 *      like ATL's conversion macros do.
 */
template < size_t cch = MAX_PATH > class _Wide2Multi
{
public:
   _Wide2Multi( LPCWSTR pwsz, int iLen = -1 ) throw()
   {
      WideCharToMultiByte(_GetConversionACP(), 0, pwsz, iLen, _chBuf, cch, NULL, NULL);
   }

   operator LPCSTR( ) throw()
   {
      return ( _chBuf );
   }

   CHAR _chBuf[cch];
};

template < size_t cch = MAX_PATH  > class _Multi2Wide
{
public:
   _Multi2Wide( LPCSTR psz, int iLen = -1 ) throw()
   {
      MultiByteToWideChar(_GetConversionACP(), 0, psz, iLen, _chBuf, cch);
   }

   operator LPCWSTR( ) throw()
   {
      return ( _chBuf );
   }

   WCHAR _chBuf[cch];
};

/**********************************************************************
 *  _Wide2Wide / _Multi2Multi
 *      Utility templates for the _Wide2T and _Multi2T macros below,
 *      which simply store a copy of the string pointer.
 */
template < size_t > class _Wide2Wide
{
public:
   _Wide2Wide( LPCWSTR pwsz, int iLen = -1 ) throw() : _pwsz(pwsz)
   {
      iLen;
   }

   operator LPCWSTR( ) throw()
   {
      return ( _pwsz );
   }

   LPCWSTR _pwsz;
};

template < size_t > class _Multi2Multi
{
public:
   _Multi2Multi( LPCSTR psz, int iLen = -1 ) throw() : _psz(psz)
   {
   }

   operator LPCSTR( ) throw()
   {
      return ( _psz );
   }

   LPCSTR _psz;
};

/**********************************************************************
 *  _WideToT / _MultiToT
 *      Wrappers for _Wide2Multi, _Multi2Wide, _Wide2Wide and 
 *      _Multi2Multi.
 */
#ifdef _UNICODE
   #define _Wide2T  _Wide2Wide
   #define _Multi2T _Multi2Wide    
   #define _T2Wide  _Wide2Wide
   #define _T2Multi _Wide2Multi
#else
   #define _Wide2T  _Wide2Multi
   #define _Multi2T _Multi2Multi
   #define _T2Wide  _Multi2Wide
   #define _T2Multi _Multi2Multi
#endif

/**********************************************************************
 *  IsWinNT
 *      Checks whether the current OS is a specified version of WinNT. 
 *
 *  Return Value
 *      TRUE if the system is running the specified NT version, FALSE
 *      otherwise.
 */
inline BOOL __stdcall IsWinNT( DWORD Major, DWORD Minor, WORD SpMajor, WORD SpMinor ) throw()
{
   OSVERSIONINFOEX osix;
   DWORDLONG       dwMask;

   ZeroMemory(&osix,
              sizeof(osix));

   osix.dwOSVersionInfoSize = sizeof(osix);
   osix.dwMajorVersion      = Major;
   osix.dwMinorVersion      = Minor;
   osix.wServicePackMajor   = SpMajor;
   osix.wServicePackMinor   = SpMinor;

   /* Build the condition mask */
   dwMask = 0;
   VER_SET_CONDITION(dwMask, VER_MAJORVERSION,     VER_GREATER_EQUAL);
   VER_SET_CONDITION(dwMask, VER_MINORVERSION,     VER_GREATER_EQUAL);
   VER_SET_CONDITION(dwMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
   VER_SET_CONDITION(dwMask, VER_SERVICEPACKMINOR, VER_GREATER_EQUAL);

   return ( VerifyVersionInfo(&osix,
                              VER_MAJORVERSION|VER_MINORVERSION|VER_SERVICEPACKMAJOR|VER_SERVICEPACKMINOR,
                              dwMask) );
}

/**********************************************************************
 *  _IsWindows
 *      Same as _IsWinNT except for Windows95/98/ME
 */
inline BOOL __stdcall _IsWindows( DWORD dwMinVer = MAKELONG(4,0) ) throw()
{
   OSVERSIONINFO osi = {sizeof(OSVERSIONINFO), 0};

   if ( GetVersionEx(&osi) )
   {
      return ( VER_PLATFORM_WIN32_WINDOWS == osi.dwPlatformId && HIWORD(dwMinVer) >= osi.dwMinorVersion && LOWORD(dwMinVer) >= osi.dwMajorVersion ? TRUE : FALSE );
   }

   return ( FALSE );
}

/* Helpers for _IsWinNT and _IsWindows */
enum
{
   WindowsNT351 = MAKELONG(3,51),
   WindowsNT40  = MAKELONG(4,0),
   Windows2000  = MAKELONG(5,0),
   WindowsXP    = MAKELONG(5,1),
   Windows2003  = MAKELONG(5,2),
   WindowsVista = MAKELONG(6,0),

   Windows95    = MAKELONG(4,0),
   Windows98    = MAKELONG(4,10),
   WindowsMe    = MAKELONG(4,90)
};

static int __cdecl _MessageBoxf( HWND hWnd, LPCTSTR pszFormat, LPCTSTR pszCaption, UINT uType, ... ) throw()
{
   int     iRet      = 0;
   va_list args      = NULL;
   TCHAR   chBuf[1024] = {_T('\0')};

   va_start(args, uType);

   if ( SUCCEEDED(StringCchVPrintf(chBuf, _countof(chBuf), pszFormat, args)) )
   {
      iRet = MessageBox(NULL != hWnd ? hWnd : (GetActiveWindow()), chBuf, pszCaption, uType);
   }

   return ( iRet );
}

static HRESULT _SHBindToParent( LPCITEMIDLIST pidl, REFIID riid, VOID** ppv, LPCITEMIDLIST* ppidlLast )
{
   if ( !ppv )
   {
      return ( E_POINTER );
   }

   if ( !pidl || (0 == pidl->mkid.cb) )
   {
      return ( E_INVALIDARG );
   }

   HRESULT       hr       = E_FAIL;
   IShellFolder* psf      = NULL;
   LPCITEMIDLIST pidlItem   = NULL;
   LPITEMIDLIST  pidlParent = NULL;
   
   __try
   {
      hr = SHGetDesktopFolder(&psf);
      if ( FAILED(hr) )
      {
         __leave;
      }

      pidlItem = pidl;

      while ( true )
      {
         LPCITEMIDLIST pidlNext = (LPCITEMIDLIST)(pidlItem->mkid.abID - sizeof(pidlItem->mkid.cb) + pidlItem->mkid.cb);

         if ( 0 == pidlNext->mkid.cb )
         {
            break;
         }

         pidlItem = pidlNext;
      }

      if ( pidlItem == pidl )
      {
         hr = psf->QueryInterface(riid, ppv);
      }
      else
      {
         size_t cbLen = (size_t)(DWORD_PTR)(pidlItem->mkid.abID - pidl->mkid.abID);

         pidlParent  = (LPITEMIDLIST)new BYTE[cbLen + sizeof(pidl->mkid.cb)];
         if ( !pidlParent )
         {
            hr = E_OUTOFMEMORY;
            /* Failure */
            __leave;
         }

         memcpy(pidlParent, pidl, cbLen);
         memset((BYTE*)pidlParent + cbLen, 0, sizeof(pidl->mkid.cb));

         hr = psf->BindToObject(pidlParent, NULL, riid, ppv);
      }
   }
   __finally
   {
      delete [] pidlParent;

      if ( NULL != psf )
      {
         psf->Release();
      }
   }

   if ( NULL != ppidlLast )
   {
      (*ppidlLast) = pidlItem;
   }

   return ( hr );
}

/**********************************************************************
 *  _GetTargetPIDL
 *      Retrieves the target PIDL of a shell item's PIDL.
 *
 *  Parameters
 *      hWnd
 *        [in] See IShellFolder::GetUIObjectOf(hwndOwner)
 *      pidl
 *        [in] The item's PIDL.
 *
 *  Return Value
 *      If the item's PIDL is a shortcut and the function succeeds, the
 *      the PIDL of the target is returned. If the item's PIDL is not a
 *      shortcut, a copy of the item's PIDL is returned. If the function
 *      fails, NULL is returned.
 *
 *  Remarks
 *      The caller is responsible for releasing the returned PIDL.
 */
static LPITEMIDLIST __stdcall _GetTargetPIDL( HWND hWnd, LPCITEMIDLIST pidl ) throw()
{   
   LPITEMIDLIST  pidlr = NULL;
   LPCITEMIDLIST pidlc = NULL;
   IShellFolder* psf   = NULL;
   IShellLink*   psl   = NULL;

   __try
   {
      if ( SUCCEEDED(_SHBindToParent(pidl, __uuidof(IShellFolder), (void**)&psf, &pidlc)) )
      {
         if ( FAILED(psf->GetUIObjectOf(hWnd, 1, &pidlc, __uuidof(IShellLink), NULL, (void**)&psl)) )
         {
            /* Item isn't a shell link */
            pidlr = ILClone(pidl);

            __leave;
         }

         psl->GetIDList(&pidlr);
      }
   }
   __finally
   {
      if ( NULL != psl )
      {
         psl->Release();
      }

      if ( NULL != psf )
      {
         psf->Release();
      }
   }

   return ( pidlr );
}

/**********************************************************************
 *  _GetSpecialFolderPIDL
 *      Retrieves the PIDL of a special folder or of a path.
 *
 *  Parameters
 *      hWnd
 *        [in] See IShellFolder::ParseDisplayName(hWnd)
 *      pszPath
 *        [in] Path, or CSIDL, of the item to retrieve a PIDL for. To
 *        specify a CSIDL use the MAKEINTRESOURCE macro.
 *
 *  Return Value
 *      Returns a PIDL for the item if successful, otherwise NULL.
 *
 *  Remarks
 *      The caller is responsible for releasing the returned PIDL.
 */
static LPITEMIDLIST __stdcall _GetSpecialFolderPIDL( HWND hWnd, LPCTSTR pszPath ) throw()
{
   IShellFolder* psf  = NULL;
   LPITEMIDLIST  pidl = NULL;
   HRESULT       hr   = E_FAIL;

   __try
   {
      if ( IS_INTRESOURCE(pszPath) )
      {
         hr = SHGetSpecialFolderLocation(hWnd, LOWORD(pszPath), &pidl);
      }
      else
      {
         DWORD dwa  = SFGAO_FILESYSTEM;

         if ( SUCCEEDED(SHGetDesktopFolder(&psf)) )
         {
            hr = psf->ParseDisplayName(hWnd, NULL, const_cast<LPOLESTR>((LPCWSTR)_T2Wide<MAX_PATH>(pszPath)), NULL, &pidl, &dwa);
         }
      }
   }
   __finally
   {
      if ( NULL != psf )
      {
         psf->Release();
      }

      if ( FAILED(hr) && (NULL != pidl) )
      {
         CoTaskMemFree(pidl);
         pidl = NULL;
      }
   }

   return ( pidl );
}

static BOOL __stdcall _GetRegSettingA( HKEY hKeyRoot, LPCSTR pszSubKey, LPCSTR pszValueName, void* pData, size_t cbData ) throw()
{
   _ASSERTE(NULL != hKeyRoot);

   BOOL  bRet   = FALSE;
   HKEY  hKey   = NULL;
   LONG  lRet   = NO_ERROR;
   DWORD dwData;

   __try
   {
      lRet = RegOpenKeyExA(hKeyRoot, pszSubKey, 0, KEY_READ, &hKey);
      if ( NO_ERROR != lRet )
      {
         SetLastError(lRet);
         /* Failure */
         __leave;
      }

      dwData = (DWORD)cbData;
      lRet = RegQueryValueExA(hKey, pszValueName, NULL, NULL, (LPBYTE)pData, &dwData);
      if ( NO_ERROR != lRet )
      {
         SetLastError(lRet);
         /* Failure */
         __leave;
      }

      bRet = TRUE;
   }
   __finally
   {
      if ( NULL != hKey )
      {
         RegCloseKey(hKey);
      }
   }

   return ( bRet );
}

static BOOL __stdcall _GetRegSettingW( HKEY hKeyRoot, LPCWSTR pszSubKey, LPCWSTR pszValueName, void* pData, size_t cbData ) throw()
{
   _ASSERTE(NULL != hKeyRoot);

   BOOL  bRet   = FALSE;
   HKEY  hKey   = NULL;
   LONG  lRet   = NO_ERROR;
   DWORD dwData;

   __try
   {
      lRet = RegOpenKeyExW(hKeyRoot, pszSubKey, 0, KEY_READ, &hKey);
      if ( NO_ERROR != lRet )
      {
         SetLastError(lRet);
         /* Failure */
         __leave;
      }

      dwData = (DWORD)cbData;
      lRet = RegQueryValueExW(hKey, pszValueName, NULL, NULL, (LPBYTE)pData, &dwData);
      if ( NO_ERROR != lRet )
      {
         SetLastError(lRet);
         /* Failure */
         __leave;
      }

      bRet = TRUE;
   }
   __finally
   {
      if ( NULL != hKey )
      {
         RegCloseKey(hKey);
      }
   }

   return ( bRet );
}

#ifdef _UNICODE
   #define _GetRegSetting _GetRegSettingW
#else
   #define _GetRegSetting _GetRegSettingA
#endif /* _UNICODE */

static BOOL __stdcall _SetRegSetting( HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValueName, DWORD dwType, void* pData, size_t cbData )
{
   BOOL bRet = FALSE;
   HKEY hKey = NULL;

   __try
   {
      if ( ERROR_SUCCESS != RegCreateKeyEx(hKeyRoot, pszSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) )
      {
         __leave;
      }

      bRet = (ERROR_SUCCESS != RegSetValueEx(hKey, pszValueName, 0, dwType, (CONST BYTE*)pData, (DWORD)cbData) ? FALSE : TRUE);
   }
   __finally
   {
      if ( NULL != hKey )
      {
         RegCloseKey(hKey);
      }
   }
   
   return ( bRet );
}

#pragma comment(lib, "version")

typedef struct _VER_TRANSLATIONINFO
{
   WORD wLanguage;
   WORD wCodePage;
}VER_TRANSLATIONINFO, *PVER_TRANSLATIONINFO;

static BOOL __stdcall _CheckModuleVersionStringInfo( LPCTSTR pszModule, LPCTSTR pszInfoName, LPCTSTR pszInfoValue ) throw()
{
   _ASSERTE(NULL != pszModule);

   if ( INVALID_FILE_ATTRIBUTES == GetFileAttributes(pszModule) )
   {
      return ( FALSE );
   }

   BOOL   bRet   = FALSE;
   LPVOID pBlock = NULL;

   __try
   {
      DWORD dwHandle = 0;
      DWORD cbData   = GetFileVersionInfoSize(pszModule, &dwHandle);

      if ( 0 != cbData && NULL != (pBlock = new BYTE[cbData]) )
      {
         memset(pBlock, 0, cbData);

         UINT cbTrans = 0;
         PVER_TRANSLATIONINFO pTrans  = NULL;

         if ( GetFileVersionInfo(pszModule, dwHandle, cbData, pBlock) && VerQueryValue(pBlock, _T("\\VarFileInfo\\Translation"), (LPVOID*)&pTrans, &cbTrans) )
         {
            for ( size_t idx = 0; idx < (cbTrans / sizeof(VER_TRANSLATIONINFO)); idx++ )
            {
                TCHAR   chBuf[128] = {_T('\0')};
                LPCTSTR pszValue   = NULL;
                UINT    cbValue    = 0;

                if ( SUCCEEDED(StringCchPrintf(chBuf, _countof(chBuf), _T("\\StringFileInfo\\%04x%04x\\%s"), pTrans[idx].wLanguage, pTrans[idx].wCodePage, pszInfoName)) && VerQueryValue(pBlock, chBuf, (LPVOID*)&pszValue, &cbValue) )
                {
                   if ( 0 == lstrcmpi(pszInfoValue, pszValue) )
                   {
                       bRet = TRUE;
                       break;
                   }
                }
            }
         }
      }
   }
   __finally
   {
      delete [] pBlock;
   }

   return ( bRet );
}

static void DumpStreamToFile( IStream* pStream, LPCTSTR pszPathPrefix )
{
   LARGE_INTEGER iTick = {0};
   QueryPerformanceCounter(&iTick);
   TCHAR chPath[MAX_PATH] = {_T('\0')};
   if ( SUCCEEDED(StringCchPrintf(chPath, _countof(chPath), _T("%s%0.16i64x.txt"), pszPathPrefix, iTick.QuadPart)) )
   {
      HANDLE hFile = INVALID_HANDLE_VALUE;
      LARGE_INTEGER iSeek = {0};      
      __try 
      {
         hFile = CreateFile(chPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
         if ( (INVALID_HANDLE_VALUE != hFile) && (NULL != pStream) )
         {
            pStream->Seek(iSeek, SEEK_CUR, (ULARGE_INTEGER*)&iSeek);
            LARGE_INTEGER li = {0};
            pStream->Seek(li, SEEK_SET, NULL);

            BYTE  bBuf[512];
            ULONG cbRead = 0;
            HRESULT hr = pStream->Read(bBuf, sizeof(bBuf), &cbRead);
            while ( SUCCEEDED(hr) && (cbRead > 0) )
            {
               ULONG cbWritten = 0;
               if ( !WriteFile(hFile, bBuf, cbRead, &cbWritten, NULL) || (cbWritten < cbRead) )
               {
                  break;
               }
               hr = pStream->Read(bBuf, sizeof(bBuf), &cbRead);
            }
         }
      }
      __finally 
      {
         if ( INVALID_HANDLE_VALUE != hFile )
         {
            CloseHandle(hFile);
         }

         if ( NULL != pStream )
         {
            pStream->Seek(iSeek, SEEK_SET, NULL);
         }
      }
   }
}

class CGuidName
{
public:
   CGuidName( const GUID& guid ) throw()
   {
      _chGUID[0] = L'\0';
      StringFromGUID2(guid, _chGUID, _countof(_chGUID));
   }
   operator LPCOLESTR( ) const throw()
   {
      return ( _chGUID );
   }
   OLECHAR _chGUID[39];
};

static LPSTR WINAPI _PathFindExtensionA( LPCSTR pszPath ) throw()
{
	_ASSERTE(NULL != pszPath);

	LPCSTR pszExt = NULL;
   LPCSTR pszTmp = pszPath;

   /* Find the end... */
   while ( '\0' != *pszTmp )
	{
	   pszTmp++;
	}

   pszExt = pszTmp;

   /* Back it up... */
   while ( pszTmp > pszPath )
   {
      pszTmp--;

      if ( '.' == *pszTmp )
      {
         return ( (LPSTR)pszTmp );
      }
   }

	return ( (LPSTR)pszExt );
}

static LPWSTR WINAPI _PathFindExtensionW( LPCWSTR pwszPath ) throw()
{
	_ASSERTE(NULL != pwszPath);

   LPCWSTR pwszExt = NULL;
   LPCWSTR pwszTmp = pwszPath;

   /* Find the end... */
   while ( L'\0' != *pwszTmp )
	{
	   pwszTmp++;
	}

   pwszExt = pwszTmp;

   /* Back it up... */
   while ( pwszTmp > pwszPath )
   {
      pwszTmp--;

      if ( '.' == *pwszTmp )
      {
         return ( (LPWSTR)pwszTmp );
      }
   }

	return ( (LPWSTR)pwszExt );
}


inline BOOL IsTokenPrivilegeEnabled( PTOKEN_PRIVILEGES pTokenPriv ) throw()
{
   if ( pTokenPriv->PrivilegeCount > 0 )
   {
      return ( pTokenPriv->Privileges[0].Attributes & SE_PRIVILEGE_ENABLED ? TRUE : FALSE );
   }

   return ( FALSE );
}

inline BOOL SetTokenPrivilege( HANDLE hToken, LPCTSTR pszPrivilege, BOOL fEnable, PTOKEN_PRIVILEGES pPreviousState ) throw()
{
   TOKEN_PRIVILEGES tp   = {0};
   LUID             luid = {0};
   DWORD            cbBuf;
   BOOL             bRet;

   if ( !LookupPrivilegeValue(NULL, pszPrivilege, &luid ) )
   {
      /* Failure */
      return ( FALSE ); 
   }

   tp.PrivilegeCount           = 1;
   tp.Privileges[0].Luid       = luid;   
   tp.Privileges[0].Attributes = (fEnable ? SE_PRIVILEGE_ENABLED : 0);

   cbBuf = (NULL != pPreviousState ? sizeof(TOKEN_PRIVILEGES) : 0);
   bRet  = AdjustTokenPrivileges(hToken, FALSE, &tp, cbBuf, pPreviousState, &cbBuf);
   return ( bRet );
}

inline BOOL SetThreadTokenPrivilege( HANDLE hThread, LPCTSTR pszPrivilege, BOOL fEnable, PTOKEN_PRIVILEGES pPreviousState ) throw()
{
   BOOL   bRet;
   HANDLE hToken;

   bRet = FALSE;

   if ( OpenThreadToken(hThread, TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, TRUE, &hToken) )
   {
      bRet = SetTokenPrivilege(hToken, pszPrivilege, fEnable, pPreviousState);
      CloseHandle(hToken);
   }

   return ( bRet );
}

inline BOOL SetProcessTokenPrivilege( HANDLE hProcess, LPCTSTR pszPrivilege, BOOL fEnable, PTOKEN_PRIVILEGES pPreviousState ) throw()
{
   BOOL   bRet;
   HANDLE hToken;

   bRet = FALSE;

   if ( OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken) )
   {
      bRet = SetTokenPrivilege(hToken, pszPrivilege, fEnable, pPreviousState);
      CloseHandle(hToken);
   }

   return ( bRet );
}

#pragma warning( pop )

#endif /* __UTILITY_H__ */