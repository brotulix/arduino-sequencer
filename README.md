# arduino-sequencer
A simple Arduino-based sequencer to turn masthead preamplifier and power amplifier on and off in sequence when beginning and ending transmission, as well as inhibiting transmit on a Yaesu radio (FT-991 or FT-817).

Additions to come later will be switching coax relays to allow co-listening with an SDR during receive period.

The same sequencer is intended for 144 and 432 MHz operation, for which I own power amplifiers with opposite PTT controls; one will activate when PTT is pulled high, one will activate when PTT is pulled low. To solve this with the least hassle, I'll control the PTT using a photo diode surrounded by four diodes, and select active high or active low using a toggle switch:

`<Insert schematic>`

I also want to add a potensiometer to set the delay for each stage. First design will use the same delay for both entering and leaving transmit state, but I may want to allow for faster delays going one direction or the other later?

Whether I'll use an interrupt to trigger on flank or simply evaluate high/low state in a loop is TBD.

Maybe some form of table-based state machine to easily add stages and transitioning to the right next state?

`<Insert state diagram>`

