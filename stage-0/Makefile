SHELL = /bin/sh
PATH  = .

CHMOD = /bin/chmod
RM    = /bin/rm
CMP   = /usr/bin/cmp


all:

check:	check-unhex

check-unhex:
	$(CHMOD) a+x unhex
	unhex < unhex.x > unhex2
	$(CMP) unhex unhex2
	$(RM) unhex2

clean:
	$(RM) -f unhex2
