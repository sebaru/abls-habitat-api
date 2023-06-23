#! /bin/sh

git pull
echo "We're on branch "$(git branch --show-current)
echo "Compiling "$(git describe --tags | tr -d '\n')

autoheader
libtoolize
aclocal --force
automake --add-missing
autoconf --force
./configure
make -j `grep -c ^processor /proc/cpuinfo`
