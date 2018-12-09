

EmbedManifest_Target:
	MT.EXE -manifest $(TARGET).manifest -outputresource:$(TARGET);1

CopyDebugSetup_Target:
	COPY /B /Y $(TARGET) + BootStrap.Dbg.Setup.msi $(O)\Setup.exe

PostBuild_Target: EmbedManifest_Target CopyDebugSetup_Target