#pragma once
#include <Arduino.h>

#define CH_1_PIN 36
#define CH_2_PIN 39
#define CH_3_PIN 34
#define CH_4_PIN 35
#define CH_5_A_PIN 32
#define CH_5_B_PIN 33
#define CH_6_A_PIN 19
#define CH_6_B_PIN 18
#define CH_7_UP_PIN 25
#define CH_7_DOWN_PIN 26
#define CH_8_UP_PIN 5
#define CH_8_DOWN_PIN 17
// add 2 more switches?

#define CH_MIN 0
#define CH_MAX 4096

void tx_setup();
void tx_loop();
void tx_data_received(const uint8_t *mac, const uint8_t *incomingData, int len);
