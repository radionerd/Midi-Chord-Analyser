#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chord_analyser.h"
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

const char *key_notes[][12]= { // Optimise this if ever short of memory
{"C" ,"D♭","D","E♭","F♭","F" ,"G♭","G","A♭","A","B♭","C♭"}, // -7♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","C♭"}, // -6♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","B" }, // -5♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","B" }, // -4♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","B" }, // -3♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","B" }, // -2♭
{"C" ,"D♭","D","E♭","E" ,"F" ,"G♭","G","A♭","A","B♭","B" }, // -1♭
{"C" ,"C♯","D","D♯","E" ,"F" ,"F♯","G","G♯","A","A♯","B" }, //  0♯
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
    int index_major = (60+note)%12;
    int index_minor = (index_major+9)%12; // minor scale is nine half steps above Major scale
    printf ( "%2s      %-2s      %-2s    ", 
      key_sf[(49+note)/7], key_notes[(49+note)/7][index_major] , key_notes[(49+note)/7][index_minor] );
    for ( int i = 0 ; i < 12 ; i++ ) {
      if ( key_notes[7][i][1] == 0 ) // Detect the white keys in C Major for same pattern in current key
        printf( "%-2s ", key_notes[(note+49)/7][(60+note+i)%12] );
    }
    printf("\r\n");
  }
  printf("\n\rTo set the key signature play the major or minor chord of the same name.\r\n");
  printf("Play the root note above♯ or below♭ middle C to select keys with 5-7♯ or 5-7♭\r\n");
}

