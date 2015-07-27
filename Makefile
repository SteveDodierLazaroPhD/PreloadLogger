all: lib

test-run: test lib
	LD_PRELOAD=$(DESTDIR)/usr/lib/libPreloadLogger.so ./preload-logger-test

lib: zlib.a
	gcc -Wall -fPIC -DPIC -shared -o lib.so lib.c zlib/libz.a zlib/gz*.o zlib/adler32.o zlib/compress.o zlib/crc32.o zlib/deflate.o zlib/infback.o zlib/inffast.o zlib/inflate.o zlib/inftrees.o zlib/trees.o zlib/uncompr.o zlib/zutil.o logger.c gslist.c -ldl -O0 -g

zlib.a:
	make -C zlib

test:
	gcc test.c -g -O0 -o preload-logger-test -lrt

clean:
	rm *~ preload-logger-test lib.so -f

install: lib
	mkdir $(DESTDIR)/usr/lib/ -p
	cp lib.so $(DESTDIR)/usr/lib/libPreloadLogger.so.0.9
	ln -fs $(DESTDIR)/usr/lib/libPreloadLogger.so.0.9 $(DESTDIR)/usr/lib/libPreloadLogger.so.0
	ln -fs $(DESTDIR)/usr/lib/libPreloadLogger.so.0 $(DESTDIR)/usr/lib/libPreloadLogger.so
	mkdir $(DESTDIR)/etc/security/ -p
#	echo "LD_PRELOAD      DEFAULT=\"$(DESTDIR)/usr/lib/libPreloadLogger.so\"" >> $(DESTDIR)/etc/security/pam_env.conf


uninstall:
#	sed -i '\@PreloadLogger.so@d' $(DESTDIR)/etc/security/pam_env.conf
	rm $(DESTDIR)/usr/lib/libPreloadLogger.so -f
	rm $(DESTDIR)/usr/lib/libPreloadLogger.so.0 -f
	rm $(DESTDIR)/usr/lib/libPreloadLogger.so.0.9 -f
	



