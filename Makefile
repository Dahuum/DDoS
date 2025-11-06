OS:= $(shell uname)

ifeq ($(OS), Linux)
	CC=cc
else ifeq ($(OS), Darwin)
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

