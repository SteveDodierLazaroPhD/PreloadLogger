all: lib

test-run: test lib
	LD_PRELOAD=/usr/lib/libPreloadLogger.so ./preload-logger-test

lib:
	gcc -Wall -fPIC -DPIC -shared -o lib.so lib.c logger.c gslist.c -ldl -O0 -g

test:
	gcc test.c -g -O0 -o preload-logger-test

clean:
	unset LD_PRELOAD
	rm *~
	rm a.out
	rm lib.so

install: lib
	mkdir /usr/lib/ -p
	cp lib.so /usr/lib/libPreloadLogger.so
	echo "LD_PRELOAD      DEFAULT=\"/usr/lib/libPreloadLogger.so\"" >> /etc/security/pam_env.conf


uninstall:
	sed -i '\@PreloadLogger.so@d' /etc/security/pam_env.conf
	unset LD_PRELOAD
	rm /usr/lib/libPreloadLogger.so
	



