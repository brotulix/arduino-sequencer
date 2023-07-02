# arduino-sequencer
A simple Arduino-based sequencer to turn masthead preamplifier and power amplifier on and off in sequence when beginning and ending transmission, as well as inhibiting transmit on a Yaesu radio (FT-991 or FT-817).

Additions will be switching coax relays to allow co-listening with an SDR during receive period.

## PTT

The same sequencer is intended for 144 and 432 MHz operation, for which I own power amplifiers with opposite PTT controls; one will activate when PTT is pulled high, one will activate when PTT is pulled low. To solve this with the least hassle, I'll control the PTT using a photo diode surrounded by four diodes, and select active high or active low using a toggle switch:

![Simplified PA PTT circuit](/doc/KiCad_Detail_Simplified_PTT.png)

Here, J1 (an RCA socket) connects to the PTT input of either KLM electronics VHF PA or Microwave Modules MML 432/100 UHF PA. SW1 is DPDT and will flip voltages for both the opto-coupler and the pull up/down.

# State machine
So, a state machine may be a useful method to keep track of where one is in the sequence of events going from RX -> TX or TX -> RX.

Maybe some form of table-based state machine to easily add stages and transitioning to the right next state?

![Basic state machine timing diagram](/doc/Wavedrom_Basic_Diagram_white.png)
`Wavedrom_Basic_Diagram.png`

Whether I'll use an interrupt to trigger on flank or simply evaluate high/low state in a loop is TBD.

## Initialization / Power-up
The system starts up in an unknown state as of yet. I think it should sets some signals to a safe-ish state immediately, then progresses through some sequence of events to reach a true RX state.

Probably the most important detail is that the PA should be switched on after the sequencer.

> Should the radio be switched on after the sequencer, too, to make sure TX inhibit is on from the start?

![Timing diagram for state transition from TX to RX](/doc/Wavedrom_Detail_Init_white.png)

## RX -> TX
The sequence when going from RX to TX is something like this:

![Generic timing diagram for state transition from TX to RX](/doc/Wavedrom_Basic_Diagram_To_TX_white.png)

### RX -> TX -> RX
The special case when TX goes low before during the transition from RX to TX should go something like this:

![Generic timing diagram for state transition from RX, partly to TX, then back to RX](/doc/Wavedrom_Detail_Diagram_RX-TX-RX_white.png)

See *dwell time*.

## TX -> RX
Transitioning from TX back to RX:

![Timing diagram for state transition from TX to RX](/doc/Wavedrom_Basic_Diagram_To_RX_white.png)

### TX -> RX -> TX
The special case where one is transmitting, but then experience a glitch in TX from radio which begins a transition to RX, but then returns to TX during the transition phase.

![Timing diagram for state transition from RX, partly to TX, then back to RX](/doc/Wavedrom_Detail_Diagram_TX-RX-TX_white.png)

See *dwell time*.

# Dwell time
> Add a potensiometer/encoder to set the delay for each stage? First design may use the same delay for both entering and leaving transmit state, but perhaps allow for faster delays going one direction or the other later?

Upon entering every state, set a minimum dwell time for that state to allow for energy to stabilize in the system (eg. DC bias for preamp; depending on coax length, you may want a certain minimum delay to allow the rising edge to reach full round-trip).

This may remove the need for a "inter-state" delay period, and depending on what the Radio TX signal is when entering (or exiting) the state determines which direction one goes. So, in principle, by toggling TX line fast enough, it may be possible to stay in the twilight zone between RX and TX indefinitely?