/*
    WDKBuild.js - Build script for using the WDK environment
 */

var szWDKRoot;
var szBuildDir;
var szBuildType;
var szPlatform;
var szTargetOS;
var szBuildCmd;

WScript.Echo('\nWDKBuild.js - Building WDKBUILD.LOCAL.BAT');

var __oShell   = WScript.CreateObject('WScript.Shell');
var __oFileSys = WScript.CreateObject('Scripting.FileSystemObject');

function GetDirectory( szName ) {
    return ( __oFileSys.GetAbsolutePathName(szName) );
}

function GetEnvironmentVariable( szName ) {
    var szEnv = __oShell.Environment('SYSTEM').Item(szName);
    return ( szEnv );
}

function GetCommandLineVariable( szName, szDefault ) {
    if ( WScript.Arguments.Named.Exists(szName) ) {
        return ( WScript.Arguments.Named.Item(szName) );
    }
    return ( szDefault );
}

/* Load command options */
szWDKRoot  = GetEnvironmentVariable('WDKROOT');
szBuildDir = GetDirectory('.');
szBuildType= GetCommandLineVariable('build', '');
szPlatform = GetCommandLineVariable('platform', 'X86');
szTargetOS = GetCommandLineVariable('target', 'WNET');

if ( !szWDKRoot || ('' == szWDKRoot) ) {
    WScript.Echo('WDKBuild.js - Error: WDKROOT environment variable is not defined.');   
    WScript.Quit(-1);
}

WScript.Echo('WDKBuild.js - WDKROOT found at: ' + szWDKRoot);

szBuildCmd = '';
szBuildCmd += 'PUSHD . \r\n';
szBuildCmd += 'CALL ' + szWDKRoot + '\\bin\\setenv.bat ' + szWDKRoot + ' ' + szBuildType + ' ' + szPlatform + ' ' + szTargetOS + '\r\n';
szBuildCmd += 'POPD \r\n'
szBuildCmd += 'CD ' + szBuildDir + '\r\n';
szBuildCmd += 'BUILD -beFW';
var rgCmd = WScript.Arguments.Unnamed;
for ( var i = 0; i < rgCmd.length; i++ ) {
    szBuildCmd += ' ' + rgCmd.Item(i);
}
szBuildCmd += '\r\n';

var szBuildPath= __oFileSys.BuildPath('.\\', 'WDKBUILD.LOCAL.BAT');
var oBuildFile = __oFileSys.CreateTextFile(szBuildPath, true, false);
if ( null != oBuildFile ) {
    oBuildFile.Write(szBuildCmd);
    oBuildFile.Close();

    WScript.Quit(0);
}

WScript.Quit(1);
