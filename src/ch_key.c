/**
  ******************************************************************************
  * @brief   KEY driver
  *  Simple KEY driver based on Chibi events mechanism
*/

#include "ch_key.h"


// Key structures
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
TKey key[KEY_QNT] = {KEY_LIST};
#pragma GCC diagnostic pop


// Mailbox for internal driver events
static msg_t key_driver_mailbox_buf[KEY_QNT];
static mailbox_t key_driver_mailbox;


// Global Event Source
event_source_t key_events;


// Debounce callback
static void key_debounce_cb(virtual_timer_t *vtp, void *p);


// Key edge isr
static void key_isr_cb(void *arg) {
    TKey* key = (TKey*)arg;

    chSysLockFromISR();
    palDisablePadEventI(key->port, key->pad);

    chVTResetI(&key->debounce_vt);
    chVTDoSetI(&key->debounce_vt, TIME_MS2I(KEY_DEBOUNCE_MS), key_debounce_cb, arg);
    chSysUnlockFromISR();
}


// Debounce timeout isr
static void key_debounce_cb(virtual_timer_t *vtp, void *p) {
    (void)vtp;
    TKey* key = (TKey*)p;
    uint8_t pad_new_state;

    chSysLockFromISR();
    palEnablePadEventI(key->port, key->pad, PAL_EVENT_MODE_BOTH_EDGES);
    palSetPadCallbackI(key->port, key->pad, key_isr_cb, p);
    chSysUnlockFromISR();

    pad_new_state = palReadPad(key->port, key->pad);

    if(pad_new_state != key->pad_last_state)
    {
        key->pad_last_state = pad_new_state;
        chMBPostI(&key_driver_mailbox, (msg_t)key);
    }
}



// Key timeout isr
static void key_hold_cb(virtual_timer_t *vtp, void *key) {
    (void)(vtp);
    ((TKey*)key)->timeout = 1;
    chMBPostI(&key_driver_mailbox, (msg_t)key);
}



// Thread - KEY dispatcher
static THD_WORKING_AREA(waKeyDisp, 128);
static THD_FUNCTION(keyDisp, arg) {
  (void)(arg);
  msg_t msg;
  TKey* key;
  bool key_pressed;


  while (true) {
  
    if (chMBFetchTimeout(&key_driver_mailbox, &msg, TIME_INFINITE) == MSG_OK) {
      key = (TKey*)(msg);
      key_pressed = (key->pad_last_state == (uint8_t)key->active_level);

      if(key->timeout){
        key->timeout = 0;

        switch (key->state)
        {
          case KEY_WAIT_HOLD:

            key->state = KEY_WAIT_IDLE;

            chVTReset(&key->hold_vt);
            chVTDoSetI(&key->hold_vt, TIME_MS2I(KEY_HOLD_REPEAT_EVENT_MS), key_hold_cb, key);

            // Rise "HOLD EVENT"
            #if (KEY_HOLD_EVENT_EN == TRUE)
            chSysLock();
            chEvtBroadcastFlags(&key_events, KEY2MSG(key->id, KEY_EVENT_HOLD));
            chSysUnlock();
            #endif

          break;


          case KEY_WAIT_IDLE:

            chVTReset(&key->hold_vt);
            chVTDoSetI(&key->hold_vt, TIME_MS2I(KEY_HOLD_REPEAT_EVENT_MS), key_hold_cb, key);

            // Rise "HOLD REPEAT EVENT"
            #if (KEY_HOLD_REPEAT_EVENT_EN == TRUE)
            chSysLock();
            chEvtBroadcastFlags(&key_events,  KEY2MSG(key->id, KEY_EVENT_HOLD_REPEAT));
            chSysUnlock();
            #endif

          break;
        }
        continue;
      }

      
      switch (key->state)
      {
        case KEY_IDLE:

          if(key_pressed){
            key->state = KEY_WAIT_HOLD;

            chVTReset(&key->hold_vt);
            chVTDoSetI(&key->hold_vt, TIME_MS2I(KEY_HOLD_EVENT_MS), key_hold_cb, key);

            // Rise "CLICK EVENT"
            #if (KEY_CLICK_EVENT_EN == TRUE)
            chSysLock();
            chEvtBroadcastFlags(&key_events, KEY2MSG(key->id, KEY_EVENT_CLICK));
            chSysUnlock();
            #endif
          }

          else{
            // Rise "IDLE AFTER RST EVENT"
            #if (KEY_HOLD_ON_PWR_UP_EVENT_EN == TRUE)
            chSysLock();
            chEvtBroadcastFlags(&key_events, KEY2MSG(key->id, KEY_EVENT_HOLD_ON_PWR_UP));
            chSysUnlock();
            #endif
          }

        break;
        

      case KEY_WAIT_HOLD:
      case KEY_WAIT_IDLE:

        if(!key_pressed){

          chSysLock();
          key->state = KEY_IDLE;
          key->timeout = 0;
          chVTReset(&key->hold_vt);
                    
          #if (KEY_RELEASE_EVENT_EN == TRUE)
          chEvtBroadcastFlags(&key_events, KEY2MSG(key->id, KEY_EVENT_RELEASE));  // Rise "BACK TO IDLE EVENT"
          chSysUnlock();
          #endif
        }
        else
          while (1);

      break;
      
      default:
      break;
      }
    }
  }
}



/**
 * @brief Initialize key driver structures and syncronization objects. Creates KEY Handler Thread
*/
void ch_key_init()
{
    for(unsigned idx = 0; idx < KEY_QNT; idx++)
    {
      if(key[idx].pull_resistor_en)
        (key[idx].active_level == KEY_ACTIVE_LOW) ? palSetPadMode(key[idx].port, key[idx].pad, PAL_MODE_INPUT_PULLUP) : palSetPadMode(key[idx].port, key[idx].pad, PAL_MODE_INPUT_PULLDOWN);
      else
        palSetPadMode(key[idx].port, key[idx].pad, PAL_MODE_INPUT);

      palEnablePadEvent(key[idx].port, key[idx].pad, PAL_EVENT_MODE_BOTH_EDGES);
      palSetPadCallback(key[idx].port, key[idx].pad, key_isr_cb, (void*)(&key[idx]));

      chVTObjectInit(&key[idx].debounce_vt);
      chVTObjectInit(&key[idx].hold_vt);

      key[idx].pad_last_state = palReadPad(key[idx].port, key[idx].pad);
    }

    chMBObjectInit(&key_driver_mailbox, key_driver_mailbox_buf, KEY_QNT);
    chEvtObjectInit(&key_events);

    chThdCreateStatic(waKeyDisp, sizeof(waKeyDisp), KEY_HADLER_THREAD_PRIORITY, keyDisp, NULL);
}