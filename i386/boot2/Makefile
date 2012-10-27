SRCROOT = $(abspath $(CURDIR)/../..)
BINROOT = $(SRCROOT)/bin
OBJROOT = $(SRCROOT)/obj/i386/boot2
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

VPATH = $(SYMROOT)

#	Makefile for i386 boot program
#	define FLOPPY and SMALL using DEFINES macro as necessary

PNGCRUSH = `which pngcrush`
PNGOPTIONS = -rem cHRM -rem gAMA -rem iCCP -rem sRGB

DIR = boot2
include ${SRCROOT}/Make.rules

OPTIM = -Os -Oz
CFLAGS	= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -arch i386 -g -Wmost -Werror \
		-fno-builtin -DSAIO_INTERNAL_USER -static $(OMIT_FRAME_POINTER_CFLAG) -fno-stack-protector \
		-march=pentium4 -msse2 -msoft-float -nostdinc -include $(SRCROOT)/autoconf.h

CPPFLAGS := $(CPPFLAGS) -nostdinc++ -include $(SRCROOT)/autoconf.h

DEFINES=
CONFIG = hd
SYMDIR = $(SYMROOT)
LIBSADIR = ../libsa
LIBSAIODIR = ../libsaio
THEME = $(CONFIG_EMBEDED_THEME)
THEMEDIR = ../../artwork/themes/$(THEME)
INC = -I. -I$(SRCROOT) -I$(SYMDIR) -I$(LIBSADIR) -I$(LIBSAIODIR) -I${SRCROOT}/i386/include

LIBS= -L$(SYMDIR) -lsaio -lsa -lklibc
LIBDEP= libsaio.a libsa.a libklibc.a 

OTHER_FILES =

# The ordering is important;
# boot2.o must be first.
OBJS = boot2.o boot.o graphics.o drivers.o prompt.o options.o lzss.o mboot.o \
	   ramdisk.o picopng.o resume.o bmdecompress.o graphic_utils.o gui.o modules.o \
	   modules_support.o boot_modules.o
# button.o browser.o scrollbar.o == NOTYET
OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

UTILDIR = ../util
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)
BOOT2ADDR = 20200
MAXBOOTSIZE = 458240


#
# Strings used to retrieve the start location for the Symbols.dylib module
#
SYMBOLS_MODULE = Symbols.dylib
SYMBOL_START= _symbols_module_start
SYMBOL_ADDR = $(shell printf "%d" 0x`nm -s __DATA __data $(SYMROOT)/boot.sys | grep " $(SYMBOL_START)$$" | cut -f 1 -d " "`)
DATA_OFFSET = $(shell otool -l $(SYMROOT)/boot.sys | grep __data -A 4  | grep __DATA -A 3 | tail -n 1 | cut -f 6 -d " ")
DATA_ADDR   = $(shell printf "%d" `otool -l $(SYMROOT)/boot.sys | grep __data -A 4  | grep __DATA -A 3 | head -n 2 | tail -n 1 | cut -f 8 -d " "`)
PATCH_ADDR  = $(shell echo ${SYMBOL_ADDR}-${DATA_ADDR}+${DATA_OFFSET} | bc)

#### NOTE #### This value is currently *not* calculated and *should* be. It is currently determined experimentally…
DATA_PAD = 3582

all: $(DIRS_NEEDED) boot all-recursive

ifeq (${CONFIG_EMBED_THEME}, y)

boot: $(CONFIG_HEADERS) $(SYMROOT)/art.h $(SYMROOT)/embedded.h $(OBJS) $(addprefix $(SYMROOT)/, $(LIBDEP))

else

boot: $(CONFIG_HEADERS) $(SYMROOT)/embedded.h $(OBJS) $(addprefix $(SYMROOT)/, $(LIBDEP))


