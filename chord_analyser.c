#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chord_analyser.h"
// TODO:
// Arpeggio mode?
// Configuration file?
// C Major sharp or flat selection
/*

chord_analyser.c
 
MIT License
 
Copyright (c) 2023 Richard Jones
  
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

# Midi Chord Analyser

## Features

Common chords are displayed with their root note, quality and scale degree.

When running the app appears as a midi input device called ChordAnalyser

### Root note: 

eg C,C♯,E,E♭ ...

### Quality: 

eg Major, m Minor, ⁺ Augmented, ° Diminished, ⁷ Seventh ...

### Scale Degree in Roman Numerals 

eg Major: I ii  iii IV V vi vii°

eg minor: i ii° III iv v VI VII 

### Example Chord

eg: Chord:  B♭   Major VI

The key signature is displayed when no chords are playing.

eg: 1♭ Key:  Dm    minor

### Key Signature

Set the Key Signature by playing far right hand notes A♯BC together followed by the new scale chord

Ref: https://en.wikipedia.org/wiki/Chord_notation

The app uses Linux ALSA, Jack compatibility may be added later.

## Building
To compile the app under linux download all the files and compile using make. 
eg *$ make*

## Using

Start a sound synthesizer 

eg: *$ fluidsynth -p Synth /usr/share/sounds/sf2/FluidR3_GM.sf2*

### Start the app

eg: *$ ./ChordAnalyser*

### Connecting outputs to inputs

Then run aconnect to connect a midi keyboard output to the chord analyser input and synth input

eg: *$ aconnect -l # discover the identity of midi devices available*

### Auto connect and view
eg: *$ aconnect 28 128 ; aconnect 28 129 ; aconnectgui*

<img src="aconnectgui.png" style="height: 272px; width:302px;"/>

NB aconnectgui has the title ALSA Sequencer

## Errors, omissions and improvements

Please contact the author


*/

// const char * flat  = "♭"; // Note: These symbols are several characters long
// const char * natural = "♮";
// const char * sharp = "♯";
// const char * double_sharp = "𝄪";
// const char * seven = "⁷";
// const char * augmented = "⁺";
// const char * diminished = "°";

#define NOTES_PER_OCTAVE 12

const char *key_notes[][NOTES_PER_OCTAVE]= { // Optimise this if ever short of memory
{"C" ,"D♭","D","E♭","F♭","F" ,"G♭","G","A♭","A","B♭","C♭"}, // -7♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","C♭"}, // -6♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","B" }, // -5♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","B" }, // -4♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","B" }, // -3♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","B" }, // -2♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","B" }, // -1♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","B" }, //  0♭
//{"C" ,"C♯","D","D♯","E" ,"F" ,"F♯","G","G♯","A","A♯","B" }, //  0♯
{"C" ,"C♯","D","D♯","E" ,"F" ,"F♯","G","G♯","A","A♯","B" }, //  1♯
{"C" ,"C♯","D","D♯","E" ,"F" ,"F♯","G","G♯","A","A♯","B" }, //  2♯
{"C" ,"C♯","D","D♯","E" ,"F" ,"F♯","G","G♯","A","A♯","B" }, //  3♯
{"C" ,"C♯","D","D♯","E" ,"F" ,"F♯","G","G♯","A","A♯","B" }, //  4♯
{"C" ,"C♯","D","D♯","E" ,"F" ,"F♯","G","G♯","A","A♯","B" }, //  5♯
{"C" ,"C♯","D","D♯","E" ,"E♯","F♯","G","G♯","A","A♯","B" }, //  6♯
{"B♯","C♯","D","D♯","E" ,"E♯","F♯","G","G♯","A","A♯","B" }  //  7♯
};

char *key_sf[] = { "7♭","6♭","5♭","4♭","3♭","2♭","1♭","  ","1♯","2♯","3♯","4♯","5♯","6♯","7♯" };

// Print table of 30 keys based of circle of 5ths ( circle of 7 semitones / half steps )

void showKeys( void ) {
  printf("%12sChord Analyser\r\n\n\n","");
  printf( "KSig  Major RelMinor  Diatonic Scale\r\n");
  for ( int note = -49 ; note < 7*8 ; note+=7 ) { // rotate clockwise at 7 half step intervals from C♭ to C♯
    int index_major = (60+note)%NOTES_PER_OCTAVE;
    int index_minor = (index_major+9)%NOTES_PER_OCTAVE; // minor scale is nine half steps above Major scale
    printf ( "%2s      %-2s      %-2s    ", 
      key_sf[(49+note)/7], key_notes[(49+note)/7][index_major] , key_notes[(49+note)/7][index_minor] );
    for ( int i = 0 ; i < NOTES_PER_OCTAVE ; i++ ) {
      if ( key_notes[7][i][1] == 0 ) // Detect the white keys in C Major for same pattern in current key
        printf( "%-2s ", key_notes[(note+49)/7][(60+note+i)%NOTES_PER_OCTAVE] );
    }
    printf("\r\n");
  }
  printf("\n\rTo set the key signature play the major or minor chord of the same name.\r\n");
  printf("Play the root note above♯ or below♭ middle C to select keys with 5-7♯ or 5-7♭\r\n");
}


