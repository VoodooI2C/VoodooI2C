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

bool VoodooI2CNativeEngine::attach(IOService* provider) {
    if (!super::attach(provider))
        return false;

    return true;
}

void VoodooI2CNativeEngine::detach(IOService* provider) {
    super::detach(provider);
}

bool VoodooI2CNativeEngine::init(OSDictionary* properties) {
    if (!super::init(properties))
        return false;

    return true;
}

MultitouchReturn VoodooI2CNativeEngine::handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp) {
    if (simulator)
        simulator->constructReport(event, timestamp);

    return MultitouchReturnContinue;
}

void VoodooI2CNativeEngine::free() {
    super::free();
}

bool VoodooI2CNativeEngine::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    simulator = OSTypeAlloc(VoodooI2CMT2SimulatorDevice);
    actuator = OSTypeAlloc(VoodooI2CMT2ActuatorDevice);
    
    if (!simulator->init(NULL) ||
        !simulator->attach(this) ||
        !simulator->start(this)) {
        IOLog("%s Could not initialise simulator\n", getName());
        OSSafeReleaseNULL(simulator);
    }
    
    if (!actuator->init(NULL) ||
        !actuator->attach(this) ||
        !actuator->start(this)) {
        IOLog("%s Could not initialise actuator\n", getName());
        OSSafeReleaseNULL(actuator);
    }

    if (!simulator || !actuator)
        return false;

    return true;
}

void VoodooI2CNativeEngine::stop(IOService* provider) {
    if (simulator) {
        simulator->stop(this);
        simulator->detach(this);
        //OSSafeReleaseNULL(simulator);
    }
    
    if (actuator) {
        actuator->stop(this);
        actuator->detach(this);
        //OSSafeReleaseNULL(actuator);
    }
    
    super::stop(provider);
}
