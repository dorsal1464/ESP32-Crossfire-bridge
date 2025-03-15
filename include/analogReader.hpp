#pragma once
#include <Arduino.h>

class analogReader {
   private:
    uint8_t _pin;
    int _average_filter;
    int _min_value;
    int _max_value;
    uint _last_value;

   public:
    analogReader(uint8_t pin, int average_filter = 16, int min_value = 0, int max_value = 4096);
    void calibrate();
    uint16_t read();
    void begin();
    int getMinValue() { return _min_value; }
    int getMaxValue() { return _max_value; }
    ~analogReader();
};

analogReader::analogReader(uint8_t pin, int average_filter, int min_value, int max_value)
    : _pin(pin), _average_filter(average_filter), _min_value(min_value), _max_value(max_value) {
    _last_value = 0;
}

analogReader::~analogReader() {}

void analogReader::begin() { pinMode(_pin, ANALOG); }

uint16_t analogReader::read() {
    float value = 0;
    for (int i = 0; i < _average_filter; i++) {
        value += analogRead(_pin);
    }
    _last_value = map((uint)(value / _average_filter), 0, 4096, _min_value, _max_value);
    // Serial.println(_last_value);
    return _last_value;
}

void analogReader::calibrate() {
    unsigned long time = millis();
    uint16_t value = 0;
    while (millis() - time < 5000) {
        value = analogRead(_pin);
        if (value < _min_value) {
            _min_value = value;
        }
        if (value > _max_value) {
            _max_value = value;
        }
    }
}
