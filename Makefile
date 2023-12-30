CC=gcc
CXX=g++
CFLAGS=-Wall -Wextra -pthread -O0 -std=c++17  -g

# OSS - operational support system 
SRC_DIRS := src src/gui src/menu src/components src/sqlite src/gps src/girouette 
SOURCES := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))
SOURCESH := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.hpp))
OBJECTS := $(SOURCES:.cpp=.o)

TARGETS=sqlite3.o oss gps_emiter

LIBS=`fltk-config --cxxflags --ldflags  --use-images --use-gl`

sqlite3.o : sqlite/sqlite3.c
	$(CC) -o $@ -c $^ -ldl

oss: $(OBJECTS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LIBS)  sqlite3.o -lm -ldl

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

# GPS emiter (emulator)
gps_emiter: src-gps_emiter/main.cpp
	$(CXX) $(CFLAGS) src-gps_emiter/main.cpp -o gps_emiter -pthread 

all : $(TARGETS)

clean:
	rm -f $(OBJECTS) $(TARGETS)

.PHONY: all, clean