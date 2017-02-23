DATA_LINK = "https://www.dropbox.com/s/ybzpfq62s673p3d/data.zip"

all: bin/MetaPath bin/KGtest bin/KGinfo data

clean:
	$(MAKE) -C src clean
	@rm data.zip || echo "Already clean"

cleanest: clean
	$(MAKE) -C src cleanest
	@rm -rf bin/MetaPath bin/KGtest bin/KGinfo || echo "Already cleanest"

.PHONY: all clean cleanest 

# bin:
# 	mkdir bin

bin/MetaPath: src/MetaPath  
	cp src/MetaPath bin/

bin/KGtest: src/KGtest  
	cp src/KGtest bin/

bin/KGinfo: src/KGinfo
	cp src/KGinfo bin/

src/KGtest:
	$(MAKE) -C src KGtest

src/MetaPath:
	$(MAKE) -C src MetaPath

src/KGinfo:
	$(MAKE) -C src KGinfo

.PHONY: src/KGtest src/MetaPath src/KGinfo

data:
	wget $(DATA_LINK)
	unzip data.zip
