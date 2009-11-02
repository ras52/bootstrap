all:	elfify unhex

elfify:	elfify.x
	../stage-0/unhex < elfify.x > elfify
	chmod a+x elfify

unhex:	elfify unhex.ts.x
	../stage-0/unhex < unhex.ts.x > unhex.ts
	./elfify unhex.ts > unhex
	chmod a+x unhex
	rm unhex.ts

check:	check-unhex check-elfify

check-unhex:	unhex elfify
	./unhex < unhex.ts.x > unhex.ts
	./elfify unhex.ts > unhex2
	chmod a+x unhex2
	./unhex2 < unhex.ts.x > unhex2.ts
	cmp unhex.ts unhex2.ts
	rm unhex.ts unhex2.ts unhex2

check-elfify:	elfify
	! ./elfify
	! ./elfify no-such-file
	
clean:
	rm -f unhex elfify unhex.ts unhex2.ts unhex2
