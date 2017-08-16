//
//  VoodooI2CACPICRSParser.hpp
//  VoodooI2C
//
//  Created by CoolStar on 8/15/17.
//  Copyright © 2017 CoolStar. All rights reserved.
//

#include <stdint.h>

#ifndef VoodooI2CACPICRSParser_h
#define VoodooI2CACPICRSParser_h

struct i2c_info {
    bool resourceConsumer;
    bool deviceInitiated;
    bool addressMode10Bit;
    
    uint32_t busSpeed;
    uint16_t address;
};

struct gpio_int_info {
    bool resourceConsumer;
    bool levelInterrupt;
    
    uint8_t interruptPolarity;
    
    bool sharedInterrupt;
    bool wakeInterrupt;
    
    uint8_t pinConfig;
    uint16_t pinNumber;
    
    int irqType;
};

struct gpio_io_info {
    bool resourceConsumer;
    uint8_t ioRestriction;
    bool sharing;
    
    uint8_t pinConfig;
    uint16_t pinNumber;
};

class VoodooI2CACPICRSParser {
public:
    bool foundI2C;
    bool foundGPIOInt;
    bool foundGPIOIO;
    
    i2c_info i2cInfo;
    gpio_int_info gpioInt;
    gpio_io_info gpioIO;
    
    VoodooI2CACPICRSParser();
    void parse_acpi_crs(uint8_t *crs, uint32_t offset, uint32_t sz);
private:
    void parse_acpi_serialbus(uint8_t *crs, uint32_t offset, uint32_t sz);
    void parse_acpi_gpio(uint8_t *crs, uint32_t offset, uint32_t sz);
};

#endif /* VoodooI2CACPICRSParser_h */
