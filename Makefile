CC = gcc
CFLAGS = -I /mingw64/include 
LDFLAGS = -L /mingw64/lib 
LDLIBS = -l SDL2 -l SDL2_mixer -l SDL2_ttf
 
sdl2_piano: sdl2_piano.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

sdl2_piano.o: sdl2_piano.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f *.o sdl2_piano
