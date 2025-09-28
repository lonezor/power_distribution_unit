#include "Adafruit_LEDBackpack.h"
#include "CH9120.h"
#include <Adafruit_GFX.h>
#include <Wire.h>

Adafruit_7segment led_display_01 = Adafruit_7segment();
Adafruit_7segment led_display_02 = Adafruit_7segment();
Adafruit_7segment led_display_03 = Adafruit_7segment();

UCHAR CH9120_LOCAL_IP[4] = {192, 168, 249, 9};
UCHAR CH9120_GATEWAY[4] = {192, 168, 249, 1};
UCHAR CH9120_SUBNET_MASK[4] = {255, 255, 255, 0};
UCHAR CH9120_REMOTE_IP[4] = {192, 168, 249, 101};
UWORD CH9120_LOCAL_PORT = 10;
UWORD CH9120_REMOTE_PORT = 10;

typedef enum {
  ZERO_PAD_OFF,
  ZERO_PAD_ON,
} zero_pad_t;

typedef enum {
  GROUP_A,
  GROUP_B,
  GROUP_C,
  GROUP_D,
} group_t;

typedef enum {
  SELECTION_SCOPE_ALL,
  SELECTION_SCOPE_GROUP,
} selection_scope_t;

typedef enum {
  DISPLAY_MODE_RELAY_ACTIVATIONS,
  DISPLAY_MODE_RELAY_TOTAL_ACTIVATION_TIME,
} display_mode_t;

// Shared state
volatile int32_t iteration = 0;
volatile zero_pad_t btn_01_zero_pad = ZERO_PAD_OFF;
volatile group_t btn_02_group = GROUP_A;
volatile selection_scope_t btn_03_selection_scope = SELECTION_SCOPE_ALL;
volatile display_mode_t btn_04_display_mode = DISPLAY_MODE_RELAY_ACTIVATIONS;

volatile int display_data_activations_all[12];
volatile int display_data_activations_group_a[12];
volatile int display_data_activations_group_b[12];
volatile int display_data_activations_group_c[12];
volatile int display_data_activations_group_d[12];
volatile int display_data_total_time_all[12];
volatile int display_data_total_time_group_a[12];
volatile int display_data_total_time_group_b[12];
volatile int display_data_total_time_group_c[12];
volatile int display_data_total_time_group_d[12];

#define PIN_BTN_01 (2)
#define PIN_BTN_02 (3)
#define PIN_BTN_03 (4)
#define PIN_BTN_04 (5)

#define PIN_LED_01 (6)
#define PIN_LED_02 (7)
#define PIN_LED_03 (8)
#define PIN_LED_04 (9)
#define PIN_LED_05 (22)
#define PIN_LED_06 (26)

void setup() {

  // 0xa means 7 segment display OFF (display zero when zero pad is active)
  for(int i=0; i<12; i++) {
    display_data_activations_all[i] = 0xa;
    display_data_activations_group_a[i] = 0xa;
    display_data_activations_group_b[i] = 0xa;
    display_data_activations_group_c[i] = 0xa;
    display_data_activations_group_d[i] = 0xa;

    display_data_total_time_all[i] = 0xa;
    display_data_total_time_group_a[i] = 0xa;
    display_data_total_time_group_b[i] = 0xa;
    display_data_total_time_group_c[i] = 0xa;
    display_data_total_time_group_d[i] = 0xa;
  }

  Serial.begin(9600);

  Serial.println("Live Stats Module");

  CH9120_init(CH9120_LOCAL_IP, CH9120_GATEWAY, CH9120_SUBNET_MASK,
              CH9120_REMOTE_IP, CH9120_LOCAL_PORT, CH9120_REMOTE_PORT);
}

