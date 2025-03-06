#include <Adafruit_SSD1306.h>
#include <AlfredoCRSF.h>
#include <Arduino.h>
#include <RotaryEncoder.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "defines.h"
#include "esp_wifi.h"
#include "tx.h"

#if defined(WROOM_TX)

static unsigned long last_pkt_time = 0;
static uint8_t peer_addr[] = RX_ADDR;
static rc_data_t rc_data = {0};
static RotaryEncoder left_encoder(CH_5_A_PIN, CH_5_B_PIN, RotaryEncoder::LatchMode::TWO03);
static RotaryEncoder right_encoder(CH_6_A_PIN, CH_6_B_PIN, RotaryEncoder::LatchMode::TWO03);
static Adafruit_SSD1306 display(128, 32, &Wire, -1);
// screen layout:
// mode -----------------------|---------signal-bars|
// batt -----------------------|-----servo-bars-----|
// distance & altitude --------|----servo-angles----|
// speed ----------------------|--------------------|
// heading --------------------|--------------------|

// splash screen logo

static unsigned get_switch_data(int pinUp, int pinDown) {
    unsigned ch = 0;
    if (digitalRead(pinUp) == LOW) {
        ch = CH_MAX;
    } else if (digitalRead(pinDown) == LOW) {
        ch = CH_MIN;
    } else {
        ch = CH_MAX / 2;
    }
    return ch;
}

void tx_setup() {
    pinMode(CH_1_PIN, ANALOG);
    pinMode(CH_2_PIN, ANALOG);
    pinMode(CH_3_PIN, ANALOG);
    pinMode(CH_4_PIN, ANALOG);
    // range is 0 to 4096
    left_encoder.setPosition(CH_MAX / 2);
    right_encoder.setPosition(CH_MAX / 2);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
    }
    display.clearDisplay();
    // display.drawBitmap(0, 0, splash_logo, 128, 32, 1);
    display.display();
    delay(2000);
    display.clearDisplay();
}

void tx_loop() {
    left_encoder.tick();
    right_encoder.tick();
    // get analog data - map analog read to CH_MIN and CH_MAX
    rc_data.channels.ch0 = map(analogRead(CH_1_PIN), 0, 4096, CH_MIN, CH_MAX);
    rc_data.channels.ch1 = map(analogRead(CH_2_PIN), 0, 4096, CH_MIN, CH_MAX);
    rc_data.channels.ch2 = map(analogRead(CH_3_PIN), 0, 4096, CH_MIN, CH_MAX);
    rc_data.channels.ch3 = map(analogRead(CH_4_PIN), 0, 4096, CH_MIN, CH_MAX);
    // cap encoders value between CH_MIN and CH_MAX
    rc_data.channels.ch4 = constrain(left_encoder.getPosition(), CH_MIN, CH_MAX);
    rc_data.channels.ch5 = constrain(right_encoder.getPosition(), CH_MIN, CH_MAX);
    // get switches data
    rc_data.channels.ch6 = get_switch_data(CH_7_UP_PIN, CH_7_DOWN_PIN);
    rc_data.channels.ch7 = get_switch_data(CH_8_UP_PIN, CH_8_DOWN_PIN);

    if (millis() - last_pkt_time > 1000 / 100) {
        esp_err_t result = esp_now_send(peer_addr, (uint8_t *)&rc_data, sizeof(rc_data));

        if (result != ESP_OK) {
            Serial.println("Error sending the data");
        }
        last_pkt_time = millis();
    }
}

void tx_data_received(const uint8_t *mac, const uint8_t *incomingData, int len) {
    Serial.printf("Got pkt: %s\n", incomingData);
    // parse telemetry data
    const telemetry_t *telemetry = (telemetry_t *)incomingData;
    Serial.printf("GPS: %d, %d, %d\n", telemetry->gps.latitude, telemetry->gps.longitude, telemetry->gps.altitude);
    Serial.printf("Vario: %d\n", telemetry->vario.verticalspd);
    Serial.printf("Baro: %d\n", telemetry->baro.altitude);
}
#else
void tx_setup() {}

void tx_loop() {}

void tx_data_received(const uint8_t *mac, const uint8_t *incomingData, int len) {}
#endif