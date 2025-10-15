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

Adafruit_7segment led_display_00 = Adafruit_7segment();
Adafruit_7segment led_display_01 = Adafruit_7segment();
Adafruit_7segment led_display_02 = Adafruit_7segment();
Adafruit_7segment led_display_03 = Adafruit_7segment();
Adafruit_7segment led_display_04 = Adafruit_7segment();
Adafruit_7segment led_display_05 = Adafruit_7segment();

UCHAR CH9120_LOCAL_IP[4] = {192, 168, 249, 4};
UCHAR CH9120_GATEWAY[4] = {192, 168, 249, 1};
UCHAR CH9120_SUBNET_MASK[4] = {255, 255, 255, 0};
UCHAR CH9120_REMOTE_IP[4] = {192, 168, 249, 1};
UWORD CH9120_LOCAL_PORT = 9;
UWORD CH9120_REMOTE_PORT = 9;

// Shared state
volatile int display_data_0[5];
volatile int display_data_1[5];
volatile int display_data_2[5];
volatile int display_data_3[5];
volatile int display_data_4[5];
volatile int display_data_5[5];

int prev_display_data_0[5];
int prev_display_data_1[5];
int prev_display_data_2[5];
int prev_display_data_3[5];
int prev_display_data_4[5];
int prev_display_data_5[5];

// loop1 state
static uint32_t display_ref_ts = 0;

void setup() {

  // 0xa means 7 segment display OFF (display zero when zero pad is active)
  for (int i = 0; i < 5; i++) {
    if (i == 0) {
      display_data_0[i] = 0xf;
      display_data_1[i] = 0xf;
      display_data_2[i] = 0xf;
      display_data_3[i] = 0xf;
      display_data_4[i] = 0xf;
      display_data_5[i] = 0xf;
    } else {
      display_data_0[i] = 0xa;
      display_data_1[i] = 0xa;
      display_data_2[i] = 0xa;
      display_data_3[i] = 0xa;
      display_data_4[i] = 0xa;
      display_data_5[i] = 0xa;
    }

    prev_display_data_0[i] = 0x0;
    prev_display_data_1[i] = 0x0;
    prev_display_data_2[i] = 0x0;
    prev_display_data_3[i] = 0x0;
    prev_display_data_4[i] = 0x0;
    prev_display_data_5[i] = 0x0;
  }

  Serial.begin(9600);

  CH9120_init(CH9120_LOCAL_IP, CH9120_GATEWAY, CH9120_SUBNET_MASK,
              CH9120_REMOTE_IP, CH9120_LOCAL_PORT, CH9120_REMOTE_PORT);
}

void setup1() {

  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();

  led_display_00.begin(0x70);
  led_display_01.begin(0x71);
  led_display_02.begin(0x72);

  led_display_00.setBrightness(4);
  led_display_01.setBrightness(4);
  led_display_02.setBrightness(4);

  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.begin();

  led_display_03.begin(0x70, &Wire1);
  led_display_04.begin(0x71, &Wire1);
  led_display_05.begin(0x72, &Wire1);

  led_display_03.setBrightness(4);
  led_display_04.setBrightness(4);
  led_display_05.setBrightness(4);
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
    return 0xa;
  case 'b':
    return 0xb;
  case 'c':
    return 0xc;
  case 'd':
    return 0xd;
  case 'e':
    return 0xe;
  case 'f':
    return 0xf;
  default:
    return 0xa;
  }
}

static void update_stats(volatile int display_data[5], char *value_str) {
  int len = strlen(value_str);
  if (len == 5) {
    for (int i = 0; i < len; i++) {
      display_data[4 - i] = ascii_to_integer(value_str[i]);
    }
  }
}

