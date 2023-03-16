#!/bin/bash

QT_HOME=/home/xtingray/Qt
QT_VERSION=5.15.2

cp --preserve=links -rv /usr/local/tupitube.desk .
rm -f tupitube.desk/bin/tupitube.desk
cp -v launcher tupitube.desk/tupitube.desk
cp -v setup tupitube.desk/
mkdir -p tupitube.desk/bin/platforms
mkdir -p tupitube.desk/bin/imageformats
mkdir -p tupitube.desk/bin/mediaservice
mkdir -p tupitube.desk/lib/ffmpeg
mkdir -p tupitube.desk/lib/qt
mkdir -p tupitube.desk/lib/quazip
mkdir -p tupitube.desk/lib/libsndfile
mkdir -p tupitube.desk/lib/ssl
cp $QT_HOME/$QT_VERSION/gcc_64/plugins/platforms/libqxcb.so tupitube.desk/bin/platforms/
cp -av $QT_HOME/$QT_VERSION/gcc_64/plugins/imageformats/* tupitube.desk/bin/imageformats/
cp -av $QT_HOME/$QT_VERSION/gcc_64/plugins/mediaservice/* tupitube.desk/bin/mediaservice/
cp -av ffmpeg/* tupitube.desk/lib/ffmpeg/
cp -av libsndfile/* tupitube.desk/lib/libsndfile/
cp -av /usr/local/quazip/lib/* tupitube.desk/lib/quazip/
cp -av $QT_VERSION/* tupitube.desk/lib/qt/
cp -av $QT_HOME/Tools/OpenSSL/binary/lib/libssl*so tupitube.desk/lib/ssl/
cp -av $QT_HOME/Tools/OpenSSL/binary/lib/libssl*1 tupitube.desk/lib/ssl/
cp -av $QT_HOME/Tools/OpenSSL/binary/lib/libcrypto*so tupitube.desk/lib/ssl/
cp -av $QT_HOME/Tools/OpenSSL/binary/lib/libcrypto*1 tupitube.desk/lib/ssl/
find tupitube.desk -iname *.debug -exec rm {} \;
rm -rf tupitube.desk/lib/quazip/cmake
rm -rf tupitube.desk/lib/quazip/pkgconfig
