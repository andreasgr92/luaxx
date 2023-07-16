#!/bin/bash

# in WSL Debian requirment
# sudo apt install autoconf
# sudo apt install libtool
# sudo apt install automake 
# sudo apt install git
# sudo apt install cmake
# sudo apt install libssl-dev:arm64

cd external

if [[ ! -d "zlib" ]]; then
    git clone https://github.com/leonkasovan/zlib.git
fi
cd zlib
export CC=aarch64-linux-gnu-gcc
export AR=aarch64-linux-gnu-ar
export RANLIB=aarch64-linux-gnu-ranlib
make -f Makefile.in distclean
dos2unix configure
./configure
make
cd ..

if [[ ! -d "curl" ]]; then
    git clone https://github.com/leonkasovan/curl.git
fi
cd curl
make clean
autoreconf -fi
./configure --host=aarch64-linux-gnu CFLAGS="-Os -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables -flto" LDFLAGS="-Wl,-s -Wl,-Bsymbolic -Wl,--gc-sections" --with-openssl --without-brotli --without-nghttp2 --without-libidn2 --without-libpsl --without-zstd --disable-alt-svc --disable-hsts --disable-tls-srp --disable-proxy --disable-ipv6 --disable-ntlm-wb --disable-ntlm --disable-dict --disable-gopher --disable-gophers --disable-imap --disable-mqtt --disable-pop3 --disable-pop3s --disable-smtp --disable-smtps --disable-telnet --disable-tftp --disable-rtsp --disable-file --disable-ldap --disable-ldaps
make
cd ..

if [[ ! -d "zziplib" ]]; then
    git clone https://github.com/leonkasovan/zziplib.git
fi
cd zziplib
rm CMakeCache.txt
make clean
cmake .
make -f Makefile

cd ../..    # back to /f/Projects/luaxx/
pwd
make -f Makefile clean
CC=aarch64-linux-gnu-gcc make -f Makefile
