; Upkun HMI Inno Setup script.
; 先运行 scripts/package_release.ps1 生成 dist/upkun-hmi，再用 ISCC 编译本脚本。

#define MyAppName "Upkun HMI"
#define MyAppVersion "0.24.0"
#define MyAppPublisher "Upkun Learning"
#define MyAppExeName "upkun-hmi.exe"
#define DistDir "..\dist\upkun-hmi"

[Setup]
AppId={{7D5D4F46-4E60-4C3F-9B80-024024024024}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\Upkun HMI
DefaultGroupName=Upkun HMI
DisableProgramGroupPage=yes
OutputDir=..\dist
OutputBaseFilename=upkun-hmi-setup-{#MyAppVersion}
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Files]
Source: "{#DistDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\Upkun HMI"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\Upkun HMI"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "创建桌面快捷方式"; GroupDescription: "附加图标："

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "启动 Upkun HMI"; Flags: nowait postinstall skipifsilent
