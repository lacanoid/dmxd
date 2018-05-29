all: dmxd


dmxd: dmxd.c
	cc -o dmxd dmxd.c -lftdi1 -llo

clean:
	rm dmxd
