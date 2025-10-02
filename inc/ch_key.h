#ifndef _CH_KEY_H
#define _CH_KEY_H

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "ch_key_config.h"

#define REGK(KEY)           (1 << (KEY + 16))
#define REGKEYEVTS(EVTS)    (EVTS)
#define KEY2MSG(KEY, EVT)   ((eventflags_t)(EVT | REGK(KEY)))
#define ISKEY(KEY, MSG)     (REGK(KEY) & MSG)
#define ISKEYEVT(EVT, MSG)  (EVT & MSG)


typedef enum
{
    KEY_ACTIVE_LOW = 0,
    KEY_ACTIVE_HIGH = 1
} TKeyActiveLvl;


enum KeyPullResistor
{
    PULL_DISABLE = 0,
    PULL_ENABLE = 1
};


typedef enum 
{
    KEY_IDLE,
    KEY_WAIT_HOLD,
    KEY_WAIT_IDLE
}TKeyState;


enum KeyEvent
{
    KEY_EVENT_CLICK       = 1,
    KEY_EVENT_HOLD        = 2,
    KEY_EVENT_HOLD_REPEAT = 4,
    KEY_EVENT_HOLD_ON_PWR_UP = 16384,
    KEY_EVENT_RELEASE        = 32768

    // Up to 16 events 
};


typedef struct{
    const uint8_t id;
    const TKeyActiveLvl active_level;
    const ioportid_t port;
    const iopadid_t pad;
    const uint8_t pull_resistor_en;

    volatile uint8_t pad_last_state;

    uint8_t state;
    volatile uint8_t timeout;

    virtual_timer_t debounce_vt;
    virtual_timer_t hold_vt;

} TKey;



// Global Event Source
extern event_source_t key_events;


void ch_key_init();


#ifdef __cplusplus
}
#endif


#endif // _CH_KEY_H