#	Makefile for i386 modules

# The order of building is important.
SUBDIRS = klibc uClibc++
#SUBDIRS = klibc uClibc++ HelloWorld


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
OBJROOT=`pwd`/../../obj/i386/modules/
SYMROOT=`pwd`/../../sym/i386/modules/
DSTROOT=`pwd`/../../dst/i386/modules/
SRCROOT=/tmp

VPATH = $(OBJROOT):$(SYMROOT)


all embedtheme optionrom tags debug install installhdrs: objroot_dirs
	@for i in ${SUBDIRS}; \
	do \
		echo ================= make $@ for $$i =================; \
		( cd $$i; $(MKDIRS) $(OBJROOT)/$$i; ${MAKE}					  \
			"OBJROOT=$(OBJROOT)/$$i"			  \
		  	"SYMROOT=$(SYMROOT)/"				  \
			"DSTROOT=$(DSTROOT)/"				  \
			"SRCROOT=$(SRCROOT)"				  \
			"RC_ARCHS=$(RC_ARCHS)"				  \
			"RC_KANJI=$(RC_KANJI)"				  \
			"JAPANESE=$(JAPANESE)"				  \
			"RC_CFLAGS=$(RC_CFLAGS)" $@ 			  \
		) || exit $$?; 						  \
	done

objroot_dirs:
	@rm -rf "$(OBJROOT)" 
	@mkdir "$(OBJROOT)" &> /dev/null
	@rm -rf module_includes
	@mkdir module_includes
	@rm -rf "$(SYMROOT)/modules/"
	@mkdir "$(SYMROOT)/modules/"

installsrc:
	tar cf - . | (cd ${SRCROOT}; tar xfBp -)
