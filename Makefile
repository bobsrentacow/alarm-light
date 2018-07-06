pwd = $(shell pwd)

test: main.c libalarm-light.so
	gcc -L$(pwd) -Wall -Werror -o test main.c -lalarm-light -lcolor-temp -lws2811 -lm
libalarm-light.so: alarm-light.o
	gcc -shared -Wall -o libalarm-light.so alarm_light.o
alarm-light.o: alarm_light.c color_temp.h ws2811.h
	gcc -c -Wall -Werror -fpic alarm_light.c
deb: libalarm-light.so
	mkdir -p libalarm-light_1.0/usr/local/lib
	cp libalarm-light.so libalarm-light_1.0/usr/local/lib
	cp -r DEBIAN libalarm-light_1.0/
	dpkg-deb --build libalarm-light_1.0
.PHONY: install
install:
	cp libalarm-light.so /usr/local/lib/
	chmod 0755 /usr/local/lib/libalarm-light.so
	ldconfig
.PHONY: uninstall
uninstall:
	rm /usr/local/lib/libalarm-light.so
	ldconfig
.PHONY: clean
clean:
	rm -f test libalarm-light_1.0.deb libalarm-light.so alarm-light.o
	rm -rf libalarm-light_1.0/
