# SRC_DIR = {folder z plikami *.cpp}
# OBJ_DIR = {folder z obiektami *.o}
# LIB_DIR = {folder z bibliotekami *.h}
# CXX = {compiler to use}
# CXXFLAGS = {flagi to kompilatora: -O2 -std=c++17 -lstdc++fs -c -Wall} 

SRC_DIR = Source/
OBJ_DIR = Objects/
CXX = g++
CXXFLAGS = -std=c++17 -x c++  -Wall -Wextra -c

.SILENT: all clean
.PHONY: dump
all:
	@echo "Compiling..."
	$(CXX) $(CXXFLAGS) $(SRC_DIR)main.cpp -o $(OBJ_DIR)main.o
	$(CXX) $(OBJ_DIR)main.o -lcrypto -lstdc++fs -o main.exe
clean:
	rm *.exe
	
install:
	sudo apt-get install libssl-dev -y

dump:
	$(CXX) $(CXXFLAGS) $(SRC_DIR)main.cpp -S

