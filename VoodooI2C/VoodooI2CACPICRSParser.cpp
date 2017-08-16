//
//  VoodooI2CACPICRSParser.cpp
//  VoodooI2C
//
//  Created by CoolStar on 8/15/17.
//  Copyright © 2017 CoolStar. All rights reserved.
//

// Demo Standalone Program: https://ghostbin.com/paste/tqt73

#include "VoodooI2CACPICRSParser.h"
#include <IOKit/IOLib.h>
#include <string.h>
#include "linuxirq.h"

VoodooI2CACPICRSParser::VoodooI2CACPICRSParser(){
    foundGPIOInt = false;
    foundI2C = false;
}

void VoodooI2CACPICRSParser::parse_acpi_serialbus(uint8_t *crs, uint32_t offset, uint32_t sz){
    if (foundI2C)
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
        return; //Only support I2C. Bus type 2 = SPI, 3 = UART
    
    uint8_t flags = crs[offset + 6];
    
    uint16_t tflags;
    memcpy(&tflags, crs + offset + 7, sizeof(uint16_t));
    
    /*uint16_t datalen;
    memcpy(&datalen, crs + offset + 10, sizeof(uint16_t));*/
    
    if (bustype == 1){
        foundI2C = true;
        
        i2cInfo.resourceConsumer = (flags >> 1) & 0x1;
        i2cInfo.deviceInitiated = flags & 0x1;
        i2cInfo.addressMode10Bit = tflags & 0x1;
        
        uint32_t busspeed;
        memcpy(&busspeed, crs + offset + 12, sizeof(uint32_t));
        i2cInfo.busSpeed = busspeed;
        
        uint16_t address;
        memcpy(&address, crs + offset + 16, sizeof(uint16_t));
        i2cInfo.address = address;
        
        /*uint8_t i2cLen = len - (12 + datalen - 3);
        
        char *i2cBus = (char *)malloc(i2cLen);
        memcpy(i2cBus, crs + offset + 12 + datalen, i2cLen);
        IOLog("I2C Bus: %s\n", i2cBus);*/
    }
}

void VoodooI2CACPICRSParser::parse_acpi_gpio(uint8_t *crs, uint32_t offset, uint32_t sz){
    if (foundGPIOInt)
        return;
    if (offset >= sz)
        return;
    
    uint8_t opcode = crs[offset];
    if (opcode != 0x8c)
        return;
    
    uint16_t len;
    memcpy(&len, crs + offset + 1, sizeof(uint16_t));
    
    uint8_t gpiotype = crs[offset + 4];
    uint8_t flags = crs[offset + 5];
    
    uint8_t gpioFlags = crs[offset + 7];
    
    uint8_t pinConfig = crs[offset + 9];
    
    uint16_t pinOffset;
    memcpy(&pinOffset, crs + offset + 14, sizeof(uint16_t));
    
    /*uint16_t resourceOffset;
    memcpy(&resourceOffset, crs + offset + 17, sizeof(uint16_t));
    
    uint16_t vendorOffset;
    memcpy(&vendorOffset, crs + offset + 19, sizeof(uint16_t));*/
    
    uint16_t pinNumber;
    memcpy(&pinNumber, crs + offset + pinOffset, sizeof(uint16_t));
    
    if (pinNumber == 0xFFFF) //pinNumber 0xFFFF is invalid
        return;
    
    /*char *gpioController = (char *)malloc(vendorOffset - resourceOffset);
    memcpy(gpioController, crs + offset + resourceOffset, vendorOffset - resourceOffset);
    IOLog("GPIO Controller: %s\n", gpioController);*/
    
    if (gpiotype == 0){
        //GPIOInt
        
        foundGPIOInt = true;
        
        gpioInt.resourceConsumer = flags & 0x1;
        gpioInt.levelInterrupt = !(gpioFlags & 0x1);
        
        gpioInt.interruptPolarity = (gpioFlags >> 1) & 0x3;
        
        gpioInt.sharedInterrupt = (gpioFlags >> 3) & 0x1;
        gpioInt.wakeInterrupt = (gpioFlags >> 4) & 0x1;
        
        gpioInt.pinConfig = pinConfig;
        gpioInt.pinNumber = pinNumber;
        
        int irq = 0;
        if (gpioInt.levelInterrupt){
            switch (gpioInt.interruptPolarity){
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
            switch (gpioInt.interruptPolarity) {
                case 0:
                    irq = IRQ_TYPE_EDGE_FALLING;
                    break;
                case 1:
                    irq = IRQ_TYPE_EDGE_RISING;
                    break;
                case 2:
                    irq = IRQ_TYPE_EDGE_BOTH;
                default:
                    irq = IRQ_TYPE_EDGE_FALLING;
                    break;
            }
        }
        gpioInt.irqType = irq;
    }
    
    if (gpiotype == 1){
        //GPIOIo
        
        foundGPIOIO = true;
        
        gpioIO.resourceConsumer = flags & 0x1;
        gpioIO.ioRestriction = gpioFlags & 0x3;
        gpioIO.sharing = (gpioFlags >> 3) & 0x1;
        
        gpioIO.pinConfig = pinConfig;
        gpioIO.pinNumber = pinNumber;
    }
}

void VoodooI2CACPICRSParser::parse_acpi_crs(uint8_t *crs, uint32_t offset, uint32_t sz){
    if (offset >= sz)
        return;
    
    uint8_t opcode = crs[offset];
    
    uint16_t len;
    memcpy(&len, crs + offset + 1, sizeof(uint16_t));
    
    if (opcode == 0x8c)
        parse_acpi_gpio(crs, offset, sz);
    if (opcode == 0x8e)
        parse_acpi_serialbus(crs, offset, sz);
    
    offset += (len + 3);
    parse_acpi_crs(crs, offset, sz);
}
