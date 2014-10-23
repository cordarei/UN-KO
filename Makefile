
.DEFAULT: foo

.PHONY: foo test

foo:
	tup --quiet

test:
	tup --quiet
	./build/test/test
