#!/bin/bash

# in WSL Debian requirment
# sudo apt install autoconf
# sudo apt install libtool
# sudo apt install automake 
# sudo apt install git
# sudo apt install cmake
# sudo apt install libssl-dev

cd external

git clone https://github.com/leonkasovan/zlib.git
cd zlib
cmake .
make
cd ..

git clone https://github.com/leonkasovan/curl.git
cd curl
autoreconf -fi
./configure CFLAGS="-Os -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables -flto" LDFLAGS="-Wl,-s -Wl,-Bsymbolic -Wl,--gc-sections" --with-openssl --without-brotli --without-nghttp2 --without-libidn2 --without-libpsl --without-zstd --disable-alt-svc --disable-hsts --disable-tls-srp --disable-proxy --disable-ipv6 --disable-ntlm-wb --disable-ntlm --disable-dict --disable-gopher --disable-gophers --disable-imap --disable-mqtt --disable-pop3 --disable-pop3s --disable-smtp --disable-smtps --disable-telnet --disable-tftp --disable-rtsp --disable-file --disable-ldap --disable-ldaps
make