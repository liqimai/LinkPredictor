CC  ?= gcc
CXX ?= g++
LD  ?= ld
CFLAGS  += -g
CXXFLAGS+= -std=c++11 -g -O3
LDFLAGS += -lmatio
SHARED_OUT += KnowledgeGraph.o LogisticRegression.o MetaPath.o verbosity.o

all: MetaPath KGtest clean

clean:
	@rm *.o || echo "Already clean"

cleanest: clean
	@rm MetaPath KGtest || echo "Already cleanest"

.PHONY: all clean cleanest

MetaPath: main.o $(SHARED_OUT)
	$(CXX) -o $@ $^ $(LDFLAGS)

KGtest: KGtest.o $(SHARED_OUT)
	$(CXX) -o $@ $^ $(LDFLAGS)
