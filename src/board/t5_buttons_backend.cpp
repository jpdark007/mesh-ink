#include <Arduino.h>
#include <driver/i2c.h>

#include "board_profile.h"
#include "t5_buttons_backend.h"

void meshink_buttons_begin() {
    pinMode(T5_PIN_BOOT_BUTTON,INPUT_PULLUP);
}

bool meshink_primary_button_pressed() {
    return digitalRead(T5_PIN_BOOT_BUTTON)==LOW;
}

const char* meshink_primary_button_name() {
    return "BOOT";
}

static bool pca9535_read(uint8_t reg,uint8_t& value) {
    return i2c_master_write_read_device(
        I2C_NUM_0,T5_PCA9535_ADDR,&reg,1,&value,1,pdMS_TO_TICKS(20))==ESP_OK;
}

static bool pca9535_write(uint8_t reg,uint8_t value) {
    const uint8_t data[2]={reg,value};
    return i2c_master_write_to_device(
        I2C_NUM_0,T5_PCA9535_ADDR,data,sizeof(data),pdMS_TO_TICKS(20))==ESP_OK;
}

bool meshink_channel_button_pressed() {
    // IO12 is PCA9535 port 1, bit 2. The button pulls it low when pressed.
    constexpr uint8_t config_port1=0x07;
    constexpr uint8_t input_port1=0x01;
    constexpr uint8_t button_bit=1U<<(T5_PCA9535_BUTTON_IO%8);
    static bool raw_state=false;
    static bool stable_state=false;
    static uint32_t raw_changed_at=0;
    uint8_t config=0,value=0;
    if(!pca9535_read(config_port1,config))return false;
    if(!(config&button_bit)&&!pca9535_write(config_port1,(uint8_t)(config|button_bit)))return false;
    if(!pca9535_read(input_port1,value))return false;
    const bool raw_pressed=(value&button_bit)==0;
    if(raw_pressed!=raw_state){raw_state=raw_pressed;raw_changed_at=millis();}
    if(millis()-raw_changed_at>=5)stable_state=raw_state;
    return stable_state;
}

const char* meshink_channel_button_name() {
    return "IO12";
}
