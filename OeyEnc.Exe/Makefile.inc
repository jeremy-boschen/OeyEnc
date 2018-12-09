

EmbedManifest_Target:
   MT.EXE -manifest $(TARGET).manifest -outputresource:$(TARGET);1
   
PostBuild_Target: EmbedManifest_Target   
   COPY /Y ..\OeyEnc.dll\$(O)\OeyEnc.dll $(O)\OeyEnc.dll
   IF EXIST PostBuild.Mutexed.bat PostBuild.Mutexed.bat $(TARGET)
