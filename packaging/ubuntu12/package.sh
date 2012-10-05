#!/bin/bash
. ../include
mkdir -p selfdiscovery/usr/bin
cp ../../variant-release/build/selfdiscovery selfdiscovery/usr/bin
mkdir -p selfdiscovery/usr/share/doc/selfdiscovery
cp ../../LICENSE.txt selfdiscovery/usr/share/doc/selfdiscovery
dpkg --build selfdiscovery .

