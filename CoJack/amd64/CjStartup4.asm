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
;     suspended process on AMD64 platforms, via an APC

include CoJack.inc

;++
; VOID
; CjStartupRoutine (
;     __in SxDATA* Record,
; )
;
; Parameters:
;     rcx - Supplies the SxDATA record
;
; Return Value:
;     None
;--

text segment para 'CODE'

   align 16
   
   public CjStartupRoutine

; This is for representing offsets of shadow space slots, and local slots for stack 
; parameters to called functions. On the stack, the frame is in reverse order as the
; stack grows down, so when a function is called, it sees this frame as RSP+0=ret-address, 
; RSP+8 = P1Shadow, RSP+16=P2Shadow, etc. It is in this order so we can use MASM syntax of 
; STRUCT.Member[rsp] syntax, which becomes [rsp+&(((STRUCT*)0).MEMBER)]
CjSRFrame struct
   P1Shadow       dq ?
   P2Shadow       dq ?
   P3Shadow       dq ?
   P4Shadow       dq ?
   CreationFlags  dq ?
   ThreadId       dq ?
CjSRFrame ends

CjStartupRoutine proc frame
      
if dbg eq 1
      mov   r11, rcx

CjSRWaitForDebugger:
      pause
      IsDebuggerPresent
      test  eax, eax
      je    CjSRWaitForDebugger

      mov   rcx, r11
      int   3
endif
   
   ; Pushing RBX will reallign the stack to 16 bytes, then we add 32 bytes for our 
   ; shadow space, as per the calling convention, then another 16 bytes for the two
   ; extra stack parameters we have to pass
      push  rbx
      sub   rsp, (sizeof CjSRFrame)

   .endprolog

   ; Save the SxDATA record in a non-volatile register
      mov   rbx, rcx

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
      
   ; Copy the SxDATA pointer so we can reset RBX
      mov   rax, rbx
      
   ; Reset the stack 
      add   rsp, (sizeof CjSRFrame)
      pop   rbx

   ; Setup a call to VirtualFree and JMP there, not CALL. This will leave the
   ; return address put up by the caller on the stack, allowing us to return
   ; there after we free all our data, which includes this code
      mov   r8d, MEM_RELEASE
      xor   rdx, rdx
      mov   rcx, rax
      jmp   SxDATA.pVirtualFree[rax]

CjSRFailure:
   ; Get the cause of failure and exit the process. We exit because there is no way 
   ; to ensure the process can continue safely
      call  SxDATA.pGetLastError[rbx]
   
      mov   ecx, eax
      call  SxDATA.pExitProcess[rbx]      

CjStartupRoutine endp

text ends

end