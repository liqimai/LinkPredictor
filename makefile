all: bin/MetaPath bin/KGtest data

clean:
	$(MAKE) -C src clean
	@rm data.zip || echo "Already clean"

cleanest: clean
	$(MAKE) -C src cleanest
	@rm -rf bin/MetaPath bin/KGtest data || echo "Already cleanest"

.PHONY: all clean cleanest 

# bin:
# 	mkdir bin

bin/MetaPath: src/MetaPath  
	cp src/MetaPath bin/

bin/KGtest: src/KGtest  
	cp src/KGtest bin/

src/KGtest:
	$(MAKE) -C src KGtest

src/MetaPath:
	$(MAKE) -C src MetaPath

.PHONY: src/KGtest src/MetaPath

data:
	wget "https://www.dropbox.com/s/ktgikfa6ybltroo/data.zip"
	unzip data.zip
