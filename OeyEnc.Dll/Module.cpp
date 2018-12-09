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

/*  OeyEnc.cpp
 *      Implementation of DLL exports.
 */

#include "Stdafx.h"
#include "Module.h"
#include "MimeMessage.h"

/**
 * _AtlModule
 *    This needs to be declared in a CPP file and not with
 *    __selectany in an H file because the linker discards
 *    it if it's declared using __selectany in an H file
 */
COeyEncModule _AtlModule;

BOOL COeyEncModule::DllMain( DWORD dwReason, LPVOID lpReserved )
{
   BOOL    bRet;

   bRet = CAtlDllModuleT< COeyEncModule >::DllMain(dwReason, lpReserved);
   if ( !bRet )
   {
      /* Failure */
      return ( bRet );
   }

   switch ( dwReason )
   {
      case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
         _CrtSetDbgFlag(((_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & 0x0000ffff)|_CRTDBG_CHECK_EVERY_128_DF)|(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF|_CRTDBG_DELAY_FREE_MEM_DF));
#endif /* _DEBUG */

         bRet = SUCCEEDED(CMimeMessage::_BindProvider(TRUE));
         break;
      case DLL_PROCESS_DETACH:         
         CMimeMessage::_BindProvider(FALSE);
         break;
   }

   /* Success / Failure */
   return ( bRet  );
}
