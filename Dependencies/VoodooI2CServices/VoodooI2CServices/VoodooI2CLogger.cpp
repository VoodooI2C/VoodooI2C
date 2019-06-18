//
//  VoodooI2CLogger.cpp
//  VoodooI2CServices
//
//  Created by Alexandre on 09/10/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CLogger.hpp"
#include "VoodooI2CServices.hpp"

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CLogger, IOService);

bool VoodooI2CLogger::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    UInt8 argument;

    if (PE_parse_boot_argn("voodooi2c-logging-level", &argument, sizeof(argument))) {
        if (argument >= 0 && argument <= 2) {
            logging_level = argument;
        } else {
            logging_level = kVoodooI2CNormalLogging;
        }
    } else {
        logging_level = kVoodooI2CNormalLogging;
    }

    if (!logging_level)
        return false;

    attachToParent(provider, VoodooI2CServices::gVoodooI2CPlane);

    return true;
}

void VoodooI2CLogger::stop(IOService *provider) {
    detachFromChild(provider, VoodooI2CServices::gVoodooI2CPlane);
    super::stop(provider);
}
