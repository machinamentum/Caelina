.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

GIT_BRANCH	:=	$(shell git rev-parse --abbrev-ref HEAD)
GIT_COMMIT	:=	$(shell git rev-parse --short HEAD)
VERSION		:=	$(GIT_BRANCH)-$(GIT_COMMIT)

all:
	make -f MakefileGLESv1
	make -f MakefileGL
	make -f MakefileCaelina

clean:
	make -f MakefileGLESv1 clean
	make -f MakefileGL clean
	make -f MakefileCaelina clean
	rm -f libcaelina-$(VERSION).tar.bz2

dist-bin: all
	@tar -cjf libcaelina-$(VERSION).tar.bz2 include lib

install: dist-bin
	mkdir -p $(DEVKITPRO)/libctru
	bzip2 -cd libcaelina-$(VERSION).tar.bz2 | tar -x -C $(DEVKITPRO)/libctru