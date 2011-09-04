SRCROOT = $(CURDIR)/../../
OBJROOT = $(SRCROOT)/obj/i386/boot1
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

include ${SRCROOT}/Make.rules

DIR = boot1

DIRS_NEEDED = $(SYMROOT)

VERS = `vers_string -f 5.0 | tr - .`
NEW_VERS = Darwin boot1h v$(VERS)



PROGRAMS  =  boot1hp boot1f32

ifeq (${CONFIG_BOOT1_HFS}, y)
PROGRAMS += boot1h
endif

ifeq (${CONFIG_BOOT1_HFS_ACTIVE}, y)
PROGRAMS += boot1he
endif


PROGRAMS := $(addprefix $(SYMROOT)/, $(PROGRAMS))

all: $(DIRS_NEEDED) $(PROGRAMS)


$(PROGRAMS): $(SRCROOT)/autoconf.inc
	@echo "\t[NASM] $(@F)" 
	@$(NASM) $(@F).s -o $@

install_i386:: all $(INSTALLDIR)
	cp $(SYMROOT)/boot1h $(INSTALLDIR)/
	cd $(INSTALLDIR); chmod u+w $(VERSIONED_FILES) 
