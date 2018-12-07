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

text SEGMENT PARA 'CODE'

   ALIGN 16
   
   PUBLIC CjStartupRoutine
         
CjStartupRoutine PROC FRAME

   ; Preserve parameter passing, and non-volatile registers. Note that this is not a
   ; standard X64 stack frame because this function isn't called, it is switched to
   ; via a CONTEXT modification by ResumeProcessWithModule()
      push  rbx
      push  rcx
      push  rdi
      push  r8
      push  r9
      
   ; Ensure the stack is aligned properly, and save any alignment so it can be reset
   ; in the epilogue
      mov   rdi, rsp
      and   rdi, (16-1)   
   ; Add scratch space for called functions, 8 bytes for each of the 4 parameter passing
   ; registers, and parameters we have to pass
      add   rdi, (4*8)+(2*8)   
      sub   rsp, rdi
      
   .ENDPROLOG

   ; ResumeProcessWithModule() will load RAX with the address of the SxDATA record for 
   ; this function, so load it into RBX where we'll use it for most of the function
      mov   rbx, rdx
      
IF DBG EQ 1
CjSRWaitForDebugger:
      mov   rcx, 15 * 1000
      call  SxDATA.pSleep[rbx]
      call  SxDATA.pIsDebuggerPresent[rbx]
      test  eax, eax
      je    CjSRWaitForDebugger

CjSRContinueWithDebugger:      
      int   3
ENDIF
      
   ; Attempt to load the specified module
      lea   rcx, SxDATA.chModuleName[rbx]
      call  SxDATA.pLoadLibraryW[rbx]
      test  rax, rax
      je    CjSRFailure

   ; Notify the calling process that we've successfully loaded the module
      mov   rcx, SxDATA.hEvent[rbx]
      call  SxDATA.pSetEvent[rbx]
   
   ; Close our copy of the event handle we just notified
      mov   rcx, SxDATA.hEvent[rbx]
      call  SxDATA.pCloseHandle[rbx]
   
   ; Create a new thread in this process that will eventually free the memory for
   ; the SxDATA record. As per the AMD64 calling convention, 
      mov   QWORD PTR [rsp+40], 0
      mov   QWORD PTR [rsp+32], 4
      mov   r9,  rbx
      call  CjSRLoadThreadRoutine
      xor   rdx, rdx
      xor   rcx, rcx
      call  SxDATA.pCreateThread[rbx]
      test  rax, rax
      je    CjSRFailure
      
   ; Close the thread handle      
      mov   rcx, rax
      call  SxDATA.pCloseHandle[rbx]
      
   ; Copy the SxDATA pointer so we can reset RBX
      mov   rax, rbx
      
   ; Reset the stack
      add   rsp, rdi      
      pop   r9
      pop   r8
      pop   rdi
      pop   rcx
      pop   rbx

   ; Everything went OK, so jump back to the startup code and allow the process to continue
      ;jmp   SxDATA.pTargetAddress[rax]
      ret
   
CjSRFailure:
   ; Get the cause of failure and exit the process. We exit because there is no way 
   ; to ensure the process can continue safely
      call  SxDATA.pGetLastError[rbx]
   
      mov   ecx, eax
      call  SxDATA.pExitProcess[rbx]      

;
; This is a stub that gets the address of the thread routine passed to
; the CreateThread() function above
;
CjSRLoadThreadRoutine:

      sub   rsp, 8

   ; Issue a CALL to the next instruction so we can put RIP up on the stack. We could use
   ; RIP-relative addressing here, but ML64 doesn't support it and this will also work for X86
      xor   rcx, rcx
      call  CjSRThreadRoutine
   
CjSRThreadRoutine:
      
   ; Check if we're being called to return an address, or to run the thread routine. For the
   ; first case, RCX=0
      test  rcx, rcx
      jne   CjSRThreadRoutineContinue
      
   ; This is a call from CjSRLoadThreadRoutine      
      pop   r8
      add   rsp, 8
      ret
      
CjSRThreadRoutineContinue:
   
IF DBG EQ 1
      int   3
ENDIF

   ; Align the stack and reserve space for scratch params
      sub   rsp, 8+(4*32)
      
   ; Save the SxDATA record pointer to a non-volatile register
      mov   rbx, rcx
   
   ; Sleep for what should be long enough to give the primary thread time to get back to
   ; running its main code      
      mov   rcx, 1000 * 60
      call  SxDATA.pSleep[rbx]
      
   ; Free the memory allocated for this routine and its data. Note that this will return
   ; to ExitThread due to the new RET address we put up on the stack      
      mov   r8d, MEM_RELEASE
      xor   rdx, rdx
      mov   rcx, rbx
      push  SxDATA.pExitThread[rbx]
      jmp   SxDATA.pVirtualFree[rbx]

CjStartupRoutine ENDP

text ENDS

END