void setup1() {

  pinMode(PIN_BTN_01, INPUT_PULLUP);
  pinMode(PIN_BTN_02, INPUT_PULLUP);
  pinMode(PIN_BTN_03, INPUT_PULLUP);
  pinMode(PIN_BTN_04, INPUT_PULLUP);

  pinMode(PIN_LED_01, OUTPUT);
  pinMode(PIN_LED_02, OUTPUT);
  pinMode(PIN_LED_03, OUTPUT);
  pinMode(PIN_LED_04, OUTPUT);
  pinMode(PIN_LED_05, OUTPUT);
  pinMode(PIN_LED_06, OUTPUT);

  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();

  led_display_01.begin(0x70);
  led_display_02.begin(0x71);
  led_display_03.begin(0x72);

  led_display_01.setBrightness(8);
  led_display_02.setBrightness(8);
  led_display_03.setBrightness(8);
}

void loop() {

  char group[8];
  memset(group, 0, sizeof(group));
  if (btn_02_group == GROUP_A) {
    group[0] = 'A';
  } else if (btn_02_group == GROUP_B) {
    group[0] = 'B';
  } else if (btn_02_group == GROUP_C) {
    group[0] = 'C';
  } else if (btn_02_group == GROUP_D) {
    group[0] = 'D';
  }

  char log_msg[1024];

  snprintf(log_msg, sizeof(log_msg),
           "display_mode=%s, selection_scope=%s, group=%s, zero_pad=%s",
           btn_04_display_mode == DISPLAY_MODE_RELAY_ACTIVATIONS
               ? "RELAY_ACTIVATIONS"
               : "TOTAL_ACTIVATION_TIME",
           btn_03_selection_scope == SELECTION_SCOPE_ALL ? "ALL" : "GROUP",
           group, btn_01_zero_pad == ZERO_PAD_OFF ? "OFF" : "ON");

  Serial.println(log_msg);

  // todo
  display_data_activations_all[0] += 1;
  if (display_data_activations_all[0] > 9) {
    display_data_activations_all[0] = 0;
  }

    display_data_activations_group_b[2] += 1;
  if (display_data_activations_group_b[2] > 9) {
    display_data_activations_group_b[2] = 0;
  }




  

  delay(100);
}

// loop1 state
static int prev_btn_01 = 1;
static int prev_btn_02 = 1;

static uint32_t btn_01_ref_ts = 0;
static uint32_t btn_02_ref_ts = 0;
static uint32_t display_ref_ts = 0;

