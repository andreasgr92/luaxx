#!/bin/bash

# in MSYS2 requirment
# pacman -S autotools
# pacman -S git
# pacman -S cmake

cd external

if [[ ! -d "zlib" ]]; then
    git clone https://github.com/leonkasovan/zlib.git
fi
cd zlib
cmake .
ninja
pwd
cd ..

if [[ ! -d "curl" ]]; then
    git clone https://github.com/leonkasovan/curl.git
fi
cd curl
./buildconf 
./configure --with-schannel --without-brotli --without-nghttp2 --without-libidn2 --without-libpsl --without-zstd --disable-alt-svc --disable-hsts --disable-tls-srp --disable-proxy --disable-ipv6 --disable-ntlm-wb --disable-ntlm --disable-dict --disable-gopher --disable-gophers --disable-imap --disable-mqtt --disable-pop3 --disable-pop3s --disable-smtp --disable-smtps --disable-telnet --disable-tftp --disable-rtsp --disable-file --disable-ldap --disable-ldaps --disable-pthreads
make
cd ..

if [[ ! -d "zziplib" ]]; then
    git clone https://github.com/leonkasovan/zziplib.git
fi
cd zziplib
cmake .
ninja

cd ../..    # back to /f/Projects/luaxx/
make -f msys.Makefile