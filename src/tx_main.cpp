#include <AlfredoCRSF.h>
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "defines.h"
#include "esp_wifi.h"

#if defined(WROOM_TX)

static unsigned long currentMillis = 0;
static uint8_t peer_addr[] = RX_ADDR;

void tx_setup() { currentMillis = millis(); }

void tx_loop() {
    if (millis() - currentMillis > 1000) {
        uint8_t myData[] = "ALIVE!\x00";
        esp_err_t result = esp_now_send(peer_addr, (uint8_t *)&myData, sizeof(myData));

        if (result == ESP_OK) {
        } else {
            Serial.println("Error sending the data");
        }
        currentMillis = millis();
    }
}

void tx_data_received(const uint8_t *mac, const uint8_t *incomingData, int len) { Serial.printf("Got pkt: %s\n", incomingData); }
#else
void tx_setup() {}

void tx_loop() {}

void tx_data_received(const uint8_t *mac, const uint8_t *incomingData, int len) {}
#endif