[Setup]
SetupIconFile=bin\raw\icon\tupitube.ico
AppName=TupiTube Desk
AppVersion=0.2.20
AppPublisher=MaeFloresta
AppPublisherURL=https://www.tupitube.com/
DefaultDirName={commonpf}\TupiTube
DefaultGroupName=TupiTube
OutputBaseFilename=tupitube_installer
OutputDir=c:\devel\installer
ChangesEnvironment=yes
ChangesAssociations=yes
; Uncomment this line for 64bit installer
ArchitecturesInstallIn64BitMode=x64

[Dirs]
Name: "{app}\bin"
Name: "{app}\bin\raw"
Name: "{app}\bin\raw\platforms"
Name: "{app}\bin\raw\imageformats"
Name: "{app}\bin\raw\mediaservice"
Name: "{app}\bin\raw\audio"
Name: "{app}\bin\raw\icon"
Name: "{app}\data"
Name: "{app}\data\html"
Name: "{app}\data\html\css"
Name: "{app}\data\html\examples"
Name: "{app}\data\html\images"
Name: "{app}\data\html\pages"
Name: "{app}\data\dictionaries"
Name: "{app}\data\dictionaries\en"
Name: "{app}\data\dictionaries\es"
Name: "{app}\data\mouths"
Name: "{app}\data\mouths\1"
Name: "{app}\data\mouths\2"
Name: "{app}\data\mouths\3"
Name: "{app}\data\mouths\4"
Name: "{app}\data\mouths\5"
Name: "{app}\data\palettes"
Name: "{app}\data\storyboard"
Name: "{app}\data\themes"
Name: "{app}\data\themes\default"
Name: "{app}\data\themes\default\config"
Name: "{app}\data\themes\default\cursors"
Name: "{app}\data\themes\default\icons"
Name: "{app}\data\themes\default\images"
Name: "{app}\data\themes\raster"
Name: "{app}\data\themes\raster\brushes"
Name: "{app}\data\themes\raster\brushes\classic"
Name: "{app}\data\themes\raster\brushes\deevad"
Name: "{app}\data\themes\raster\brushes\experimental"
Name: "{app}\data\themes\raster\brushes\ramon"
Name: "{app}\data\themes\raster\brushes\tanda"
Name: "{app}\data\themes\raster\resources"
Name: "{app}\data\translations"
Name: "{app}\data\xml"
Name: "{app}\data\xml\en"
Name: "{app}\data\xml\es"
Name: "{app}\data\xml\fr"
Name: "{app}\data\xml\pt"
Name: "{app}\plugins"
Name: "{app}\lib"
Name: "{app}\lib\qt5"
Name: "{app}\lib\ffmpeg"
Name: "{app}\lib\raster"
Name: "{app}\lib\libsndfile"

