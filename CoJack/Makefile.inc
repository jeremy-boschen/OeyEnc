
MakeObjectDatFiles:
   -FOR %%I IN ($(OBJ_PATH)\$(O)\*.obj) DO ObjEx.exe %%I > $(_BUILDARCH)-%%~nI.dat
