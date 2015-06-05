all: lib

test-run: test lib
	LD_PRELOAD=$(DESTDIR)/usr/lib/libPreloadLogger.so ./preload-logger-test

lib:
	gcc -Wall -fPIC -DPIC -shared -o lib.so lib.c logger.c gslist.c -ldl -O0 -g

test:
	gcc test.c -g -O0 -o preload-logger-test -lrt

clean:
	unset LD_PRELOAD
	rm *~ preload-logger-test lib.so -f

install: lib
	mkdir $(DESTDIR)/usr/lib/ -p
	cp lib.so $(DESTDIR)/usr/lib/libPreloadLogger.so.0.8
	ln -s $(DESTDIR)/usr/lib/libPreloadLogger.so.0.8 $(DESTDIR)/usr/lib/libPreloadLogger.so.0
	ln -s $(DESTDIR)/usr/lib/libPreloadLogger.so.0 $(DESTDIR)/usr/lib/libPreloadLogger.so
	mkdir $(DESTDIR)/etc/security/ -p
#	echo "LD_PRELOAD      DEFAULT=\"$(DESTDIR)/usr/lib/libPreloadLogger.so\"" >> $(DESTDIR)/etc/security/pam_env.conf


uninstall:
#	sed -i '\@PreloadLogger.so@d' $(DESTDIR)/etc/security/pam_env.conf
	unset LD_PRELOAD
	rm $(DESTDIR)/usr/lib/libPreloadLogger.so -f
	rm $(DESTDIR)/usr/lib/libPreloadLogger.so.0 -f
	rm $(DESTDIR)/usr/lib/libPreloadLogger.so.0.8 -f
	



