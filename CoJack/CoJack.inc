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

;  CoJack.inc
;     CoJack assembly definitions

MAX_PATH         = 260
PAGE_READWRITE   = 04h
MEM_RELEASE      = 08000h
NtCurrentProcess = -1

PVOID TYPEDEF PTR BYTE 

SxDATA STRUCT 16
   pLoadLibraryW           PVOID  ?
      ALIGN(16)
   pSetEvent               PVOID  ?
      ALIGN(16)
   pCloseHandle            PVOID  ?
      ALIGN(16)

   pGetLastError           PVOID  ?
      ALIGN(16)
   pExitProcess            PVOID  ?
      ALIGN(16)

   pCreateThread           PVOID  ?
      ALIGN(16)   
   pExitThread             PVOID  ?
      ALIGN(16)
   pVirtualFree            PVOID  ?
      ALIGN(16)
   pSleep                  PVOID  ?
      ALIGN(16)
   
IF DBG EQ 1
   pIsDebuggerPresent      PVOID  ?
      ALIGN(16)
ENDIF

   hEvent                  PVOID   ?
      ALIGN(16)

   chModuleName            WORD MAX_PATH DUP(?)   
SxDATA ENDS

TxDATA STRUCT 16
   pLoadLibraryW           PVOID  ?
      ALIGN(16)
   pGetLastError           PVOID  ?
      ALIGN(16)

   chModuleName            WORD MAX_PATH DUP(?)
TxDATA ENDS

PEB STRUCT
    InheritedAddressSpace     BYTE ?
    ReadImageFileExecOptions  BYTE ?
    BeingDebugged             BYTE ?
PEB ENDS        

NT_TIB STRUCT
   ExceptionList        PVOID ?
   StackBase            PVOID ?
   StackLimit           PVOID ?
   SubSystemTib         PVOID ?
   FiberData            PVOID ?
   ArbitraryUserPointer PVOID ?
   Self                 PVOID ?
NT_TIB ENDS

CLIENT_ID STRUCT
  UniqueProcess   PVOID ?
  UniqueThread    PVOID ?
CLIENT_ID ENDS

TEB STRUCT
   Tib                        NT_TIB    <?>
   EnvironmentPointer         PVOID      ?
   Cid                        CLIENT_ID <?>
   ActiveRpcHandle            PVOID      ?
   ThreadLocalStoragePointer  PVOID      ?
   ProcessEnvironmentBlock    PVOID      ?
TEB ENDS

IFDEF _X86_
   IsDebuggerPresent MACRO
      mov   eax, DWORD PTR fs:[18h]
      mov   ecx, DWORD PTR TEB.ProcessEnvironmentBlock[eax]
      movzx eax, BYTE PTR PEB.BeingDebugged[ecx]
   ENDM
ELSEIFDEF _AMD64_
   IsDebuggerPresent MACRO
      mov   rax, QWORD PTR gs:[NT_TIB.Self]
      mov   rcx, QWORD PTR TEB.ProcessEnvironmentBlock[rax]
      movzx eax, BYTE PTR PEB.BeingDebugged[rcx]
   ENDM
ELSE
   .ERR Unsupported target architecture
ENDIF
