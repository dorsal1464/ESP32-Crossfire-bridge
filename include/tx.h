#pragma once
#include <Arduino.h>

void tx_setup();
void tx_loop();
void tx_data_received(const uint8_t *mac, const uint8_t *incomingData, int len);
