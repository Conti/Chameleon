SRCROOT = $(abspath $(CURDIR)/../..)
OBJROOT = $(SRCROOT)/obj/i386/libsaio
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = libsaio
include ${SRCROOT}/Make.rules

UTILDIR  = ../util
LIBSADIR = ../libsa
BOOT2DIR = ../boot2

DEFINES = -DNOTHING
#DEFINES = -DDEBUG_CPU=1 -DDEBUG_MEM=1 -DDEBUG_SPD=1 -DDEBUG_PCI=1 -DDEBUG_SMBIOS=1

CFLAGS	:= $(CFLAGS) $(RC_CFLAGS) $(MORECPP) -arch i386 \
    -D__ARCHITECTURE__=\"i386\" -DSAIO_INTERNAL_USER \
    -DRCZ_COMPRESSED_FILE_SUPPORT \
    -fno-builtin -static $(OMIT_FRAME_POINTER_CFLAG) \
    -mpreferred-stack-boundary=2 -fno-align-functions -fno-stack-protector \
    -march=pentium4 -msse2 -mfpmath=sse -msoft-float -nostdinc -include $(SRCROOT)/autoconf.h

CPPFLAGS := $(CPPFLAGS) -nostdinc++

INC = -I. -I$(SRCROOT) -I$(SYMROOT) -I$(LIBSADIR) -I$(BOOT2DIR) -I${SRCROOT}/i386/include

SAIO_OBJS = table.o asm.o bios.o biosfn.o \
	disk.o sys.o cache.o bootstruct.o \
	stringTable.o load.o pci.o allocate.o misc.o \
	befs.o freebsd.o openbsd.o \
	vbe.o nbp.o hfs.o hfs_compare.o \
	xml.o ntfs.o msdos.o md5c.o device_tree.o \
	cpu.o platform.o acpi_patcher.o \
	smbios.o smbios_getters.o smbios_decode.o \
	fake_efi.o ext2fs.o \
	hpet.o dram_controllers.o spd.o usb.o pci_setup.o \
	device_inject.o nvidia.o ati.o gma.o pci_root.o \
	convert.o aml_generator.o console.o exfat.o

SAIO_OBJS := $(addprefix $(OBJROOT)/, $(SAIO_OBJS))

LIBS  = libsaio.a
LIBS := $(addprefix $(SYMROOT)/, $(LIBS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)


all: $(DIRS_NEEDED) $(CONFIG_HEADERS) $(LIBS) all-recursive

$(LIBS): $(SAIO_OBJS)
	@echo "\t[RM] $@"
	@rm -f $@
	@echo "\t[AR] $(@F)"
	@ar q $@ $^ &> /dev/null
	@echo "\t[RANLIB] $(@F)"
	@ranlib $(SYMROOT)/$(@F)

clean-local:
	@rm -f $(LIBS) $(SAIO_OBJS)

# dependencies
-include $(OBJROOT)/Makedep
