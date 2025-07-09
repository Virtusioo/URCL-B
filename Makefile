
CXX := g++
FLAGS := -O2 -I.

SRC := $(wildcard src/*.cpp) 
UTIL := $(wildcard util/*.cpp)

NAME := bcc

all:
	$(CXX) $(SRC) $(UTIL) -o $(NAME) $(FLAGS)