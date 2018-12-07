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
 
/* VerifierMem.h
 *    DDK Driver Verifier type memory allocator for detecting buffer
 *    under/overruns.
 *
 * The
 */

#pragma once

#ifndef __VERIFIERMEM_H__
#define __VERIFIERMEM_H__

typedef enum _PROTECTION_TYPE
{
   eBufferUnderrun = 0,
   eBufferOverrun  = 1
}PROTECTION_TYPE;

__inline void* __AllocateVerifierMemory( __in size_t cb, int eProtectionType )
{
   void*       pMemBase;
   void*       pMemGuard;
   void*       pMemRet;
   size_t      cbAlloc;
   size_t      cbPageSize;
   DWORD       flOldProtect;
   SYSTEM_INFO cSysInfo;

   GetSystemInfo(&cSysInfo);
   cbPageSize = cSysInfo.dwPageSize;

   /* Determine the size of the buffer required to force the caller's request
    * to either the start or end of a system page */
   cbAlloc = cb + cbPageSize;
   cbAlloc = ((cbAlloc + (cbPageSize - 1)) & ~(cbPageSize - 1));
   if ( cbAlloc < cb )
   {
      SetLastError(ERROR_ARITHMETIC_OVERFLOW);
      /* Failure */
      return ( NULL );
   }

   /* Do the allocation */
   pMemBase = VirtualAlloc(NULL, cbAlloc, MEM_COMMIT, PAGE_READWRITE);
   if ( !pMemBase )
   {
      /* Failure - VirtualAlloc calls SetLastError() */
      return ( NULL );
   }

   /* Assign a memory address that forces either the start or end of the caller's
    * buffer to be bound by a guard page */
   switch ( eProtectionType )
   {
      case eBufferUnderrun:
         pMemGuard = pMemBase;
         pMemRet   = (void*)((BYTE*)pMemBase + cSysInfo.dwPageSize);
         break;

      case eBufferOverrun:
         pMemGuard = (void*)((BYTE*)pMemBase + cbAlloc - cSysInfo.dwPageSize);
         pMemRet   = (void*)((BYTE*)pMemGuard - cb);
         break;

      default:
         _ASSERT(FALSE);
         SetLastError(ERROR_INVALID_PARAMETER);
         /* Failure */
         pMemRet   = NULL;
         pMemGuard = NULL;
         break;
   }

   if ( !pMemRet )
   {
      VirtualFree(pMemBase, 0, MEM_RELEASE);
      /* Failure */
      return ( NULL );
   }

   /* Remove all access rights from the guard page */
   VirtualProtect(pMemGuard, cSysInfo.dwPageSize, PAGE_NOACCESS, &flOldProtect);

   /* Success */
   return ( pMemRet );
}

__inline void __FreeVerifierMemory( __in void* pv )
{
   MEMORY_BASIC_INFORMATION mbi;
   if ( 0 != VirtualQuery(pv, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) )
   {
      VirtualFree(mbi.AllocationBase, 0, MEM_RELEASE);
   }
}

#endif /* __VERIFIERMEM_H__ */


