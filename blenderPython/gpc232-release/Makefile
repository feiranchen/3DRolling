CC=gcc
CFLAGS=-I. -g -O0 -ggdb2 
DEPS = gpc.h
OBJ = main.o gpc.o my_gpc.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

exe: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm *.o exe
