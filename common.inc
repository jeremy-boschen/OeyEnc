
MakeMUI:
   @ECHO Splitting MUI binaries
   ..\MAKEMUI.BAT /platform:$(_BUILDARCH) /out:$O /cultures:MUI.languages /target:$(TARGETNAME).$(TARGETEXT) /rcbase:$(TARGETNAME) /rcconfig:MUI.rcconfig /rcoptions:"$(RCOPTIONS) -I$O$(INCLUDES)"

DeleteTarget:
   @ECHO Deleting target file
   DEL /Q $O\$(TARGETNAME).$(TARGETEXT) 2>nul
   DEL /Q $O\*.res 2>nul

CopyToSharedOut:
   @ECHO Copying targets to shared out directory   
   @MKDIR ..\out\$O\ 
   @COPY /Y $O\$(TARGETNAME).$(TARGETEXT) ..\out\$O\ 
   @COPY /Y $O\$(TARGETNAME).pdb ..\out\$O\ 
   -FOR /F %I IN (MUI.languages) DO MKDIR ..\out\$O\%I & COPY /Y $O\%I\$(TARGETNAME).$(TARGETEXT).mui ..\out\$O\%I\$(TARGETNAME).$(TARGETEXT).mui 


#
# Custom COM registration target that will only register AMD64 builds on an AMD64 machine,
# and X86 builds on an X86 machine
#
REGSVR_CMD = $(OBJ_PATH)\$O\regsvr.bat

RegisterTargetSharedOut:
   @type <<$(REGSVR_CMD)
IF "%~x1"==".dll" ( REGSVR32.EXE /c /s "%~f1" ) ELSE IF "%~x1"==".exe" ( "%~1" /REGSERVER )
<<$(BUILD_NOKEEP)
!IF ("$(PROCESSOR_ARCHITECTURE)" == "AMD64" || "$(PROCESSOR_ARCHITEW6432)" == "AMD64") && ("$(_BUILDARCH)" == "AMD64")
!IF !$(FREEBUILD)
   @ECHO Registering X64 target
   $(REGSVR_CMD) /s ..\out\$O\$(TARGETNAME).$(TARGETEXT)
!ENDIF
!ELSEIF ("$(PROCESSOR_ARCHITECTURE)" != "AMD64") && ("$(PROCESSOR_ARCHITEW6432)" != "AMD64") && ("$(_BUILDARCH)" != "AMD64") 
!IF !$(FREEBUILD)
   @ECHO Registering X86 target
   $(REGSVR_CMD) /s ..\out\$O\$(TARGETNAME).$(TARGETEXT)
!ENDIF
!ENDIF

#
# These are targets to pull in a local file that isn't included with the project source, that can be used
# to do something location specific. 
#
!IF ("$(BUILD_PASS)" == "PASS0") && EXIST($(MAKEDIR)\prebuildlocal)
!INCLUDE $(MAKEDIR)\prebuildlocal
!ELSE
CustomPreBuildLocal_Target:
   @ECHO No custom pre build local target >nul
!ENDIF

!IF ("$(BUILD_PASS)" == "PASS1") && EXIST($(MAKEDIR)\buildlocal)
!INCLUDE $(MAKEDIR)\buildlocal
!ELSE
CustomBuildLocal_Target:
   @ECHO No custom build local target >nul
!ENDIF

!IF ("$(BUILD_PASS)" == "PASS2") && EXIST($(MAKEDIR)\postbuildlocal)
!INCLUDE $(MAKEDIR)\postbuildlocal
!ELSE
CustomPostBuildLocal_Target:
   @ECHO No custom post build local target >nul
!ENDIF
