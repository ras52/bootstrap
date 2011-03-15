SHELL = /bin/sh
PATH  = .

CHMOD = /bin/chmod
RM    = /bin/rm
CMP   = /usr/bin/cmp

all:	as

as:	as.ts.xl
	../stage-1/unhexl < as.ts.xl > as.ts
	../stage-1/elfify as.ts > as
	$(CHMOD) a+x as
	$(RM) as.ts

check:	check-as

check-as:	as as.s
	as < as.s > as2.ts
	../stage-1/elfify as2.ts > as2
	$(CHMOD) a+x as2
	as2 < as.s > as3.ts
	$(CMP) as2.ts as3.ts
	$(RM) as2.ts as3.ts as2

clean:
	$(RM) -f as as.ts as2.ts as3.ts as2
