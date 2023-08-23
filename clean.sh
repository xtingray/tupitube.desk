find . -iname .moc -exec rm -rf {} \;
find . -iname .obj -exec rm -rf {} \;
find . -iname release -exec rm -rf {} \;
find . -iname debug -exec rm -rf {} \;
find . -iname .qmake.stash -exec rm -rf {} \; 
rm log.txt
rm tupitube.desk.pro.user.*
rm tupitube.desk.pro.user
rm -rf .git
rm -rf .github