endif
	@${MKDIRS} $(OBJROOT)/../boot2_modules/
	
	@### First pass, don't worry abount segment alignment.
	@echo "\t[LD] boot.sys"
	@$(CC) -Wl,-preload -Wl,-alias,boot2,start \
			-nostdlib -arch i386  -Wl,-pie \
			-Wl,-read_only_relocs,suppress \
			-lcc_kext \
			$(filter %.o,$^) `find $(OBJROOT)/../boot2_modules/ -name \*.o`  $(LIBS) \
			-o ${SYMROOT}/$@.sys

	@echo "\t[LD] boot.sys"
	@$(CC) -Wl,-preload -Wl,-alias,boot2,start \
			-nostdlib -arch i386  -Wl,-pie \
			-Wl,-segaddr,__INIT,`echo obase=16\; $$((0x${BOOT2ADDR})) | bc` \
			-Wl,-segaddr,__TEXT,`echo obase=16\; $$((0x${BOOT2ADDR})) + \`${SYMROOT}/segsize ${SYMROOT}/$@.sys __INIT\` | bc` \
			-Wl,-segaddr,__DATA,`echo obase=16\; $$((0x${BOOT2ADDR})) + 3582 + \`${SYMROOT}/segsize ${SYMROOT}/$@.sys __INIT\` + \`${SYMROOT}/segsize ${SYMROOT}/$@.sys __TEXT\` | bc` \
			-Wl,-read_only_relocs,suppress \
			-lcc_kext \
			$(filter %.o,$^) `find $(OBJROOT)/../boot2_modules/ -name \*.o`  $(LIBS) \
			-o ${SYMROOT}/$@.sys


ifeq (${CONFIG_MODULES}, y)

	@cp  $(SYMROOT)/boot.sys  $(SYMROOT)/boot2.sys

	@# Generate the Symbols.dylib file
	@echo "\t[dyldsymboltool] Symbols.dylib"
	@$(SYMROOT)/dyldsymboltool $(SYMROOT)/boot.sys $(SYMROOT)/${SYMBOLS_MODULE}

	@echo "\t[LD] boot.sys"
	@$(CC) -Wl,-preload -Wl,-alias,boot2,start \
			-nostdlib -arch i386  -Wl,-pie \
			-Wl,-sectcreate,__DATA,__Symbols,$(SYMROOT)/Symbols.dylib \
			-Wl,-segaddr,__INIT,`echo obase=16\; $$((0x${BOOT2ADDR})) | bc` \
			-Wl,-segaddr,__TEXT,`echo obase=16\; $$((0x${BOOT2ADDR})) + \`${SYMROOT}/segsize ${SYMROOT}/$@.sys __INIT\` | bc` \
			-Wl,-segaddr,__DATA,`echo obase=16\; $$((0x${BOOT2ADDR})) + 3582 + \`${SYMROOT}/segsize ${SYMROOT}/$@.sys __INIT\` + \`${SYMROOT}/segsize ${SYMROOT}/$@.sys __TEXT\` | bc` \
			-Wl,-read_only_relocs,suppress \
			-lcc_kext \
			$(filter %.o,$^) `find $(OBJROOT)/../boot2_modules/ -name \*.o`  $(LIBS) \
			-o ${SYMROOT}/$@.sys


	@# Second pass, fixup global var locations
	@${RM} $(SYMROOT)/${SYMBOLS_MODULE}

	@# Generate the Symbols.dylib file
	@echo "\t[dyldsymboltool] Symbols.dylib"
	@$(SYMROOT)/dyldsymboltool $(SYMROOT)/boot.sys $(SYMROOT)/${SYMBOLS_MODULE}

	@echo "\t[LD] boot.sys"
	@$(CC) -Wl,-preload -Wl,-alias,boot2,start \
			-nostdlib -arch i386  -Wl,-pie \
			-Wl,-sectcreate,__DATA,__Symbols,$(SYMROOT)/Symbols.dylib \
			-Wl,-segaddr,__INIT,`echo obase=16\; $$((0x${BOOT2ADDR})) | bc` \
			-Wl,-segaddr,__TEXT,`echo obase=16\; $$((0x${BOOT2ADDR})) + \`${SYMROOT}/segsize ${SYMROOT}/$@.sys __INIT\` | bc` \
			-Wl,-segaddr,__DATA,`echo obase=16\; $$((0x${BOOT2ADDR})) + 3582 + \`${SYMROOT}/segsize ${SYMROOT}/$@.sys __INIT\` + \`${SYMROOT}/segsize ${SYMROOT}/$@.sys __TEXT\` | bc` \
			-Wl,-read_only_relocs,suppress \
			-lcc_kext \
			$(filter %.o,$^) `find $(OBJROOT)/../boot2_modules/ -name \*.o`  $(LIBS) \
			-o ${SYMROOT}/$@.sys

	@${RM} $(SYMROOT)/${SYMBOLS_MODULE}

	@$(LD) -arch i386 \
	-undefined dynamic_lookup \
	-dylib -read_only_relocs suppress \
	-S -x -Z -dead_strip_dylibs \
	-no_uuid \
	$(filter %.o,$^) $(LIBS) \
	-final_output Symbols \
	-macosx_version_min 10.6 \
	-o $(OBJROOT)/Symbols_LINKER_ONLY.dylib

