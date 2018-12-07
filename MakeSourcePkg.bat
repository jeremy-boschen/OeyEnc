@ECHO OFF

REM
REM This script will read the SourceListing.txt file in the solution
REM directory to create zip file containing a non-source-controled 
REM copy of the entire solution .
REM
REM Usage:
REM   MAKESOURCEPKG.BAT PathOfSourceListing.txt PathOfZipFileToCreate
REM

REM Path and command line for whatever zip utility is used on the
REM target system. 7-zip is the default

@ECHO ON
SET ZIPUTL=C:\Program Files\7-Zip\7z.exe
SET ZIPCMD=a -tzip %~f2 "%TMP%\SRCINSTALL\*"

IF EXIST %~f1 (

   REM Create any directories for the COPY command in the
   REM temp working directory
   REM 
   REM In order to extract the directory names from each line
   REM in SourceListing.txt as relative directories a bogus
   REM path is constructed using the root folder of the path
   REM to the SourceListing.txt file
   
   IF EXIST %TMP%\SRCLIST.TMP (
      DEL /Q %TMP%\SRCLIST.TMP
   )
   
   FOR /F %%I IN (%~f1) DO (
      ECHO %%~dI\%%~I >> %TMP%\SRCLIST.TMP
   )
      
   FOR /F %%I IN (%TMP%\SRCLIST.TMP) DO (
      IF NOT EXIST "%TMP%\SRCINSTALL%%~pI" (
         ECHO Creating directory "%TMP%\SRCINSTALL%%~pI"
         MKDIR "%TMP%\SRCINSTALL%%~pI"
      )
   )
   
   REM DEL /Q %TMP%\SRCLIST.TMP

   REM Copy each file listed in SourceListing.txt to the
   REM temp working directory
   
   FOR /F %%I IN (%~f1) DO (
      ECHO Copying file %%I
      COPY "%~dp1%%I" "%TMP%\SRCINSTALL\%%~I" 1>nul
   )
   
   REM Call RemoveSccInfo.js to remove the SCC provider
   REM information from the solution copy
   
   IF ERRORLEVEL 0 (
      CSCRIPT.EXE //NOLOGO RemoveSccInfo.js "%TMP%\SRCINSTALL" 2>nul
   )
   
   REM Zip up the solution copy
   
   IF ERRORLEVEL 0 (
      CALL "%ZIPUTL%" %ZIPCMD% 2>nul
   )   
)

SET ERRORLEVEL=0
