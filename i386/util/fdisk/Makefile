SRCROOT = $(abspath $(CURDIR)/../../..)
OBJROOT = $(SRCROOT)/obj/i386/util/fdisk
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot


DIR = fdisk

include ${SRCROOT}/Make.rules


LDFLAGS := $(LDFALGS) -mmacosx-version-min=10.5


OBJS =  cmd.o32 disk.o32 fdisk.o32 getrawpartition.o32 mbr.o32 misc.o32 opendev.o32 part.o32 user.o32 auto.o32 \
		cmd.o64 disk.o64 fdisk.o64 getrawpartition.o64 mbr.o64 misc.o64 opendev.o64 part.o64 user.o64 auto.o64

OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

PROGRAM = fdisk440
PROGRAM:= $(addprefix $(SYMROOT)/, $(PROGRAM))

ifeq ($(CONFIG_FDISK440),y)

all: $(SYMROOT) $(OBJROOT) $(PROGRAM)

$(PROGRAM): $(OBJS)
	@echo "\t[LD32] $@_32"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch i386 -o $@_32 $(filter %.o32,$^)
	@echo "\t[LD64] $@_64"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch x86_64 -o $@_64 $(filter %.o64,$^)
	@echo "\t[LIPO] $@"
	@lipo -create -arch i386 $@_32 -arch x86_64 $@_64 -output $@
	@rm $@_32 $@_64

else

all:

endif


#dependencies
-include $(OBJROOT)/Makedep

clean-local:
	@for p in $(PROGRAMS); do if [ -f "$${p}" ];then echo "\t[RM] $${p}"; fi; done
	@for o in $(OBJS); do if [ -f "$${o}" ];then echo "\t[RM] $${o}"; fi; done
	@rm -f $(PROGRAM) $(OBJS)
