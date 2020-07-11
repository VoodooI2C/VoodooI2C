//
//  VoodooI2CMultitouchEngine.cpp
//  VoodooI2C
//
//  Created by Alexandre on 22/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CMultitouchEngine.hpp"
#include "VoodooI2CMultitouchInterface.hpp"

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CMultitouchEngine, IOService);

UInt8 VoodooI2CMultitouchEngine::getScore() {
    return 0x0;
}

MultitouchReturn VoodooI2CMultitouchEngine::handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp) {
    if (event.contact_count)
        IOLog("Contact Count: %d\n", event.contact_count);
    
    for (int index = 0, count = event.transducers->getCount(); index < count; index++) {
        VoodooI2CDigitiserTransducer* transducer = OSDynamicCast(VoodooI2CDigitiserTransducer, event.transducers->getObject(index));
        
        if (!transducer)
            continue;
        
        if (transducer->tip_switch)
            IOLog("Transducer ID: %d, X: %d, Y: %d, Z: %d, Pressure: %d\n", transducer->secondary_id, transducer->coordinates.x.value(), transducer->coordinates.y.value(), transducer->coordinates.z.value(), transducer->tip_pressure.value());
    }

    return MultitouchReturnContinue;
}

bool VoodooI2CMultitouchEngine::willTerminate(IOService* provider, IOOptionBits options) {
    if (provider->isOpen(this))
        provider->close(this);

    return super::willTerminate(provider, options);
}

bool VoodooI2CMultitouchEngine::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    interface = OSDynamicCast(VoodooI2CMultitouchInterface, provider);

    if (!interface)
        return false;

    interface->open(this);

    setProperty("VoodooI2CServices Supported", kOSBooleanTrue);

    registerService();

    return true;
}

void VoodooI2CMultitouchEngine::onPropertyChange() {
    return;
}
