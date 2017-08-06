//
//  VoodooI2CControllerDriver.cpp
//  VoodooI2C
//
//  Created by Alexandre on 03/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CControllerDriver.hpp"

#define readRegister(X) nub->readRegister(X)
#define writeRegister(X, Y) nub->writeRegister(X, Y)

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CControllerDriver, IOService);

/**
 Frees VoodooI2CControllerNub class - releases objects instantiated in `init`
 */

void VoodooI2CControllerDriver::free() {
    IOFree(bus_device->acpi_config, sizeof(VoodooI2CControllerBusConfig));
    IOFree(bus_device, sizeof(bus_device));

    super::free();
}

/**
 Requests the nub to fetch bus configuration values from the ACPI tables

 @return returns kIOReturnSuccess if all desired values were obtained,
         else returns kIOReturnNotFound if (some or all) configuration values
         are missing
 */

IOReturn VoodooI2CControllerDriver::getBusConfig() {
    bool error = false;

    bus_device->tx_fifo_depth = 32;
    bus_device->rx_fifo_depth = 32;

    if (nub->getACPIParams((const char*)"SSCN", &bus_device->acpi_config->ss_hcnt, &bus_device->acpi_config->ss_lcnt, NULL) != kIOReturnSuccess)
        error = true;

    if (nub->getACPIParams((const char*)"FMCN", &bus_device->acpi_config->fs_hcnt, &bus_device->acpi_config->fs_lcnt, &bus_device->acpi_config->sda_hold) != kIOReturnSuccess)
        error = true;

    if (error)
        return kIOReturnNotFound;
    else
        return kIOReturnSuccess;
}

/**
 Initialises VoodooI2CControllerNub class
 
 @param properties OSDictionary* representing the matched personality
 
 @return returns true on successful initialisation, else returns false
 */

bool VoodooI2CControllerDriver::init(OSDictionary* properties) {
    if (!super::init(properties))
        return false;

    bus_device = reinterpret_cast<VoodooI2CControllerBusDevice*>(IOMalloc(sizeof(VoodooI2CControllerBusDevice)));
    bus_device->acpi_config = reinterpret_cast<VoodooI2CControllerBusConfig*>(IOMalloc(sizeof(VoodooI2CControllerBusConfig)));

    return true;
}

/**
 Initialises the bus by writing in configuration values

 @return returns kIOReturnSuccess on successful initialisation, else returns kIOReturnError
 */

IOReturn VoodooI2CControllerDriver::initialiseBus() {
    if (toggleBusState(kVoodooI2CPowerStateOff) != kIOReturnSuccess)
        return kIOReturnError;

    writeRegister(bus_device->acpi_config->ss_hcnt, DW_IC_SS_SCL_HCNT);
    writeRegister(bus_device->acpi_config->ss_lcnt, DW_IC_SS_SCL_LCNT);
    writeRegister(bus_device->acpi_config->fs_hcnt, DW_IC_FS_SCL_HCNT);
    writeRegister(bus_device->acpi_config->fs_lcnt, DW_IC_FS_SCL_LCNT);

    UInt32 reg = readRegister(DW_IC_COMP_VERSION);

    if  (reg >= DW_IC_SDA_HOLD_MIN_VERS)
        writeRegister(bus_device->acpi_config->sda_hold, DW_IC_SDA_HOLD);
    else
        IOLog("%s::%s Warning: hardware too old to adjust SDA hold time\n", getName(), bus_device->name);

    writeRegister(bus_device->tx_fifo_depth - 1, DW_IC_TX_TL);
    writeRegister(0, DW_IC_RX_TL);
    writeRegister(bus_device->bus_config, DW_IC_CON);

    return kIOReturnSuccess;
}

/**
 Probes the device to determine whether or not we can drive it
 
 @param provider IOService* representing the matched entry in the IORegistry
 @param score    Probe score as specified in the matched personality
 
 @return returns a pointer to this instance of VoodooI2CControllerDriver
 */

VoodooI2CControllerDriver* VoodooI2CControllerDriver::probe(IOService* provider, SInt32* score) {
    UInt32 reg;

    if (!super::probe(provider, score)) {
        return NULL;
    }

    nub = OSDynamicCast(VoodooI2CControllerNub, provider);

    bus_device->name = nub->name;

    IOLog("%s::%s Probing controller\n", getName(), bus_device->name);

    reg = readRegister(DW_IC_COMP_TYPE);

    if (reg == DW_IC_COMP_TYPE_VALUE) {
        IOLog("%s::%s Found valid Synopsys component, continuing with initialisation\n", getName(), bus_device->name);
    } else {
        IOLog("%s::%s Unknown Synopsys component type: 0x%08x\n", getName(), bus_device->name, reg);
        return NULL;
    }

    return this;
}

bool VoodooI2CControllerDriver::start(IOService* provider) {
    if (getBusConfig() != kIOReturnSuccess)
        IOLog("%s::%s Warning: Error getting bus config, using defaults where necessary\n", getName(), bus_device->name);
    else
        IOLog("%s::%s Got bus configuration values\n", getName(), bus_device->name);

    bus_device->functionality = I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR | I2C_FUNC_SMBUS_BYTE | I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA | I2C_FUNC_SMBUS_I2C_BLOCK;
    bus_device->bus_config = DW_IC_CON_MASTER | DW_IC_CON_SLAVE_DISABLE | DW_IC_CON_RESTART_EN | DW_IC_CON_SPEED_FAST;

    if (initialiseBus() != kIOReturnSuccess) {
        IOLog("%s::%s Could not initialise bus\n", getName(), bus_device->name);
        return false;
    }

    return true;
}

/**
 Toggle the bus's enabled state
 
 @param enabled VoodooI2CPowerState representing the enabled state
 
 @return returns kIOReturnSuccess on successful state toggle, else returns kIOReturnTimeout
 */

IOReturn VoodooI2CControllerDriver::toggleBusState(VoodooI2CPowerState enabled) {
    int timeout = 500;

    do {
        writeRegister(enabled, DW_IC_ENABLE);

        if ((readRegister(DW_IC_ENABLE_STATUS) & 1) == enabled) {
            toggleClockGating(enabled);
            return kIOReturnSuccess;
        }

        IODelay(250);
    } while (timeout--);

    IOLog("%s::%s Timed out waiting for bus to change state\n", getName(), bus_device->name);
    return kIOReturnTimeout;
}

inline void VoodooI2CControllerDriver::toggleClockGating(VoodooI2CPowerState enabled) {
    writeRegister(enabled, LPSS_PRIVATE_CLOCK_GATING);
}
