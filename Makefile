CFLAGS = -O3

minecraft: main.c minecraft.o
	cc $(CFLAGS) $^ -o $@ -lSDL2

minecraft.o: minecraft.c
	cc $(CFLAGS) -c $< -o $@
	@echo "Size: $$(stat -c%s $@) B"

clean:
	rm minecraft.o minecraft