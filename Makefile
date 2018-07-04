pwd = $(shell pwd)
alarm-clock: main.o
	gcc -L$(pwd) -Wall -Werror -o alarm-clock main.o -lcolor-temp -lws2811 -lm
main.o: main.c color_temp.h ws2811.h
	gcc -c -Wall -Werror main.c
.PHONY: clean
clean:
	rm -f alarm-clock main.o
