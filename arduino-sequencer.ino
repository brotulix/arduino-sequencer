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

const unsigned long loop_interval_ms = 50;

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

struct l_properties
{
    levels_t index;
    unsigned long delay_us;
    int pin;
    struct l_transitions transitions;
};

struct l_properties levels[LEVEL_NUM_LEVELS] = {
//   level,                   delay, pin,                 {down transition,       up transition}
    {LEVEL_UNINITIALIZED,         0, -1,                  {LEVEL_RX,              LEVEL_RX}},
    {LEVEL_RX,               100000, -1,                  {LEVEL_RX,              LEVEL_PREAMP}},
    {LEVEL_PREAMP,           100000, pin_preamp,          {LEVEL_RX,              LEVEL_COAX_RELAY_A}},
    {LEVEL_COAX_RELAY_A,     100000, pin_coax_relay_a,    {LEVEL_PREAMP,          LEVEL_COAX_RELAY_B}},
    {LEVEL_COAX_RELAY_B,     100000, pin_coax_relay_b,    {LEVEL_COAX_RELAY_A,    LEVEL_PA_PTT}},
    {LEVEL_PA_PTT,           100000, pin_pa_ptt,          {LEVEL_COAX_RELAY_B,    LEVEL_TX_INHIBIT}},
    {LEVEL_TX_INHIBIT,       100000, pin_tx_inhibit,      {LEVEL_PA_PTT,          LEVEL_TX}},
    {LEVEL_TX,               100000, -1,                  {LEVEL_TX_INHIBIT,      LEVEL_TX}}
};

unsigned long countdown_us = 0; //levels[LEVEL_UNINITIALIZED].delay_us;

unsigned long micros_previous_loop = 0;

void setState(states_t new_state)
{

    if(state == new_state)
    {
        return;
    }

    Serial.print("state = ");
    Serial.print(state);
    Serial.print(", new_state = ");
    Serial.println(new_state);

    switch(new_state)
    {
        default:
        case STATE_RX:
        {
            if(state == STATE_UNINITIALIZED || state == STATE_TRANSITION)
            {
                digitalWrite(pin_rx, 1);
                digitalWrite(pin_tx, 0);
                state = new_state;
                Serial.println("Entered full RX");
            }
            break;
        }

        case STATE_TX:
        {
            if(state == STATE_TRANSITION)
            {
                digitalWrite(pin_rx, 0);
                digitalWrite(pin_tx, 1);
                state = new_state;
                Serial.println("Entered full TX");
            }
            break;
        }

        case STATE_TRANSITION:
        {
            if(state == STATE_RX || state == STATE_TX)
            {
                digitalWrite(pin_rx, 0);
                digitalWrite(pin_tx, 0);
                state = new_state;
                Serial.println("Entered transitioning state");
            }
            break;
        }
    }

    if(state != new_state)
    {
        Serial.print("Illegal state change? ");
        Serial.print(state);
        Serial.print(" -> ");
        Serial.println(new_state);
    }

}

void setCountdown(unsigned long us)
{
    if(us > 0)
    {
        Serial.print("Setting countdown: ");
        Serial.println(us);
    }
    countdown_us = us;
}

