#define TEST 1

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



void Transition(bool to_tx)
{
    // We move "up" in levels towards TX and "down" towards RX
    char level_modifier = to_tx ? 1 : -1;

    
}

void Initialize()
{


}




void setup()
{
    Serial.begin(115200);
    
}


void loop()
{
    millis();

}