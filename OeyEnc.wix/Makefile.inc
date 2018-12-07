
CustomPreBuild_Target: 

CustomBuild_Target: 

CustomPostBuild_Target: 
   COPY /y /b "$(OBJ_PATH:oeyenc.wix=bootstrap)\objfre_w2k_x86\i386\bootstrap.exe" + "$(O)\$(TARGETNAME).msi" "$(O)\$(TARGETNAME).exe"

