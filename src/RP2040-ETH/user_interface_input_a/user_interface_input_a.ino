#include "CH9120.h"

UCHAR CH9120_LOCAL_IP[4] = {192, 168, 249, 2};
UCHAR CH9120_GATEWAY[4] = {192, 168, 249, 1};
UCHAR CH9120_SUBNET_MASK[4] = {255, 255, 255, 0};
UCHAR CH9120_REMOTE_IP[4] = {192, 168, 249, 101};
UWORD CH9120_LOCAL_PORT = 10;
UWORD CH9120_REMOTE_PORT = 10;

volatile int adc_value = 0;
volatile int32_t iteration = 0;

void setup() {
    Serial.begin(115200);
  
    CH9120_init(CH9120_LOCAL_IP,
                CH9120_GATEWAY,
                CH9120_SUBNET_MASK,
                CH9120_REMOTE_IP,
                CH9120_LOCAL_PORT,
                CH9120_REMOTE_PORT);
}


void setup1() {
  analogReadResolution(12); 
}

void loop() {
  char msg[256];
  //snprintf(msg, sizeof(msg), "TX: Iteration %d, adc_value %d\n", iteration, adc_value);
  //Serial.print(msg);

  //SendUdpPacket(msg);

  char rx_msg[256];
  memset(rx_msg, 0, sizeof(rx_msg));
  
  bool data_available = RecvUdpPacket(rx_msg, sizeof(rx_msg));
  int chars = snprintf(msg, sizeof(msg), "RX: %s\n", rx_msg);
  if (data_available) {
    Serial.print(msg);
  }
  
  //delay(200);
}



void loop1() {
  iteration++;
  adc_value = analogRead(A0);
}
