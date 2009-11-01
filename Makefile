STAGES = 1

all:
	set -e; for n in `seq 1 $(STAGES)`; do $(MAKE) -C stage-$$n all; done

clean:
	for n in `seq 1 $(STAGES)`; do $(MAKE) -C stage-$$n clean; done
