flags=-O2 -Wall -std=c23 -I. 
soflags=-ldl -shared -fPIC
ldflags=

.PHONY:  clean

all: clean osapi.so

osapi.so: osapi.o disk.o
	cc $(flags) $(CFLAGS) $(soflags) $^ -o $@

osapi.o: osapi.c osapi.h os.h
	cc $(flags) $(CFLAGS) -c $<

disk.o: disk.c disk.h
	cc $(flags) $(CFLAGS) -c $<
	
clean:
	rm -f *.o  *.so osapi