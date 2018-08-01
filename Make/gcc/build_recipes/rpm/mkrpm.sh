# Package all binaries for Linux.
export CLI_VER=$1
export CWD=`pwd`
echo $CWD
dos2unix *.spec
mkdir -p OpenSeaChest-1
cp ../../openseachest_exes/* OpenSeaChest-1
tar czf OpenSeaChest-1.0.tar.gz OpenSeaChest-1
cp OpenSeaChest.spec ~/rpmbuild/SPECS/
cp -drf OpenSeaChest-1* ~/rpmbuild/SOURCES/
#ls ~/rpmbuild/SOURCES/OpenSeaChest-1*
cd ~/rpmbuild/
rpmbuild -ba --define "$CLI_VER" SPECS/OpenSeaChest.spec
#rpmbuild -ba --define "$CLI_VER" --target noarch OpenSeaChest.spec
cp -drf ~/rpmbuild/RPMS/noarch/OpenSeaChest*.rpm $CWD
rm -drf ~/rpmbuild/SOURCES/OpenSeaChest-1*
#cp -drf ~/rpmbuild/RPMS/noarch/OpenSeaChest-1-0.noarch.rpm $CWD
cd $CWD
  #cd ..
 # cp $CLI_DIR/rpm/RPMS/noarch/*OpenSeaChest*.rpm .
 # rm $CLI_DIR/rpm/RPMS/noarch/*

