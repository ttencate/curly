if WITH_CHECK
MAYBE_TESTS = tests
endif
SUBDIRS = src . $(MAYBE_TESTS)

.PHONY: test
test: src/curly check
	./run_tests.sh
