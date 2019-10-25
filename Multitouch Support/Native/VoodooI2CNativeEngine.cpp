//
//  VoodooI2CNativeEngine.cpp
//  VoodooI2C
//
//  Created by Alexandre on 10/02/2018.
//  Copyright © 2018 Alexandre Daoud and Kishor Prins. All rights reserved.
//

#include "VoodooI2CNativeEngine.hpp"

#define super VoodooI2CMultitouchEngine
OSDefineMetaClassAndStructors(VoodooI2CNativeEngine, VoodooI2CMultitouchEngine);

MultitouchReturn VoodooI2CNativeEngine::handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp) {
    if (simulator)
        simulator->constructReport(event, timestamp);

    return MultitouchReturnContinue;
}

bool VoodooI2CNativeEngine::init(OSDictionary* properties) {
    if (!super::init(properties))
        return false;

    simulator = OSTypeAlloc(VoodooI2CMT2SimulatorDevice);
    actuator = OSTypeAlloc(VoodooI2CMT2ActuatorDevice);

    if (!simulator || !actuator) {
        OSSafeReleaseNULL(simulator);
        OSSafeReleaseNULL(actuator);
        return false;
    }

    return true;
}

void VoodooI2CNativeEngine::free() {
    OSSafeReleaseNULL(simulator);
    OSSafeReleaseNULL(actuator);

    super::free();
}

bool VoodooI2CNativeEngine::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    // Initialize simulator device
    if (!simulator->init(NULL) || !simulator->attach(this))
        goto exit;
    else if (!simulator->start(this)) {
        simulator->detach(this);
        goto exit;
    }

    // Initialize actuator device
    if (!actuator->init(NULL) || !actuator->attach(this))
        goto exit;
    else if (!actuator->start(this)) {
        actuator->detach(this);
        goto exit;
    }

    return true;

exit:
    IOLog("%s Failed initializing simulator and actuator devices", getName());
    return false;
}

void VoodooI2CNativeEngine::stop(IOService* provider) {
    if (simulator) {
        simulator->stop(this);
        simulator->detach(this);
    }

    if (actuator) {
        actuator->stop(this);
        actuator->detach(this);
    }

    super::stop(provider);
}
