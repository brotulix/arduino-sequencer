# arduino-sequencer
A simple Arduino-based sequencer to turn masthead preamplifier and power amplifier on and off in sequence when beginning and ending transmission, as well as inhibiting transmit on a Yaesu radio (FT-991 or FT-817).

Additions to come later will be switching coax relays to allow co-listening with an SDR during receive period.

The same sequencer is intended for 144 and 432 MHz operation, for which I own power amplifiers with opposite PTT controls; one will activate when PTT is pulled high, one will activate when PTT is pulled low. To solve this with the least hassle, I'll control the PTT using a photo diode surrounded by four diodes, and select active high or active low using a toggle switch:

![Simplified PA PTT circuit](/doc/KiCad_Detail_Simplified_PTT.png)

I also want to add a potensiometer to set the delay for each stage. First design will use the same delay for both entering and leaving transmit state, but I may want to allow for faster delays going one direction or the other later?

Whether I'll use an interrupt to trigger on flank or simply evaluate high/low state in a loop is TBD.

Maybe some form of table-based state machine to easily add stages and transitioning to the right next state?

![Basic state machine timing diagram](/doc/Wavedrom_Basic_Diagram_white.png)
`Wavedrom_Basic_Diagram.png`

# State machine
So, a state machine may be a useful method to keep track of where one is in the sequence of events going from RX -> TX or TX -> RX.

## Initialization / Power-up
![Timing diagram for state transition from TX to RX](/doc/Wavedrom_Detail_Init_white.png)

## RX -> TX
![Timing diagram for state transition from TX to RX](/doc/Wavedrom_Basic_Diagram_To_TX_white.png)

### RX -> TX -> RX

![Timing diagram for state transition from RX, partly to TX, then back to RX](/doc/Wavedrom_Detail_Diagram_RX-TX-RX_white.png)

## TX -> RX
![Timing diagram for state transition from TX to RX](/doc/Wavedrom_Basic_Diagram_To_RX_white.png)

### TX -> RX -> TX
This is where one is transmitting, but experience a glitch in TX from radio which begins a transition to RX, but aborts somewhere and returns to TX.

![Timing diagram for state transition from RX, partly to TX, then back to RX](/doc/Wavedrom_Detail_Diagram_TX-RX-TX_white.png)

# Dwell time
Upon entering every state, set a minimum dwell time for that state to allow for energy to stabilize in the system (eg. DC bias for preamp; depending on coax length, you want a certain minimum to allow the rising edge to reach full round-trip).

This may remove the need for a "inter-state" delay period, and depending on what the Radio TX signal is when entering (or exiting) the state determines which direction one goes. So, in principle, by toggling TX line fast enough, it may be possible to stay in the twilight zone between RX and TX indefinitely?