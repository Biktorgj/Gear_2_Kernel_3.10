TARGETS = breakpoints
TARGETS += cpu-hotplug
TARGETS += efivarfs
TARGETS += kcmp
TARGETS += memfd
TARGETS += memory-hotplug
TARGETS += mqueue
TARGETS += mount
TARGETS += net
TARGETS += ptrace
TARGETS += vm

all:
	for TARGET in $(TARGETS); do \
		make -C $$TARGET; \
	done;

run_tests: all
	for TARGET in $(TARGETS); do \
		make -C $$TARGET run_tests; \
	done;

clean:
	for TARGET in $(TARGETS); do \
		make -C $$TARGET clean; \
	done;
