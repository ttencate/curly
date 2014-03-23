if WITH_CHECK
MAYBE_TESTS = tests
endif
SUBDIRS = src . $(MAYBE_TESTS)

.PHONY: test
test: check
	make src/curly && ./run_tests.sh
