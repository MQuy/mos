OUTPUT = kernel.bin

ROOTDIR := $(shell cd .. && pwd)

C_SOURCES = $(wildcard main.c *.c ../libraries/shared/*.c utils/*.c utils/**/*.c memory/*.c cpu/*.c devices/*.c devices/**/*.c system/*.c fs/*.c fs/**/*.c proc/*.c locking/*.c ipc/*.c net/*.c net/devices/*.c)
HEADERS = $(wildcard *.h ../libraries/shared/*.h utils/*.h memory/*.h cpu/*.h devices/*.h devices/**/*.h system/*.h fs/*.h fs/**/*.h proc/*.h locking/*.h ipc/*.h net/*.h net/devices/*.h)

# Nice syntax for file extension replacement
OBJ = ${C_SOURCES:.c=.o boot.o cpu/interrupt.o cpu/descriptor.o proc/scheduler.o proc/user.o}

# -g: Use debugging symbols in gcc
CFLAGS = -g -std=gnu18 -ffreestanding -Wall -Wextra -Wno-unused-parameter -Wno-discarded-qualifiers -Wno-comment -Wno-multichar -Wno-sequence-point -Wno-switch -Wno-unused-function -Wno-unused-value -Wno-sign-compare -Wno-implicit-fallthrough -I$(ROOTDIR)/kernel -I$(ROOTDIR)/libraries

include ../rules/platform.mk
include ../rules/targets.mk
