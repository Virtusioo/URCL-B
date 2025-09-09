

# Find all .b files
SRC := $(wildcard examples/*.b)
OBJ := $(SRC:.b=.urcl)

all: $(OBJ)

%.urcl: %.b
	bcc $< -o $@

