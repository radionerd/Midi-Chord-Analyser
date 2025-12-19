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
    if ( ( ev->type == SND_SEQ_EVENT_NOTEOFF ) || ( ev->type == SND_SEQ_EVENT_NOTEON ) ) {
        int note = ev->data.note.note ;
        int velocity = ev->data.note.velocity ;
        int channel = ev->data.note.channel ;
        int on = ( ev->type == SND_SEQ_EVENT_NOTEON );
        if ( ! flush )
          chord_analyser( note, velocity, channel, on );
    }
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
}

char *help = { "\
USAGE: ChordAnalyser [OPTIONS]\r\n\
Options\r\n\
-h --help	This Help\r\n\
-i              Request default input from midi through 14:0\r\n\
-i  src:port    Request input from src:port eg -i 28:0\r\n\
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
Set the Key Signature by playing the highest notes A♯BC together followed by the new scale chord\r\n\
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
    if ( strncmp(argv[i],"-i",2 ) == 0 ) {
      src_client = 14 ; // Default Midi through
      if ( argc > i+1 ) {
        sscanf(argv[++i],"%d:%d",&src_client,&src_port);
      }
    } else {
      printf("%s", help);  
    }
  }

  snd_seq_t *seq_handle;
  int npfd;
  struct pollfd *pfd;
    
  seq_handle = open_seq();
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
  printf("%s", help);  
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
    flush = 0; // flush extraneeous input from before connection
  }
}

