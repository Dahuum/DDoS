flags=-O2 -Wall -std=c23 -I. -g -w -fPIC 
soflags=-ldl -shared 
ldflags=


.PHONY:  clean

all: clean osapi.so ddosutil

osapi.so: osapi.o disk.o omnistd.o fs.o
	$(CC) $(flags) $(CFLAGS) $(soflags) $^ -o $@

osapi.o: osapi.c osapi.h os.h
	$(CC) $(flags) $(CFLAGS) -c $<

disk.o: disk.c disk.h
	$(CC) $(flags) $(CFLAGS) -c $<

omnistd.o: omnistd.c omnistd.h
	$(CC) $(flags) $(CFLAGS) -c $<

fs.o: fs.c fs.h
	 $(CC) $(flags) $(CFLAGS) -c $<

ddosutil: ddosutil.o osapi.o disk.o omnistd.o fs.o
	$(CC) $(flags) $(CFLAGS) $^ -o $@
	mv -f ddosutil ../util/

ddosutil.o: ddosutil.c osapi.h
	$(CC) $(flags) $(CFLAGS) -c $<

clean:
	rm -f *.o  *.so osapi
