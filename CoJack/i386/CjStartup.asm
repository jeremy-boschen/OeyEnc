; OeyEnc - Outlook Express plugin for decoding messages encoded with
; the yEnc format. 
;
; Copyright (C) 2004-2008 Jeremy Boschen. All rights reserved.
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software. 
; 
;  Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it and redistribute it
; freely, subject to the following restrictions:
; 
; 1. The origin of this software must not be misrepresented; you must not
; claim that you wrote the original software. If you use this software in
; a product, an acknowledgment in the product documentation would be
; appreciated but is not required.
; 
; 2. Altered source versions must be plainly marked as such, and must not
; be misrepresented as being the original software.
; 
; 3. This notice may not be removed or altered from any source
; distribution.
;

;  x86-CjStartup.asm
;     CjStartProc() procedure for loading a library into an initially 
;     suspended process on X86 platforms

.386
.MODEL FLAT, STDCALL
OPTION CASEMAP:none
OPTION PROLOGUE:None
OPTION EPILOGUE:None

_TEXT SEGMENT

.DATA

INCLUDE CoJack.inc

.CODE

CjStartProc PROC

; MASM will reference this as [ebp-4]
   LOCAL dwProtect:DWORD
   
; Setup an alias for the SxDATA pointer loaded into edi   
   pData EQU [edi.SxDATA]
   
IF DBG EQ 1
   int   3
ENDIF

; Remove the parameter pushed by the calling code. This is done first because the
; calling code uses JMP to transfer so there is no return address on the stack.
   pop   eax
   
; Setup a standard stack frame
   push  ebp
   mov   ebp, esp
   sub   esp, 4
   push  edi

; Load the SxDATA pointer
   mov   edi, eax
     
; Attempt to load the specified module
   lea   eax, pData.chModuleName
   push  eax
   call  pData.pLoadLibraryW   
   test  eax, eax
   je    @FAILURE
   
; Enable write access to the original startup code location
   lea   eax, dwProtect
   push  eax
   push  PAGE_READWRITE
   push  pData.cbTargetSize
   push  pData.pTargetAddress
   call  pData.pVirtualProtect
   test  eax, eax
   je    @FAILURE
   
; Copy the original startup code back. EDI is pData, so it must be setup last
   push  edi
   push  esi
   mov   ecx, pData.cbTargetSize
   mov   esi, pData.pCodeAddress
   mov   edi, pData.pTargetAddress
   cld
   rep movsb   
   pop   esi
   pop   edi
   
; Return the page access to its original state
   lea   eax, dwProtect
   push  eax
   push  dwProtect
   push  pData.cbTargetSize
   push  pData.pTargetAddress
   call  pData.pVirtualProtect
   test  eax, eax
   je    @FAILURE
   
; Flush the instruction cache for the startup code location
   push  pData.cbTargetSize
   push  pData.pTargetAddress
   push  NtCurrentProcess
   call  pData.pFlushInstructionCache
   
; Notify the calling process that we're done
   push  pData.hEvent
   call  pData.pSetEvent
   
;  Close the event handle
   push  pData.hEvent
   call  pData.pCloseHandle
   
; Save the pData pointer so we can reset edi
   mov   eax, edi
   
; Reset the stack 
   pop   edi
   add   esp, 4
   pop   ebp

; Build a call stack to free this memory and return to the target address. This
; has to be done via a JMP and not a CALL because the CALL instruction would put
; up a return address to the memory being freed, which when VirtualFree() issued
; a RET wouldn't be around to return to
   push  MEM_RELEASE
   push  0
   push  eax
   push  [eax.SxDATA].pTargetAddress
   jmp   [eax.SxDATA].pVirtualFree
   
@FAILURE:
; Get the cause of failure and exit the process. We exit because there is no way 
; to ensure the process can continue safely
   call  pData.pGetLastError
   
   push  eax
   call  pData.pExitProcess
   
CjStartProc ENDP

_TEXT ENDS

END
