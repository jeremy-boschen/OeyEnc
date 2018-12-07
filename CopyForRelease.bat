SET ZIPUTL=C:\Program Files\7-Zip\7z.exe
SET ZIPCMD=a -tzip

IF NOT EXIST WebSite (
   MKDIR WebSite
)

IF EXIST ".\WebSite\OeyEncSource.zip" (
  DEL .\WebSite\OeyEncSource.zip
)

CALL MakeSRCPKG.bat /srcfile:SourceListing.txt /outfile:WebSite\OeyEnc-Source.zip > MkSource.log

COPY /Y .\OeyEnc.wix\License.txt .\WebSite\
COPY /Y .\OeyEnc.wix\History.txt .\WebSite\

:: Copy x86 OeyEnc files...
COPY /Y .\OeyEnc.wix\objfre_w2k_x86\i386\OeyEnc-Setup-x86.msi .\WebSite\OeyEnc-Setup-x86.msi
COPY /Y .\OeyEnc.wix\objfre_w2k_x86\i386\OeyEnc-Setup-x86.exe .\WebSite\OeyEnc-Setup-x86.exe
COPY /Y .\OeyEnc.exe\objfre_w2k_x86\i386\OeyEnc.pdb .\WebSite\OeyEnc.exe-x86.pdb
COPY /Y .\OeyEnc.dll\objfre_w2k_x86\i386\OeyEnc.pdb .\WebSite\OeyEnc.dll-x86.pdb

:: Copy x64 OeyEnc files...
COPY /Y .\OeyEnc.wix\objfre_wnet_amd64\amd64\OeyEnc-Setup-x64.msi .\WebSite\OeyEnc-Setup-x64.msi
COPY /Y .\OeyEnc.wix\objfre_wnet_amd64\amd64\OeyEnc-Setup-x64.exe .\WebSite\OeyEnc-Setup-x64.exe
COPY /Y .\OeyEnc.exe\objfre_wnet_amd64\amd64\OeyEnc.pdb .\WebSite\OeyEnc.exe-x64.pdb
COPY /Y .\OeyEnc.dll\objfre_wnet_amd64\amd64\OeyEnc.pdb .\WebSite\OeyEnc.dll-x64.pdb

:: Copy x86 BootStrap files (these are always x86, never amd64)
COPY /Y .\BootStrap\\objfre_w2k_x86\i386\BootStrap.exe .\WebSite\BootStrap-x86.exe
COPY /Y .\BootStrap\\objfre_w2k_x86\i386\BootStrap.pdb .\WebSite\BootStrap-x86.pdb


PAUSE