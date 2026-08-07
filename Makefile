CFLAGS = -O3
CFLAGS += -g
CFLAGS += -fsanitize=address -fsanitize=undefined
CFLAGS += -Wall -Wextra -Wcast-qual -Wcast-align
CFLAGS += -Wpedantic


minecraft: main.c minecraft.o
	cc $(CFLAGS) $^ -o $@ -lSDL2 -lm

minecraft.o: minecraft.c
	cc $(CFLAGS) -c $< -o $@
	@echo "Size: $$(stat -c%s $@) B"

clean:
	rm minecraft.o minecraft