[Files]
Source: "bin\*"; DestDir: "{app}\bin"
Source: "bin\raw\*"; DestDir: "{app}\bin\raw"
Source: "bin\raw\icon\*"; DestDir: "{app}\bin\raw\icon"
Source: "bin\raw\platforms\*"; DestDir: "{app}\bin\raw\platforms"
Source: "bin\raw\imageformats\*"; DestDir: "{app}\bin\raw\imageformats"
Source: "bin\raw\mediaservice\*"; DestDir: "{app}\bin\raw\mediaservice"
Source: "bin\raw\audio\*"; DestDir: "{app}\bin\raw\audio"
Source: "data\html\css\*"; DestDir: "{app}\data\html\css"
Source: "data\html\examples\*"; DestDir: "{app}\data\html\examples"
Source: "data\html\images\*"; DestDir: "{app}\data\html\images"
Source: "data\html\pages\*"; DestDir: "{app}\data\html\pages"
Source: "data\dictionaries\en\*"; DestDir: "{app}\data\dictionaries\en"
Source: "data\dictionaries\es\*"; DestDir: "{app}\data\dictionaries\es"
Source: "data\mouths\1\*"; DestDir: "{app}\data\mouths\1"
Source: "data\mouths\2\*"; DestDir: "{app}\data\mouths\2"
Source: "data\mouths\3\*"; DestDir: "{app}\data\mouths\3"
Source: "data\mouths\4\*"; DestDir: "{app}\data\mouths\4"
Source: "data\mouths\5\*"; DestDir: "{app}\data\mouths\5"
Source: "data\palettes\*"; DestDir: "{app}\data\palettes"
Source: "data\storyboard\*"; DestDir: "{app}\data\storyboard"
Source: "data\themes\default\config\*"; DestDir: "{app}\data\themes\default\config"
Source: "data\themes\default\cursors\*"; DestDir: "{app}\data\themes\default\cursors"
Source: "data\themes\default\icons\*"; DestDir: "{app}\data\themes\default\icons"
Source: "data\themes\default\images\*"; DestDir: "{app}\data\themes\default\images"
Source: "data\themes\raster\brushes\*"; DestDir: "{app}\data\themes\raster\brushes"
Source: "data\themes\raster\brushes\classic\*"; DestDir: "{app}\data\themes\raster\brushes\classic"
Source: "data\themes\raster\brushes\deevad\*"; DestDir: "{app}\data\themes\raster\brushes\deevad"
Source: "data\themes\raster\brushes\experimental\*"; DestDir: "{app}\data\themes\raster\brushes\experimental"
Source: "data\themes\raster\brushes\ramon\*"; DestDir: "{app}\data\themes\raster\brushes\ramon"
Source: "data\themes\raster\brushes\tanda\*"; DestDir: "{app}\data\themes\raster\brushes\tanda"
Source: "data\themes\raster\resources\*"; DestDir: "{app}\data\themes\raster\resources"
Source: "data\translations\*"; DestDir: "{app}\data\translations"
Source: "data\xml\en\*"; DestDir: "{app}\data\xml\en"
Source: "data\xml\es\*"; DestDir: "{app}\data\xml\es"
Source: "data\xml\fr\*"; DestDir: "{app}\data\xml\fr"
Source: "data\xml\pt\*"; DestDir: "{app}\data\xml\pt"
Source: "plugins\*"; DestDir: "{app}\plugins"
Source: "lib\*"; DestDir: "{app}\lib"
Source: "lib\qt5\*"; DestDir: "{app}\lib\qt5"
Source: "lib\ffmpeg\*"; DestDir: "{app}\lib\ffmpeg"
Source: "lib\libsndfile\*"; DestDir: "{app}\lib\libsndfile"
Source: "lib\raster\*"; DestDir: "{app}\lib\raster"

[Registry]
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: string; ValueName: "TUPITUBE_HOME"; ValueData: "{app}"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: string; ValueName: "TUPITUBE_SHARE"; ValueData: "{app}\data"; Flags: uninsdeletevalue
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: string; ValueName: "TUPITUBE_PLUGIN"; ValueData: "{app}\plugins"; Flags: uninsdeletevalue
Root: HKCR; Subkey: ".tup"; ValueType: string; ValueName: ""; ValueData: "TupiTubeDesk"; Flags: uninsdeletevalue 
Root: HKCR; Subkey: "TupiTubeDesk"; ValueType: string; ValueName: ""; ValueData: "TupiTube Desk"; Flags: uninsdeletekey
Root: HKCR; Subkey: "TupiTubeDesk\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\raw\tupitube.desk.exe,0"; Flags: uninsdeletekey
Root: HKCR; Subkey: "TupiTubeDesk\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\TupiTube.bat"" ""%1"""; Flags: uninsdeletekey

[Icons]
Name: "{group}\TupiTube Desk"; Filename: "{app}\bin\TupiTube.bat"; IconFilename: "{app}\bin\raw\icon\tupitube.ico"
Name: "{commondesktop}\TupiTube Desk"; Filename: "{app}\bin\TupiTube.bat"; IconFilename: "{app}\bin\raw\icon\tupitube.ico"; Comment: "2D Animation Tool"
