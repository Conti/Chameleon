#	Makefile for i386 modules

# The order of building is important.
SUBDIRS = klibc uClibc++ Resolution KernelPatcher KextPatcher
#SUBDIRS = klibc uClibc++ Resolution HelloWorld


CFLAGS= -O $(MORECPP) -arch i386 -g -static
DEFINES=
CONFIG = hd
LIBDIR = libsa
INC = -I. -I$(LIBDIR) 
ifneq "" "$(wildcard /bin/mkdirs)"
  MKDIRS = /bin/mkdirs
else
  MKDIRS = /bin/mkdir -p
endif
AS = as
LD = ld

#
# these paths are only valid in subdirectories of this directory
#
OBJROOT=../../obj/i386/modules/
SYMROOT=../../sym/i386/modules/
DSTROOT=../../dst/i386/modules/
SRCROOT=/tmp

VPATH = $(OBJROOT):$(SYMROOT)

DIRS_NEEDED = $(OBJROOT) $(SYMROOT) module_includes

all embedtheme optionrom tags debug: $(DIRS_NEEDED)
	@for i in ${SUBDIRS}; \
	do \
		echo ================= make $@ for $$i =================; \
		( cd $$i; $(MKDIRS) $(OBJROOT)/$$i; ${MAKE}					  \
			"OBJROOT=../$(OBJROOT)/$$i"			  \
		  	"SYMROOT=../$(SYMROOT)/"				  \
			"DSTROOT=../$(DSTROOT)/"				  \
			"SRCROOT=$(SRCROOT)"				  \
			"RC_ARCHS=$(RC_ARCHS)"				  \
			"RC_KANJI=$(RC_KANJI)"				  \
			"JAPANESE=$(JAPANESE)"				  \
			"RC_CFLAGS=$(RC_CFLAGS)" $@ 			  \
		) || exit $$?; 						  \
	done

	
clean:
	@echo "\t[RM] $(OBJROOT)"
	@rm -rf ${OBJROOT}
	@echo "\t[RM] $(SYMROOT)"
	@rm -rf ${SYMROOT}
	@echo "\t[RM] module_includes"
	@rm -rf module_includes &> /dev/null



include ../MakeInc.dir


