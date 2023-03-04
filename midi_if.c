/* Adapted from seqdemo.c by Matthias Nagorni */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include "chord_analyser.h"

snd_seq_t *open_seq();
void midi_action(snd_seq_t *seq_handle);

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

void midi_action(snd_seq_t *seq_handle) {

  snd_seq_event_t *ev;

  do {
    snd_seq_event_input(seq_handle, &ev);
    if ( ( ev->type == SND_SEQ_EVENT_NOTEOFF ) || ( ev->type == SND_SEQ_EVENT_NOTEON ) ) {
        int note = ev->data.note.note ;
        int velocity = ev->data.note.velocity ;
        int channel = ev->data.note.channel ;
        int on = ( ev->type == SND_SEQ_EVENT_NOTEON );
        chord_analyser( note, velocity, channel, on );
    }
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
}

char *help = { "\
Midi Chord Analyser\r\n\
-------------------\r\n\
\r\n\
Common chords are displayed with their root note, quality and scale degree\r\n\
Root note: eg C,C♯,E,E♭ ...\r\n\
Quality: eg Major, m Minor, ⁺ Augmented, ° Diminished, ⁷ Seventh ...\r\n\
Scale Degree: Roman Numeral eg I IV V (major) i iv v (minor)\r\n\
Set the Key Signature by playing the highest notes A♯BC together followed by the new scale chord\r\n\
eg C Major C+E+G\r\n\
\r\n\
To make sure that midi events are sent to the chord analyser open 3 terminal windows:\r\n\
Connect midi keyboard output to chord analyser input and synth\r\n\
eg: $ fluidsynth -p Synth /usr/share/sounds/sf2/FluidR3_GM.sf2 # start sound synth\r\n\
eg: $ aconnect -l # discover the identity of midi devices available\r\n\
eg: $ aconnect 28 128 ; aconnect 28 129 ; aconnectgui\r\n\
"};


int main(int argc, char *argv[]) {

  printf("%s%s","\x1B[2J",help);
  snd_seq_t *seq_handle;
  int npfd;
  struct pollfd *pfd;
    
  seq_handle = open_seq();
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
  while (1) {
    if (poll(pfd, npfd, 100000) > 0) {
      midi_action(seq_handle);
    }  
  }
}
