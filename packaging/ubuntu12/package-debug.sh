#!/bin/bash
. ../include
mkdir -p selfdiscovery/usr/bin
cp ../../variant-debug/app/build/selfdiscovery selfdiscovery/usr/bin
mkdir -p selfdiscovery/usr/share/doc/selfdiscovery
cp ../../license.txt selfdiscovery/usr/share/doc/selfdiscovery
cp ../../lualicense.txt selfdiscovery/usr/share/doc/selfdiscovery
dpkg --build selfdiscovery .
rm -r selfdiscovery/usr

