MidiManager
===========

MidiManager is a VST plugin that allows you to right scipts using [Pure programming language] (http://purelang.bitbucket.org/). The
plugin is written using [Juce Framework] (http://www.juce.com/). MidiManager is currenlty only available for Mac OS X. The plugin requires
Pure installed on your system, which can be simply installed using brew:

```
brew intall pure
```

The project is still in a very early stage and currently has a very basic interface that allows you to load a pure script,
 and the plugin will then load and compile the script and execute it.

The plugin currently supports only MIDI input and output and will look for a function called ```processMidiBuffer``` that accepts a list 
of tuples, where each tuple on the form ```(event, position)``` representing the MIDI event and the position of this event in the buffer. There
is a Pure script that contains some helper functions to assist extracting information about the current MIDI event as well as constructing
new events, and it will be available to the plugin using the Pure ```using midi;``` statement. The full script can be found [here](https://raw.githubusercontent.com/hkarim/MidiManager/master/Pure/midi.pure)

Examples:
---------

Here's a pure script that will simply echo every event it gets:

```
processMidiBuffer buf = buf;
```


The following example turns any note into a major chord:

```
using midi;

let major = [0,4,7];

processMidiBuffer buf =
  result with
    process m = 
      case to_note_t m of
        (NoteOn c n v) = map (\i -> (note_on_message c (n+i) v (position m))) major;
        (NoteOff c n)  = map (\i -> (note_off_message c (n+i) (position m))) major;
        _              = [m];
      end;
    result = to_tuple_list $ catmap process $ to_message_t buf;
  end;

```

In this example, the result rule turns the input into a list of type ```message_t``` which holds the MIDI 
event along with its position in the buffer. Then output is sent to the process rule to create messages and finally
the output is turned again into a tuple format using the ```to_tuple_list``` function. The rules: ```to_message_t, to_tuple_list, to_note_t```
are all made available through the imported ```using midi``` file.



