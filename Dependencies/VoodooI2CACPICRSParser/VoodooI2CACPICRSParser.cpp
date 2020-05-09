//
//  VoodooI2CACPICRSParser.cpp
//  VoodooI2C
//
//  Created by CoolStar on 8/15/17.
//  Copyright © 2017 CoolStar. All rights reserved.
//

// Demo Standalone Program: https://ghostbin.com/paste/tqt73

#include <IOKit/IOLib.h>
#include <string.h>

#include "VoodooI2CACPICRSParser.hpp"
#include "linuxirq.hpp"

VoodooI2CACPICRSParser::VoodooI2CACPICRSParser() {
    found_gpio_interrupts = false;
    found_i2c = false;
}

void VoodooI2CACPICRSParser::parseACPISerialBus(uint8_t const* crs, uint32_t offset, uint32_t sz) {
    if (found_i2c)
        return;
    if (offset >= sz)
        return;
    uint8_t opcode = crs[offset];
    if (opcode != 0x8e)
        return;
    
    uint16_t len;
    memcpy(&len, crs + offset + 1, sizeof(uint16_t));
    
    uint8_t bustype = crs[offset + 5];
    if (bustype != 1)
        return; // Only support I2C. Bus type 2 = SPI, 3 = UART
    
    uint8_t flags = crs[offset + 6];
    
    uint16_t tflags;
    memcpy(&tflags, crs + offset + 7, sizeof(uint16_t));
    
    /*uint16_t datalen;
    memcpy(&datalen, crs + offset + 10, sizeof(uint16_t));*/
    
    if (bustype == 1) {
        found_i2c = true;
        
        i2c_info.resource_consumer = (flags >> 1) & 0x1;
        i2c_info.device_initiated = flags & 0x1;
        i2c_info.address_mode_10Bit = tflags & 0x1;
        
        uint32_t busspeed;
        memcpy(&busspeed, crs + offset + 12, sizeof(uint32_t));
        i2c_info.bus_speed = busspeed;
        
        uint16_t address;
        memcpy(&address, crs + offset + 16, sizeof(uint16_t));
        i2c_info.address = address;
        
        /*uint8_t i2cLen = len - (12 + datalen - 3);
        
        char *i2cBus = (char *)malloc(i2cLen);
        memcpy(i2cBus, crs + offset + 12 + datalen, i2cLen);
        IOLog("I2C Bus: %s\n", i2cBus);*/
    }
}

void VoodooI2CACPICRSParser::parseACPIGPIO(uint8_t const* crs, uint32_t offset, uint32_t sz) {
    if (found_gpio_interrupts)
        return;
    if (offset >= sz)
        return;
    
    uint8_t opcode = crs[offset];
    if (opcode != 0x8c)
        return;
    
    uint16_t len;
    memcpy(&len, crs + offset + 1, sizeof(uint16_t));
    
    uint8_t gpio_type = crs[offset + 4];
    uint8_t flags = crs[offset + 5];
    
    uint8_t gpio_flags = crs[offset + 7];
    
    uint8_t pin_config = crs[offset + 9];
    
    uint16_t pin_offset;
    memcpy(&pin_offset, crs + offset + 14, sizeof(uint16_t));
    
    /*uint16_t resourceOffset;
    memcpy(&resourceOffset, crs + offset + 17, sizeof(uint16_t));
    
    uint16_t vendorOffset;
    memcpy(&vendorOffset, crs + offset + 19, sizeof(uint16_t));*/
    
    uint16_t pin_number;
    memcpy(&pin_number, crs + offset + pin_offset, sizeof(uint16_t));
    
    if (pin_number == 0xFFFF) // pinNumber 0xFFFF is invalid
        return;
    
    /*char *gpioController = (char *)malloc(vendorOffset - resourceOffset);
    memcpy(gpioController, crs + offset + resourceOffset, vendorOffset - resourceOffset);
    IOLog("GPIO Controller: %s\n", gpioController);*/
    
    if (gpio_type == 0) {
        // GPIOInt
        
        found_gpio_interrupts = true;
        
        gpio_interrupts.resource_consumer = flags & 0x1;
        gpio_interrupts.level_interrupt = !(gpio_flags & 0x1);
        
        gpio_interrupts.interrupt_polarity = (gpio_flags >> 1) & 0x3;
        
        gpio_interrupts.shared_interrupt = (gpio_flags >> 3) & 0x1;
        gpio_interrupts.wake_interrupt = (gpio_flags >> 4) & 0x1;
        
        gpio_interrupts.pin_config = pin_config;
        gpio_interrupts.pin_number = pin_number;
        
        int irq = 0;
        if (gpio_interrupts.level_interrupt) {
            switch (gpio_interrupts.interrupt_polarity) {
                case 0:
                    irq = IRQ_TYPE_LEVEL_HIGH;
                    break;
                case 1:
                    irq = IRQ_TYPE_LEVEL_LOW;
                    break;
                default:
                    irq = IRQ_TYPE_LEVEL_LOW;
                    break;
            }
        } else {
            switch (gpio_interrupts.interrupt_polarity) {
                case 0:
                    irq = IRQ_TYPE_EDGE_FALLING;
                    break;
                case 1:
                    irq = IRQ_TYPE_EDGE_RISING;
                    break;
                case 2:
                    irq = IRQ_TYPE_EDGE_BOTH;
                    break;
                default:
                    irq = IRQ_TYPE_EDGE_FALLING;
                    break;
            }
        }
        gpio_interrupts.irq_type = irq;
    }
    
    if (gpio_type == 1) {
        // GPIOIo
        
        found_gpio_io = true;
        
        gpio_io.resource_consumer = flags & 0x1;
        gpio_io.io_restriction = gpio_flags & 0x3;
        gpio_io.sharing = (gpio_flags >> 3) & 0x1;
        
        gpio_io.pin_config = pin_config;
        gpio_io.pin_number = pin_number;
    }
}

void VoodooI2CACPICRSParser::parseACPICRS(uint8_t const* crs, uint32_t offset, uint32_t sz) {
    if (offset >= sz)
        return;
    
    uint8_t opcode = crs[offset];
    
    uint16_t len;
    memcpy(&len, crs + offset + 1, sizeof(uint16_t));
    
    if (opcode == 0x8c)
        parseACPIGPIO(crs, offset, sz);
    if (opcode == 0x8e)
        parseACPISerialBus(crs, offset, sz);
    
    offset += (len + 3);
    parseACPICRS(crs, offset, sz);
}
