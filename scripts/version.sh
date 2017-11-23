#! /bin/bash
set -e
# set -v

cat ../src/version.h

echo "Enter new major version number (increment when backwards compatibility is broken)"
read major
echo "Enter new mid version number (increment when new features are added)"
read mid
echo "Enter new minor version number (increment for any other changes)"
read minor

echo "#define VERSION \"${major}.${mid}.${minor}\"" > ../src/version.h
