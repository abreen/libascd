LDFLAGS = -lfann -lgmp -lm
LIBS_PATH = $(HOME)/lib/
CLEAN = *.o *.so .tmp

libascd.so: libascd.o
	gcc -shared -o libascd.so libascd.o

libascd.o:
	gcc -ggdb -c -fPIC libascd.c

install: libascd.so
	install libascd.so $(LIBS_PATH)

clean:
	rm -f $(wildcard $(CLEAN))

.PHONY: install clean
