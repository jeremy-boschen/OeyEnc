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

INCLUDE CoJack.inc

;++
; VOID
; CjStartupRoutine (
;     __in SxDATA* Paramter
; )
;
; Parameters:
;     [esp] - Supplies the SxDATA record
;
; Return Value:
;     None
;--

SrFrame STRUCT
   MemoryProtection  DWORD ?   
   Alignment         DWORD ?
SrFrame ENDS

_text SEGMENT PARA 'CODE'

   ALIGN 16
   
   PUBLIC CjStartupRoutine
   
CjStartupRoutine PROC FRAME

IF DBG EQ 1
      db    048h
      int   3
      ALIGN 16
ENDIF

   ; Remove the parameter pushed by the calling code. This is done first because the
   ; calling code uses JMP to transfer to this location, so there is no return address
   ; on the stack like there would be with a CALL instruction
      pop   rax

   ; Setup the stack frame
      push  rdi
   .PUSHREG rdi
      push  rsi
   .PUSHREG rsi
      push  rbx
   .PUSHREG rbx
      
      sub   rsp, SIZEOF SrFrame
   .ALLOCSTACK SIZEOF SrFrame
      
   .ENDPROLOG
      
   ; Load the SxDATA pointer
      mov   rbx, rax
      
   ; Attempt to load the specified module
      lea   rcx, SxDATA.chModuleName[rbx]
      call  SxDATA.pLoadLibraryW[rbx]
      test  rax, rax
      je    CjSR0
   
   ; Enable write access to the original startup code location
      lea   r9,  QWORD PTR SrFrame.MemoryProtection[rsp]
      mov   r8d, PAGE_READWRITE
      mov   edx, SxDATA.cbTargetSize[rbx]
      mov   rcx, SxDATA.pTargetAddress[rbx]
      call  SxDATA.pVirtualProtect[rbx]
      test  eax, eax
      je    CjSR0
   
   ; Copy the original startup code back.
      mov   ecx, DWORD PTR SxDATA.cbTargetSize[rbx]
      mov   rdi, SxDATA.pTargetAddress[rbx]
      mov   rsi, SxDATA.pCodeAddress[rbx]
      cld
      rep movsb   
   
   ; Return the page access to its original state
      lea   r9, DWORD PTR SrFrame.MemoryProtection[rsp]
      mov   r8d, SrFrame.MemoryProtection[rsp]
      mov   edx, SxDATA.cbTargetSize[rbx]
      mov   rcx, SxDATA.pTargetAddress[rbx]
      call  SxDATA.pVirtualProtect[rbx]
      test  eax, eax
      je    CjSR0
   
   ; Flush the instruction cache for the startup code location
      mov   r8d, SxDATA.cbTargetSize[rbx]
      mov   rdx, SxDATA.pTargetAddress[rbx]
      mov   rcx, NtCurrentProcess
      call  SxDATA.pFlushInstructionCache[rbx]
      
   ; Notify the calling process that we're done
      mov   rcx, SxDATA.hEvent[rbx]
      call  SxDATA.pSetEvent[rbx]
   
   ; Close the event handle
      mov   rcx, SxDATA.hEvent[rbx]
      call  SxDATA.pCloseHandle[rbx]
   
   ; Copy the SxDATA pointer so we can reset RBX
      mov   rax, rbx
      
   ; Reset the stack
      add   rsp, sizeof SrFrame
      pop   rbx
      pop   rsi
      pop   rdi

   ; Build a call stack to free this memory and return to the target address. This
   ; has to be done via a JMP and not a CALL because the CALL instruction would put
   ; up a return address to the memory being freed, which when VirtualFree() issued
   ; a RET wouldn't be around to return to
      mov   r8d, MEM_RELEASE
      xor   rdx, rdx
      mov   rcx, rax
      push  SxDATA.pTargetAddress[rax]
      jmp   SxDATA.pVirtualFree[rax]
   
CjSR0:
   ; Get the cause of failure and exit the process. We exit because there is no way 
   ; to ensure the process can continue safely
      call  SxDATA.pGetLastError[rbx]
   
      mov   ecx, eax
      call  SxDATA.pExitProcess[rbx]

CjStartupRoutine ENDP

_text ENDS

END