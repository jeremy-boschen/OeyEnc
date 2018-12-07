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

;  x86-CjThread.asm
;     CjThreadProc procedure for loading a library into running process
;     via CreateRemoteThread()

.386
.MODEL FLAT, STDCALL
OPTION CASEMAP:none
OPTION PROLOGUE:None
OPTION EPILOGUE:None

_TEXT SEGMENT

.DATA

INCLUDE CoJack.inc

.CODE

CjThreadProc PROC

; Setup an alias for the SxDATA pointer loaded into rdi
pData EQU [edi.TxDATA]

IF DBG EQ 1
   int   3
ENDIF

; Setup a standard stack frame
   push  ebp
   mov   ebp, esp
   push  edi
   
; Load the TxDATA pointer
   mov   edi, DWORD PTR [ebp+08h]
      
; Attempt to load the specified module   
   lea   eax, pData.chModuleName
   push  eax
   call  pData.pLoadLibraryW   
   test  eax, eax
   xor   eax, eax
   jne   @EXIT
   
; Get the cause of the failure
   call  pData.pGetLastError

@EXIT:
; Cleanup the stack
   pop   edi
   pop   ebp
   
; Exit this thread
   ret   4
   
CjThreadProc ENDP

_TEXT ENDS

END
