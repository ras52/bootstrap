SHELL = /bin/sh
PATH  = .

CHMOD = /bin/chmod
RM    = /bin/rm
CMP   = /usr/bin/cmp


all:	elfify unhexl

elfify:	elfify.x
	../stage-0/unhex < elfify.x > elfify
	$(CHMOD) a+x elfify

unhexl:	elfify unhexl.ts.x
	../stage-0/unhex < unhexl.ts.x > unhexl.ts
	elfify unhexl.ts > unhexl
	$(CHMOD) a+x unhexl
	$(RM) unhexl.ts

check:	check-unhexl check-elfify

check-unhexl:	unhexl elfify unhexl.ts.xl
	unhexl < unhexl.ts.xl > unhexl.ts
	elfify unhexl.ts > unhexl2
	$(CHMOD) a+x unhexl2
	unhexl2 < unhexl.ts.xl > unhexl2.ts
	$(CMP) unhexl.ts unhexl2.ts
	$(RM) unhexl.ts unhexl2.ts unhexl2

check-elfify:	elfify
	! elfify
	! elfify no-such-file
	
clean:
	$(RM) -f unhexl elfify unhexl.ts unhexl2.ts unhexl2
