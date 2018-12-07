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
;     CjThreadProc procedure for loading a library into running process
;     via CreateRemoteThread() 

OPTION CASEMAP:none
OPTION PROLOGUE:None
OPTION EPILOGUE:None

_TEXT SEGMENT

.DATA

INCLUDE CoJack.inc

.CODE

CjThreadProc PROC

; Setup an alias for the SxDATA pointer loaded into rdi
; by the calling code
   pData EQU [rdi.TxDATA]

IF DBG EQ 1
   db 15 dup(090h)
   int   3
ENDIF

; Setup a standard stack frame
   push  rbp
   mov   rbp, rsp
   sub   rsp, (4*8)+8
   push  rdi
   
; Load the TxDATA pointer
   mov   rdi, rcx

; Attempt to load the specified module   
   lea   rcx, pData.chModuleName
   call  pData.pLoadLibraryW
   test  rax, rax
   xor   rax, rax
   je    @EXIT
   
; Get the cause of the failure
   call  pData.pGetLastError

@EXIT:
; Cleanup the stack
   pop   rdi
   add   rsp, (4*8)+8
   pop   rbp
   
; Exit this thread
   ret
   
CjThreadProc ENDP

_TEXT ENDS

END
