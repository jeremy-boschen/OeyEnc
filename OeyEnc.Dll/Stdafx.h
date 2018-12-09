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
   //#define _STRSAFE_USE_SECURE_CRT  1
   #define ISOLATION_AWARE_ENABLED  1
   //#define __STDC_WANT_SECURE_LIB__ 1
#endif /* _WDKBUILD */

#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <tchar.h>
#include <crtdbg.h>

#include <sal.h>
#include <windows.h>
#include <strsafe.h>
#include <winnt.h>
#include <shlwapi.h>

//#define STRCONSTA(x,y)  EXTERN_C __declspec(selectany) const char x[] = y
//#define STRCONSTW(x,y)  EXTERN_C __declspec(selectany) const WCHAR x[] = L##y
//#include <mimeole.h>

/**********************************************************************

    ATL Specific

 **********************************************************************/
//#define _ATL_NO_COM_SUPPORT
//#define _ATL_NO_PERF_SUPPORT
//#define _ATL_ALL_WARNINGS
//#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS   
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
#include <atlcom.h>
#include <atlmem.h>
using namespace ATL;

/**********************************************************************

    Project Specific

 **********************************************************************/
#define OEYENC_REG_SUBKEY "Software\\jBoschen\\OeyEnc"

#include "Utility.h"
#include "StreamImpl.h"

#include "Resource.h"
#include "Module.h"

