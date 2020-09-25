CC = clang

ROOT_DIR = .

CFLAGS = -std=c11
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wundef
CFLAGS += -Wno-ignored-qualifiers
CFLAGS += -I.

DEPS = timer.h
SRC = closh.c timer.c
OBJS = ${SRC:.c=.o}

.PHONY: clean test

closh: $(OBJS)
	$(CC) $(OBJS) -o $@ $(CFLAGS)

$(OBJS): $(SRC) $(DEPS)
	$(CC) -c $^ $(CFLAGS)

clean:
	@rm ./*.o -f
	@rm ./temp -rf
	@rm closh
