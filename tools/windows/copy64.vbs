REM Developed by Santiago Paz Mendieta

Set fileSystem = CreateObject("Scripting.FileSystemObject")

Dim Folder
Folder="C:\tupitube"

quazipDir = "c:\Quazip\bin"
sourceDir = "c:\devel\sources\tupitube.desk\src"
tupiDir = "c:\tupitube"

If fileSystem.FolderExists(tupiDir) Then
fileSystem.DeleteFolder tupiDir
End If

fileSystem.CreateFolder tupiDir
fileSystem.CreateFolder tupiDir & "\bin"
fileSystem.CreateFolder tupiDir & "\bin\raw"
fileSystem.CreateFolder tupiDir & "\bin\raw\platforms"
fileSystem.CreateFolder tupiDir & "\bin\raw\imageformats"
fileSystem.CreateFolder tupiDir & "\bin\raw\mediaservice"
fileSystem.CreateFolder tupiDir & "\bin\raw\audio"
fileSystem.CreateFolder tupiDir & "\bin\raw\icon"
fileSystem.CreateFolder tupiDir & "\data"
fileSystem.CreateFolder tupiDir & "\data\palettes"
fileSystem.CreateFolder tupiDir & "\data\themes"
fileSystem.CreateFolder tupiDir & "\data\themes\default"
fileSystem.CreateFolder tupiDir & "\data\dictionaries"
fileSystem.CreateFolder tupiDir & "\plugins"
fileSystem.CreateFolder tupiDir & "\lib"
fileSystem.CreateFolder tupiDir & "\lib\qt5"
fileSystem.CreateFolder tupiDir & "\lib\ffmpeg"
fileSystem.CreateFolder tupiDir & "\lib\libsndfile"
fileSystem.CreateFolder tupiDir & "\lib\raster"

