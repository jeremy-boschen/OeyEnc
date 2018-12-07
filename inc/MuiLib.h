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
 
/*  MuiLib.h
 *     MUI wrappers
 */

#pragma once

#include <Muiload.h>
#pragma comment(lib, "Muiload")

#pragma warning( push )
#pragma warning( disable : 4127 )

static DWORD MuiInitializeLibrary( LPCWSTR pwszModule, HMODULE* phMui ) throw()
{
   OSVERSIONINFO VerInfo;

   (*phMui) = NULL;

   ZeroMemory(&VerInfo, 
              sizeof(VerInfo));

   VerInfo.dwOSVersionInfoSize = sizeof(VerInfo);

   if ( !GetVersionEx(&VerInfo) )
   {
      /* Failure */
      return ( GetLastError() );
   }

#pragma warning( push )
#pragma warning( disable : 4296 )
   if ( (VER_PLATFORM_WIN32_NT == VerInfo.dwPlatformId) && (VerInfo.dwMinorVersion >= 0) && (VerInfo.dwMajorVersion >= 6) )
#pragma warning( pop )
   {
      (*phMui) = GetModuleHandle(pwszModule);
      /* Success - VISTA style resource loading*/
      return ( NO_ERROR );
   }

   (*phMui) = LoadMUILibrary(pwszModule,
                             MUI_LANGUAGE_ID,
                             0);

   if ( !(*phMui) )
   {
      /* Fall back to the lang neutral file */
      (*phMui) = GetModuleHandle(pwszModule);
   }

   return ( NO_ERROR );
}

static void MuiUninitializeLibrary( LPCWSTR pwszModule, HMODULE hMui ) throw()
{
   if ( hMui != GetModuleHandle(pwszModule) )
   {
      FreeMUILibrary(hMui);
   }
}

struct MChar
{
public:
   virtual LPWSTR _GetBuf( ) throw() = 0;
   virtual void _SetBuf( LPWSTR pwsz ) throw() = 0;
   
   operator LPWSTR( ) throw()
   {
      return ( _GetBuf() );
   }

   UINT _GetBufLength( ) throw()
   {
      return ( ~F_LOCALBUF & _cch );
   }

   bool _SetBufLength( UINT cch ) throw()
   {
      LPWSTR pwszBuf;

      /* If the length is less than the current length, reuse the dynamic or local buffer */
      if ( cch < _GetBufLength() )
      {
         if ( !_IsLocalBuf() )
         {
            delete [] _GetBuf();
            _cch &= ~F_LOCALBUF;
         }

         /* Success */
         return ( true );
      }

      /* Allocate a new buffer */
      pwszBuf = new WCHAR[cch + 1];
      if ( NULL != pwszBuf )
      {
         if ( !_IsLocalBuf() )
         {
            delete [] _GetBuf();
         }

         _cch = F_LOCALBUF|cch;
         _SetBuf(pwszBuf);
      }

      return ( NULL != pwszBuf );
   }

protected:
   bool _IsLocalBuf( ) throw()
   {
      return ( F_LOCALBUF & _cch ? false : true );
   }

   enum
   {
      F_LOCALBUF = 0x80000000
   };

   UINT _cch;
};

typedef MChar*       PMChar;
typedef const MChar* PCMChar;

template < UINT _cchBuffer > struct 
MCharT : public MChar
{
   MCharT( ) throw() : pwszBuffer(NULL)
   {
      ZeroMemory(chBuffer,
                 sizeof(chBuffer));
      _cch = _cchBuffer;
   }

   ~MCharT( ) throw()
   {
      if ( !_IsLocalBuf() )
      {
         delete [] pwszBuffer;
      }
   }

   LPWSTR _GetBuf( ) throw()
   {
      if ( _IsLocalBuf() )
      {
         return ( chBuffer );
      }

      return ( pwszBuffer );
   }

   void _SetBuf( LPWSTR pwsz ) throw()
   {
      pwszBuffer = pwsz;
   }

   union
   {
      LPWSTR   pwszBuffer;
      WCHAR    chBuffer[_cchBuffer];
   };
};

/**
 */
static int MuiLoadString( HINSTANCE hInstance, UINT uID, PMChar pString ) throw()
{
   int    cch;
   LPWSTR pwszBuf;
   
   while ( true )
   {
      pwszBuf = pString->_GetBuf();

      cch = LoadString(hInstance, 
                       uID, 
                       pwszBuf, 
                       pString->_GetBufLength());

      if ( static_cast<UINT>(cch) < (pString->_GetBufLength() - 1) )
      {
         break;
      }

      if ( !pString->_SetBufLength(cch * 2) )
      {
         /* Failure */
         return ( 0 );
      }
   }

   return ( cch );
}

#pragma warning( pop )
