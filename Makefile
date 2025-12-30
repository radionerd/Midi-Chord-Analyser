ChordAnalyser : midi_if.o chord_analyser.o Makefile
	gcc midi_if.o chord_analyser.o -o ChordAnalyser -lasound 

midi_if.o : midi_if.c
	gcc -c -Wall midi_if.c # compile / assemble only, all warnings
chord_analyser.o : chord_analyser.c chord_analyser.h
	gcc -c -Wall chord_analyser.c # compile / assemble only, all warnings
clean :
	rm ChordAnalyser midi_if.o chord_analyser.o