fileSystem.CopyFile quazipDir & "\quazip.dll", tupiDir & "\lib\quazip.dll"
fileSystem.CopyFile "c:\devel\sources\tupitube.desk\tools\windows\tupitube.iss", tupiDir & "\tupitube.iss"
fileSystem.CopyFile "c:\devel\sources\tupitube.desk\tools\windows\tupitube.ico", tupiDir & "\bin\raw\icon\tupitube.ico"
fileSystem.CopyFile "c:\devel\sources\tupitube.desk\tools\windows\TupiTube.bat", tupiDir & "\bin\TupiTube.bat"
fileSystem.CopyFile "c:\devel\sources\tupitube.desk\tools\windows\TupiTube.debug.bat", tupiDir & "\bin\TupiTube.debug.bat"
fileSystem.CopyFile sourceDir & "\shell\release\tupitube.desk.exe", tupiDir & "\bin\raw\tupitube.desk.exe"
fileSystem.CopyFile "c:\devel\sources\ssl\libcrypto-1_1-x64.dll", tupiDir & "\bin\raw\libcrypto-1_1-x64.dll"
fileSystem.CopyFile "c:\devel\sources\ssl\libssl-1_1-x64.dll", tupiDir & "\bin\raw\libssl-1_1-x64.dll"
fileSystem.CopyFile sourceDir & "\framework\core\release\tupifwcore.dll", tupiDir & "\lib\tupifwcore.dll"
fileSystem.CopyFile sourceDir & "\framework\gui\release\tupifwgui.dll", tupiDir & "\lib\tupifwgui.dll"
fileSystem.CopyFile sourceDir & "\store\release\tupistore.dll", tupiDir & "\lib\tupistore.dll"
fileSystem.CopyFile sourceDir & "\libbase\release\tupibase.dll", tupiDir & "\lib\tupibase.dll"
fileSystem.CopyFile sourceDir & "\libtupi\release\tupi.dll", tupiDir & "\lib\tupi.dll"
fileSystem.CopyFile sourceDir & "\libui\release\tupigui.dll", tupiDir & "\lib\tupigui.dll"
fileSystem.CopyFile sourceDir & "\libcolor\release\tupicolor.dll", tupiDir & "\lib\tupicolor.dll"
fileSystem.CopyFile sourceDir & "\components\player\release\tupiplayer.dll", tupiDir & "\lib\tupiplayer.dll"
fileSystem.CopyFile sourceDir & "\components\colorpalette\release\tupicolorpalette.dll", tupiDir & "\lib\tupicolorpalette.dll"
fileSystem.CopyFile sourceDir & "\components\export\release\tupiexport.dll", tupiDir & "\lib\tupiexport.dll"
fileSystem.CopyFile sourceDir & "\components\exposure\release\tupiexposure.dll", tupiDir & "\lib\tupiexposure.dll"
fileSystem.CopyFile sourceDir & "\components\import\release\tupimport.dll", tupiDir & "\lib\tupimport.dll"
fileSystem.CopyFile sourceDir & "\components\library\release\tupilibrary.dll", tupiDir & "\lib\tupilibrary.dll"
fileSystem.CopyFile sourceDir & "\components\workspace\release\tupiworkspace.dll", tupiDir & "\lib\tupiworkspace.dll"
fileSystem.CopyFile sourceDir & "\components\brush\release\tupibrush.dll", tupiDir & "\lib\tupibrush.dll"
fileSystem.CopyFile sourceDir & "\components\scenes\release\tupiscenes.dll", tupiDir & "\lib\tupiscenes.dll"
fileSystem.CopyFile sourceDir & "\components\timeline\release\tupitimeline.dll", tupiDir & "\lib\tupitimeline.dll"
fileSystem.CopyFile sourceDir & "\components\news\release\tupinews.dll", tupiDir & "\lib\tupinews.dll"
fileSystem.CopyFile sourceDir & "\components\micmanager\release\tupimicmanager.dll", tupiDir & "\lib\tupimicmanager.dll"
fileSystem.CopyFile sourceDir & "\components\papagayo\release\tupipapagayo.dll", tupiDir & "\lib\tupipapagayo.dll"
fileSystem.CopyFile sourceDir & "\mypaint\json-c\release\json-c.dll", tupiDir & "\lib\raster\json-c.dll"
fileSystem.CopyFile sourceDir & "\mypaint\libmypaint\release\libmypaint.dll", tupiDir & "\lib\raster\libmypaint.dll"
fileSystem.CopyFile sourceDir & "\mypaint\qtmypaint\release\qtmypaint.dll", tupiDir & "\lib\raster\qtmypaint.dll"
fileSystem.CopyFile sourceDir & "\mypaint\raster\brushes\release\rasterbrushes.dll", tupiDir & "\lib\raster\rasterbrushes.dll"
fileSystem.CopyFile sourceDir & "\mypaint\raster\color\release\rastercolor.dll", tupiDir & "\lib\raster\rastercolor.dll"
fileSystem.CopyFile sourceDir & "\mypaint\raster\main\release\rastermain.dll", tupiDir & "\lib\raster\rastermain.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\common\release\tupiplugincommon.dll", tupiDir & "\lib\tupiplugincommon.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\color\release\tupicoloringtool.dll", tupiDir & "\plugins\tupicoloringtool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\fill\release\tupifilltool.dll", tupiDir & "\plugins\tupifilltool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\geometric\release\tupigeometrictool.dll", tupiDir & "\plugins\tupigeometrictool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\ink\release\tupiinktool.dll", tupiDir & "\plugins\tupiinktool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\nodes\release\tupinodestool.dll", tupiDir & "\plugins\tupinodestool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\opacity\release\tupiopacitytool.dll", tupiDir & "\plugins\tupiopacitytool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\papagayo\release\tupipapagayotool.dll", tupiDir & "\plugins\tupipapagayotool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\pencil\release\tupipenciltool.dll", tupiDir & "\plugins\tupipenciltool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\polyline\release\tupipolylinetool.dll", tupiDir & "\plugins\tupipolylinetool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\motion\release\tupimotiontool.dll", tupiDir & "\plugins\tupimotiontool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\rotation\release\tupirotationtool.dll", tupiDir & "\plugins\tupirotationtool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\scale\release\tupiscaletool.dll", tupiDir & "\plugins\tupiscaletool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\selection\release\tupiselectiontool.dll", tupiDir & "\plugins\tupiselectiontool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\shear\release\tupisheartool.dll", tupiDir & "\plugins\tupisheartool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\text\release\tupitexttool.dll", tupiDir & "\plugins\tupitexttool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\eyedropper\release\tupieyedropper.dll", tupiDir & "\plugins\tupieyedropper.dll"
fileSystem.CopyFile sourceDir & "\plugins\export\imageplugin\release\tupiimageplugin.dll", tupiDir & "\plugins\tupiimageplugin.dll"
fileSystem.CopyFile sourceDir & "\plugins\export\ffmpegplugin\release\tupiffmpegplugin.dll", tupiDir & "\plugins\tupiffmpegplugin.dll"
fileSystem.CopyFile "c:\devel\sources\libsndfile\bin\sndfile.dll", tupiDir & "\lib\libsndfile\sndfile.dll"

