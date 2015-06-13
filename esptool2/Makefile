CFLAGS = -Os -O2
CC = gcc
LD = gcc

all: esptool2

esptool2.o: esptool2.c esptool2.h esptool2_elf.h elf.h
	$(CC) $(CFLAGS) -c $< -o $@

esptool2_elf.o: esptool2_elf.c esptool2.h esptool2_elf.h elf.h
	$(CC) $(CFLAGS) -c $< -o $@

esptool2: esptool2.o esptool2_elf.o
	$(LD) -o $@ $^

clean:
	@rm -f *.o
	@rm -f esptool2
