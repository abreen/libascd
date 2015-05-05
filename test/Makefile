CFLAGS = -ggdb
LDFLAGS = -lfann -lgmp -lm
OBJECTS = main.o field.o mode.o regime.o program.o learning_data.o
CLEAN = testall *.o *.gch

testall: ../libascd.o $(OBJECTS)
	gcc $(CFLAGS) -o testall ../libascd.o $(OBJECTS) $(LDFLAGS)

main.o: test.h
field.o: test.h
mode.o: test.h
regime.o: test.h
program.o: test.h

clean:
	rm -f $(wildcard $(CLEAN))

.PHONY: clean
