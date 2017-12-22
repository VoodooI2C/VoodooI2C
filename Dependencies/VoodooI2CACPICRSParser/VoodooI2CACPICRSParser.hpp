//
//  VoodooI2CACPICRSParser.hpp
//  VoodooI2C
//
//  Created by CoolStar on 8/15/17.
//  Copyright © 2017 CoolStar. All rights reserved.
//

#include <stdint.h>

#ifndef VoodooI2CACPICRSParser_hpp
#define VoodooI2CACPICRSParser_hpp

struct i2c_info {
    bool resource_consumer;
    bool device_initiated;
    bool address_mode_10Bit;
    
    uint32_t bus_speed;
    uint16_t address;
};

struct gpio_int_info {
    bool resource_consumer;
    bool level_interrupt;
    
    uint8_t interrupt_polarity;
    
    bool shared_interrupt;
    bool wake_interrupt;
    
    uint8_t pin_config;
    uint16_t pin_number;
    
    int irq_type;
};

struct gpio_io_info {
    bool resource_consumer;
    uint8_t io_restriction;
    bool sharing;
    
    uint8_t pin_config;
    uint16_t pin_number;
};

class VoodooI2CACPICRSParser {
public:
    bool found_i2c;
    bool found_gpio_interrupts;
    bool found_gpio_io;
    
    i2c_info i2c_info;
    gpio_int_info gpio_interrupts;
    gpio_io_info gpio_io;
    
    VoodooI2CACPICRSParser();
    void parseACPICRS(uint8_t const* crs, uint32_t offset, uint32_t sz);
private:
    void parseACPISerialBus(uint8_t const* crs, uint32_t offset, uint32_t sz);
    void parseACPIGPIO(uint8_t const* crs, uint32_t offset, uint32_t sz);
};

#endif /* VoodooI2CACPICRSParser_hpp */
