#include "Adafruit_LEDBackpack.h"
#include "CH9120.h"
#include <Adafruit_GFX.h>
#include <Wire.h>

#define DISPLAY_FLAG_00 (1 << 0)
#define DISPLAY_FLAG_01 (1 << 1)
#define DISPLAY_FLAG_02 (1 << 2)
#define DISPLAY_FLAG_03 (1 << 3)
#define DISPLAY_FLAG_04 (1 << 4)
#define DISPLAY_FLAG_05 (1 << 5)
#define DISPLAY_FLAG_06 (1 << 6)
#define DISPLAY_FLAG_07 (1 << 7)
#define DISPLAY_FLAG_08 (1 << 8)
#define DISPLAY_FLAG_09 (1 << 9)
#define DISPLAY_FLAG_10 (1 << 10)
#define DISPLAY_FLAG_11 (1 << 11)

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

Adafruit_7segment led_display_01 = Adafruit_7segment();
Adafruit_7segment led_display_02 = Adafruit_7segment();
Adafruit_7segment led_display_03 = Adafruit_7segment();

UCHAR CH9120_LOCAL_IP[4] = {192, 168, 249, 9};
UCHAR CH9120_GATEWAY[4] = {192, 168, 249, 1};
UCHAR CH9120_SUBNET_MASK[4] = {255, 255, 255, 0};
UCHAR CH9120_REMOTE_IP[4] = {192, 168, 249, 1};
UWORD CH9120_LOCAL_PORT = 9;
UWORD CH9120_REMOTE_PORT = 9;

