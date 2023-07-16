#!/bin/bash

# in WSL Debian requirment
# sudo apt install autoconf
# sudo apt install libtool
# sudo apt install automake 
# sudo apt install git
# sudo apt install cmake
# sudo apt install libssl-dev

cd external

if [[ ! -d "zlib" ]]; then
    git clone https://github.com/leonkasovan/zlib.git
fi
cd zlib
make clean
cmake .
make
cd ..

if [[ ! -d "curl" ]]; then
    git clone https://github.com/leonkasovan/curl.git
fi
cd curl
autoreconf -fi
./configure CFLAGS="-Os -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables -flto" LDFLAGS="-Wl,-s -Wl,-Bsymbolic -Wl,--gc-sections" --with-openssl --without-brotli --without-nghttp2 --without-libidn2 --without-libpsl --without-zstd --disable-alt-svc --disable-hsts --disable-tls-srp --disable-proxy --disable-ipv6 --disable-ntlm-wb --disable-ntlm --disable-dict --disable-gopher --disable-gophers --disable-imap --disable-mqtt --disable-pop3 --disable-pop3s --disable-smtp --disable-smtps --disable-telnet --disable-tftp --disable-rtsp --disable-file --disable-ldap --disable-ldaps
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
make -f Makefile