CALL D:\DEVKIT\DDK\6001.18002\bin\setenv.bat D:\DEVKIT\DDK\6001.18002 CHK X86 W2K
CD D:\Projects\Programming\Active\OeyEnc.WDK\OeyEnc.wix
DEL /Q buildchk_w2k_x86.err 2>nul
DEL /Q buildchk_w2k_x86.wrn 2>nul
DEL /Q buildchk_w2k_x86.log 2>nul
SET PATH=%PATH%;%WSKROOT%\bin;%WIXROOT%
BUILD -begFW -j buildchk_w2k_x86 -c
