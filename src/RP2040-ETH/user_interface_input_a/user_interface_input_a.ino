#include "CH9120.h"

UCHAR CH9120_LOCAL_IP[4] = {192, 168, 249, 10};
UCHAR CH9120_GATEWAY[4] = {192, 168, 249, 1};
UCHAR CH9120_SUBNET_MASK[4] = {255, 255, 255, 0};
UCHAR CH9120_REMOTE_IP[4] = {192, 168, 249, 101};
UWORD CH9120_LOCAL_PORT = 10;
UWORD CH9120_REMOTE_PORT = 10;

volatile int adc_value = 0;
volatile int rms_sample_index = 0;
volatile int32_t iteration = 0;

int block_size = 4096 / 15;

void setup() {
    Serial.begin(115200);
    Serial.println("User Interface Input Module #1");
  
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
  char msg[128];
  snprintf(msg, sizeof(msg), "Iteration %d, adc_value %d, index %d\n", iteration, adc_value, rms_sample_index);
  
  Serial.print(msg);
  SendUdpPacket(msg);
  
  delay(250);
}



void loop1() {
  iteration++;
  adc_value = analogRead(A0);

  rms_sample_index = adc_value / block_size;
  
}
