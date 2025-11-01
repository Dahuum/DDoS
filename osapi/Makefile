flags=-O2 -Wall -std=c23 -I. -g -w 
soflags=-ldl -shared -fPIC
ldflags=

.PHONY:  clean

all: clean osapi.so ddosutil

osapi.so: osapi.o disk.o omnistd.o fs.o
	cc $(flags) $(CFLAGS) $(soflags) $^ -o $@

osapi.o: osapi.c osapi.h os.h
	cc $(flags) $(CFLAGS) -c $<

disk.o: disk.c disk.h
	cc $(flags) $(CFLAGS) -c $<
	
omnistd.o: omnistd.c omnistd.h
	cc $(flags) $(CFLAGS) -c $<
	
fs.o: fs.c fs.h
	cc $(flags) $(CFLAGS) -c $<

ddosutil: ddosutil.o osapi.so
	cc $(flags) $(CFLAGS) $^ -o $@
	mv -f ddosutil ../util/

ddosutil.o: ddosutil.c
	cc $(flags) $(CFLAGS) -c $<
	
clean:
	rm -f *.o  *.so osapi