CXX=g++
CP=cp
CXXFLAGS= -c -Wall -g -O2 -std=c++11
INCLUDE = -Iinclude
LDFLAGS= -lpthread -lportaudio -lmpg123 -lboost_regex -lboost_iostreams -lboost_system -lboost_filesystem -ljsoncpp -ltag -lavformat -lavutil -lavcodec 
RM=rm
SOURCES= $(wildcard src/*.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
DEPS = $(SOURCES:.cpp=.d)

EXECUTABLE=player

all: $(SOURCES) $(EXECUTABLE)

compile: $(OBJECTS)

recompile: clean all

depends: $(SOURCES)
	rm -f ./depends.d
	make do_make_deps

do_make_deps: $(DEPS)

%.d : %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MG -MM -MP -MT"$(<:.cpp=.o)" $< >> depends.d

install:
	$(CP) $(EXECUTABLE) /usr/bin/
uninstall:
	$(RM) /usr/bin/$(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDE) $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)

-include depends.d
