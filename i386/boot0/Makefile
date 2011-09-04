SRCROOT = $(abspath $(CURDIR)/../..)
OBJROOT = $(SRCROOT)/obj/i386/boot0
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = boot0
include ${SRCROOT}/Make.rules

DIRS_NEEDED = $(SYMROOT)

OBJS  = boot0 boot0hfs boot0md chain0
OBJS := $(addprefix $(SYMROOT)/, $(OBJS))

all: $(DIRS_NEEDED) $(OBJS)

$(OBJS): $(SRCROOT)/autoconf.inc
	@echo "\t[NASM] $(@F)"
	@$(NASM) $(@F).s -o $@

clean-local:
	@for o in $(OBJS); do if [ -f "$${o}" ];then echo "\t[RM] $${o}"; fi; done
	@rm -f $(OBJS)
