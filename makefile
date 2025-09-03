# Old makefile was contributed to by servusDei2018 and hamzainaan #

# Engine
NAME ?= cee
VERSION ?= 1.5-dev

# Allow override from command line, e.g. make TARGET_OS=windows CC=x86_64-w64-mingw32-gcc
CC ?= gcc
CFLAGS ?= -O3 -s -Wall -fopenmp
LDFLAGS ?= -lm -fopenmp

# Set target OS: windows, linux or android (default: detect from OS)
ifeq ($(OS),Windows_NT)
TARGET_OS ?= windows
else
TARGET_OS ?= linux
endif

ifeq ($(TARGET_OS),windows)
	RM = powershell -Command "Remove-Item -Force -ErrorAction Ignore"
	EXE_EXTENSION = .exe
	SRC_DIR = ./src
	BIN_DIR = ./bin
	TARGET = $(BIN_DIR)/$(NAME)-$(VERSION)
	OS_CFLAGS =
else ifeq ($(TARGET_OS),android)
	RM = rm -rf
	EXE_EXTENSION = -android
	SRC_DIR = ./src
	BIN_DIR = ./bin
	TARGET = $(BIN_DIR)/$(NAME)-$(VERSION)
	OS_CFLAGS = -D ANDROID -D LINUX
	CFLAGS := $(filter-out -fopenmp,$(CFLAGS))
	LDFLAGS := $(filter-out -fopenmp,$(LDFLAGS))
else
	RM = rm -rf
	EXE_EXTENSION = -linux
	SRC_DIR = ./src
	BIN_DIR = ./bin
	TARGET = $(BIN_DIR)/$(NAME)-$(VERSION)
	OS_CFLAGS = -D LINUX
endif

# Automatically discover all source and header files in the ./src directory
SRCS := $(wildcard $(SRC_DIR)/*.c)
HEADERS := $(wildcard $(SRC_DIR)/*.h)

.PHONY: all help clean

all: $(TARGET)$(EXE_EXTENSION)

help:
	@echo "Available targets:"
	@echo "  all     : Build the engine ($(NAME)-$(VERSION)) for $(TARGET_OS) platform."
	@echo "  clean   : Remove the built binary."
	@echo "  help    : Show this help message."
	@echo ""
	@echo "Variables you can override:"
	@echo "  NAME, VERSION, TARGET_OS, CC, CFLAGS, LDFLAGS"
	@echo "Example: make NAME=myengine VERSION=2.0 TARGET_OS=windows CC=x86_64-w64-mingw32-gcc"
	@echo "For Android: make TARGET_OS=android CC=/path/to/android-ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang"

$(TARGET)$(EXE_EXTENSION): $(SRCS)
	$(CC) $^ -o $@ $(CFLAGS) $(OS_CFLAGS) $(LDFLAGS)

clean:
	-$(RM) "$(TARGET)$(EXE_EXTENSION)"
