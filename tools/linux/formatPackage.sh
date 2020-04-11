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
cp /home/user/Qt5.14.0/5.14.0/gcc_64/plugins/platforms/libqxcb.so tupitube.desk/bin/platforms/
cp -av /home/user/Qt5.14.0/5.14.0/gcc_64/plugins/imageformats/* tupitube.desk/bin/imageformats/
cp -av /home/user/Qt5.14.0/5.14.0/gcc_64/plugins/mediaservice/* tupitube.desk/bin/mediaservice/
cp -av ffmpeg/* tupitube.desk/lib/ffmpeg/
cp -av /usr/local/quazip/lib/* tupitube.desk/lib/quazip/
cp -av Qt5.14.0/* tupitube.desk/lib/qt/
