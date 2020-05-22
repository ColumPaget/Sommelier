OBJ=common.o config.o apps.o platforms.o command-line.o desktopfiles.o regedit.o doom.o download.o install.o uninstall.o
LIBS=-lssl -lcrypto -lUseful -lz -lcrypto -lssl  
prefix=/usr/local
exec_prefix=${prefix}
CFLAGS=-g -O2 -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DSTDC_HEADERS=1 -D_FILE_OFFSET_BITS=64 -DHAVE_LIBSSL=1 -DHAVE_LIBCRYPTO=1 -DHAVE_LIBZ=1 -DHAVE_LIBUSEFUL=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_LIBCRYPTO=1 -DHAVE_LIBSSL=1 -DINSTALL_PREFIX=\"/usr/local\"

all: $(OBJ) main.c 
	gcc $(CFLAGS) -osommelier $(OBJ) $(LIBUSEFUL) $(LIBS) main.c 

libUseful-4/libUseful.a:
	$(MAKE) -C libUseful-4

common.o: common.h common.c
	gcc $(CFLAGS) -c common.c

config.o: config.h config.c
	gcc $(CFLAGS) -c config.c

apps.o: apps.h apps.c
	gcc $(CFLAGS) -c apps.c

doom.o: doom.h doom.c
	gcc $(CFLAGS) -c doom.c

platforms.o: platforms.h platforms.c
	gcc $(CFLAGS) -c platforms.c

command-line.o: command-line.h command-line.c
	gcc $(CFLAGS) -c command-line.c

desktopfiles.o: desktopfiles.h desktopfiles.c
	gcc $(CFLAGS) -c desktopfiles.c

download.o: download.h download.c
	gcc $(CFLAGS) -c download.c

install.o: install.h install.c
	gcc $(CFLAGS) -c install.c

uninstall.o: uninstall.h uninstall.c
	gcc $(CFLAGS) -c uninstall.c


regedit.o: regedit.h regedit.c
	gcc $(CFLAGS) -c regedit.c


clean:
	rm -f sommelier *.o libUseful-4/*.o libUseful-4/*.so libUseful-4/*.a

install:
	mkdir -p $(HOME)/bin
	mkdir -p $(HOME)/.sommelier
	cp sommelier $(HOME)/bin/
	cp *.apps $(HOME)/.sommelier/

install_global:
	mkdir -p $(DESTDIR)$(exec_prefix)/bin
	mkdir -p $(DESTDIR)$(exec_prefix)/etc/sommelier
	cp sommelier $(DESTDIR)$(exec_prefix)/bin/
	cp *.apps $(DESTDIR)$(exec_prefix)/etc/sommelier/
	cp sommelier.1 $(DESTDIR)$(exec_prefix)/share/man/man1/

test:
	echo "no tests"