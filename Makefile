# CC = x86_64-w64-mingw32-gcc
CC = gcc
# CFLAGS = -O2 -Wall -Isrc -D_FILE_OFFSET_BITS=64 -DLUA_USE_LINUX -DCURL_STATICLIB 
CFLAGS = -O2 -Wall -Isrc -D_FILE_OFFSET_BITS=64 -DCURL_STATICLIB 
SRC_DIR = src
OBJ_DIR = obj

# List all C source files
SRCS := $(wildcard $(SRC_DIR)/*.c)

# Generate corresponding object file names
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Final executable name
TARGET = luaxx

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -s external/curl/lib/.libs/libcurl.a external/zlib/libzlibstatic.a -lWS2_32 -lcrypt32 -lbcrypt

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/* $(TARGET)

.PHONY: all clean
