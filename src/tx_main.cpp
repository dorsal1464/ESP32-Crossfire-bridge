#include <Adafruit_SSD1306.h>
#include <AlfredoCRSF.h>
#include <Arduino.h>
#include <RotaryEncoder.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include "analogReader.hpp"
#include "defines.h"
#include "esp_wifi.h"
#include "tx.h"

#if defined(WROOM_TX)

static unsigned long last_pkt_time = 0;
static unsigned long last_print = 0;
static uint8_t peer_addr[] = RX_ADDR;
static rc_data_t rc_data = {0};
static RotaryEncoder left_encoder(CH_5_A_PIN, CH_5_B_PIN, RotaryEncoder::LatchMode::TWO03);
static RotaryEncoder right_encoder(CH_6_A_PIN, CH_6_B_PIN, RotaryEncoder::LatchMode::TWO03);
static analogReader ch1(CH_1_PIN, 16, CH_MIN, CH_MAX);
static analogReader ch2(CH_2_PIN, 16, CH_MIN, CH_MAX);
static analogReader ch3(CH_3_PIN, 16, CH_MIN, CH_MAX);
static analogReader ch4(CH_4_PIN, 16, CH_MIN, CH_MAX);

#ifdef HAS_DISPLAY
static Adafruit_SSD1306 display(128, 32, &Wire, -1);
#endif
// screen layout:
// mode -----------------------|---------signal-bars|
// batt -----------------------|-----servo-bars-----|
// distance & altitude --------|----servo-angles----|
// speed ----------------------|--------------------|
// heading --------------------|--------------------|

// splash screen logo

static unsigned get_switch_data(int pinUp, int pinDown, bool dbg = 0) {
    unsigned ch = 0;
    if (dbg) Serial.printf("Switch data: %d, %d\n", digitalRead(pinUp), digitalRead(pinDown));

    if (digitalRead(pinUp) == LOW) {
        ch = CH_MAX - 1;
    } else if (digitalRead(pinDown) == LOW) {
        ch = CH_MIN;
    } else {
        ch = CH_MAX / 2;
    }
    return ch;
}

static int get_rotary_data(RotaryEncoder &encoder) {
    // Serial.printf("Encoder: %d\n", encoder.getPosition() * STEP);
    long pos = encoder.getPosition() * STEP;
    if (pos < CH_MIN) {
        encoder.setPosition(CH_MIN);
        pos = CH_MIN;
    }
    if (pos > CH_MAX - 1) {
        encoder.setPosition(((CH_MAX) / STEP) - 1);
        pos = CH_MAX - 1;
    }
    return pos;
}

void tx_setup() {
    ch1.begin();
    ch2.begin();
    ch3.begin();
    ch4.begin();
    // skip ch 5 and 6
    pinMode(CH_7_DOWN_PIN, INPUT_PULLUP);
    pinMode(CH_7_UP_PIN, INPUT_PULLUP);
    pinMode(CH_8_DOWN_PIN, INPUT_PULLUP);
    pinMode(CH_8_UP_PIN, INPUT_PULLUP);
    // range is 0 to 4096
    left_encoder.setPosition(0);
    right_encoder.setPosition(0);
#ifdef HAS_DISPLAY
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
    }
    display.clearDisplay();
    // display.drawBitmap(0, 0, splash_logo, 128, 32, 1);
    display.display();
    delay(2000);
    display.clearDisplay();
#endif
}

void tx_loop() {
    left_encoder.tick();
    right_encoder.tick();
    // get analog data - map analog read to CH_MIN and CH_MAX
    rc_data.channels.ch0 = ch1.read();
    rc_data.channels.ch1 = ch2.read();
    rc_data.channels.ch2 = ch3.read();
    rc_data.channels.ch3 = ch4.read();
    // cap encoders value between CH_MIN and CH_MAX
    int pos = left_encoder.getPosition();
    rc_data.channels.ch4 = get_rotary_data(left_encoder);
    rc_data.channels.ch5 = get_rotary_data(right_encoder);
    // get switches data
    rc_data.channels.ch6 = get_switch_data(CH_7_UP_PIN, CH_7_DOWN_PIN);
    rc_data.channels.ch7 = get_switch_data(CH_8_UP_PIN, CH_8_DOWN_PIN);

    if (millis() - last_pkt_time > 1000 / 250) {
        esp_err_t result = esp_now_send(peer_addr, (uint8_t *)&rc_data, sizeof(rc_data));

        if (result != ESP_OK) {
            // Serial.println("Error sending the data");
        }
        last_pkt_time = millis();
    }
    if (millis() - last_print > 1000 / 20) {
        // Serial.printf("%d\n", rc_data.channels.ch4);
        get_switch_data(CH_7_UP_PIN, CH_7_DOWN_PIN, 0);
        get_switch_data(CH_8_UP_PIN, CH_8_DOWN_PIN, 0);
        Serial.printf("Channels: [%04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d]\n", rc_data.channels.ch0, rc_data.channels.ch1,
                      rc_data.channels.ch2, rc_data.channels.ch3, rc_data.channels.ch4, rc_data.channels.ch5,
                      rc_data.channels.ch6, rc_data.channels.ch7);
        last_print = millis();
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