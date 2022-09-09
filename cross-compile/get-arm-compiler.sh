#!/bin/bash

URL_BASE="https://toolchains.bootlin.com/downloads/releases/toolchains"

ARCH="aarch64"
#ARCH="armv7-eabihf"

LIBC="glibc"
#LIBC="uclibc"
#LIBC="musl"

FILE=$ARCH"--"$LIBC"--stable-2021.11-1.tar.bz2"

URL=$URL_BASE"/"$ARCH"/tarballs/"$FILE

#https://toolchains.bootlin.com/downloads/releases/toolchains/armv7-eabihf/tarballs/armv7-eabihf--uclibc--stable-2021.11-1.tar.bz2
echo $URL

wget $URL
#tar xvf $FILE
