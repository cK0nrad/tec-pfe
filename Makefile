CC=g++
CFLAGS=-Wall -pthread -ggdb3 -std=c++17

# OSS - operational support system 
TARGET=oss
SOURCES := $(wildcard src/*.cpp)
HEADERS := $(wildcard src/*.hpp)

GUISOURCE := $(wildcard src/gui/*.cpp)
GUIHEADER := $(wildcard src/gui/*.hpp)

MENUSOURCE := $(wildcard src/menu/*.cpp)
MENUHEADER := $(wildcard src/menu/*.hpp)

COMPONENTS := $(wildcard src/components/*.cpp)
COMPONENTSHEADERS := $(wildcard src/components/*.hpp)

GPS := $(wildcard src/gps/*.cpp)
GPSHEADERS := $(wildcard src/gps/*.hpp)

LIBS=`fltk-config --cxxflags --ldflags  --use-images --use-gl`

$(TARGET): $(SOURCES) $(GUISOURCE) $(MENUSOURCE) $(HEADERS) $(GUIHEADER) $(MENUHEADER) $(COMPONENTS) $(COMPONENTSHEADERS) $(GPS) $(GPSHEADERS)
	$(CC) $(CFLAGS) $(SOURCES) $(GUISOURCE) $(MENUSOURCE) $(COMPONENTS) $(COMPONENTSHEADERS) $(GPS) -o $(TARGET) $(LIBS)

# GPS emiter (emulator)
gps_emiter: src-gps_emiter/main.cpp
	$(CC) $(CFLAGS) src-gps_emiter/main.cpp -o gps_emiter -pthread 

clean:
	rm -f $(TARGET)
