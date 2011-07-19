#	Makefile for kernel booter
SRCROOT = $(shell pwd)
OBJROOT = $(SRCROOT)/obj
SYMROOT = $(SRCROOT)/sym
DSTROOT = $(SRCROOT)/dst
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot


include Make.rules


THEME = default

VERSION = `cat version`
REVISION = `cat revision`
PRODUCT = Chameleon-$(VERSION)-r$(REVISION)
CDLABEL = ${PRODUCT}
ISOIMAGE = ${SYMROOT}/${CDLABEL}.iso
DISTFILE = ${SYMROOT}/${PRODUCT}

EXCLUDE = --exclude=.svn --exclude=.DS_Store --exclude=sym --exclude=obj \
		--exclude=package --exclude=archive --exclude=User_Guide_src --exclude=*.sh

ARCHLESS_RC_CFLAGS=`echo $(RC_CFLAGS) | sed 's/-arch [a-z0-9]*//g'`

GENERIC_SUBDIRS =

#
# Currently builds for i386
#
config rebuild_config:
	@if [ -e ".svn" ]; then svnversion -n | tr -d [:alpha:] > revision; fi
	@echo ================= make config for i386 =================;
	@( OBJROOT=$(OBJROOT)/i386;				  \
	  SYMROOT=$(SYMROOT)/i386;				  \
	  DSTROOT=$(DSTROOT);					  \
	  SRCROOT=$(SRCROOT);					  \
	    cd i386; ${MAKE}					  \
		"OBJROOT=$$OBJROOT"		 	  	  \
	  	"SYMROOT=$$SYMROOT"				  \
		"DSTROOT=$$DSTROOT"				  \
		"SRCROOT=$$SRCROOT"				  \
		"RC_ARCHS=$$RC_ARCHS"				  \
		"TARGET=$$i"					  \
		"RC_KANJI=$(RC_KANJI)"				  \
		"JAPANESE=$(JAPANESE)"				  \
		"RC_CFLAGS=$$XCFLAGS" $@			  \
	) || exit $$?; 						  \


all: $(SYMROOT) $(OBJROOT) $(SRCROOT)/auto.conf $(SRCROOT)/autoconf.h $(SRCROOT)/autoconf.inc $(SRCROOT)/.config $(SYMROOT)/i386/vers.h
	@if [ -e ".svn" ]; then svnversion -n | tr -d [:alpha:] > revision; fi
	@if [ -z "$(RC_ARCHS)" ]; then					  \
		RC_ARCHS="i386";					  \
	fi;								  \
	SUBDIRS="$(GENERIC_SUBDIRS) $$RC_ARCHS";			  \
	for i in $$SUBDIRS; 						  \
	do \
	    if [ -d $$i ]; then						  \
		echo ================= make $@ for $$i =================; \
		( OBJROOT=$(OBJROOT)/$${i};				  \
		  SYMROOT=$(SYMROOT)/$${i};				  \
		  DSTROOT=$(DSTROOT);					  \
		  SRCROOT=$(SRCROOT);					  \
	          XCFLAGS=$(ARCHLESS_RC_CFLAGS);			  \
	          GENSUBDIRS="$(GENERIC_SUBDIRS)";			  \
	          for x in $$GENSUBDIRS;				  \
	          do							  \
	              if [ "$$x" == "$$i" ]; then			  \
	                  XCFLAGS="$(RC_CFLAGS)";			  \
	                  break;					  \
	              fi						  \
	          done;							  \
		    cd $$i; ${MAKE}					  \
			"OBJROOT=$$OBJROOT"		 	  	  \
		  	"SYMROOT=$$SYMROOT"				  \
			"DSTROOT=$$DSTROOT"				  \
			"SRCROOT=$$SRCROOT"				  \
			"RC_ARCHS=$$RC_ARCHS"				  \
			"TARGET=$$i"					  \
			"RC_KANJI=$(RC_KANJI)"				  \
			"JAPANESE=$(JAPANESE)"				  \
			"RC_CFLAGS=$$XCFLAGS" $@			  \
		) || exit $$?; 						  \
	    else							  \
	    	echo "========= nothing to build for $$i =========";	  \
	    fi;								  \
	done

dist image: all
	@echo "================= Distrib ================="
	@echo "\t[RM] ${IMGROOT}"
	@rm -rf ${IMGROOT}	
	@echo "\t[MKDIR] ${IMGROOT}/usr/standalone/i386"			  	  
	@mkdir -p ${IMGROOT}/usr/standalone/i386
	@echo "\t[MKDIR] ${IMGROOT}/Extra/modules"
	@mkdir -p ${IMGROOT}/Extra/modules				
	@if [ -e "$(IMGSKELROOT)" ]; then				\
		@echo "\t[CP] ${IMGROOTSKEL} ${IMGROOT}"		\
		@cp -R -f "${IMGSKELROOT}"/* "${IMGROOT}";		\
	fi;								  
	@cp -f ${SYMROOT}/i386/cdboot ${CDBOOT}
	@cp -f ${SYMROOT}/i386/modules/* ${IMGROOT}/Extra/modules
	@cp -f ${SYMROOT}/i386/boot ${IMGROOT}/usr/standalone/i386
	@cp -f ${SYMROOT}/i386/boot0 ${IMGROOT}/usr/standalone/i386
        @cp -f ${SYMROOT}/i386/boot0md ${IMGROOT}/usr/standalone/i386
	@cp -f ${SYMROOT}/i386/boot1h ${IMGROOT}/usr/standalone/i386
	@cp -f ${SYMROOT}/i386/boot1f32 ${IMGROOT}/usr/standalone/i386
	@echo "\t[HDIUTIL] ${ISOIMAGE}"
	@hdiutil makehybrid -iso -joliet -hfs -hfs-volume-name \
		${CDLABEL} -eltorito-boot ${CDBOOT} -no-emul-boot -ov -o   \
		"${ISOIMAGE}" ${IMGROOT} -quiet 		  	  
	@echo "\t[GZ] ${DISTFILE}.tar.gz"
	@rm -f ${DISTFILE}.tar.gz
	@tar -cf ${DISTFILE}.tar ${IMGROOT}
	@gzip --best ${DISTFILE}.tar

pkg installer: all
	@# TODO: remove sudo
	sudo ${SRCROOT}/package/buildpkg.sh ${SYMROOT}/package;

$(SYMROOT)/i386/vers.h: version
	@echo "#define I386BOOT_VERSION \"5.0.132\"" > $@
	@echo "#define I386BOOT_BUILDDATE \"`date \"+%Y-%m-%d %H:%M:%S\"`\"" >> $@
	@echo "#define I386BOOT_CHAMELEONVERSION \"`cat version`\"" >> $@
	@echo "#define I386BOOT_CHAMELEONREVISION \"`svnversion -n | tr -d [:alpha:]`\"" >> $@


.PHONY: $(SYMROOT)/i386/vers.h
.PHONY: config
.PHONY: clean
.PHONY: image
.PHONY: pkg
.PHONY: installer