int RotateOctaveByN ( int pattern , int n ){
  while ( n < 0 ) n += 12;
  if ( n >= 12 ) n = n%12;
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
const char * ScaleDegree(int note,int chord,int key_note,int key_is_minor){

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
  const char * roman_major [] = { "I","ii" ,"iii","IV","V","vi","vii°" };
  const char * roman_minor [] = { "i","ii°","III","iv","v","VI","VII" };

  int n = key_note; // transpose the playing chord to cmajor
  if ( key_is_minor ) n-=9; // Minor keys are 9 halfsteps from their relative major
  int transposed_chord = RotateOctaveByN ( chord , n ); 
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


const int Major      =    1;
const int minor      =    2;
const int lowest     =    4;
const int new_key    =    8;
const int log_toggle = 0x10;
// chord Bitmaps 12 bits one octave C=0x1, C♯=0x2,D=0x4,D♯=0x8 ... B=0x800
const struct { int notes;char *name;int flags; } chord_defs[] = {
 { 0x091,"   Major",      Major }, // C E  G
 { 0x891,"⁷  Major⁷",     Major }, // C E  G  B
 { 0x811,"⁷  Major ⁷",    Major }, // C E     B
 { 0x491,"dom⁷ dominant⁷",Major }, // C E  G  B♭
 { 0x411,"dom⁷ dominant⁷",Major }, // C E     B♭ no 5th
 { 0x111,"⁺  augmented",  lowest}, // C E  G♯
 { 0x089,"m  minor",      minor }, // C E♭ G
 { 0x489,"m⁷ minor⁷",     minor }, // C E♭ G  B♭
 { 0x049,"°  diminished", minor }, // C E♭ G♭
 { 0x249,"°⁷ diminished⁷",lowest}, // C E♭ G♭ B♭♭ minor seventh flat five?
 { 0x085,"sus² suspended²",0    }, // CD  G      no 3rd *** beware inversions and naming
 { 0x0A1,"sus⁵ suspended⁵",0    }, // C  FG      no 3rd *** beware inversions and naming
 { 0x4a5,"9sus4 dominant9th",0  }, // C–F–G–B♭–D
 { 0xb01,"Log toggle", log_toggle},// A♭ A B C
 { 0xc01,"Play Major or minor chord to set new key",new_key}, // B♭ B C
};
const int NUM_CHORD_DEFS = 15;

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

void chord_analyser( int note, int velocity, int channel , int on ) {

  const char *off_on[] = { "Off","On" };
  const char *major_minor[] = { "   Major","m  minor",""};
  const int midi_middle_c = 60;
  const char VT100_CLEAR[] = "\x1B[2J";
//const char VT100_CLEAR_EOL[] = "\x1B[0K";
//const char VT100_ERASE_DOWN[] = "\x1B[J";
//const char VT100_CURSOR_00[] = "\x1B[0;0H";
//const char VT100_UNDERLINE[] = "\x1B[4m";
//const char VT100_NO_UNDERLINE[] = "\x1B[0m"; 

  const int MAX_INT = 0x7fffffff;
  const int KEY_UNKNOWN = 2;
#define NUM_NOTES 128
  int notes = 0; // one octave of notes 2^0 == C, 2^1 == C♯...
  static int lowest_note = MAX_INT;
  static int log_enable = 0;
  static int key_note = 0;
  static int key_is_minor = -1 ;
  static int num_sharps_flats = 0;
  static int keyboard_image[NUM_NOTES];
  
  // make an image of all active keyboard notes
  if ( key_is_minor == -1 ) {
    for ( int i = 0 ; i < NUM_NOTES ; i++ ) keyboard_image[i] = 0;
    key_is_minor = KEY_UNKNOWN;
  }
  notes = 0; // collapse all the active keyboard notes into a single octave called notes
  if ( note > 0 && note < NUM_NOTES ) {
    keyboard_image[note] = on;
    for ( int i = 0 ; i < NUM_NOTES ; i++ ) {
      if ( keyboard_image[i] ) {
        notes |= ( 1 << ( i % 12) ) ;
      }
    }
    if ( on ) {
      if ( lowest_note > note )
      lowest_note = note;
    } else {
      if ( lowest_note == note )
        lowest_note = MAX_INT;
    }
  }
  /*
  if ( on ) {
    notes |= 1<< ( note % 12 );
    if ( lowest_note > note )
      lowest_note = note;
  } else {
    notes &= 0xFFF ^ ( 1<< ( note % 12 ) ) ;
    if ( lowest_note == note )
      lowest_note = MAX_INT;
  } */
  
  char chord_msg[80] = {""};
  int chord = notes;
  for ( int i = 0 ; i < 12 ; i++ ) {
    for ( int chord_id =0 ; chord_id < NUM_CHORD_DEFS ; chord_id++ ) {
      if ( notes == chord_defs[chord_id].notes ) {
        if ( chord_defs[chord_id].flags & new_key ) {
          if ( lowest_note > 91 ) { // Only notes at top of keyboard set key
             sprintf (chord_msg ,"Play Major or minor chord to set new key");
             key_is_minor = KEY_UNKNOWN; key_note = 12;
           }
        } else if (chord_defs[chord_id].flags & log_toggle ) {
           if ( lowest_note > 91 ) {
              log_enable ^= 1; sprintf ( chord_msg, "Log = %s",off_on[log_enable] );
            }
        } else {
           if ( ( key_is_minor == KEY_UNKNOWN ) &&
               ((chord_defs[chord_id].flags&Major) ||(chord_defs[chord_id].flags&minor)) ) {
               key_note = lowest_note;
               key_is_minor = 0;
               if ( chord_defs[chord_id].flags&minor ) {
                 key_is_minor = 1;
               }
               num_sharps_flats = key_sf_index[(key_note - key_is_minor*9)%12];
               if ( num_sharps_flats <= -5 )
                 if ( lowest_note >= midi_middle_c )
                   num_sharps_flats += 12; // User select 5-7♯ Key B, F♯, C♯
           }
           int note_id = i;
           if ( chord_defs[chord_id].flags&lowest )
             note_id = lowest_note%12;
           const char * scale_degree = "";
           if ( (chord_defs[chord_id].flags&Major) || (chord_defs[chord_id].flags&minor) )
             scale_degree = ScaleDegree( i,chord,key_note,key_is_minor);
           sprintf ( chord_msg ,"%s%s %s",
              key_notes[num_sharps_flats+7][note_id],chord_defs[chord_id].name,scale_degree );
        }
      }
    }
    notes = RotateOctaveByN(notes,1);
  }
  if ( log_enable ) {
    printf( "Midi note=%d, mask=%03x notes=%03x %s\r\n", note, 1 << note % 12, notes, chord_msg );
  } else {
    if ( notes ) {
        printf("\r%s  Chord:  %s\r", VT100_CLEAR, chord_msg );
    } else {
      if ( key_is_minor == KEY_UNKNOWN ) {
        showKeys();
      } else {
        printf("\r %s Key:  %s%s\r",
          key_sf[num_sharps_flats+7], key_notes[num_sharps_flats+7][ key_note % 12 ] , major_minor[key_is_minor]);
      } 
    }
  }
  fflush(stdout);
}

