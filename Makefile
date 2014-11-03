
.DEFAULT: foo

.PHONY: foo test

foo:
	tup --quiet

test:
	tup --quiet
	./build/test/lib/test_lib && ./build/test/classifier/test_classifier && ./build/test/parser/test_parser && ./build/test/external/test_external
