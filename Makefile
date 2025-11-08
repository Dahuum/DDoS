OS:= $(shell uname)

ifeq ($(OS), Linux) # Linux
	CC=cc
else ifeq ($(OS), Darwin) # MacOS (flcase diali, brew install gcc-15 ltst version btw)
	CC=gcc-15
else 
	CC=NoBodyKnow # hadi khassni nchuf blanha
endif
export CC

all:
	$(MAKE) -C command/ 

clean:
	$(MAKE) -C command/ clean

fclean:
	$(MAKE) -C command/ fclean

