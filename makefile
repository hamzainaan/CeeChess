# Old makefile was contributed to by servusDei2018 and hamzainaan #

# Engine
NAME ?= cee
VERSION ?= 1.5-dev

# Allow override from command line, e.g. make TARGET_OS=windows CC=x86_64-w64-mingw32-gcc
CC ?= gcc
CFLAGS ?= -O3 -s -Wall
LDFLAGS ?= -lm

# Set target OS: windows or linux (default: detect from OS)
TARGET_OS ?= $(shell if [ "$(OS)" = "Windows_NT" ]; then echo windows; else echo linux; fi)

ifeq ($(TARGET_OS),windows)
	RM = del /Q
	EXE_EXTENSION = .exe
	SRC_DIR = ./src
	BIN_DIR = ./bin
	TARGET = $(BIN_DIR)/$(NAME)-$(VERSION)
	OS_CFLAGS =
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

$(TARGET)$(EXE_EXTENSION): $(SRCS) $(HEADERS)
	$(CC) $^ -o $@ $(CFLAGS) $(OS_CFLAGS) $(LDFLAGS)

clean:
	-$(RM) $(TARGET)$(EXE_EXTENSION)
