#!/bin/bash
. ../include
mkdir -p src
cp ../../variant-release/build/selfdiscovery src
cp ../../LICENSE.txt src
makepkg --repackage --force

