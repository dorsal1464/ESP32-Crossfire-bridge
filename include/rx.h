#pragma once
#include <Arduino.h>
#define PIN_RX 20
#define PIN_TX 21
#define LED_PIN 8

void rx_setup();
void rx_loop();
void rx_turn_on_led();
void rx_data_received(const uint8_t *mac, const uint8_t *incomingData, int len);