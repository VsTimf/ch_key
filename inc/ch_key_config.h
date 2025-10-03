#ifndef _CH_KEY_CONFIG_H
#define _CH_KEY_CONFIG_H

/* This is simple Key handler library based on ChibiOs HAL for STM32 */

/* HOW TO USE */

/* 1. Define timing in milliseconds */
#define KEY_DEBOUNCE_MS                 25                  // Rise "KEY_CLICK" event after pin start front
#define KEY_HOLD_EVENT_MS               1500                // Rise "KEY_HOLD" event after holding key specified amount of time
#define KEY_HOLD_REPEAT_EVENT_MS        500                 // Rise "KEY_HOLD_REPEAT" event after "KEY_HOLD" event every specified amount of time when key is hold for long time

/* 2. Select used key events */
#define KEY_CLICK_EVENT_EN              TRUE
#define KEY_HOLD_EVENT_EN               TRUE
#define KEY_HOLD_REPEAT_EVENT_EN        FALSE
#define KEY_RELEASE_EVENT_EN            FALSE
#define KEY_HOLD_ON_PWR_UP_EVENT_EN     FALSE


/* 3. Enumerate up to 16 KEYS you plan to use. Last enumeration value should always be KEY_QNT */
enum KEYS {
    KEY_SEL,
    KEY_UP,

    KEY_QNT             /* Do not change! */
};



/* 4. Define "PORT", "PIN", "ACTIVE LEVEL" and PULL RESISTOR of keys in p.3 enumeration order. 
 *    !  Library based on Chibi "palEnablePadEvent", so using pins with the same number (aka GPIOA5 and GPIOB5) is NOT allowed 
 *    !  There is no need to initialize the pins manually as they are automatically initialized when the function "ch_key_init()" is called. 
 *    !  The Pull-Up or Pull-Down selection is made automatically based on "ACTIVE LEVEL". */
#define KEY_LIST    {KEY_SEL, KEY_ACTIVE_HIGH, GPIOB, 12, PULL_ENABLE}, \
                    {KEY_UP,  KEY_ACTIVE_LOW, GPIOB, 13, PULL_ENABLE}


/* 5. Define priority of KEY handler Thread*/
#define KEY_HADLER_THREAD_PRIORITY (NORMALPRIO + 1)

/* 6. Call ch_key_init() once before use driver */

/* 7. In every thread you need to handle keys:
 * 
 * 7.1 Registy event flag mask: chEvtRegisterMaskWithFlags(&key_events, &key_el, 0x01, REGK(KEY_SEL) | REGK(KEY_UP) | REGKEYEVTS(KEY_EVENT_CLICK | KEY_EVENT_HOLD));
 * 7.2 Wait for events: eventmask_t events = chEvtWaitAny(ALL_EVENTS); or if(chEvtWaitAnyTimeout(ALL_EVENTS, TIME_IMMEDIATE)) or ...
 * 7.3 Get event flags: msg = chEvtGetAndClearFlags(&el);
 * 7.4 Check key: if (ISKEY(KEY_SEL, msg)) ...
 * 7.5 Check event: if(ISKEYEVT(KEY_EVENT_CLICK, msg)) ... */

/* 8. Optionally, you can change key dispather stack size */
#define KEY_THREAD_STACK_SIZE 256

#endif /* _CH_KEY_CONFIG_H */