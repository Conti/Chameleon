SRCROOT = $(abspath $(CURDIR)/../..)
OBJROOT = $(SRCROOT)/obj/i386/libsa
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = libsa
include ${SRCROOT}/Make.rules

LIBSAIODIR = ../libsaio

CFLAGS	:= $(CFLAGS) $(RC_CFLAGS) $(MORECPP) -arch i386 \
	      -fno-builtin -static $(OMIT_FRAME_POINTER_CFLAG) \
	      -march=pentium4 -msse2 -msoft-float -fno-stack-protector \
	      -nostdinc -include $(SRCROOT)/autoconf.h

CPPFLAGS := $(CPPFLAGS) -nostdinc++

INC = -I. -I$(SYMROOT) -I$(LIBSAIODIR) -I${SRCROOT}/i386/include

OBJS = prf.o printf.o zalloc.o \
	string.o strtol.o error.o \
	setjmp.o qsort.o efi_tables.o

OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

LIBS = libsa.a
LIBS := $(addprefix $(SYMROOT)/, $(LIBS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme: $(DIRS_NEEDED) $(LIBS)


$(LIBS): $(OBJS)
	@echo "\t[RM] $@"
	@rm -f $@
	@echo "\t[AR] $(@F)"
	@ar q $@ $^ &> /dev/null
	@echo "\t[RANLIB] $(@F)"
	@ranlib $@

# dependencies
-include $(OBJROOT)/Makedep

clean-local:
	@for o in $(OBJS); do if [ -f "$${o}" ];then echo "\t[RM] $${o}"; fi; done
	@for l in $(LIBS); do if [ -f "$${l}" ];then echo "\t[RM] $${l}"; fi; done
	@rm -f $(LIBS) $(OBJS)
