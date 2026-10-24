; The Windows installer of a staged release (packaging/stage.py windows), compiled by packaging/windows/make_installer.ps1
; with Inno Setup 6:
;   ISCC /DSudokuVersion=<MAJOR.MINOR.BUILD> /DStageDir=<dist> /DOutputDir=<out> packaging\windows\sudoku.iss
; The program, the puzzle bank it offers new puzzles from (puzzles\ beside it), its end-user licence (shown before
; anything is installed) and the third-party notices, a Start-menu entry and an uninstaller. A per-user install by default (no administrator asked, under the user's Programs folder:
; Reckless Technology\Sudoku there); the dialog offers a machine-wide one, under Program Files\Reckless Technology\Sudoku,
; beside the organisation's other products. The uninstaller takes away what the installer put there and nothing else:
; the person's settings in their own folder (reckless-technology\sudoku under %APPDATA%) are theirs, and the other
; products' folders under Reckless Technology, and the Start menu's Reckless Technology group they share, keep
; everything of theirs.
;
; The installer's and the uninstaller's icon is the program's own, assets\icon\icon.ico; the uninstaller's entry in
; Settings shows the installed executable's.
;
; /DTestRoot=<dir> makes a test build (packaging/windows/test_installer.ps1): every part under that folder, an id and a
; name of its own so it never meets a real install, and no previous folder reused.

#ifndef SudokuVersion
  #error Pass the version: /DSudokuVersion=MAJOR.MINOR.BUILD (packaging\windows\make_installer.ps1 does)
#endif
#ifndef StageDir
  #error Pass the staged release: /DStageDir=<dist>
#endif
#ifndef OutputDir
  #define OutputDir "."
#endif
#define IconFile AddBackslash(SourcePath) + "..\..\assets\icon\icon.ico"
#define AppExe "RecklessSudoku.exe"
#ifdef TestRoot
  #define AppName "Reckless Sudoku (installer test)"
#else
  #define AppName "Reckless Sudoku"
#endif

[Setup]
#ifdef TestRoot
AppId={{F6982E01-3859-41C3-A4E4-BA30730F2DFE}
DefaultDirName={#TestRoot}\Sudoku
UsePreviousAppDir=no
OutputBaseFilename=reckless-sudoku-{#SudokuVersion}-win64-setup-test
#else
; One id for every version, so an install replaces the last and one uninstaller serves.
AppId={{CD71D7D9-F2F6-4DDD-B498-73ED6FEB1F70}
DefaultDirName={autopf}\Reckless Technology\Sudoku
OutputBaseFilename=reckless-sudoku-{#SudokuVersion}-win64-setup
#endif
AppName={#AppName}
AppVersion={#SudokuVersion}
AppVerName={#AppName} {#SudokuVersion}
AppPublisher=Reckless Technology
AppPublisherURL=https://recklesstechnology.com
AppCopyright=Copyright (C) 2008-2026 Jonny Reckless
DefaultGroupName=Reckless Technology
DisableProgramGroupPage=yes
LicenseFile={#StageDir}\LICENSE.txt
OutputDir={#OutputDir}
Compression=lzma2/max
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
WizardStyle=modern
UninstallDisplayName={#AppName}
UninstallDisplayIcon={app}\{#AppExe}
SetupIconFile={#IconFile}
VersionInfoVersion={#SudokuVersion}
VersionInfoCompany=Reckless Technology
VersionInfoDescription={#AppName} installer
; Signed, where the script is told how (make_installer.ps1 -SignWithESigner): the installer and the uninstaller it
; writes, each through the SignTool of that name, so neither is flagged by SmartScreen as unsigned.
#ifdef SignToolName
SignTool={#SignToolName}
SignedUninstaller=yes
#endif

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#StageDir}\{#AppExe}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#StageDir}\puzzles\*.txt"; DestDir: "{app}\puzzles"; Flags: ignoreversion
Source: "{#StageDir}\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#StageDir}\THIRD-PARTY-NOTICES.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExe}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExe}"; Description: "{cm:LaunchProgram,{#StringChange(AppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
