all: lib

test-run: test lib
	LD_PRELOAD=/usr/Preload/lib.so ./a.out

lib:
	gcc -Wall -fPIC -DPIC -shared -o lib.so lib.c logger.c gslist.c -ldl

test:
	gcc test.c -g -O0

clean:
	unset LD_PRELOAD
	rm *~
	rm a.out
	rm lib.so

install: lib
	mkdir /usr/Preload/ -p
	cp lib.so /usr/Preload/lib.so
	echo "LD_PRELOAD      DEFAULT=\"/usr/Preload/lib.so\"" >> /etc/security/pam_env.conf


uninstall:
	sed -i '\@Preload/lib.so@d' /etc/security/pam_env.conf
	unset LD_PRELOAD
	rm /usr/Preload/lib.so
	rmdir /usr/Preload
	



