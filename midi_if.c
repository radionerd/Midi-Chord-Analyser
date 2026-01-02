/* Adapted from seqdemo.c by Matthias Nagorni */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include "chord_analyser.h"

snd_seq_t *open_seq();
//void midi_action(snd_seq_t *seq_handle);

snd_seq_t *open_seq() {

  snd_seq_t *seq_handle;
  int portid;

  if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0) < 0) {
    fprintf(stderr, "Error opening ALSA ChordAnalyser.\n");
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, "ChordAnalyser");
  if ((portid = snd_seq_create_simple_port(seq_handle, "ChordAnalyser",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating analyser port.\n");
    exit(1);
  }
  return(seq_handle);
}

void midi_action(snd_seq_t *seq_handle, int flush ) {

  snd_seq_event_t *ev;

  do {
    snd_seq_event_input(seq_handle, &ev);
    if ( ( ev->type == SND_SEQ_EVENT_NOTEOFF ) || ( ev->type == SND_SEQ_EVENT_NOTEON ) ) { // ref /usr/install/alsa/seq_event.h
        int note = ev->data.note.note ;
        int velocity = ev->data.note.velocity ;
        int channel = ev->data.note.channel ;
        int on = ( ev->type == SND_SEQ_EVENT_NOTEON );
        if ( ! flush )
          chord_analyser( note, velocity, channel, on );
    }
    if ( ev->type == SND_SEQ_EVENT_CONTROLLER ) {
        /* Ref /usr/include/alsa/seqmid.h
    	//((ev)->type = SND_SEQ_EVENT_CONTROLLER,\
	// snd_seq_ev_set_fixed(ev),\
	// (ev)->data.control.channel = (ch),\
	// (ev)->data.control.param = (cc),\
	// (ev)->data.control.value = (val)) */
	#define PEDAL_SUSTAIN 64 // RH Piano Pedal 0-127
	#define PEDAL_SOSTENUTO 66 // Middle 0 or 127
	#define PEDAL_SOFT 67 // LH Piano pedal 0-127	
	switch (ev->data.control.param ) { // Ref /usr/include/alsa/seqmid.h
	  case PEDAL_SUSTAIN :
	    setArpegioMode( ev->data.control.value>63 );
	    break;
	  case PEDAL_SOSTENUTO :
	    if ( ev->data.control.value > 63 )
	      listChords();
	    break;
	  case PEDAL_SOFT :
	    if ( ev->data.control.value == 127 )
	      showKeys();
	    break;
	  default:  
            printf("\r\nMIDI SND_SEQ_EVENT_CONTROLLER ev->data.control .channel=%d .param=%d .value=%d \r\n", (ev)->data.control.channel, (ev)->data.control.param, (ev)->data.control.value );
        }       
    }
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
}

char *help = { "\
USAGE: ChordAnalyser [OPTIONS]\r\n\
Options\r\n\
-e              List detected enharmonic equivalents (for testing)\r\n\
-h --help	This Help\r\n\
-i              Request default input from midi through 14:0\r\n\
-i  src:port    Request input from src:port eg -i 28:0\r\n\
-l              List supported chords\\r\n\
\r\n\
Midi Chord Analyser\r\n\
-------------------\r\n\
\r\n\
Common block chords are displayed with their root note, quality and scale degree.\r\n\
Root note: eg C,C♯,E,E♭ ...\r\n\
Quality: eg Major, m Minor, ⁺ Augmented, ° Diminished, ⁷ Seventh ...\r\n\
Scale Degree: Roman Numeral eg I IV V (major) i iv v (minor)\r\n\
\r\n\
To make sure that midi events are sent to the chord analyser:\r\n\
Connect midi keyboard output to chord analyser input and synth\r\n\
eg: $ qysnth & # start sound synth\r\n\
eg: % qjackctl & # press the graph button to view and make connections\r\n\
Or use the ChordAnalyser -i option to connect to the required midi source.\r\n\
If all else fails run midisnoop to view midi events.\r\n\
\n\
List the supported chords by playing the highest notes G# + B + C \r\n\
Set the Key Signature by playing the highest notes A♯ + B + C followed by the new scale chord\r\n\
eg C Major C+E+G\r\n\
"};

int main(int argc, char *argv[]) {
  int src_client = 24; // My Kawai piano input
  int src_port = 0;
  int flush = 1 ;
  int last_connect = 0; // Suppress connected message on first start
  struct timespec ts;
  uint64_t seconds = 0;
  
  // check formal parameters
  for ( int i = 1 ; i < argc ; i++ ) {
    if ( argv[i][0] == '-' ) {
      switch ( argv[i][1] ) {
        case 'e' :
          printf( "Enharmonic Equivalents List\r\n");
          listEnharmonicEquivalents( );
          return 0;
        case 'i' :
          src_client = 14; // midi through
          if ( ( argc > i+1 ) && ( argv[i+1][0] != '-' ) ) {
            sscanf(argv[i+1],"%d:%d",&src_client,&src_port);
            i++;            
          }
          break;
        case 'l' :
          listChords ( );
          return 0;
        default :
          printf("%s", help);
          return 0;          
      }      
    }
  }

  snd_seq_t *seq_handle;
  int npfd;
  struct pollfd *pfd;
    
  seq_handle = open_seq();
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
  //printf("%s", help);  
  showKeys( );
  while (1) {
    // Check configured midi connection every second
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    if ( seconds != ts.tv_sec ) {
      seconds = ts.tv_sec ;
      if ( src_client ) {
        // -22 not found, -16 already connected, 0 success
        int connect = snd_seq_connect_from(seq_handle, 0, src_client, src_port);
        if ( last_connect != connect ) {
          last_connect = connect;
          if ( connect == 0 ) {
            printf("\r\nMidi Device %d:%d Connected\r\n",src_client,src_port);
          }
          if ( connect == -22 ) {
            printf("\r\nMidi Device %d:%d Not Connected\r\n",src_client,src_port);
          }
        }
      }
    }
    // connect midi events to app
    if (poll(pfd, npfd, 100) > 0) { // poll 100 times then return
      midi_action( seq_handle,flush );
    } 
    flush = 0; // flush extraneous input from before connection
  }
}

