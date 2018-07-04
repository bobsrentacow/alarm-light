pwd = $(shell pwd)
alarm-light: main.o
	gcc -L$(pwd) -Wall -Werror -o alarm-light main.o -lcolor-temp -lws2811 -lm
main.o: main.c color_temp.h ws2811.h
	gcc -c -Wall -Werror main.c
.PHONY: clean
clean:
	rm -f alarm-light main.o
