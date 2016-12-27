CC  ?= gcc
CXX ?= g++
LD  ?= ld
CFLAGS  += 
CXXFLAGS+= -std=c++11 
LDFLAGS += -lmatio
SHARED_OUT += KnowledgeGraph.o LogisticRegression.o MetaPath.o verbosity.o

all: MetaPath test clean

clean:
	@rm *.o || echo "Already clean"

cleanest: clean
	@rm MetaPath test || echo "Already cleanest"

.PHONY: all clean cleanest

MetaPath: main.o $(SHARED_OUT)
	$(CXX) -o $@ $^ $(LDFLAGS)

test: test.o $(SHARED_OUT)
	$(CXX) -o $@ $^ $(LDFLAGS)
