all: main

main: main.c 
	gcc -o $@ $< `pkg-config --cflags --libs gtk4` -lm -lX11 -lXrandr

