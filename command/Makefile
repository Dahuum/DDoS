flags=-O2 -Wall -std=c23 -I../osapi  -fPIC -g -w
ldflags=
CC=gcc-15

.PHONY: clean clean-osapi 

all: clean  clean-osapi command.com

../osapi/osapi.so: ../osapi/osapi.c ../osapi/osapi.h ../osapi/disk.c ../osapi/disk.h
	$(MAKE) -C ../osapi/ -sk "CFLAGS=$(flags)" 
	
command.com: command.o ../osapi/osapi.so 
	$(CC) $(flags) $^ -o $@ $(ldflags)

command.o: command.c command.h
	$(CC) $(flags) -c $<

clean-osapi:
	$(MAKE) -C ../osapi clean
	
clean:
	rm -f *.so *.o  command.com
	
fclean: clean clean-osapi
