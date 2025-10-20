flags=-O2 -Wall -std=c23 -I../osapi -w
ldflags=

.PHONY: clean clean-osapi clean-osapi

all: clean clean-osapi command.com

../osapi/osapi.o: ../osapi/osapi.c ../osapi/osapi.h
	$(MAKE) -C ../osapi/ -sk "CFLAGS=$(flags)" 
	
command.com: command.o ../osapi/osapi.o 
	cc $(flags) $^ -o $@ $(ldflags)

command.o: command.c command.h
	cc $(flags) -c $<

clean-osapi:
	$(MAKE) -C ../osapi clean
	
clean:
	rm -f *.o command.com
	
clean-all: clean clean-osapi