char * getNotesMsg ( int notes ) {
  static char msg[80];
  //sprintf( msg, "0x%03x ",notes);
  msg[0]=0;
  for ( int i = 0 ; i < NOTES_PER_OCTAVE ; i++ ) {
    if ( notes & 1 )
      sprintf( msg + strlen(msg), "%s", key_notes[7][i]) ;
    notes = notes >> 1 ;
  }
  return msg;
}


int RotateOctaveByN ( int pattern , int n ){
  while ( n < 0 ) n += NOTES_PER_OCTAVE;
  if ( n >= NOTES_PER_OCTAVE ) n = n%NOTES_PER_OCTAVE;
  while ( n > 0 ) {
    int lsb = pattern & 1; 
    pattern = pattern >> 1 ;
    if ( lsb )
      pattern |= 0x800;
    n--;
  }
  return pattern;
}

// return the scale degree of chord being played if it matches the scale degree list
const char * OldScaleDegree(int note,int chord,int key_note,int key_is_minor){

  const int c_major_1_7[] = { 
    0x091, // C Major I
    0x224, // D Minor ii
    0x890, // E Minor iii
    0x221, // F Major IV
    0x884, // G Major V
    0x211, // A Minor vi
    0x824  // B dim   vii°
  };// Chords I .. VII
  // Key to Roman Numbers
  const char * roman_major [] = { "I   ","ii  ","iii ","IV  ","V   ","vi  ","vii°" };
  const char * roman_minor [] = { "i   ","ii° ","III ","iv  ","v   ","VI  ","VII " };

  int n = key_note; // transpose the playing chord to cmajor
  if ( key_is_minor ) n-=9; // Minor keys are 9 halfsteps from their relative major
  int transposed_chord = RotateOctaveByN ( chord , n ); 
  printf( "Transposed chord = %03X\n",transposed_chord );
  for ( int degree = 0 ; degree < ( sizeof(c_major_1_7)/sizeof(int) ); degree++ ) {
    if ( c_major_1_7[degree] == transposed_chord ) {
      if( key_is_minor )
        return roman_minor[(degree+2)%7]; // eg A minor vi maps to i
      else
        return roman_major[degree];
    }
  }
  return "";
}

const int new_key    =   16;
const int log_toggle = 0x10;
// chord Bitmaps 12 bits one octave C=0x1, C♯=0x2,D=0x4,D♯=0x8 ... B=0x800
// 001 002 004 008 010 020 040 080 100 200 400 800
//  C   Df  D   Ef  E   F   Gf  G   Af  A   Bf  B
const int MAJOR = 0x10; // E
const int MINOR = 0x08; // E flat
struct { const int notes;const char *name;const int flags; char * enh_eq; } chord_defs[] = {
 { 0x091,""       ,0,"" }, // C E  G     Major
 { 0x891,"Maj⁷,△⁷",0,"" }, // C E  G  B  Major⁷
 { 0x811,"Maj⁷",   0,"" }, // C E     B  Major⁷
 { 0x491,"⁷,dom⁷", 0,"" }, // C E  G  B♭ dominant⁷
 { 0x411,"⁷,dom⁷", 0,"" }, // C E     B♭ no 5th  dominant⁷
 { 0x111,"⁺,Aug",  0,""}, // C E  G♯  augmented
 { 0x089,"m,-,min,minor",0,"" }, // C E♭ G  minor
 { 0x289,"m⁶,-⁶,min⁶,minor⁶",0,"" }, // C E♭ G A minor⁶
 { 0x489,"m⁷,⁻⁷,minor⁷",0,"" }, // C E  G  B♭ minor⁷
 { 0x049,"°,diminished",0,"" }, // C E♭ G♭  diminished
 { 0x449,"𝆩⁷,minor⁷flat⁵,halfDiminished⁷",0,""}, // C E♭ G♭ B♭ minor seventh flat five? half diminished⁷
 { 0x249,"°⁷,dim⁷,diminished⁷",0,""}, // C E♭ G♭ B♭♭ minor seventh flat five? diminished⁷
 { 0x085,"sus²",0,""     }, // CD  G      no 3rd *** beware inversions and naming. suspended²
 { 0x0A1,"sus4",0,""     }, // C  FG      no 3rd *** beware inversions and naming. suspended4
 { 0x4a5,"9sus4",0,""    }, // C–F–G–B♭–D dominant9th
 { 0xb01,"Log toggle", log_toggle,""},// A♭ A B C
 { 0xc01,"Play Major or minor chord to set new key",new_key,""}, // B♭ B C
 { 0x000,"",0,"" }, // end
};