static void parse_latest_stats(char *input_str) {
  // D_0=aaaae;D_1=aaaae;D_2=aaaae;D_3=aaaae;D_4=aaaae;D_5=aaaae\n

  const char *pair_delimiters = ";\n";

  char *pair_token = strtok(input_str, pair_delimiters);

  int idx = 0;

  while (pair_token != NULL) {
    char *equals_sign = strchr(pair_token, '=');

    if (equals_sign != NULL) {
      char *value = equals_sign + 1;

      switch (idx) {
      case 0:
        // D_0
        update_stats(display_data_0, value);
        break;
      case 1:
        // D_1
        update_stats(display_data_1, value);
        break;
      case 2:
        // D_2
        update_stats(display_data_2, value);
        break;
      case 3:
        // D_3
        update_stats(display_data_3, value);
        break;
      case 4:
        // D_4
        update_stats(display_data_4, value);
        break;
      case 5:
        // D_5
        update_stats(display_data_5, value);
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
    Serial.println("Current Sensor Display Module");
    first_iteration = false;
  }

  static char rx_msg[1500];
  memset(rx_msg, 0, sizeof(rx_msg));

  bool data_available = RecvUdpPacket(rx_msg, sizeof(rx_msg));
  if (data_available) {
    parse_latest_stats(rx_msg);
  }
}

uint16_t identify_delta(volatile int display_data[5],
                        int prev_display_data[5]) {
  uint16_t bitfield = 0;

  for (int i = 0; i < 4; i++) {
    if (display_data[i+1] != prev_display_data[i+1]) {
      bitfield |= 1 << i;
    }
  }

  if (display_data[0] != prev_display_data[0]) {
    bitfield = 0xf;
  }

  return bitfield;
}

static void volatile_copy(int dst[5], volatile int src[5]) {
  // memcpy() should not be used
  for (int i = 0; i < 5; i++) {
    dst[i] = src[i];
  }
}

void loop1_update_display(Adafruit_7segment* seven_segment_display, volatile int display_data[5], int prev_display_data[5]) {
  uint16_t update_bf = identify_delta(display_data, prev_display_data);

  if (update_bf > 0) {
    volatile_copy(prev_display_data, display_data);
  }

  uint16_t d01_flags =
      DISPLAY_FLAG_00 | DISPLAY_FLAG_01 | DISPLAY_FLAG_02 | DISPLAY_FLAG_03;

  ////////////////////////////////////////////////////////////
  ///////// ZERO PADDING STATE (0xa indication) //////////////
  ////////////////////////////////////////////////////////////
  // Zero pad means modifying positions that only relay
  // structure, not any real information. Display turned off
  // or zero.

  if (display_data[4] == 0xa && update_bf & DISPLAY_FLAG_03) {
    seven_segment_display->writeDigitRaw(0, 0);
  }

  if (display_data[3] == 0xa && update_bf & DISPLAY_FLAG_02) {
    seven_segment_display->writeDigitRaw(1, 0);
  }

  if (display_data[2] == 0xa && update_bf & DISPLAY_FLAG_01) {
    seven_segment_display->writeDigitRaw(3, 0);
  }

  if (display_data[1] == 0xa && update_bf & DISPLAY_FLAG_00) {
    seven_segment_display->writeDigitRaw(4, 0);
  }

  if (update_bf & d01_flags) {
    seven_segment_display->writeDisplay();
  }

  //////////////////////////////////////////////////////////////
  ///////////// VALUE 0-9 STATE AND DECIMAL POINT //////////////
  //////////////////////////////////////////////////////////////

  if (display_data[4] != 0xa && update_bf & DISPLAY_FLAG_03) {
    if (display_data[0] == 0xb) {
      seven_segment_display->writeDigitNum(0, display_data[4], true);
    } else {
      seven_segment_display->writeDigitNum(0, display_data[4], false);
    }
  }

  if (display_data[3] != 0xa && update_bf & DISPLAY_FLAG_02) {
    if (display_data[0] == 0xc) {
      seven_segment_display->writeDigitNum(1, display_data[3], true);
    } else {
      seven_segment_display->writeDigitNum(1, display_data[3], false);
    }
  }

  if (display_data[2] != 0xa && update_bf & DISPLAY_FLAG_01) {
    if (display_data[0] == 0xd) {
      seven_segment_display->writeDigitNum(3, display_data[2], true);
    } else {
      seven_segment_display->writeDigitNum(3, display_data[2], false);
    }
  }

  if (display_data[1] != 0xa && update_bf & DISPLAY_FLAG_00) {
    // Forth decimal not used
    seven_segment_display->writeDigitNum(4, display_data[1], false);
  }
  if (update_bf & d01_flags) {
    seven_segment_display->writeDisplay();
  }
}

void loop1() {

  uint32_t display_elapsed = (uint32_t)millis() - display_ref_ts;

  if (display_elapsed > 0) {
    loop1_update_display(&led_display_00, display_data_0, prev_display_data_0);
    loop1_update_display(&led_display_01, display_data_1, prev_display_data_1);
    loop1_update_display(&led_display_02, display_data_2, prev_display_data_2);
    loop1_update_display(&led_display_03, display_data_3, prev_display_data_3);
    loop1_update_display(&led_display_04, display_data_4, prev_display_data_4);
    loop1_update_display(&led_display_05, display_data_5, prev_display_data_5);

    display_ref_ts = (uint32_t)millis();
  }
}
