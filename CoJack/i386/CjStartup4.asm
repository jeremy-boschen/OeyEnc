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

.386
.model flat, stdcall

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

   public CjStartupRoutine

CjStartupRoutine proc 
      
if DBG eq 1
      int   3
      align 16
endif

   ; Save non-volatile registers
      push  esi
   
   ; Store the SxDATA parameter in a non-volatile register
      mov   esi, DWORD PTR [esp+8]
   
   ; Attempt to load the specified module
      lea   eax, DWORD PTR SxDATA.chModuleName[esi]
      push  eax
      call  SxDATA.pLoadLibraryW[esi]
      test  eax, eax
      je    CjSRFailure

   ; Notify the calling process that we're done
      push  SxDATA.hEvent[esi]      
      call  SxDATA.pSetEvent[esi]
      
   ; Close the event handle
      push  SxDATA.hEvent[esi]
      call  SxDATA.pCloseHandle[esi]
   
   ; Save SxDATA record
      mov   eax, esi
   
   ; Restore the stack
      pop   esi
      
   ; Save the caller's return address
      pop   ecx
   
   ; Build a new call frame to VirtualFree(). The stack parameter passed to this function
   ; remains, so overwrite with the one for VirtualFree() that needs its slot
      mov   DWORD PTR [esp+0], MEM_RELEASE
      push  0
      push  eax
   
   ; Put the caller's return address back on the stack, and transfer to VirtualFree()
      push  ecx
      jmp   SxDATA.pVirtualFree[eax]      
            
CjSRFailure:

CjStartupRoutine endp

text ends

end