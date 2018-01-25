//
//  VoodooI2CMultitouchInterface.cpp
//  VoodooI2C
//
//  Created by Alexandre on 22/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CMultitouchInterface.hpp"
#include "VoodooI2CMultitouchEngine.hpp"

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CMultitouchInterface, IOService);

void VoodooI2CMultitouchInterface::handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp) {
    int i, count;
    VoodooI2CMultitouchEngine* engine;

    for (i = 0, count = engines->getCount(); i < count; i++) {
        engine = OSDynamicCast(VoodooI2CMultitouchEngine, engines->getObject(i));
        if (!engine)
            continue;

        if (engine->handleInterruptReport(event, timestamp) == MultitouchReturnBreak)
            break;
    }
}

bool VoodooI2CMultitouchInterface::open(IOService* client) {
    VoodooI2CMultitouchEngine* engine = OSDynamicCast(VoodooI2CMultitouchEngine, client);

    if (!engine)
        return false;

    engines->setObject(engine);

    return true;
}

SInt8 VoodooI2CMultitouchInterface::orderEngines(VoodooI2CMultitouchEngine* a, VoodooI2CMultitouchEngine* b) {
    if (a->getScore() > b->getScore())
        return 1;
    else if (a->getScore() < b->getScore())
        return -1;
    else
        return 0;
}

bool VoodooI2CMultitouchInterface::start(IOService* provider) {
    if (!super::start(provider)) {
        return false;
    }

    engines = OSOrderedSet::withCapacity(1, (OSOrderedSet::OSOrderFunction)VoodooI2CMultitouchInterface::orderEngines);

    OSNumber* number = OSNumber::withNumber("0", 32);
    setProperty(kIOFBTransformKey, number);
    setProperty("VoodooI2CServices Supported", OSBoolean::withBoolean(true));

    return true;
}

void VoodooI2CMultitouchInterface::stop(IOService* provider) {
    if (engines) {
        engines->flushCollection();
        OSSafeReleaseNULL(engines);
    }
}
