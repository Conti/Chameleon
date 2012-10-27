
SRCROOT = $(abspath $(CURDIR)/../../../..)
OBJROOT = $(SRCROOT)/obj/i386/util
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
KEYMAPSROOT = $(SRCROOT)/Keymaps

INC = -I$(SRCROOT)/i386/libsaio/ -I. -I..

OBJS     = cham-mklayout.o32 cham-mklayout.o64

OBJS    := $(addprefix $(OBJROOT)/, $(OBJS))

MKLAYOUT = cham-mklayout
MKLAYOUT := $(addprefix $(SYMROOT)/, $(MKLAYOUT))

PROGRAMS = $(MKLAYOUT)

LAYOUTS_SRC_DIR  = layouts-src
KEYBOARD_LAYOUTS = $(patsubst $(LAYOUTS_SRC_DIR)/%.slt, $(KEYMAPSROOT)/%.lyt, \
					$(wildcard $(LAYOUTS_SRC_DIR)/*.slt))

DIRS_NEEDED = $(KEYMAPSROOT)

include ${SRCROOT}/Make.rules

all: $(DIRS_NEEDED) $(OBJROOT) $(SYMROOT) keymaps

$(PROGRAMS): $(OBJS)
	@echo "\t[LD32] $(@F)_32"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) $(INCLUDES) -arch i386   \
		-o $(SYMROOT)/$(@F)_32 $(OBJROOT)/$(@F).o32
	@echo "\t[LD64] $(@F)_64"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) $(INCLUDES) -arch x86_64 \
		-o $(SYMROOT)/$(@F)_64 $(OBJROOT)/$(@F).o64
	@echo "\t[LIPO] $(@F)"
	@lipo -create -arch i386 $(SYMROOT)/$(@F)_32 -arch x86_64 $(SYMROOT)/$(@F)_64 \
		-output $(SYMROOT)/$(@F)
	@$(RM) $(SYMROOT)/$(@F)_32 $(SYMROOT)/$(@F)_64

keymaps: $(KEYBOARD_LAYOUTS)

$(KEYMAPSROOT)/%.lyt: $(LAYOUTS_SRC_DIR)/%.slt $(MKLAYOUT)
	@echo "\t[LAYOUT] Creating keyboard layout: $*"
	@$(MKLAYOUT) -i $< -o $@

#.PHONY layouts
clean-local:
	@for o in $(OBJS); do if [ -f "$${o}" ];then echo "\t[RM] $${o}"; fi; done
	@for p in $(PROGRAMS); do if [ -f "$${p}" ];then echo "\t[RM] $${p}"; fi; done
	@for k in $(KEYBOARD_LAYOUTS); do if [ -f "$${k}" ];then echo "\t[RM] $${k}"; fi; done
	@$(RM) -f $(PROGRAMS) $(OBJS)

distclean-local:
	@if [ -d "$(KEYMAPSROOT)" ];then echo "\t[RMDIR] $(KEYMAPSROOT)"; fi
	@$(RM) -rf $(KEYMAPSROOT)
