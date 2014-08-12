DIRS = $(filter-out Makefile,$(wildcard *))

all:
	@for d in $(DIRS); do \
		make -C $$d all; \
	done

clean:
	@for d in $(DIRS); do \
		make -C $$d clean; \
	done