fileSystem.CopyFile "c:\devel\sources\platforms\qwindows.dll", tupiDir & "\bin\raw\platforms\qwindows.dll"

fileSystem.CreateFolder tupiDir & "\data\themes\raster"
fileSystem.CopyFolder sourceDir & "\shell\data", tupiDir & "\data"
fileSystem.CopyFolder sourceDir & "\shell\html", tupiDir & "\data\html"
fileSystem.CopyFolder sourceDir & "\libcolor\palettes", tupiDir & "\data\palettes"
fileSystem.CopyFolder sourceDir & "\themes\default", tupiDir & "\data\themes\default"
fileSystem.CopyFolder sourceDir & "\mypaint\raster\main\brushes", tupiDir & "\data\themes\raster\brushes"
fileSystem.CopyFolder sourceDir & "\mypaint\raster\main\resources", tupiDir & "\data\themes\raster\resources"
fileSystem.CopyFolder "c:\devel\sources\qt5", tupiDir & "\lib\qt5"
fileSystem.CopyFolder "c:\devel\sources\imageformats", tupiDir & "\bin\raw\imageformats"
fileSystem.CopyFolder "c:\devel\sources\mediaservice", tupiDir & "\bin\raw\mediaservice"
fileSystem.CopyFolder "c:\devel\sources\audio", tupiDir & "\bin\raw\audio"
fileSystem.CopyFolder "c:\devel\sources\ffmpeg", tupiDir & "\lib\ffmpeg"

Dim objFSO
Set objFSO = CreateObject("Scripting.FileSystemObject")
RemoveProFiles objFSO.GetFolder("C:\tupitube")
RemoveMkFiles objFSO.GetFolder("C:\tupitube")

Sub RemoveProFiles(objFolder)
    Dim objFile, objSubFolder

    For Each objFile In objFolder.Files
        If LCase(objFSO.GetExtensionName(objFile.Name)) = "pro" Then
			Set proFile = createobject("Scripting.FileSystemObject")
			proFile.DeleteFile objFile.Path
        End If
    Next

    For Each objSubFolder In objFolder.SubFolders
        RemoveProFiles objSubFolder
    Next
End Sub

Sub RemoveMkFiles(objFolder)
    Dim objFile, objSubFolder, mkFile
	mkFile  = "Makefile"

    For Each objFile In objFolder.Files
        If StrComp(objFile.Name, mkFile) = 0 Then
			Set mk = createobject("Scripting.FileSystemObject")
			mk.DeleteFile objFile.Path
        End If		
    Next

    For Each objSubFolder In objFolder.SubFolders
        RemoveMkFiles objSubFolder
    Next
End Sub

MsgBox "Copy is done! :D",0,"TupiTube Desk"
