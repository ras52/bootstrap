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

check:

clean:
	$(RM) -f as as.ts 
