#define TEST 1

// @ Todo: maybe use a mega instead for more digital I/O during development?
const int pin_preamp = 2;
const int pin_coax_relay_a = 3;
const int pin_coax_relay_b = 4;
const int pin_pa_ptt = 5;
const int pin_tx_inhibit = 6;
const int pin_rx = 7;
const int pin_tx = 9;

typedef enum
{
    STATE_UNINITIALIZED,
    STATE_RX,
    STATE_TRANSITION,
    STATE_TX,
    STATE_NUM_STATES
} states_t;

states_t state = STATE_UNINITIALIZED;

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

unsigned long delays_ms[LEVEL_NUM_LEVELS] =
{
    0,      // LEVEL_UNINITIALIZED
    0,      // LEVEL_RX
    1000,   // LEVEL_PREAMP
    1000,   // LEVEL_COAX_RELAY_A
    1000,   // LEVEL_COAX_RELAY_B
    1000,   // LEVEL_PA_PTT
    1000,   // LEVEL_TX_INHIBIT
    0       // LEVEL_TX
            // LEVEL_NUM_LEVELS
};

void Transition(bool to_tx)
{
    // We move "up" in levels towards TX and "down" towards RX
    char level_modifier = to_tx ? 1 : -1;
    pinMode(pin_tx_inhibit, OUTPUT);
    digitalWrite(pin_tx_inhibit, 1);

    delay(delays_ms[LEVEL_TX_INHIBIT]);

    
    
}

void Initialize()
{

    // Set each pin as output and to a safe-ish state, then set state to RX.



}




void setup()
{
    Serial.begin(115200);
    
}


void loop()
{
    millis();

}