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

;  x64-CjThread.asm
;     CjThreadRoutine procedure for loading a library into running process
;     via CreateRemoteThread() 


INCLUDE CoJack.inc

;++
;
; VOID
; CjThreadRoutine (
;     __in PVOID Parameter
;  )
;
; Parameters:
;     rcx - Supplies the TxDATA record
;
; Return Value:
;     None
;--

_TEXT segment para 'CODE'

   align 16
   
   public CjThreadRoutine
   
CjThreadRoutine proc frame

IF DBG EQ 1
      db    048h
      int   3
ENDIF
      db    048h
      push  rdi
      .pushreg rdi

      .endprolog

      mov   rdi, rcx

      lea   rcx, TxDATA.chModuleName[rdi]
      call  TxDATA.pLoadLibraryW[rdi]
      test  rax, rax
      xor   rax, rax
      je    CjTR0
      
      call  [rdi.TxDATA].pGetLastError      

CjTR0:
      pop   rdi   
      ret

CjThreadRoutine endp

_TEXT ends

end