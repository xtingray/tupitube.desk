REM Developed by Santiago Paz Mendieta

Set fileSystem = CreateObject("Scripting.FileSystemObject")

Dim Folder
Folder="C:\tupitube"

quazipDir = "c:\Quazip\bin"
sourceDir = "c:\devel\sources\tupitube.desk\src"
tupiDir = "c:\tupitube"
tupiPath = "c:\tupitube\"
pluginsDir = "c:\tupitube\plugins\"

If fileSystem.FolderExists(tupiPath) Then
fileSystem.DeleteFolder tupiDir
End If

fileSystem.CreateFolder tupiDir
fileSystem.CreateFolder tupiPath & "platforms"
fileSystem.CreateFolder tupiPath & "imageformats"
fileSystem.CreateFolder tupiPath & "mediaservice"
fileSystem.CreateFolder tupiPath & "audio"
fileSystem.CreateFolder tupiPath & "icon"
fileSystem.CreateFolder tupiPath & "plugins"
fileSystem.CreateFolder tupiPath & "data"
fileSystem.CreateFolder tupiPath & "data\palettes"
fileSystem.CreateFolder tupiPath & "data\themes"
fileSystem.CreateFolder tupiPath & "data\themes\default"
fileSystem.CreateFolder tupiPath & "data\dictionaries"

