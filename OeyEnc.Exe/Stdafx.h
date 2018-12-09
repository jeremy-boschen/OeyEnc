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

/*  Stdafx.h
 *      Include file for standard system include files, or project specific 
 *      include files that are used frequently, but are changed infrequently.
 */

#pragma once

/**********************************************************************

    Global Stuff

 **********************************************************************/
#pragma warning( disable : 4505 )

/**********************************************************************

    Platform SDK & C/C++ Runtime Specific

 **********************************************************************/
#ifndef _WDKBUILD
   #define STRICT
   #define WINVER                   0x0501
   #define _WIN32_WINNT             0x0501
   #define WIN32_IE                 0x0600
   #define _WIN32_OE                0x0501
   #define STRSAFE_NO_DEPRECATE
   #define ISOLATION_AWARE_ENABLED  1
#endif /* _WDKBUILD */

#include <crtdbg.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <tchar.h>

#include <windows.h> 
#include <userenv.h>
#include <WinCred.h>
#include <strsafe.h>
#include <winnt.h>
#include <shlobj.h>
#include <shlwapi.h>

#ifdef _M_IX86
   #pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' language='*' publicKeyToken='6595b64144ccf1df' processorArchitecture='x86'\"")
#endif /* _M_IX86 */

#ifdef _M_AMD64
   #pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' language='*' publicKeyToken='6595b64144ccf1df' processorArchitecture='amd64'\"")            
#endif

/**********************************************************************

    Project Specific

 **********************************************************************/

/* Override STRCONSTA in msoeapi.h */
#define STRCONSTA(x,y)  EXTERN_C __declspec(selectany) const char x[] = y
#define STRCONSTW(x,y)  EXTERN_C __declspec(selectany) const WCHAR x[] = L##y

STRCONSTA(STR_MSOEAPI_BROWSERCLASSA, "Outlook Express Browser Class");
STRCONSTW(STR_MSOEAPI_BROWSERCLASSW, "Outlook Express Browser Class");

#ifdef _UNICODE
   #define STR_MSOEAPI_BROWSERCLASS STR_MSOEAPI_BROWSERCLASSW
#else /* _UNICODE */
   #define STR_MSOEAPI_BROWSERCLASS STR_MSOEAPI_BROWSERCLASSA
#endif /* _UNICODE */

#include "Resource.h"

#include "Utility.h"
#include "Cojack.h"
#include "MuiLib.h"

