
CXX := g++
FLAGS := -O2 -I.
SRC := $(wildcard src/*.cpp) 
NAME := bcc

all:
	$(CXX) $(SRC) -o $(NAME) $(FLAGS)