void listChords ( void ) {
  int i = 0;
  int notes;

  while ( ( notes = chord_defs[i].notes) ) {
    printf( "%s %s\r\n",chord_defs[i].name,getNotesMsg(notes) );
    i++;
  }
}

// return empty string or "EqN" if (N) enharmonic equivalents found
char * EnharmonicEquivalents ( int kbd ) {
  int i = 0;
  int equivs = 0;
  static char msg[4]="Eq0";

  while ( chord_defs[i].notes ) {
      for( int j = 0 ; j < NOTES_PER_OCTAVE ; j++ ) {
        kbd =  RotateOctaveByN ( kbd , 1 ) ;
        if ( kbd == chord_defs[i].notes ) {
          equivs++;
        }
      }
    i++;
  }
  msg[2]=equivs+'0';
  if ( equivs < 2 )
    return ("");
  else
    return (msg);
}

void printChordMessage( int notes ) ;

void listEnharmonicEquivalents(void) {
  int i = 0;
  while ( chord_defs[i].notes ) {
    char * result = EnharmonicEquivalents(chord_defs[i].notes);
    if ( result[0] == 'E' ) {
        printChordMessage( chord_defs[i].notes );
        printf("\r\n");
    }
    i++;   
  }
}



const char * ScaleDegree(int root,int chord_id,int key_note,int key_is_minor){
  const int    arabic_id_M [] = { 1,0,2,0,3,4,0,5,0,6,0,7 };// Major white notes
  const int    arabic_id_m [] = { 3,0,4,0,5,6,0,7,0,1,0,2 };// minor white notes
  const char * roman_major [] = { "","I","II","III","IV","V","VI","VII" };
  const char * roman_minor [] = { "","i","ii","iii","iv","v","vi","vii" };
  int chord = chord_defs[chord_id].notes;
  if ( ( chord & 0x018 ) != 0 ) {// Transposed chord contains either E or E♭ ?
   int note_id = ( 144 + root - key_note ) % NOTES_PER_OCTAVE ;
   int arabic_id = arabic_id_M [ note_id ];
   if ( key_is_minor )
     arabic_id   = arabic_id_m [ (note_id+9) % NOTES_PER_OCTAVE ];
   if ( chord & 0x008 ) { // Minor chord containing E♭
    // printf( "minor Transposed chord=%03X note_id=%d name=%s roman=%s\n",
    //  chord, note_id , key_notes[7][note_id],roman_minor[ arabic_id ] );
    return roman_minor[ arabic_id ] ;
   } else {
    // printf( "major Transposed chord=%03X note_id=%d name=%s roman=%s\n",
    //  chord, note_id , key_notes[7][note_id],roman_major[ arabic_id ] );
    return roman_major[ arabic_id ] ;
   }
  }
  return "";
}


const int key_sf_index[] = { // Major key: -ve number of flats, +v number of sharps
 0, // C
-5, // D♭ or -5+12 C♯
 2, // D
-3, // E♭
 4, // E
-1, // F
-6, // G♭ or -6+12 F♯
 1, // G
-4, // A♭
 3, // A
-2, // B♭
-7, // C♭ or -7+12 B
};

const int MAX_INT = 0x7fffffff;
#define NUM_NOTES 128
int lowest_note;

int getActiveNotes( int note, int velocity, int channel , int on ) {

  int notes; // one octave of notes 2^0 == C, 2^1 == C♯...
  static int started ;
  static int keyboard_image[NUM_NOTES];

  // make an image of all active keyboard notes
  if ( ! started ) { // clear image on first call
    started++;
    for ( int i = 0 ; i < NUM_NOTES ; i++ ) 
      keyboard_image[i] = 0;
  }
  notes = 0; // collapse all the active keyboard notes into a single octave called notes
  lowest_note = MAX_INT;
  if ( ( note > 0 ) && ( note < NUM_NOTES ) ) {
    keyboard_image[note] = on;
    for ( int i = 0 ; i < NUM_NOTES ; i++ ) {
      if ( keyboard_image[i] ) {
        notes |= ( 1 << ( i % NOTES_PER_OCTAVE ) ) ;
        if ( lowest_note == MAX_INT )
          lowest_note = i;
      }
    }
  }
  return notes; 
}

