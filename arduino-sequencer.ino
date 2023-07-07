#define TEST 1

// @ Todo: maybe use a mega instead for more digital I/O during development?
const int pin_preamp = 2;
const int pin_coax_relay_a = 3;
const int pin_coax_relay_b = 4;
const int pin_pa_ptt = 5;
const int pin_tx_inhibit = 6;
const int pin_rx = 7;
const int pin_tx = 9;
const int pin_txenable = 10;

typedef enum
{
    STATE_UNINITIALIZED,
    STATE_RX,
    STATE_TRANSITION,
    STATE_TX,
    STATE_NUM_STATES
} states_t;

states_t state = STATE_UNINITIALIZED;

struct s_transitions
{
    states_t up;
    states_t down;
};

struct s_transitions state_transitions[STATE_NUM_STATES] = 
{
    // STATE_UNINITIALIZED can only go to STATE_RX or remain STATE_UNINITIALIZED
    {STATE_UNINITIALIZED, STATE_RX},

    // STATE_RX can only go to STATE_TRANSITION or remain STATE_RX
    {STATE_RX, STATE_TRANSITION},

    // STATE_TRANSITION can go either to STATE_RX or STATE_TX
    {STATE_RX, STATE_TX},
    
    // STATE_TX can only go to STATE_TRANSITION or remain STATE_TX
    {STATE_TRANSITION, STATE_TX}
};

typedef enum
{
    LEVEL_UNINITIALIZED,
    LEVEL_RX,
    LEVEL_PREAMP,
    LEVEL_COAX_RELAY_A,
    LEVEL_COAX_RELAY_B,
    LEVEL_PA_PTT,
    LEVEL_TX_INHIBIT,
    LEVEL_TX,
    LEVEL_NUM_LEVELS
} levels_t;

levels_t level = LEVEL_UNINITIALIZED;

struct l_transitions
{
    levels_t down;
    levels_t up;
};

struct l_transitions level_transitions[LEVEL_NUM_LEVELS] = 
{
    // LEVEL_UNINITIALIZED can only go to LEVEL_RX or stay LEVEL_UNINITIALIZED
    {LEVEL_UNINITIALIZED, LEVEL_RX},
    
    // LEVEL_RX, can only go to LEVEL_PREAMP or stay at LEVEL_RX
    {LEVEL_RX, LEVEL_PREAMP},
    
    // LEVEL_PREAMP can go both ways
    {LEVEL_RX, LEVEL_COAX_RELAY_A},
    
    // LEVEL_COAX_RELAY_A can go both ways
    {LEVEL_PREAMP, LEVEL_COAX_RELAY_B},
    
    // LEVEL_COAX_RELAY_B can go both ways
    {LEVEL_COAX_RELAY_A, LEVEL_PA_PTT},
    
    // LEVEL_PA_PTT can go both ways
    {LEVEL_COAX_RELAY_B, LEVEL_TX_INHIBIT},
    
    // LEVEL_TX_INHIBIT can go both ways
    {LEVEL_PA_PTT, LEVEL_TX},
    
    // LEVEL_TX can only go to LEVEL_TX_INHIBIT or stay at LEVEL_TX
    {LEVEL_TX_INHIBIT, LEVEL_TX}
};

unsigned long delays_us[LEVEL_NUM_LEVELS] =
{
    0,         // LEVEL_UNINITIALIZED
    1000000,   // LEVEL_RX
    1000000,   // LEVEL_PREAMP
    1000000,   // LEVEL_COAX_RELAY_A
    1000000,   // LEVEL_COAX_RELAY_B
    1000000,   // LEVEL_PA_PTT
    1000000,   // LEVEL_TX_INHIBIT
    1000000    // LEVEL_TX
               // LEVEL_NUM_LEVELS
};

unsigned long countdown_us = delays_us[LEVEL_UNINITIALIZED];

unsigned long micros_previous_loop = 0;

void Transition(bool to_tx)
{
    // We move "up" in levels towards TX and "down" towards RX
    char level_modifier = to_tx ? 1 : -1;

    if(to_tx)
    {
        Serial.print("Going DOWN from level ");
        Serial.println(level);
        countdown_us = delays_us[level];
        digitalWrite(pin_rx, 0);
        if(level_transitions[level].up == LEVEL_TX)
        {
            state = STATE_TX;
            digitalWrite(pin_tx, 1);
            Serial.println("Entered full TX");
        }
        else
        {
            level = level_transitions[level].up;
        }
    }
    else
    {
        Serial.print("Going DOWN from level ");
        Serial.println(level);
        countdown_us = delays_us[level];
        digitalWrite(pin_tx, 0);
        if(level_transitions[level].down == LEVEL_RX)
        {
            state = STATE_RX;
            digitalWrite(pin_rx, 1);
            Serial.println("Entered full RX");
        }
        else
        {
            level = level_transitions[level].down;
        }
    }
}

void Initialize()
{

    Serial.println("Initializing...");

    // Set each pin as output and to a safe-ish state, then set state to RX.
    pinMode(pin_tx_inhibit, OUTPUT);
    digitalWrite(pin_tx_inhibit, 1);
    Serial.println("TX Inhibit...");
    delay(1000 * delays_us[LEVEL_TX_INHIBIT]);

    pinMode(pin_pa_ptt, OUTPUT);
    digitalWrite(pin_pa_ptt, 0);
    Serial.println("PA PTT...");
    delay(1000 * delays_us[LEVEL_PA_PTT]);

    pinMode(pin_coax_relay_b, OUTPUT);
    digitalWrite(pin_coax_relay_b, 0);
    Serial.println("Coax relay B...");
    delay(1000 * delays_us[LEVEL_COAX_RELAY_B]);

    pinMode(pin_coax_relay_a, OUTPUT);
    digitalWrite(pin_coax_relay_a, 0);
    Serial.println("Coax relay A...");
    delay(1000 * delays_us[LEVEL_COAX_RELAY_A]);

    pinMode(pin_preamp, OUTPUT);
    digitalWrite(pin_preamp, 0);
    Serial.println("Pre-amplifier...");
    delay(1000 * delays_us[LEVEL_PREAMP]);

    pinMode(pin_tx, OUTPUT);
    digitalWrite(pin_tx, 0);
    
    pinMode(pin_rx, OUTPUT);
    digitalWrite(pin_rx, 1);
    
    pinMode(pin_txenable, INPUT);

    state = STATE_RX;
    level = LEVEL_RX;

    Serial.println("Entered RX state!");
}

unsigned long getDeltaMicros()
{
    unsigned long delta_micros = 0;
    unsigned long this_micros = micros();

    // Check for wrap-around of micros counter
    if(this_micros < micros_previous_loop)
    {
        // Add the remainder until wrap-around for previous loop
        delta_micros = (unsigned long)-1 - micros_previous_loop;

        // Add this loop's micros
        delta_micros += this_micros;
    }
    else
    {
        delta_micros = this_micros - micros_previous_loop;
    }

    micros_previous_loop = this_micros;

    return delta_micros;
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    Initialize();
    //delay(500);
}

void loop()
{
    bool update = false;
    bool txenable = false;
    unsigned long delta_micros = getDeltaMicros();
    if(delta_micros >= countdown_us)
    {
        txenable = digitalRead(pin_txenable);
        Serial.print("Time's up! TXEnable: ");
        Serial.println(txenable);
        Transition(txenable);
    }
    else
    {
        countdown_us -= delta_micros;
    }

}