endif

	@# this is done in a sub process after boot.sys exists so the strings are populated correctly
	@make embed_symbols

	@${RM} $(SYMROOT)/boot2.sys

	@( size=`ls -l $(SYMROOT)/boot | awk '{ print $$5}'` ; \
	  if expr "$$size" ">" "$(MAXBOOTSIZE)" > /dev/null ;\
	  then \
	   	echo "\t******* boot is $$size bytes *******"; \
	    echo "Booter executable larger than $(MAXBOOTSIZE) bytes" ;\
	    rm $(SYMROOT)/boot ;\
	    exit 1;\
	  else \
	  	echo "\t******* boot is $$size bytes *******"; \
	  fi)

embed_symbols:
ifeq (${CONFIG_MODULES}, y)
	@echo ================= Embedding Symbols.dylib =================
	@echo "\t[MACHOCONV] boot"
	@$(SYMROOT)/machOconv $(SYMROOT)/boot2.sys $(SYMROOT)/boot &> /dev/null

	@echo "\t******* Patching at $(PATCH_ADDR) ******"
	@stat -f%z $(SYMROOT)/boot | perl -ane "print pack('V',@F[0]);" | dd conv=notrunc of=${SYMROOT}/boot.sys bs=1 count=4 seek=$(PATCH_ADDR) &> /dev/null
endif
	@echo "\t[MACHOCONV] boot"
	@$(SYMROOT)/machOconv $(SYMROOT)/boot.sys $(SYMROOT)/boot

$(SYMROOT)/art.h:
	@if [ "$(PNGCRUSH)" ]; then	\
		echo "optimizing art files ...\n$(PNGCRUSH) $(PNGOPTIONS) artwork/$(THEME)"; \
		cd $(THEMEDIR) && find . -name '*.png' -exec $(PNGCRUSH) -q $(PNGOPTIONS) -d $(SYMROOT)/embed {} \;; \
		cp theme.plist $(SYMROOT)/embed; \
	else \
		cp -R $(THEMEDIR) $(SYMROOT)/embed; \
	fi;
	@echo "#ifndef __BOOT2_ART_H" >> $(SYMROOT)/art.h
	@echo "#define __BOOT2_ART_H" >> $(SYMROOT)/art.h
	@cd $(SYMROOT)/embed && find . -name 'theme.plist' -exec xxd -i >> $(SYMROOT)/art.h {} \;
	@cd $(SYMROOT)/embed && find . -name '*.png' -exec xxd -i >> $(SYMROOT)/art.h {} \;

	@echo "typedef struct {" >> $(SYMROOT)/art.h
	@echo "	char			name[32];" >> $(SYMROOT)/art.h
	@echo "	unsigned char	*pngdata;" >> $(SYMROOT)/art.h
	@echo "	unsigned int	*length;" >> $(SYMROOT)/art.h
	@echo "} embeddedpng_t;" >> $(SYMROOT)/art.h
	@echo "" >> $(SYMROOT)/art.h
	@echo "embeddedpng_t embeddedImages[] = {" >> $(SYMROOT)/art.h
	@cd $(SYMROOT)/embed && find . -name '*.png' | sort | cut -f 2 -d '/' | cut -f 1 -d '.' | \
		awk '{ printf "\t{.name = \"%s\", .pngdata = __%s_png, .length = &__%s_png_len},\n", $$1, $$1, $$1 }' >> $(SYMROOT)/art.h
	@echo "};" >> $(SYMROOT)/art.h

	@echo "#endif /* !__BOOT2_ART_H */"  >> $(SYMROOT)/art.h ;\

$(SYMROOT)/embedded.h:
	@cd $(SYMROOT)/../../doc && xxd -i BootHelp.txt > $(SYMROOT)/embedded.h


#dependencies
-include $(OBJROOT)/Makedep

clean-local:
	@for o in $(SYMROOT)/boot $(SYMROOT)/boot.sys $(SYMROOT)/embedded.h $(OBJROOT)/Symbols_LINKER_ONLY.dylib; do \
		if [ -f "$${o}" ];then echo "\t[RM] $${o}"; fi; done
	@if [ -d "$(OBJROOT)" ];then echo "\t[RM] all objects in $(OBJROOT)"; fi
	@rm -f $(SYMROOT)/boot $(SYMROOT)/boot.sys $(SYMROOT)/embedded.h $(OBJS)
	@rm -rf $(OBJROOT)
