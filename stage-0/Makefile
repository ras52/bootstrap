all:

check:	check-unhex

check-unhex:
	chmod a+x unhex
	./unhex < unhex.x > unhex2
	cmp unhex unhex2
	rm unhex2

clean:
	rm -f unhex2
