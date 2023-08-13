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

# Usage:
# ./tools/mac/build_mac_osx_app.sh /Users/username/tupitube/sources/tupitube.desk /Users/username/tupitube/installer

QT_PATH=/Users/daniel/Qt
export PATH=$QT_PATH/5.15.2/clang_64/bin:$PATH
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$QT_PATH/5.15.2/clang_64/lib
export DYLD_FRAMEWORK_PATH=$QT_PATH/5.15.2/clang_64/lib

TUPITUBE_GIT_REPOSITORY=$1
INSTALLATION_PATH=$2
INSTALLER_SCRIPT=$TUPITUBE_GIT_REPOSITORY/tools/mac/update_dylib_path.rb
FRAMEWORKS_PATH=$INSTALLATION_PATH/TupiTube.app/Contents/Frameworks
TUPITUBE_VERSION=0.2.21

# libtupinet.dylib
declare -a LIBS=('libtupifwcore.dylib' 'libtupifwgui.dylib' 'libtupigui.dylib' 'libtupistore.dylib' 'libtupi.dylib' \
'libtupibase.dylib' 'libtupicolorpalette.1.dylib' 'libtupimicmanager.1.dylib' 'libtupipapagayo.1.dylib' 'libtupiworkspace.1.dylib' 'libtupicolor.1.dylib' \
'libtupiplayer.1.dylib' 'libtupibrush.1.dylib' 'libtupimport.1.dylib' 'libtupiexport.1.dylib' 'libtupiexposure.1.dylib' \
'libjson-c.1.dylib' 'liblibmypaint.1.dylib' 'libqtmypaint.1.dylib' \
'librasterbrushes.1.dylib' 'librastercolor.1.dylib' 'librastermain.1.dylib' \
'libtupitimeline.1.dylib' 'libtupilibrary.1.dylib' 'libtupinews.1.dylib' 'libtupiplugincommon.1.dylib');

cd $TUPITUBE_GIT_REPOSITORY

echo "* Starting script..."
echo "cd $TUPITUBE_GIT_REPOSITORY"
echo "Running -> make install"
make install

echo ""
echo "---"
echo ""
mkdir -p $INSTALLATION_PATH/TupiTube.app/Contents/Resources
mkdir -p $INSTALLATION_PATH/TupiTube.app/Contents/MacOS

cp -v launcher/icons/tupitube.desk.png $INSTALLATION_PATH/TupiTube.app/Contents/Resources/tup.png
cp -v bin/TupiTube.app/Contents/Resources/tupitube.desk.icns $INSTALLATION_PATH/TupiTube.app/Contents/Resources/tupitube.desk.icns
cp -v bin/TupiTube.app/Contents/MacOS/TupiTube $INSTALLATION_PATH/TupiTube.app/Contents/MacOS/

cd $INSTALLATION_PATH
cp -rv lib/tupitube/plugins TupiTube.app/Contents/MacOS

mkdir -pv TupiTube.app/Contents/MacOS/share
cp -rv share/pixmaps TupiTube.app/Contents/MacOS/share/
cp -rv share/tupitube/data TupiTube.app/Contents/MacOS/share/
cp -rv share/tupitube/themes TupiTube.app/Contents/MacOS/share/

cd TupiTube.app/Contents/MacOS/plugins

find . -name "*.dylib" -exec $INSTALLER_SCRIPT $INSTALLATION_PATH/TupiTube.app/Contents/MacOS/plugins/{} \/usr\/local\/lib\/ @executable_path/../Frameworks/ \;

for lib in ${LIBS[@]}; do
    echo "";
    echo "[Phase I] Updating paths for $lib";
    find . -name "*.dylib" -exec $INSTALLER_SCRIPT $INSTALLATION_PATH/TupiTube.app/Contents/MacOS/plugins/{} $lib @executable_path/../Frameworks/$lib \;
done

cd $INSTALLATION_PATH/TupiTube.app/Contents/MacOS

for lib in ${LIBS[@]}; do
    echo "";
    echo "[Phase II] Updating paths for (Frameworks I) $lib";
    $INSTALLER_SCRIPT $INSTALLATION_PATH/TupiTube.app/Contents/MacOS/TupiTube $lib @executable_path/../Frameworks/$lib  \;
done

# cp -rv $QT_PATH/MaintenanceTool.app/Contents/Resources/qt_menu.nib $INSTALLATION_PATH/TupiTube.app/Contents/Resources

cd $INSTALLATION_PATH
mkdir -pv $FRAMEWORKS_PATH

echo ""
echo "* Copying TupiTube libraries into Frameworks folder"
for lib in ${LIBS[@]}; do
    cp -v $INSTALLATION_PATH/lib/tupitube/$lib $FRAMEWORKS_PATH
done

for lib in ${LIBS[@]}; do
    echo "";
    echo "[Phase III] Updating paths for (Frameworks II) $lib";    
    $INSTALLER_SCRIPT $INSTALLATION_PATH/TupiTube.app/Contents/Frameworks/$lib  \/usr\/local\/lib\/ @executable_path/../Frameworks/

    for sublib in ${LIBS[@]}; do
        echo " * Updating paths for (sublib) $sublib";
        $INSTALLER_SCRIPT $INSTALLATION_PATH/TupiTube.app/Contents/Frameworks/$lib $sublib @executable_path/../Frameworks/$sublib \;
    done
done

echo ""
echo "* Copying additional lib dependencies..."
# Libsndfile lib
cp -av /usr/local/libsndfile/lib/libsndfile.*.dylib $FRAMEWORKS_PATH
# Quazip lib
# cp -av /usr/local/quazip/lib/libquazip1-qt5.1.3.dylib $FRAMEWORKS_PATH
cp -av /usr/local/quazip/lib/libquazip1-qt5.*.dylib $FRAMEWORKS_PATH

echo "* Creating DMG package..."
macdeployqt TupiTube.app -dmg -libpath=/usr/local/lib
mv TupiTube.dmg TupiTube_Desk_$TUPITUBE_VERSION.dmg
ls -al TupiTube_Desk_$TUPITUBE_VERSION.dmg