void printChordMessage( int kbd ) {
  char chord_msg[80] = {""};
  const int KEY_UNKNOWN = 2; 
  static int key_note = 0;
  static int key_is_minor = KEY_UNKNOWN ;
  static int line_count=0;
  static int log_enable = 0;
  const char *major_minor[] = { " ","m",""};
  const  int midi_middle_c = 60;
  static int num_sharps_flats = 0;
  const char *off_on[] = { "Off","On" };

  const char * scale_degree = "";
  int notes = kbd;
  for ( int i = 0 ; i < NOTES_PER_OCTAVE ; i++ ) { // Transpose the C chord to all 12 note offsets, check for pattern match through chord definitions
    int chord_id = 0;
    while ( chord_defs[chord_id].notes  ) { 
    //for ( int chord_id =0 ; chord_id < NUM_CHORD_DEFS ; chord_id++ ) {
      if ( notes == chord_defs[chord_id].notes ) {
        if ( chord_defs[chord_id].flags & new_key ) {
          if ( lowest_note > 91 ) { // Only notes at top of keyboard set key
             sprintf (chord_msg ,"Play Major or minor chord to set new key");
             key_is_minor = KEY_UNKNOWN; 
             key_note = 12;
           }
        } else if (chord_defs[chord_id].flags & log_toggle ) {
           if ( lowest_note > 91 ) {
              log_enable ^= 1; sprintf ( chord_msg, "Log = %s",off_on[log_enable] );
            }
        } else {
           if ( ( key_is_minor == KEY_UNKNOWN ) &&
               ((chord_defs[chord_id].notes&MAJOR) ||(chord_defs[chord_id].notes&MINOR)) ) {
               key_note = lowest_note;
               key_is_minor = 0;
               if ( chord_defs[chord_id].notes&MINOR ) {
                 key_is_minor = 1;
               }
               num_sharps_flats = key_sf_index[(key_note - key_is_minor*9) % NOTES_PER_OCTAVE ];
               if ( num_sharps_flats <= -5 )
                 if ( lowest_note >= midi_middle_c )
                   num_sharps_flats += NOTES_PER_OCTAVE ; // User select 5-7♯ Key B, F♯, C♯
               line_count = 0 ; // Trigger heading message
               //printf("\\r\nchord_id=%d chord_defs[chord_id].notes=0x%02x key_note=%d lowest_note=%d key_is_minor=%d num_sharps_flats=%d\r\n",
               //  chord_id,chord_defs[chord_id].notes,key_note, lowest_note, key_is_minor, num_sharps_flats  );
           }
           int note_id = i;
           //if ( chord_defs[chord_id].flags&lowest )
           //  note_id = lowest_note % NOTES_PER_OCTAVE ;
           if ( (chord_defs[chord_id].notes&MAJOR) || (chord_defs[chord_id].notes&MINOR) )
             scale_degree = ScaleDegree( note_id,chord_id,key_note,key_is_minor);
           sprintf ( chord_msg ,"%s%s",
              key_notes[num_sharps_flats+7][note_id],chord_defs[chord_id].name);
           if ( note_id != lowest_note % NOTES_PER_OCTAVE ) // Insert slash notation where appropriate
             sprintf(chord_msg+strlen(chord_msg),"/%s ",key_notes[num_sharps_flats+7][ lowest_note % NOTES_PER_OCTAVE ] ); 
           if ( --line_count <= 0 ) {
             line_count = 20;
             printf("Key    Key   Scale\r\n");
             printf("Sig    Name  Degree   Chord\r\n");
           }
           if ( scale_degree[0] == 0 )
             scale_degree = "    ";// Unicode confuses %4s
           const char * CLR_EOL = "\033[0K";       // ANSI Clear to end of line
           //char * eequiv = "" ;
           printf( "\r\n%2s    %2s%s    %4s %3s %s %s %s",
             key_sf[num_sharps_flats+7], key_notes[num_sharps_flats+7][ key_note %NOTES_PER_OCTAVE ] , major_minor[key_is_minor],
             scale_degree,EnharmonicEquivalents(notes),chord_msg,getNotesMsg(kbd), CLR_EOL );
         }
      }
      chord_id++;
    }
    notes = RotateOctaveByN(notes,1);
  }
  if ( log_enable ) {
    //printf( "Midi note=%d, mask=%03x notes=%03x %s\r\n", note, 1 << note % NOTES_PER_OCTAVE, notes, chord_msg );
  } else {
    if ( key_is_minor == KEY_UNKNOWN ) {
      if ( line_count >= 0 ) {
        line_count = -1;
        showKeys();
      }
    } else {
      if ( chord_msg[0] ) {
      } else {
        printf("\r");
      }
    }
    fflush(stdout);
  }
}

void chord_analyser( int note, int velocity, int channel , int on ) {
  int notes = getActiveNotes( note, velocity, channel , on );
  printChordMessage( notes );
}