fileSystem.CopyFile "c:\devel\sources\tupitube.desk\tools\windows\tupitube.iss", tupiPath & "tupitube.iss"
fileSystem.CopyFile "c:\devel\sources\tupitube.desk\tools\windows\tupitube.ico", tupiPath & "icon\tupitube.ico"
fileSystem.CopyFile quazipDir & "\libquazip1-qt5.dll", tupiPath & "libquazip1-qt5.dll"
fileSystem.CopyFile sourceDir & "\shell\release\tupitube.desk.exe", tupiPath & "tupitube.desk.exe"
fileSystem.CopyFile "c:\devel\sources\ssl\libcrypto-1_1-x64.dll", tupiPath & "libcrypto-1_1-x64.dll"
fileSystem.CopyFile "c:\devel\sources\ssl\libssl-1_1-x64.dll", tupiPath & "libssl-1_1-x64.dll"
fileSystem.CopyFile sourceDir & "\framework\core\release\tupifwcore.dll", tupiPath & "tupifwcore.dll"
fileSystem.CopyFile sourceDir & "\framework\gui\release\tupifwgui.dll", tupiPath & "tupifwgui.dll"
fileSystem.CopyFile sourceDir & "\store\release\tupistore.dll", tupiPath & "tupistore.dll"
fileSystem.CopyFile sourceDir & "\libbase\release\tupibase.dll", tupiPath & "tupibase.dll"
fileSystem.CopyFile sourceDir & "\libtupi\release\tupi.dll", tupiPath & "tupi.dll"
fileSystem.CopyFile sourceDir & "\libui\release\tupigui.dll", tupiPath & "tupigui.dll"
fileSystem.CopyFile sourceDir & "\libcolor\release\tupicolor.dll", tupiPath & "tupicolor.dll"
fileSystem.CopyFile sourceDir & "\components\player\release\tupiplayer.dll", tupiPath & "tupiplayer.dll"
fileSystem.CopyFile sourceDir & "\components\colorpalette\release\tupicolorpalette.dll", tupiPath & "tupicolorpalette.dll"
fileSystem.CopyFile sourceDir & "\components\export\release\tupiexport.dll", tupiPath & "tupiexport.dll"
fileSystem.CopyFile sourceDir & "\components\exposure\release\tupiexposure.dll", tupiPath & "tupiexposure.dll"
fileSystem.CopyFile sourceDir & "\components\import\release\tupimport.dll", tupiPath & "tupimport.dll"
fileSystem.CopyFile sourceDir & "\components\library\release\tupilibrary.dll", tupiPath & "tupilibrary.dll"
fileSystem.CopyFile sourceDir & "\components\workspace\release\tupiworkspace.dll", tupiPath & "tupiworkspace.dll"
fileSystem.CopyFile sourceDir & "\components\brush\release\tupibrush.dll", tupiPath & "tupibrush.dll"
REM fileSystem.CopyFile sourceDir & "\components\scenes\release\tupiscenes.dll", tupiPath & "\lib\tupiscenes.dll"
fileSystem.CopyFile sourceDir & "\components\timeline\release\tupitimeline.dll", tupiPath & "tupitimeline.dll"
fileSystem.CopyFile sourceDir & "\components\news\release\tupinews.dll", tupiPath & "tupinews.dll"
fileSystem.CopyFile sourceDir & "\components\micmanager\release\tupimicmanager.dll", tupiPath & "tupimicmanager.dll"
fileSystem.CopyFile sourceDir & "\components\papagayo\release\tupipapagayo.dll", tupiPath & "tupipapagayo.dll"
fileSystem.CopyFile sourceDir & "\mypaint\json-c\release\json-c.dll", tupiPath & "json-c.dll"
fileSystem.CopyFile sourceDir & "\mypaint\libmypaint\release\libmypaint.dll", tupiPath & "libmypaint.dll"
fileSystem.CopyFile sourceDir & "\mypaint\qtmypaint\release\qtmypaint.dll", tupiPath & "qtmypaint.dll"
fileSystem.CopyFile sourceDir & "\mypaint\raster\brushes\release\rasterbrushes.dll", tupiPath & "rasterbrushes.dll"
fileSystem.CopyFile sourceDir & "\mypaint\raster\color\release\rastercolor.dll", tupiPath & "rastercolor.dll"
fileSystem.CopyFile sourceDir & "\mypaint\raster\size\release\rastersize.dll", tupiPath & "rastersize.dll"
fileSystem.CopyFile sourceDir & "\mypaint\raster\main\release\rastermain.dll", tupiPath & "rastermain.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\common\release\tupiplugincommon.dll", tupiPath & "tupiplugincommon.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\color\release\tupicoloringtool.dll", pluginsDir & "tupicoloringtool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\bucket\release\tupibuckettool.dll", pluginsDir & "tupibuckettool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\geometric\release\tupigeometrictool.dll", pluginsDir & "tupigeometrictool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\ink\release\tupiinktool.dll", pluginsDir & "tupiinktool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\nodes\release\tupinodestool.dll", pluginsDir & "tupinodestool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\opacity\release\tupiopacitytool.dll", pluginsDir & "tupiopacitytool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\papagayo\release\tupipapagayotool.dll", pluginsDir & "tupipapagayotool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\pencil\release\tupipenciltool.dll", pluginsDir & "tupipenciltool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\polyline\release\tupipolylinetool.dll", pluginsDir & "tupipolylinetool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\motion\release\tupimotiontool.dll", pluginsDir & "tupimotiontool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\rotation\release\tupirotationtool.dll", pluginsDir & "tupirotationtool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\scale\release\tupiscaletool.dll", pluginsDir & "tupiscaletool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\selection\release\tupiselectiontool.dll", pluginsDir & "tupiselectiontool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\shear\release\tupisheartool.dll", pluginsDir & "tupisheartool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\text\release\tupitexttool.dll", pluginsDir & "tupitexttool.dll"
fileSystem.CopyFile sourceDir & "\plugins\tools\eyedropper\release\tupieyedropper.dll", pluginsDir & "tupieyedropper.dll"
fileSystem.CopyFile sourceDir & "\plugins\export\imageplugin\release\tupiimageplugin.dll", pluginsDir & "tupiimageplugin.dll"
fileSystem.CopyFile sourceDir & "\plugins\export\ffmpegplugin\release\tupiffmpegplugin.dll", pluginsDir & "tupiffmpegplugin.dll"
fileSystem.CopyFile "c:\devel\sources\libsndfile\bin\sndfile.dll", tupiPath & "sndfile.dll"

fileSystem.CopyFile "c:\devel\sources\platforms\qwindows.dll", tupiPath & "platforms\qwindows.dll"