typedef enum {
  ZERO_PAD_OFF,
  ZERO_PAD_ON,
  ZERO_PAD_INIT,
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
volatile int module_disabled = 0;

// loop0 state
static uint32_t eth_ref_ts = 0;

// loop1 state
static int prev_btn_01 = 1;
static int prev_btn_02 = 1;

static uint32_t btn_01_ref_ts = 0;
static uint32_t btn_02_ref_ts = 0;
static uint32_t display_ref_ts = 0;

static int prev_display_data[12];
static zero_pad_t prev_zero_pad = ZERO_PAD_INIT;

void setup() {

  // 0xa means 7 segment display OFF (display zero when zero pad is active)
  for (int i = 0; i < 12; i++) {
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

    prev_display_data[i] = 0x0;
  }

  Serial.begin(9600);

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

  led_display_01.setBrightness(4);
  led_display_02.setBrightness(4);
  led_display_03.setBrightness(4);
}

static int ascii_to_integer(char c) {
  switch (c) {
  case '0':
    return 0x0;
  case '1':
    return 0x1;
  case '2':
    return 0x2;
  case '3':
    return 0x3;
  case '4':
    return 0x4;
  case '5':
    return 0x5;
  case '6':
    return 0x6;
  case '7':
    return 0x7;
  case '8':
    return 0x8;
  case '9':
    return 0x9;
  case 'a':
  default:
    return 0xa;
  }
}

static void update_stats(volatile int display_data[12], char *value_str) {
  int len = strlen(value_str);
  if (len == 12) {
    for (int i = 0; i < len; i++) {
      display_data[11 - i] = ascii_to_integer(value_str[i]);
    }
  }
}

static void check_led_status(char* value_str)
{
  // disabling LEDs is combined with no 'aaaa' display data. With zero feedback to user, disable whole module
  if (value_str[0] == '0') {
    module_disabled = 1;
  } else {
    module_disabled = 0;
  }
}

static void parse_latest_stats(char *input_str) {
  // A_ALL=aaaaa5285059;A_A=aaaaa4706325;A_B=aaaaaa521216;A_C=aaaaaaa38409;A_D=aaaaaaa19109;T_ALL=aaaa48784808;T_A=aaaa44179768;T_B=aaaaa4100239;T_C=aaaaaa463603;T_D=aaaaaaa41198

  const char *pair_delimiters = ";\n";

  char *pair_token = strtok(input_str, pair_delimiters);

  int idx = 0;

  while (pair_token != NULL) {
    char *equals_sign = strchr(pair_token, '=');

    if (equals_sign != NULL) {
      char *value = equals_sign + 1;

      switch (idx) {
      case 0:
        // A_ALL
        update_stats(display_data_activations_all, value);
        break;
      case 1:
        // A_A
        update_stats(display_data_activations_group_a, value);
        break;
      case 2:
        // A_B
        update_stats(display_data_activations_group_b, value);
        break;
      case 3:
        // A_C
        update_stats(display_data_activations_group_c, value);
        break;
      case 4:
        // A_D
        update_stats(display_data_activations_group_d, value);
        break;
      case 5:
        // T_ALL
        update_stats(display_data_total_time_all, value);
        break;
      case 6:
        // T_A
        update_stats(display_data_total_time_group_a, value);
        break;
      case 7:
        // T_B
        update_stats(display_data_total_time_group_b, value);
        break;
      case 8:
        // T_C
        update_stats(display_data_total_time_group_c, value);
        break;
      case 9:
        // T_D
        update_stats(display_data_total_time_group_d, value);
        break;
      case 10:
        check_led_status(value);
        break;
      default:
        break;
      }

      idx++;
    }

    pair_token = strtok(NULL, pair_delimiters);
  }
}

void loop() {
  static bool first_iteration = true;
  if (first_iteration) {
    Serial.println("Live Stats Module");
    first_iteration = false;
  }

  static char rx_msg[1500];
  memset(rx_msg, 0, sizeof(rx_msg));

  bool data_available = RecvUdpPacket(rx_msg, sizeof(rx_msg));
  if (data_available) {
    parse_latest_stats(rx_msg);
  }

  uint32_t eth_elapsed = (uint32_t)millis() - eth_ref_ts;
  if ( eth_elapsed >= 40) { // 25 Hz (Wireshark reports slightly lower packets/s)
    static char group[8];
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

    static char udp_msg[1024];
    snprintf(udp_msg, sizeof(udp_msg),
           "display_mode=%s, selection_scope=%s, group=%s, zero_pad=%s\n",
           btn_04_display_mode == DISPLAY_MODE_RELAY_ACTIVATIONS
               ? "RELAY_ACTIVATIONS"
               : "TOTAL_ACTIVATION_TIME",
           btn_03_selection_scope == SELECTION_SCOPE_ALL ? "ALL" : "GROUP",
           group, btn_01_zero_pad == ZERO_PAD_OFF ? "OFF" : "ON");
    
    SendUdpPacket(udp_msg);

    eth_ref_ts = (uint32_t)millis();
  }
}

uint16_t identify_delta(volatile int display_data[12],
                        int prev_display_data[12]) {
  uint16_t bitfield = 0;

  for (int i = 0; i < 12; i++) {
    if (display_data[i] != prev_display_data[i]) {
      bitfield |= 1 << i;
    }
  }

  return bitfield;
}

static void volatile_copy(int dst[12], volatile int src[12]) {
  // memcpy() should not be used
  for (int i = 0; i < 12; i++) {
    dst[i] = src[i];
  }
}

void loop1_update_display(volatile int display_data[12]) {
  if (module_disabled) {
    led_display_01.writeDigitRaw(0, 0);
    led_display_01.writeDigitRaw(1, 0);
    led_display_01.writeDigitRaw(3, 0);
    led_display_01.writeDigitRaw(4, 0);
    led_display_01.writeDisplay();
    
    led_display_02.writeDigitRaw(0, 0);
    led_display_02.writeDigitRaw(1, 0);
    led_display_02.writeDigitRaw(3, 0);
    led_display_02.writeDigitRaw(4, 0);
    led_display_02.writeDisplay();
    
    led_display_03.writeDigitRaw(0, 0);
    led_display_03.writeDigitRaw(1, 0);
    led_display_03.writeDigitRaw(3, 0);
    led_display_03.writeDigitRaw(4, 0);
    led_display_03.writeDisplay();

    prev_zero_pad = ZERO_PAD_INIT;

    return;
  }
  
  uint16_t update_bf = identify_delta(display_data, prev_display_data);

  if (update_bf > 0) {
    volatile_copy(prev_display_data, display_data);
  }

  if (btn_01_zero_pad != prev_zero_pad) {
    prev_zero_pad = btn_01_zero_pad;
    update_bf = 0xffff;
  }

  uint16_t d01_flags =
      DISPLAY_FLAG_00 | DISPLAY_FLAG_01 | DISPLAY_FLAG_02 | DISPLAY_FLAG_03;
  uint16_t d02_flags =
      DISPLAY_FLAG_04 | DISPLAY_FLAG_05 | DISPLAY_FLAG_06 | DISPLAY_FLAG_07;
  uint16_t d03_flags =
      DISPLAY_FLAG_08 | DISPLAY_FLAG_09 | DISPLAY_FLAG_10 | DISPLAY_FLAG_11;

  ////////////////////////////////////////////////////////////
  ///////// ZERO PADDING STATE (0xa indication) //////////////
  ////////////////////////////////////////////////////////////
  // Zero pad means modifying positions that only relay
  // structure, not any real information. Display turned off
  // or zero.

  if (display_data[3] == 0xa && update_bf & DISPLAY_FLAG_03) {
    if (btn_01_zero_pad == ZERO_PAD_ON) {
      led_display_01.writeDigitNum(0, 0, false);
    } else {
      led_display_01.writeDigitRaw(0, 0);
    }
  }

  if (display_data[2] == 0xa && update_bf & DISPLAY_FLAG_02) {
    if (btn_01_zero_pad == ZERO_PAD_ON) {
      led_display_01.writeDigitNum(1, 0, false);
    } else {
      led_display_01.writeDigitRaw(1, 0);
    }
  }

  if (display_data[1] == 0xa && update_bf & DISPLAY_FLAG_01) {
    if (btn_01_zero_pad == ZERO_PAD_ON) {
      led_display_01.writeDigitNum(3, 0, false);
    } else {
      led_display_01.writeDigitRaw(3, 0);
    }
  }

  if (display_data[0] == 0xa && update_bf & DISPLAY_FLAG_00) {
    if (btn_01_zero_pad == ZERO_PAD_ON) {
      led_display_01.writeDigitNum(4, 0, false);
    } else {
      led_display_01.writeDigitRaw(4, 0);
    }
  }

  if (update_bf & d01_flags) {
    led_display_01.writeDisplay();
  }

  if (display_data[7] == 0xa && update_bf & DISPLAY_FLAG_07) {
    if (btn_01_zero_pad == ZERO_PAD_ON) {
      led_display_02.writeDigitNum(0, 0, false);
    } else {
      led_display_02.writeDigitRaw(0, 0);
    }
  }

  if (display_data[6] == 0xa && update_bf & DISPLAY_FLAG_06) {
    if (btn_01_zero_pad == ZERO_PAD_ON) {
      led_display_02.writeDigitNum(1, 0, false);
    } else {
      led_display_02.writeDigitRaw(1, 0);
    }
  }

  if (display_data[5] == 0xa && update_bf & DISPLAY_FLAG_05) {
    if (btn_01_zero_pad == ZERO_PAD_ON) {
      led_display_02.writeDigitNum(3, 0, false);
    } else {
      led_display_02.writeDigitRaw(3, 0);
    }
  }

  if (display_data[4] == 0xa && update_bf & DISPLAY_FLAG_04) {
    if (btn_01_zero_pad == ZERO_PAD_ON) {
      led_display_02.writeDigitNum(4, 0, false);
    } else {
      led_display_02.writeDigitRaw(4, 0);
    }
  }

  if (update_bf & d02_flags) {
    led_display_02.writeDisplay();
  }

  if (display_data[11] == 0xa && update_bf & DISPLAY_FLAG_11) {
    if (btn_01_zero_pad == ZERO_PAD_ON) {
      led_display_03.writeDigitNum(0, 0, false);
    } else {
      led_display_03.writeDigitRaw(0, 0);
    }
  }

  if (display_data[10] == 0xa && update_bf & DISPLAY_FLAG_10) {
    if (btn_01_zero_pad == ZERO_PAD_ON) {
      led_display_03.writeDigitNum(1, 0, false);
    } else {
      led_display_03.writeDigitRaw(1, 0);
    }
  }

  if (display_data[9] == 0xa && update_bf & DISPLAY_FLAG_09) {
    if (btn_01_zero_pad == ZERO_PAD_ON) {
      led_display_03.writeDigitNum(3, 0, false);
    } else {
      led_display_03.writeDigitRaw(3, 0);
    }
  }

  if (display_data[8] == 0xa && update_bf & DISPLAY_FLAG_08) {
    if (btn_01_zero_pad == ZERO_PAD_ON) {
      led_display_03.writeDigitNum(4, 0, false);
    } else {
      led_display_03.writeDigitRaw(4, 0);
    }
  }

  if (update_bf & d03_flags) {
    led_display_03.writeDisplay();
  }
  

  //////////////////////////////////////////////////////////////
  /////////////////// VALUE 0-9 STATE //////////////////////////
  //////////////////////////////////////////////////////////////

  if (display_data[3] != 0xa && update_bf & DISPLAY_FLAG_03) {
    led_display_01.writeDigitNum(0, display_data[3], false);
  }

  if (display_data[2] != 0xa && update_bf & DISPLAY_FLAG_02) {
    led_display_01.writeDigitNum(1, display_data[2], false);
  }

  if (display_data[1] != 0xa && update_bf & DISPLAY_FLAG_01) {
    led_display_01.writeDigitNum(3, display_data[1], false);
  }

  if (display_data[0] != 0xa && update_bf & DISPLAY_FLAG_00) {
    led_display_01.writeDigitNum(4, display_data[0], false);
  }
  if (update_bf & d01_flags) {
    led_display_01.writeDisplay();
  }

  if (display_data[7] != 0xa && update_bf & DISPLAY_FLAG_07) {
    led_display_02.writeDigitNum(0, display_data[7], false);
  }

  if (display_data[6] != 0xa && update_bf & DISPLAY_FLAG_06) {
    led_display_02.writeDigitNum(1, display_data[6], false);
  }

  if (display_data[5] != 0xa && update_bf & DISPLAY_FLAG_05) {
    led_display_02.writeDigitNum(3, display_data[5], false);
  }

  if (display_data[4] != 0xa && update_bf & DISPLAY_FLAG_04) {
    led_display_02.writeDigitNum(4, display_data[4], false);
  }

  if (update_bf & d02_flags) {
    led_display_02.writeDisplay();
  }

  if (display_data[11] != 0xa && update_bf & DISPLAY_FLAG_11) {
    led_display_03.writeDigitNum(0, display_data[11], false);
  }

  if (display_data[10] != 0xa && update_bf & DISPLAY_FLAG_10) {
    led_display_03.writeDigitNum(1, display_data[10], false);
  }

  if (display_data[9] != 0xa && update_bf & DISPLAY_FLAG_09) {
    led_display_03.writeDigitNum(3, display_data[9], false);
  }

  if (display_data[8] != 0xa && update_bf & DISPLAY_FLAG_08) {
    led_display_03.writeDigitNum(4, display_data[8], false);
  }
  if (update_bf & d03_flags) {
    led_display_03.writeDisplay();
  }
}

static void update_seven_segment_displays()
{
  uint32_t display_elapsed = (uint32_t)millis() - display_ref_ts;

  if (display_elapsed > 0) {
    if (btn_04_display_mode == DISPLAY_MODE_RELAY_ACTIVATIONS) {
      if (btn_03_selection_scope == SELECTION_SCOPE_ALL) {
        loop1_update_display(display_data_activations_all);
      } else if (btn_02_group == GROUP_A) {
        loop1_update_display(display_data_activations_group_a);
      } else if (btn_02_group == GROUP_B) {
        loop1_update_display(display_data_activations_group_b);
      } else if (btn_02_group == GROUP_C) {
        loop1_update_display(display_data_activations_group_c);
      } else if (btn_02_group == GROUP_D) {
        loop1_update_display(display_data_activations_group_d);
      }
    } else if (btn_04_display_mode ==
               DISPLAY_MODE_RELAY_TOTAL_ACTIVATION_TIME) {
      if (btn_03_selection_scope == SELECTION_SCOPE_ALL) {
        loop1_update_display(display_data_total_time_all);
      } else if (btn_02_group == GROUP_A) {
        loop1_update_display(display_data_total_time_group_a);
      } else if (btn_02_group == GROUP_B) {
        loop1_update_display(display_data_total_time_group_b);
      } else if (btn_02_group == GROUP_C) {
        loop1_update_display(display_data_total_time_group_c);
      } else if (btn_02_group == GROUP_D) {
        loop1_update_display(display_data_total_time_group_d);
      }
    }
    display_ref_ts = (uint32_t)millis();
  }
}

void loop1() {

  // Do not allow any state transition when all displays are disabled
  // Pushing buttons while giving no feedback should be ignored until
  // user activates screens again. This mainly affects momentary buttons
  // but treat it as two modes for consistency.
  if (module_disabled) {
    digitalWrite(PIN_LED_01, LOW);
    digitalWrite(PIN_LED_02, LOW);
    digitalWrite(PIN_LED_03, LOW);
    digitalWrite(PIN_LED_04, LOW);
    digitalWrite(PIN_LED_05, LOW);
    digitalWrite(PIN_LED_06, LOW);

    update_seven_segment_displays();
    
    return;
  }
  
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

  update_seven_segment_displays();

  prev_btn_01 = btn_01;
  prev_btn_02 = btn_02;
}