void loop1_update_display(volatile int display_data[12])
{
   ////////////////////////////////////////////////////////////
    ///////// ZERO PADDING STATE (0xa indication) //////////////
    ////////////////////////////////////////////////////////////
    // Zero pad means modifying positions that only relay
    // structure, not any real information. Display turned off
    // or zero.

    if (display_data[3] == 0xa) {
      if (btn_01_zero_pad == ZERO_PAD_ON) {
        led_display_01.writeDigitNum(0, 0, false);
      } else {
        led_display_01.writeDigitRaw(0, 0);
      }
    }

    if (display_data[2] == 0xa) {
      if (btn_01_zero_pad == ZERO_PAD_ON) {
        led_display_01.writeDigitNum(1, 0, false);
      } else {
        led_display_01.writeDigitRaw(1, 0);
      }
    }

    if (display_data[3] == 0xa) {
      if (btn_01_zero_pad == ZERO_PAD_ON) {
        led_display_01.writeDigitNum(3, 0, false);
      } else {
        led_display_01.writeDigitRaw(3, 0);
      }
    }

    if (display_data[4] == 0xa) {
      if (btn_01_zero_pad == ZERO_PAD_ON) {
        led_display_01.writeDigitNum(4, 0, false);
      } else {
        led_display_01.writeDigitRaw(4, 0);
      }
    }
    led_display_01.writeDisplay();


    if (display_data[7] == 0xa) {
      if (btn_01_zero_pad == ZERO_PAD_ON) {
        led_display_02.writeDigitNum(0, 0, false);
      } else {
        led_display_02.writeDigitRaw(0, 0);
      }
    }

    if (display_data[6] == 0xa) {
      if (btn_01_zero_pad == ZERO_PAD_ON) {
        led_display_02.writeDigitNum(1, 0, false);
      } else {
        led_display_02.writeDigitRaw(1, 0);
      }
    }

    if (display_data[5] == 0xa) {
      if (btn_01_zero_pad == ZERO_PAD_ON) {
        led_display_02.writeDigitNum(3, 0, false);
      } else {
        led_display_02.writeDigitRaw(3, 0);
      }
    }

    if (display_data[4] == 0xa) {
        if (btn_01_zero_pad == ZERO_PAD_ON) {
          led_display_02.writeDigitNum(4, 0, false);
        } else {
        led_display_02.writeDigitRaw(4, 0);
      }
    }
    
    led_display_02.writeDisplay();


    if (display_data[11] == 0xa) {
      if (btn_01_zero_pad == ZERO_PAD_ON) {
        led_display_03.writeDigitNum(0, 0, false);
      } else {
        led_display_03.writeDigitRaw(0, 0);
      }
    }

    if (display_data[10] == 0xa) {
      if (btn_01_zero_pad == ZERO_PAD_ON) {
        led_display_03.writeDigitNum(1, 0, false);
      } else {
        led_display_03.writeDigitRaw(1, 0);
      }
    }

    if (display_data[9] == 0xa) {
      if (btn_01_zero_pad == ZERO_PAD_ON) {
        led_display_03.writeDigitNum(3, 0, false);
      } else {
        led_display_03.writeDigitRaw(3, 0);
      }
    }

    if (display_data[8] == 0xa) {
      if (btn_01_zero_pad == ZERO_PAD_ON) {
        led_display_03.writeDigitNum(4, 0, false);
      } else {
        led_display_03.writeDigitRaw(4, 0);
      }
    }
    led_display_03.writeDisplay();


    //////////////////////////////////////////////////////////////
    /////////////////// VALUE 0-9 STATE //////////////////////////
    //////////////////////////////////////////////////////////////
  
    if (display_data[3] != 0xa) {
      led_display_01.writeDigitNum(0, display_data[3], false);
    }

    if (display_data[2] != 0xa) {
      led_display_01.writeDigitNum(1, display_data[2], false);
    }

    if (display_data[1] != 0xa) {
      led_display_01.writeDigitNum(3, display_data[1], false);
    }

    if (display_data[0] != 0xa) {
      led_display_01.writeDigitNum(4, display_data[0], false);
    }
    led_display_01.writeDisplay();


    if (display_data[7] != 0xa) {
      led_display_02.writeDigitNum(0, display_data[7], false);
    }

    if (display_data[6] != 0xa) {
      led_display_02.writeDigitNum(1, display_data[6], false);
    }

    if (display_data[5] != 0xa) {
      led_display_02.writeDigitNum(3, display_data[5], false);
    }

    if (display_data[4] != 0xa) {
    led_display_02.writeDigitNum(4, display_data[4], false);
    }
    
    led_display_02.writeDisplay();


    if (display_data[11] != 0xa) {
      led_display_03.writeDigitNum(0, display_data[11], false);
    }

    if (display_data[10] != 0xa) {
      led_display_03.writeDigitNum(1, display_data[10], false);
    }

    if (display_data[9] != 0xa) {
      led_display_03.writeDigitNum(3, display_data[9], false);
    }

    if (display_data[8] != 0xa) {
      led_display_03.writeDigitNum(4, display_data[8], false);
    }
    led_display_03.writeDisplay();
}

