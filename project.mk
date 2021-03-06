
WSK_INC_PATH		= $(WSKROOT)\Include
WTL_INC_PATH		= $(WTLROOT)\Include

!IF "$(_BUILDARCH)"=="AMD64"
WSK_LIB_PATH		= $(WSKROOT)\LIB\x64
!ELSE
WSK_LIB_PATH		= $(WSKROOT)\LIB
!ENDIF

INCLUDES = \
   $(INCLUDES);      \
   $(WSK_INC_PATH);	\
   $(WTL_INC_PATH);  \
   ..\inc\;          \
   ..\CoJack\;       \
   ..\Setup.Lib\;    \
   
!IF $(FREEBUILD)
USE_MSVCRT        = 1
!ELSE
MSC_OPTIMIZATION  = /Odi
USE_LIBCMT        = 1
DEBUG_CRTS        = 1
C_DEFINES         = $(C_DEFINES) /D_ATL_DEBUG_INTERFACES /D_ATL_DEBUG_QI
!IF "$(_BUILDARCH)" == "AMD64"
C_DEFINES			= $(C_DEFINES) /D_CRTDBG_MAP_ALLOC
!ENDIF
!ENDIF
NTDEBUGTYPE       = both

C_DEFINES         = $(C_DEFINES) /D_WDKBUILD /D_UNICODE /DUNICODE /DSTRICT /DSTRSAFE_NO_DEPRECATE /DISOLATION_AWARE_ENABLED=1 /DNO_SHLWAPI_PATH
USER_C_FLAGS      = $(USER_C_FLAGS) /wd4201 /wd4505

LINKER_FLAGS      = $(LINKER_FLAGS) /IGNORE:4505 /NXCOMPAT
!IF "$(_BUILDARCH)" == "AMD64"
!MESSAGE Setting LTCG linker option for AMD64 build
LINKER_FLAGS      = $(LINKER_FLAGS) /LTCG /OPT:lbr
!ENDIF

MSC_WARNING_LEVEL = /W4

UNICODE           = 1
UMTYPE            = windows

# This is set explicitly so we can build for previous versions using new features in the headers,
# but still use the downlevel linker settings
WIN32_WINNT_VERSION=$(LATEST_WIN32_WINNT_VERSION)
NTDDI_VERSION=$(LATEST_NTDDI_VERSION) 
!IF "$(_BUILDARCH)" == "x86"
_NT_TARGET_VERSION=$(_NT_TARGET_VERSION_WIN2K)
!ELSEIF "$(_BUILDARCH)" == "AMD64"
_NT_TARGET_VERSION=$(_NT_TARGET_VERSION_WS03)
!ENDIF

PROJECT_ROOT      = ..\