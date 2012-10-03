#
# Until I can remove the dependency on the appkit,
# we'll just keep the generated files in this directory
# and install them directly, rather than generating them again.
#
SRCROOT = $(abspath $(CURDIR)/../..)
OBJROOT = $(SRCROOT)/obj/i386/config
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = util
include ${SRCROOT}/Make.rules

OBJS = cconfig.o32_nodep cconfig.o64_nodep zconf.tab.o32_nodep zconf.tab.o64_nodep \
		  yesno.o32_nodep yesno.o64_nodep textbox.o32_nodep textbox.o64_nodep menubox.o32_nodep \
		  menubox.o64_nodep checklist.o32_nodep checklist.o64_nodep inputbox.o32_nodep inputbox.o64_nodep

OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

DEFINES = -DKBUILD_NO_NLS -DCURSES_LOC=\<ncurses.h\> -DPATH_MAX=256 -DPACKAGE=\"chameleon\" 
LDFLAGS = -lncurses -lmenu

PROGRAMS = cconfig

SYMPROG = $(addprefix $(SYMROOT)/, $(PROGRAMS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all: $(DIRS_NEEDED) $(SYMPROG)

$(SYMPROG): $(OBJS)
	@echo "\t[LD32] $(@F)_32"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch i386 -o $(SYMROOT)/$(@F)_32 $(OBJROOT)/*.o32_nodep
	@echo "\t[LD64] $(@F)_64"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch x86_64 -o $(SYMROOT)/$(@F)_64 $(OBJROOT)/*.o64_nodep
	@echo "\t[LIPO] $(@F)"
	@lipo -create -arch i386 $(SYMROOT)/$(@F)_32 -arch x86_64 $(SYMROOT)/$(@F)_64 -output $(SYMROOT)/$(@F)
	@$(RM) $(SYMROOT)/$(@F)_32 $(SYMROOT)/$(@F)_64

config: $(DIRS_NEEDED) $(SYMPROG)
	@cd ${SRCROOT} && $(SYMPROG) $(SRCROOT)/Cconfig

rebuild_config: $(DIRS_NEEDED) $(SYMPROG)
	@cd ${SRCROOT} && $(SYMPROG) $(SRCROOT)/Cconfig rebuild


.PHONY: config rebuild_config

#dependencies
-include $(OBJROOT)/Makedep

distclean-local:
	@rm -f $(SYMPROG) $(OBJS) $(OBJROOT)/Makedep
