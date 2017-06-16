EXEC     = $(shell basename $$(pwd))
CC       = gcc

CFLAGS   = -std=gnu99 -O3 -Wall -Wextra -Wpedantic -Wstrict-aliasing
CFLAGS   += -g -fno-stack-protector

SRC      = $(wildcard src/*.c)
OBJ      = $(patsubst src/%.c,obj/%.o,$(SRC))

all: $(EXEC)

${EXEC}: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean

clean:
	@rm -rf obj/ $(EXEC)
