#
# Until I can remove the dependency on the appkit,
# we'll just keep the generated files in this directory
# and install them directly, rather than generating them again.
#
SRCROOT = $(abspath $(CURDIR)/../..)
OBJROOT = $(SRCROOT)/obj/i386/util
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot


DIR = util
include ${SRCROOT}/Make.rules

PROGRAMS = machOconv dyldsymboltool segsize
OBJS = dyldsymboltool.o32 dyldsymboltool.o64 machOconv.o32 machOconv.o64 segsize.o32 segsize.o64


ifeq (${CONFIG_BDMESG}, y)
PROGRAMS += bdmesg
OBJS += bdmesg.o32 bdmesg.o64
endif

ifeq (${CONFIG_OPENUP}, y)
PROGRAMS += openUp
OBJS += openUp.o64 openUp.o32
endif

OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

LDFLAGS := $(LDFALGS) -framework IOKit -framework CoreFoundation -mmacosx-version-min=10.5

SYMPROG = $(addprefix $(SYMROOT)/, $(PROGRAMS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

SUBDIRS = fdisk

all: $(DIRS_NEEDED) $(SYMPROG) all-recursive

$(SYMPROG): $(OBJS)
	@echo "\t[LD32] $(@F)_32"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch i386 -o $(SYMROOT)/$(@F)_32 $(OBJROOT)/$(@F).o32
	@echo "\t[LD64] $(@F)_64"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch x86_64 -o $(SYMROOT)/$(@F)_64 $(OBJROOT)/$(@F).o64
	@echo "\t[LIPO] $(@F)"
	@lipo -create -arch i386 $(SYMROOT)/$(@F)_32 -arch x86_64 $(SYMROOT)/$(@F)_64 -output $(SYMROOT)/$(@F)
	@$(RM) $(SYMROOT)/$(@F)_32 $(SYMROOT)/$(@F)_64

#dependencies
-include $(OBJROOT)/Makedep

clean-local:
	@for o in $(OBJS); do if [ -f "$${o}" ];then echo "\t[RM] $${o}"; fi; done
	@for p in $(SYMPROG); do if [ -f "$${p}" ];then echo "\t[RM] $${p}"; fi; done
	@rm -f $(SYMPROG) $(OBJS)
