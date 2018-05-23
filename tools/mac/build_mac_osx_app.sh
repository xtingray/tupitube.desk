#!/bin/bash

###########################################################################
#   Project TUPITUBE DESK                                                 #
#   Project Contact: info@maefloresta.com                                 #
#   Project Website: http://www.maefloresta.com                           #
#   Project Leader: Gustav Gonzalez <info@maefloresta.com>                #
#                                                                         #
#   Developers:                                                           #
#   2010:                                                                 #
#    Gustavo Gonzalez / xtingray                                          #
#                                                                         #
#   KTooN's versions:                                                     #
#                                                                         #
#   2006:                                                                 #
#    David Cuadrado                                                       #
#    Jorge Cuadrado                                                       #
#   2003:                                                                 #
#    Fernado Roldan                                                       #
#    Simena Dinas                                                         #
#                                                                         #
#   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       #
#   License:                                                              #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program.  If not, see <http://www.gnu.org/licenses/>. #
###########################################################################

#Usage:
# ./tools/build_mac_osx_app.sh /Users/username/tupitube/sources/tupitube.desk /Users/username/tupitube/installer

QT_PATH=/Users/xtingray/Qt5.10.1
export PATH=$QT_PATH/5.10.1/clang_64/bin:$PATH
export DYLD_LIBRARY_PATH=/usr/local/lib:$DYLD_LIBRARY_PATH:$QT_PATH/5.10.1/clang_64/lib
export DYLD_FRAMEWORK_PATH=$QT_PATH/5.10.1/clang_64/lib

TUPITUBE_GIT_REPOSITORY=$1
INSTALLATION_PATH=$2
INSTALLER_SCRIPT=$TUPITUBE_GIT_REPOSITORY/tools/mac/update_dylib_path.rb
TUPI_VERSION=0.2.11

declare -a LIBS=('libtupifwcore.dylib' 'libtupifwgui.dylib' 'libtupigui.dylib' 'libtupistore.dylib' 'libtupi.dylib' \
'libtupibase.dylib' 'libtupinet.dylib' 'libtupicolorpalette.1.dylib' 'libtupiworkspace.1.dylib' \
'libtupiplayer.1.dylib' 'libtupibrush.1.dylib' 'libtupihelp.1.dylib' 'libtupimport.1.dylib' 'libtupiexport.1.dylib' 'libtupiexposure.1.dylib' \
'libtupitimeline.1.dylib' 'libtupilibrary.1.dylib' 'libtupiscenes.1.dylib' 'libtupitwitter.1.dylib' 'libtupiplugincommon.1.dylib');

cd $TUPITUBE_GIT_REPOSITORY

make install

cp launcher/icons/tupitube.desk.png $INSTALLATION_PATH/TupiTube.app/Contents/Resources/tup.png

cd $INSTALLATION_PATH
cp -r lib/tupitube/plugins TupiTube.app/Contents/MacOS

mkdir TupiTube.app/Contents/MacOS/share
cp -r share/pixmaps TupiTube.app/Contents/MacOS/share/
cp -r share/tupitube/data TupiTube.app/Contents/MacOS/share/
cp -r share/tupitube/themes TupiTube.app/Contents/MacOS/share/

cd TupiTube.app/Contents/MacOS/plugins

find . -name "*.dylib" -exec $INSTALLER_SCRIPT $INSTALLATION_PATH/TupiTube.app/Contents/MacOS/plugins/{} \/usr\/local\/lib\/ @executable_path/../Frameworks/ \;

for lib in ${LIBS[@]}; do
    find . -name "*.dylib" -exec $INSTALLER_SCRIPT $INSTALLATION_PATH/TupiTube.app/Contents/MacOS/plugins/{} $lib @executable_path/../Frameworks/$lib \;
done

cd $INSTALLATION_PATH/TupiTube.app/Contents/MacOS

for lib in ${LIBS[@]}; do
    $INSTALLER_SCRIPT $INSTALLATION_PATH/TupiTube.app/Contents/MacOS/TupiTube $lib @executable_path/../Frameworks/$lib  \;
done

cp -r $QT_PATH/MaintenanceTool.app/Contents/Resources/qt_menu.nib $INSTALLATION_PATH/TupiTube.app/Contents/Resources

cd $INSTALLATION_PATH

mkdir TupiTube.app/Contents/Frameworks/

for lib in ${LIBS[@]}; do
    cp lib/tupitube/$lib TupiTube.app/Contents/Frameworks/
done
for lib in ${LIBS[@]}; do
    $INSTALLER_SCRIPT $INSTALLATION_PATH/TupiTube.app/Contents/Frameworks/$lib  \/usr\/local\/lib\/ @executable_path/../Frameworks/
    for sublib in ${LIBS[@]}; do
        $INSTALLER_SCRIPT $INSTALLATION_PATH/TupiTube.app/Contents/Frameworks/$lib $sublib @executable_path/../Frameworks/$sublib
    done
done

macdeployqt TupiTube.app -dmg -libpath=/usr/local/lib
mv TupiTube.dmg TupiTube_Desk_$TUPI_VERSION.dmg
