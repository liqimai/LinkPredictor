all: bin bin/MetaPath bin/test

clean:
	$(MAKE) -C src clean

cleanest: clean
	$(MAKE) -C src cleanest
	@rm bin/MetaPath bin/test || echo "Already cleanest"

.PHONY: all clean cleanest

bin:
	mkdir bin

bin/MetaPath: src/MetaPath 
	mv src/MetaPath bin/

bin/test: src/test
	mv src/test bin/

src/test:
	$(MAKE) -C src test

src/MetaPath:
	$(MAKE) -C src MetaPath
