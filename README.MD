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
~~~
  Chord:  B♭   Major VI
~~~

The key signature is displayed when no chords are playing.
~~~
 1♭ Key:  Dm   minor
~~~

### Setting the Key Signature

Set the Key Signature by playing far right hand notes A♯BC together followed by the new scale chord.

Example Key Signature Prompt:
~~~
           Chord Analyser


KSig  Major RelMinor  Diatonic Scale
7♭      C♭      A♭    C♭ D♭ E♭ F♭ G♭ A♭ B♭ 
6♭      G♭      E♭    G♭ A♭ B♭ C♭ D♭ E♭ F  
5♭      D♭      B♭    D♭ E♭ F  G♭ A♭ B♭ C  
4♭      A♭      F     A♭ B♭ C  D♭ E♭ F  G  
3♭      E♭      C     E♭ F  G  A♭ B♭ C  D  
2♭      B♭      G     B♭ C  D  E♭ F  G  A  
1♭      F       D     F  G  A  B♭ C  D  E  
        C       A     C  D  E  F  G  A  B  
1♯      G       E     G  A  B  C  D  E  F♯ 
2♯      D       B     D  E  F♯ G  A  B  C♯ 
3♯      A       F♯    A  B  C♯ D  E  F♯ G♯ 
4♯      E       C♯    E  F♯ G♯ A  B  C♯ D♯ 
5♯      B       G♯    B  C♯ D♯ E  F♯ G♯ A♯ 
6♯      F♯      D♯    F♯ G♯ A♯ B  C♯ D♯ E♯ 
7♯      C♯      A♯    C♯ D♯ E♯ F♯ G♯ A♯ B♯ 

To set the key signature play the major or minor chord of the same name.
Play the root note above♯ or below♭ middle C to select keys with 5-7♯ or 5-7♭

~~~

eg: To set Dminor key. On the midi keyboard play top A♯+B+C followed by D+F+A

Ref: https://en.wikipedia.org/wiki/Chord_notation

The app uses Linux ALSA. Jack compatibility may be added later.

## Building
To compile the app under linux download all the files and compile using make.
~~~
 $ make
~~~

## Using

*** Start a sound synthesizer 
~~~
 $ fluidsynth -p Synth /usr/share/sounds/sf2/FluidR3_GM.sf2
~~~

### Start the app
~~~
 $ ./ChordAnalyser
~~~

### Connecting outputs to inputs

Run aconnect to connect a midi keyboard output to the chord analyser input and synth input
~~~
 $ aconnect -l # discover the identity of midi devices available
~~~
### Auto connect and view
~~~
 $ aconnect 28 128 ; aconnect 28 129 ; aconnectgui
~~~

<img src="aconnectgui.png" style="height: 272px; width:302px;"/>

N.B. aconnectgui has the title 'ALSA Sequencer'

## Errors, omissions and improvements

Please contact the author

