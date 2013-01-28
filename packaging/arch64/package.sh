#!/bin/bash
. ../include
mkdir -p src
cp ../../variant-release/build/selfdiscovery src
cp ../../license.txt src
cp ../../lualicense.txt src
makepkg --repackage --force

