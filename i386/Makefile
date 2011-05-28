#	Makefile for i386 boot program

#
# these paths are only valid in subdirectories of this directory
#
SRCROOT = $(shell pwd)/../
OBJROOT = $(SRCROOT)/obj/i386
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

include ${SRCROOT}/Make.rules

# The order of building is important.
SUBDIRS_PRE = util
SUBDIRS = libsa libsaio boot0 boot1 boot2 cdboot modules

all:
	@for i in ${SUBDIRS_PRE}; \
	do \
		echo ================= make $@ for $$i =================; \
		( cd $$i; ${MAKE}					  \
			"OBJROOT=$(OBJROOT)/$$i"			  \
		  	"SYMROOT=$(SYMROOT)"				  \
			"DSTROOT=$(DSTROOT)"				  \
			"SRCROOT=$(SRCROOT)"				  \
			"RC_ARCHS=$(RC_ARCHS)"				  \
			"RC_KANJI=$(RC_KANJI)"				  \
			"JAPANESE=$(JAPANESE)"				  \
			"RC_CFLAGS=$(RC_CFLAGS)" $@ 			  \
		) || exit $$?; 						  \
	done

	@cd modules; ${MAKE} BUILT_IN=yes $@


	@for i in ${SUBDIRS}; \
	do \
		echo ================= make $@ for $$i =================; \
		( cd $$i; ${MAKE}					  \
			"OBJROOT=$(OBJROOT)/$$i"			  \
		  	"SYMROOT=$(SYMROOT)"				  \
			"DSTROOT=$(DSTROOT)"				  \
			"SRCROOT=$(SRCROOT)"				  \
			"RC_ARCHS=$(RC_ARCHS)"				  \
			"RC_KANJI=$(RC_KANJI)"				  \
			"JAPANESE=$(JAPANESE)"				  \
			"RC_CFLAGS=$(RC_CFLAGS)" $@ 			  \
		) || exit $$?; 						  \
	done
	
config rebuild_config:
	@for i in config; \
	do \
		echo ================= make $@ for $$i =================; \
		( cd $$i; ${MAKE}					  \
			"OBJROOT=$(OBJROOT)/$$i"			  \
		  	"SYMROOT=$(SYMROOT)"				  \
			"DSTROOT=$(DSTROOT)"				  \
			"SRCROOT=$(SRCROOT)"				  \
			"RC_ARCHS=$(RC_ARCHS)"				  \
			"RC_KANJI=$(RC_KANJI)"				  \
			"JAPANESE=$(JAPANESE)"				  \
			"RC_CFLAGS=$(RC_CFLAGS)" $@ 			  \
		) || exit $$?; 						  \
	done


.PHONY: config