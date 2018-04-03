pwd = $(shell pwd)
alarm-clock: main.o libws2811.a
	gcc -L$(pwd) -Wall -Werror -o alarm-clock main.o libws2811.a -lcolor-temp -lm
main.o: main.c color_temp.h ws2811.h
	gcc -c -Wall -Werror main.c
.PHONY: clean
clean:
	rm -f alarm-clock main.o
