#!/bin/bash

# in MSYS2 requirment
# pacman -S autotools
# pacman -S git
# pacman -S cmake

cd external

# git clone https://github.com/leonkasovan/zlib.git
# cd zlib
# cmake .
# ninja
# cd ..

git clone https://github.com/leonkasovan/curl.git
cd curl
./buildconf 
./configure --with-schannel --without-brotli --without-nghttp2 --without-libidn2 --without-libpsl --without-zstd --disable-alt-svc --disable-hsts --disable-tls-srp --disable-proxy --disable-ipv6 --disable-ntlm-wb --disable-ntlm --disable-dict --disable-gopher --disable-gophers --disable-imap --disable-mqtt --disable-pop3 --disable-pop3s --disable-smtp --disable-smtps --disable-telnet --disable-tftp --disable-rtsp --disable-file --disable-ldap --disable-ldaps --disable-pthreads
make


