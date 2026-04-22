# Don't instrument UML-specific code
KCOV_INSTRUMENT                := n

ARCH_DIR := arch/um
HOST_DIR := arch/$(HEADER_ARCH)

obj-y = \
	console.o \
	cpu.o \
	irq.o \
	main.o \
	mmu.o \
	os.o \
	process.o \
	signal.o \
	start_up.o \
	string.o \
	vxdwraps.o

CFLAGS_console.o += -I $(srctree)/drivers/tty/hvc

USER_OBJS := $(user-objs-y)

include $(srctree)/arch/um/scripts/Makefile.rules