void loop1() {
  iteration++;

  int btn_01 = digitalRead(PIN_BTN_01);
  int btn_02 = digitalRead(PIN_BTN_02);
  int btn_03 = digitalRead(PIN_BTN_03);
  int btn_04 = digitalRead(PIN_BTN_04);

  uint32_t btn_01_elapsed = (uint32_t)millis() - btn_01_ref_ts;
  if (btn_01 != prev_btn_01 && btn_01 == LOW && btn_01_elapsed > 175) {
    if (btn_01_zero_pad == ZERO_PAD_OFF) {
      btn_01_zero_pad = ZERO_PAD_ON;
    }

    else if (btn_01_zero_pad == ZERO_PAD_ON) {
      btn_01_zero_pad = ZERO_PAD_OFF;
    }

    btn_01_ref_ts = (uint32_t)millis();
  }

  uint32_t btn_02_elapsed = (uint32_t)millis() - btn_02_ref_ts;
  if (btn_02 != prev_btn_02 && btn_02 == LOW && btn_02_elapsed > 175) {

    if (btn_03_selection_scope == SELECTION_SCOPE_GROUP) {
      if (btn_02_group == GROUP_A) {
        btn_02_group = GROUP_B;
      }

      else if (btn_02_group == GROUP_B) {
        btn_02_group = GROUP_C;
      }

      else if (btn_02_group == GROUP_C) {
        btn_02_group = GROUP_D;
      }

      else if (btn_02_group == GROUP_D) {
        btn_02_group = GROUP_A;
      }
    }

    btn_02_ref_ts = (uint32_t)millis();
  }

  if (btn_03 == 0) {
    digitalWrite(PIN_LED_01, HIGH);
    btn_03_selection_scope = SELECTION_SCOPE_GROUP;

  } else {
    digitalWrite(PIN_LED_01, LOW);
    btn_03_selection_scope = SELECTION_SCOPE_ALL;
  }

  if (btn_04 == 0) {
    digitalWrite(PIN_LED_02, HIGH);
    btn_04_display_mode = DISPLAY_MODE_RELAY_TOTAL_ACTIVATION_TIME;
  } else {
    digitalWrite(PIN_LED_02, LOW);
    btn_04_display_mode = DISPLAY_MODE_RELAY_ACTIVATIONS;
  }

  if (btn_02_group == GROUP_A ||
      btn_03_selection_scope == SELECTION_SCOPE_ALL) {
    digitalWrite(PIN_LED_03, HIGH);
  } else {
    digitalWrite(PIN_LED_03, LOW);
  }

  if (btn_02_group == GROUP_B ||
      btn_03_selection_scope == SELECTION_SCOPE_ALL) {
    digitalWrite(PIN_LED_04, HIGH);
  } else {
    digitalWrite(PIN_LED_04, LOW);
  }

  if (btn_02_group == GROUP_C ||
      btn_03_selection_scope == SELECTION_SCOPE_ALL) {
    digitalWrite(PIN_LED_05, HIGH);
  } else {
    digitalWrite(PIN_LED_05, LOW);
  }

  if (btn_02_group == GROUP_D ||
      btn_03_selection_scope == SELECTION_SCOPE_ALL) {
    digitalWrite(PIN_LED_06, HIGH);
  } else {
    digitalWrite(PIN_LED_06, LOW);
  }

  uint32_t display_elapsed = (uint32_t)millis() - display_ref_ts;
  if (display_elapsed > 50) {
    if (btn_04_display_mode == DISPLAY_MODE_RELAY_ACTIVATIONS) {
      if (btn_03_selection_scope == SELECTION_SCOPE_ALL) {
        loop1_update_display(display_data_activations_all);
      } else if (btn_02_group == GROUP_A) {
        loop1_update_display(display_data_activations_group_a);
      }  else if (btn_02_group == GROUP_B) {
        loop1_update_display(display_data_activations_group_b);
      }  else if (btn_02_group == GROUP_C) {
        loop1_update_display(display_data_activations_group_c);
      } else if (btn_02_group == GROUP_D) {
        loop1_update_display(display_data_activations_group_d);
      }
    } else if (btn_04_display_mode == DISPLAY_MODE_RELAY_TOTAL_ACTIVATION_TIME) {
      if (btn_03_selection_scope == SELECTION_SCOPE_ALL) {
        loop1_update_display(display_data_total_time_all);
      } else if (btn_02_group == GROUP_A) {
        loop1_update_display(display_data_total_time_group_a);
      }  else if (btn_02_group == GROUP_B) {
        loop1_update_display(display_data_total_time_group_b);
      }  else if (btn_02_group == GROUP_C) {
        loop1_update_display(display_data_total_time_group_c);
      } else if (btn_02_group == GROUP_D) {
        loop1_update_display(display_data_total_time_group_d);
      }      
    }
    display_ref_ts = (uint32_t)millis();
  }

  prev_btn_01 = btn_01;
  prev_btn_02 = btn_02;
}