void transition(bool to_tx)
{
    // We move "up" in levels towards TX and "down" towards RX
    char level_modifier = to_tx ? 1 : -1;
    levels_t new_level = LEVEL_UNINITIALIZED;

    if(to_tx)
    {
        if(level == LEVEL_TX)
        {
            // We're already at the top
            return;
        }
        new_level = levels[level].transitions.up;
    }
    else
    {
        if(level == LEVEL_RX)
        {
            // We're already at the bottom
            return;
        }
        new_level = levels[level].transitions.down;
    }

    Serial.print("Transition: TX = ");
    Serial.print(to_tx);
    Serial.print(", level = ");
    Serial.print(level);
    Serial.print(", new_level = ");
    Serial.println(new_level);

    switch(new_level)
    {
        default:
        case LEVEL_RX:
        {
            if(level != LEVEL_UNINITIALIZED)
            {
                // Unset pin for previous level
                digitalWrite(levels[level].pin, 0);
            }
            
            // Set minimum delay to stay in RX before accepting new state.
            setCountdown(levels[level].delay_us);
            
            // Set level and state
            level = new_level;
            setState(STATE_RX);
            
            break;
        }
        case LEVEL_PREAMP:
        {
            if(to_tx)
            {
                setState(STATE_TRANSITION);
                digitalWrite(levels[new_level].pin, 1);
            }
            else
            {
                digitalWrite(levels[level].pin, 0);
            }
            setCountdown(levels[new_level].delay_us);
            level = new_level;
            break;
        }
        case LEVEL_COAX_RELAY_A:
        {
            if(to_tx)
            {
                digitalWrite(levels[new_level].pin, 1);
            }
            else
            {
                digitalWrite(levels[level].pin, 0);
            }
            setCountdown(levels[new_level].delay_us);
            level = new_level;
            break;
        }
        case LEVEL_COAX_RELAY_B:
        {
            if(to_tx)
            {
                digitalWrite(levels[new_level].pin, 1);
            }
            else
            {
                digitalWrite(levels[level].pin, 0);
            }
            setCountdown(levels[new_level].delay_us);
            level = new_level;
            break;
        }
        case LEVEL_PA_PTT:
        {
            if(to_tx)
            {
                digitalWrite(levels[new_level].pin, 1);
            }
            else
            {
                digitalWrite(levels[level].pin, 0);
            }
            setCountdown(levels[new_level].delay_us);
            level = new_level;
            break;
        }
        case LEVEL_TX_INHIBIT:
        {
            if(to_tx)
            {
                digitalWrite(levels[new_level].pin, 1);
            }
            else
            {
                setState(STATE_TRANSITION);
                digitalWrite(levels[level].pin, 0);
            }
            setCountdown(levels[new_level].delay_us);
            level = new_level;
            break;
        }
        case LEVEL_TX:
        {
            // Set minimum delay to stay in TX before accepting new state.
            setCountdown(levels[new_level].delay_us);
            
            // Set level and state
            level = new_level;
            setState(STATE_TX);
            
            break;
        }
    }

    Serial.print("Transitioned to level ");
    Serial.println(level);

}

void Initialize()
{

    Serial.println("Initializing...");

    // Set each pin as output and to a safe-ish state, then set state to RX.
    pinMode(levels[LEVEL_TX_INHIBIT].pin, OUTPUT);
    digitalWrite(levels[LEVEL_TX_INHIBIT].pin, 0);
    Serial.println("TX Inhibit...");
    delay(levels[LEVEL_TX_INHIBIT].delay_us / 1000);

    pinMode(levels[LEVEL_PA_PTT].pin, OUTPUT);
    digitalWrite(levels[LEVEL_PA_PTT].pin, 0);
    Serial.println("PA PTT...");
    delay(levels[LEVEL_PA_PTT].delay_us / 1000);

    pinMode(levels[LEVEL_COAX_RELAY_B].pin, OUTPUT);
    digitalWrite(levels[LEVEL_COAX_RELAY_B].pin, 0);
    Serial.println("Coax relay B...");
    delay(levels[LEVEL_COAX_RELAY_B].delay_us / 1000);

    pinMode(levels[LEVEL_COAX_RELAY_A].pin, OUTPUT);
    digitalWrite(levels[LEVEL_COAX_RELAY_A].pin, 0);
    Serial.println("Coax relay A...");
    delay(levels[LEVEL_COAX_RELAY_A].delay_us / 1000);

    pinMode(levels[LEVEL_PREAMP].pin, OUTPUT);
    digitalWrite(levels[LEVEL_PREAMP].pin, 0);
    Serial.println("Pre-amplifier...");
    delay(levels[LEVEL_PREAMP].delay_us / 1000);

    pinMode(pin_tx, OUTPUT);
    digitalWrite(pin_tx, 0);
    
    pinMode(pin_rx, OUTPUT);
    digitalWrite(pin_rx, 0);
    
    pinMode(pin_txenable, INPUT);

    transition(false);
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
    static bool first = true;
    bool txenable = false;
    
    unsigned long delta_micros = getDeltaMicros();
    // Don't count down if already at zero
    if(delta_micros && countdown_us)
    {
        if(countdown_us >= delta_micros)
        {
            countdown_us -= delta_micros;
            Serial.print(".");
            first = true;
        }
        else
        {
            Serial.print("Countdown reached near zero: ");
            Serial.println(countdown_us);
            setCountdown(0);
        }
    }
    
    if(countdown_us < (loop_interval_ms * 1000))
    {
        if(first)
        {
            Serial.println();
            first = false;
        }
        txenable = digitalRead(pin_txenable);
        //Serial.print("Time's up! TXEnable: ");
        //Serial.println(txenable);
        transition(txenable);
    }
    delay(loop_interval_ms);
}