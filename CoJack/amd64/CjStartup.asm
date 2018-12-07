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

;  x64-CjStartup.asm
;     CjStartProc() procedure for loading a library into an initially 
;     suspended process on AMD64 platforms

OPTION CASEMAP:none
OPTION PROLOGUE:None
OPTION EPILOGUE:None

.DATA

INCLUDE CoJack.inc

STACK_SAVE_SIZE = (4*8)
.CODE

CjStartup PROC

; MASM will reference this as [rbp-4]
   LOCAL dwProtect:DWORD
   
; Setup an alias for the SxDATA pointer loaded into rdi
; by the calling code
   pData EQU [rdi.SxDATA]

IF DBG EQ 1
   db 15 dup(090h)
   int   3
ENDIF

; Remove the parameter pushed by the calling code. This is done first because the
; calling code uses JMP to transfer so there is no return address on the stack.
   pop   rax

; Setup a standard stack frame. We use a single 8 byte local, 32 bytes for the register
; save area and 8 more bytes to force RSP to 16-byte alignment
   push  rbp
   mov   rbp, rsp
   sub   rsp, 8+32+8
   push  rdi

; Load the SxDATA pointer 
   mov   rdi, rax
   
; Attempt to load the specified module
   lea   rcx, pData.chModuleName
   call  pData.pLoadLibraryW   
   test  rax, rax
   je    @FAILURE
   
; Enable write access to the original startup code location
   lea   r9, dwProtect
   mov   r8d, PAGE_READWRITE
   mov   edx, pData.cbTargetSize
   mov   rcx, pData.pTargetAddress
   call  pData.pVirtualProtect
   test  eax, eax
   je    @FAILURE
   
; Copy the original startup code back. RDI is pData, so it must be setup last
   push  rdi
   push  rsi
   movsxd rcx, DWORD PTR pData.cbTargetSize
   mov   rsi, pData.pCodeAddress
   mov   rdi, pData.pTargetAddress
   cld
   rep movsb   
   pop   rsi
   pop   rdi
   
; Return the page access to its original state
   lea   r9, dwProtect
   mov   r8d, dwProtect
   mov   edx, pData.cbTargetSize
   mov   rcx, pData.pTargetAddress
   call  pData.pVirtualProtect
   test  eax, eax
   je    @FAILURE   
   
; Flush the instruction cache for the startup code location
   mov   r8d, pData.cbTargetSize
   mov   rdx, pData.pTargetAddress
   mov   rcx, NtCurrentProcess
   call  pData.pFlushInstructionCache
   
; Notify the calling process that we're done
   mov   rcx, pData.hEvent
   call  pData.pSetEvent
   
; Close the event handle
   mov   rcx, pData.hEvent
   call  pData.pCloseHandle
   
; Save the pData pointer so we can reset rdi
   mov   rax, rdi
      
; Reset the stack
   pop   rdi
   add   rsp, 4+32+4
   pop   rbp

; Build a call stack to free this memory and return to the target address. This
; has to be done via a JMP and not a CALL because the CALL instruction would put
; up a return address to the memory being freed, which when VirtualFree() issued
; a RET wouldn't be around to return to
   mov   r8d, MEM_RELEASE
   xor   rdx, rdx
   mov   rcx, rax
   push  [rax.SxDATA].pTargetAddress
   jmp   [rax.SxDATA].pVirtualFree
   
@FAILURE:
; Get the cause of failure and exit the process. We exit because there is no way 
; to ensure the process can continue safely
   call  pData.pGetLastError
   
   mov   ecx, eax
   call  pData.pExitProcess

CjStartup ENDP

END
