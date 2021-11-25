compile: clean
	gcc -g host.c -o host `pkg-config --cflags --libs cairo` -lX11 -ljpeg
clean:
	rm -f host