OS_MAC :=
ifeq ($(shell uname -s),Darwin)
OS_MAC := 1
endif

perftest = perftest
perftestd = perftestd

ifdef OS_MAC
CXX ?= clang++
else
CXX ?= g++
endif
LD = $(CXX)
RM = rm -rf

srcdir = src
tmpdir = tmp
debugdir = tmp/debug
bindir = bin

fltk-config = $(bindir)/fltk-config

CXXFLAGS := -std=c++17 -I$(srcdir) $(shell $(fltk-config) --cxxflags) $(CXXFLAGS)
LDFLAGS := $(shell $(fltk-config) --ldstaticflags) $(LDFLAGS)

RELEASEFLAGS = -DNDEBUG -O3 -flto
DEBUGFLAGS = -DDEBUG -D_DEBUG -O0 -g -ggdb3 -Wall -Wextra -pedantic -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter

COMMON = $(wildcard $(srcdir)/*.h)
SOURCES = $(wildcard $(srcdir)/*.cpp)
OBJECTS = $(SOURCES:$(srcdir)/%.cpp=$(tmpdir)/%.o)
DEBUGOBJECTS = $(SOURCES:$(srcdir)/%.cpp=$(debugdir)/%.o)

TARGET = $(bindir)/$(perftest)
DEBUGTARGET = $(bindir)/$(perftestd)

.PHONY: all $(perftest) $(perftestd) release debug clean

.SUFFIXES: .o .cpp

all: $(perftest)

$(perftest): release
$(perftestd): debug

release: CXXFLAGS := $(RELEASEFLAGS) $(CXXFLAGS)
release: $(TARGET)

debug: CXXFLAGS := $(DEBUGFLAGS) $(CXXFLAGS)
debug: $(DEBUGTARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(LD) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

$(DEBUGTARGET): $(DEBUGOBJECTS)
	@mkdir -p $(@D)
	$(LD) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

$(tmpdir)/%.o: $(srcdir)/%.cpp $(COMMON)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

$(debugdir)/%.o: $(srcdir)/%.cpp $(COMMON)
	@mkdir -p $(@D)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean:
	$(RM) $(TARGET) $(DEBUGTARGET) $(OBJECTS) $(DEBUGOBJECTS)
