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

/*  ObjEx.cpp
 *    OBJ file .TEXT byte code exporter
 */

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <winnt.h>

#define OBJX_LINE_LENGTH 16

BOOL IsCodeSection( PIMAGE_SECTION_HEADER pSection ) throw()
{
   if ( (0 != (pSection->Characteristics & IMAGE_SCN_CNT_CODE)) &&
        (0 != (pSection->Characteristics & IMAGE_SCN_MEM_EXECUTE)) )
   {
      return ( TRUE );
   }

   return ( FALSE );
}

size_t DumpExecutableCodeSections( PIMAGE_FILE_HEADER pImageFile )
{
   WORD                  iIdx;
   PIMAGE_SECTION_HEADER pSection;

   DWORD                 iJdx;
   BYTE*                 pbCode;
   
   size_t                cWritten;

   iIdx     = 0;
   pSection = (PIMAGE_SECTION_HEADER)((ULONG_PTR)pImageFile + sizeof(IMAGE_FILE_HEADER) + pImageFile->SizeOfOptionalHeader);
   cWritten = 0;
   
   for ( iIdx = 0; iIdx < pImageFile->NumberOfSections; iIdx++ )
   {
      if ( IsCodeSection(pSection) )
      {
         pbCode = (BYTE*)((ULONG_PTR)pImageFile + pSection->PointerToRawData);

         for ( iJdx = 0; iJdx < pSection->SizeOfRawData; iJdx++ )
         {
            _tprintf(_T("0x%02x, "), 
                     pbCode[iJdx]);

            cWritten++;

            if ( 0 == ((iJdx + 1) % OBJX_LINE_LENGTH) )
            {
               _tprintf(_T("\n"));
            }
         }
      }

      pSection++;
   }

   return ( cWritten );
}

int __cdecl _tmain( int argc, TCHAR* argv[] )
{
   HANDLE                hFile;
   LPVOID                pData;
   ULONG                 cbFile;
   BOOL                  bRet;

   PIMAGE_FILE_HEADER    pImageFile;

   size_t                cWritten;
   size_t                idx;

   if ( argc <= 1 )
   {
      _tprintf(_T("Usage: ObjEx.exe Test.obj\n"));
      return ( -1 );
   }

   cWritten = 0;

   while ( --argc )
   {
      hFile = CreateFile(argv[argc],
                         GENERIC_READ,
                         0,
                         NULL,
                         OPEN_EXISTING,
                         0,
                         NULL);

      if ( INVALID_HANDLE_VALUE != hFile )
      {
         cbFile = GetFileSize(hFile,
                              NULL);

         if ( INVALID_FILE_SIZE != cbFile )
         {
            pData = HeapAlloc(GetProcessHeap(),
                              HEAP_ZERO_MEMORY,
                              cbFile);

            if ( NULL != pData )
            {
               bRet = ReadFile(hFile,
                               pData,
                               cbFile,
                               &cbFile,
                               NULL);

               if ( bRet )
               {
                  pImageFile = (PIMAGE_FILE_HEADER)pData;

                  if ( (IMAGE_FILE_MACHINE_AMD64 != pImageFile->Machine) || (IMAGE_FILE_MACHINE_I386 != pImageFile->Machine) )
                  {
                     cWritten += DumpExecutableCodeSections(pImageFile);
                  }
               }
            }

            HeapFree(GetProcessHeap(),
                     0,
                     pData);
         }

         if ( INVALID_HANDLE_VALUE != hFile )
         {
            CloseHandle(hFile);
         }
      }
   }

   /* Finalize the output with a few CC instruction (INT 3 on both X86 and AMD64) to pad
    * everything to 16 bytes */
   for ( idx = 0; idx < 16; idx++ )
   {
      _tprintf(_T("0xcc"));
      cWritten++;      
      
      if ( 0 == (cWritten % OBJX_LINE_LENGTH) )
      {
         _tprintf(_T("\n"));
         break;
      }

      _tprintf(_T(", "));
   }

	return ( 0 );
}

