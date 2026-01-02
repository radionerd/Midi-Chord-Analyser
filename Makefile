ChordAnalyser : midi_if.o chord_analyser.o Makefile
	gcc midi_if.o chord_analyser.o -o ChordAnalyser -lasound # link

midi_if.o : midi_if.c
	gcc -c -Werror -Wall midi_if.c # compile / assemble only, treat all warnings as errors, list all warnings
chord_analyser.o : chord_analyser.c chord_analyser.h
	gcc -c -Werror -Wall chord_analyser.c # compile / assemble only, treat all warnings as errors, list all warnings
clean :
	rm ChordAnalyser midi_if.o chord_analyser.o
