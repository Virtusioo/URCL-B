
SRC := $(wildcard examples/*.b)
OBJ := $(SRC:.b=.urcl)

.PHONY: all FORCE
all: $(OBJ)

%.urcl: %.b FORCE
	bcc $< -o $@

FORCE:
