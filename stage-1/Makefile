all:	unhex

unhex:
	../stage-0/unhex < ../stage-0/unhex.x > unhex
	chmod a+x unhex
	cmp ../stage-0/unhex unhex

clean:
	rm unhex
