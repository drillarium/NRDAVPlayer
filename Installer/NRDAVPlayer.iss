; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyDateTimeString GetDateTimeString('yyyymmddhhnnss', '-', ':');
#define InstallVersion MyDateTimeString

#define MyAppName "NRDAVPlayer"
#define MyAppVersion "0.0.2"
#define MyAppPublisher "NRD Multimedia."
#define MyAppURL "https://nrd.es/"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{aee705fd-a852-4a73-87be-95030b9efdfd}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName=C:\avio\{#MyAppName}
DisableProgramGroupPage=yes
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
OutputDir=.\
OutputBaseFilename=NRDAVPlayerInstaller.{#InstallVersion}
OutputManifestFile=NRDAVPlayerInstaller.{#InstallVersion}.txt
SetupIconFile=T:\Tecnica\Produccio\VBoxApps\NRDAVPlayer\Installer\home.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Components]
Name: "Common"; Description: "Common components"; Types: full custom; Flags: fixed
Name: "NRDAVPlayer"; Description: "NRDAVPlayer App"; Types: full

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Redist\MSVC\14.16.27012\vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall; Components: Common
Source: "T:\Tecnica\Produccio\VBoxApps\NRDAVPlayer\Installer\Qt\*"; DestDir: "{app}\..\Common"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: Common
Source: "T:\Tecnica\Produccio\VBoxApps\NRDAVPlayer\Installer\MFormats\*"; DestDir: "{app}\..\Common\MFormats"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: Common
Source: "T:\Tecnica\Produccio\VBoxApps\NRDAVPlayer\Installer\NRDAVPlayer.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: NRDAVPlayer
Source: "T:\Tecnica\Produccio\VBoxApps\NRDAVPlayer\Installer\qt.conf"; DestDir: "{app}"; Flags: ignoreversion; Components: NRDAVPlayer
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Run]
Filename: {tmp}\vc_redist.x64.exe; Parameters: "/q /norestart"; StatusMsg: "Installing VC++ redistributables..."
Filename: {app}\..\Common\MFormats\Reg_x64.bat; Flags: runhidden; StatusMsg: "Installing MFormats..."

[Registry]
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; \
    ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}\..\Common;"; \
    Check: NeedsAddPath(ExpandConstant('{app}\..\Common'))

[Code]
function NeedsAddPath(Param: string): boolean;
var
  OrigPath: string;
begin
  if not RegQueryStringValue(HKEY_LOCAL_MACHINE,
    'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
    'Path', OrigPath)
  then begin
    Result := True;
    exit;
  end;
  { look for the path with leading and trailing semicolon }
  { Pos() returns 0 if not found }
  Result := Pos(';' + Param + ';', ';' + OrigPath + ';') = 0;
end;

[Setup]
; Tell Windows Explorer to reload the environment
ChangesEnvironment=yes

[Icons]
Name: "{autoprograms}\NRDAVPlayer\NRDAVplayer.exe"; Filename: "{app}\NRDAVPlayer.exe"
Name: "{autodesktop}\NRDAVPlayer.exe"; Filename: "{app}\NRDAVPlayer.exe"; Tasks: desktopicon