fileSystem.CopyFile "c:\devel\sources\qt5\libgcc_s_seh-1.dll", tupiPath & "libgcc_s_seh-1.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\libGLESv2.dll", tupiPath & "libGLESv2.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\libstdc++-6.dll", tupiPath & "libstdc++-6.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\libwinpthread-1.dll", tupiPath & "libwinpthread-1.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\Qt5Core.dll", tupiPath & "Qt5Core.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\Qt5Gui.dll", tupiPath & "Qt5Gui.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\Qt5Multimedia.dll", tupiPath & "Qt5Multimedia.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\Qt5MultimediaWidgets.dll", tupiPath & "Qt5MultimediaWidgets.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\Qt5Network.dll", tupiPath & "Qt5Network.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\Qt5OpenGL.dll", tupiPath & "Qt5OpenGL.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\Qt5PrintSupport.dll", tupiPath & "Qt5PrintSupport.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\Qt5Svg.dll", tupiPath & "Qt5Svg.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\Qt5Widgets.dll", tupiPath & "Qt5Widgets.dll"
fileSystem.CopyFile "c:\devel\sources\qt5\Qt5Xml.dll", tupiPath & "Qt5Xml.dll"

fileSystem.CopyFile "c:\devel\sources\imageformats\qgif.dll", tupiPath & "imageformats\qgif.dll"
fileSystem.CopyFile "c:\devel\sources\imageformats\qjpeg.dll", tupiPath & "imageformats\qjpeg.dll"
fileSystem.CopyFile "c:\devel\sources\imageformats\qsvg.dll", tupiPath & "imageformats\qsvg.dll"
fileSystem.CopyFile "c:\devel\sources\imageformats\qwebp.dll", tupiPath & "imageformats\qwebp.dll"

fileSystem.CopyFile "c:\devel\sources\mediaservice\dsengine.dll", tupiPath & "mediaservice\dsengine.dll"
fileSystem.CopyFile "c:\devel\sources\mediaservice\qtmedia_audioengine.dll", tupiPath & "mediaservice\qtmedia_audioengine.dll"

fileSystem.CopyFile "c:\devel\sources\audio\qtaudio_windows.dll", tupiPath & "audio\qtaudio_windows.dll"

fileSystem.CopyFile "c:\devel\sources\ffmpeg\avcodec-58.dll", tupiPath & "avcodec-58.dll"
fileSystem.CopyFile "c:\devel\sources\ffmpeg\avdevice-58.dll", tupiPath & "avdevice-58.dll"
fileSystem.CopyFile "c:\devel\sources\ffmpeg\avfilter-7.dll", tupiPath & "avfilter-7.dll"
fileSystem.CopyFile "c:\devel\sources\ffmpeg\avformat-58.dll", tupiPath & "avformat-58.dll"
fileSystem.CopyFile "c:\devel\sources\ffmpeg\avutil-56.dll", tupiPath & "avutil-56.dll"
fileSystem.CopyFile "c:\devel\sources\ffmpeg\png.dll", tupiPath & "png.dll"
fileSystem.CopyFile "c:\devel\sources\ffmpeg\postproc-55.dll", tupiPath & "postproc-55.dll"
fileSystem.CopyFile "c:\devel\sources\ffmpeg\swresample-3.dll", tupiPath & "swresample-3.dll"
fileSystem.CopyFile "c:\devel\sources\ffmpeg\swscale-5.dll", tupiPath & "swscale-5.dll"
fileSystem.CopyFile "c:\devel\sources\ffmpeg\zlib1.dll", tupiPath & "zlib1.dll"

fileSystem.CreateFolder tupiPath & "data\themes\raster"
fileSystem.CopyFolder sourceDir & "\shell\data", tupiPath & "data"
fileSystem.CopyFolder sourceDir & "\shell\html", tupiPath & "data\html"
fileSystem.CopyFolder sourceDir & "\libcolor\palettes", tupiPath & "data\palettes"
fileSystem.CopyFolder sourceDir & "\themes\default", tupiPath & "data\themes\default"
fileSystem.CopyFolder sourceDir & "\mypaint\raster\main\brushes", tupiPath & "data\themes\raster\brushes"
fileSystem.CopyFolder sourceDir & "\mypaint\raster\main\resources", tupiPath & "data\themes\raster\resources"

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
