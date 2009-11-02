all:	elfify unhex

elfify:	elfify.x
	../stage-0/unhex < elfify.x > elfify
	chmod a+x elfify

unhex:	elfify unhex.tx
	../stage-0/unhex < unhex.tx > unhex.t
	./elfify unhex.t > unhex
	chmod a+x unhex
	rm unhex.t

check:	check-unhex

check-unhex:	unhex elfify
	./unhex < unhex.tx > unhex.t
	./elfify unhex.t > unhex2
	chmod a+x unhex2
	./unhex2 < unhex.tx > unhex.t2
	cmp unhex.t unhex.t2
	rm unhex.t2 unhex2
	
clean:
	rm -f unhex elfify unhex.t unhex.t2 unhex2
