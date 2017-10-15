//
//  VoodooI2CServices.cpp
//  VoodooI2CServices
//
//  Created by Alexandre on 30/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CServices.hpp"

const IORegistryPlane* VoodooI2CServices::gVoodooI2CPlane = NULL;

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CServices, IOService);

bool VoodooI2CServices::attachDevice(void* target, void* ref_con, IOService* new_service, IONotifier* notifier) {
    IOLog("class: %s\n", new_service->getMetaClass()->getClassName());
    OSBoolean* bool_obj = OSDynamicCast(OSBoolean, new_service->getProperty("isI2CController"));
    if (bool_obj && bool_obj->isTrue()) {
        new_service->attachToParent(getRegistryRoot(), gVoodooI2CPlane);
    } else {
        IORegistryEntry* parent = new_service->getParentEntry(gIOServicePlane);
        new_service->attachToParent(parent, gVoodooI2CPlane);
        // parent->release();
    }

    return true;
}

bool VoodooI2CServices::detachDevice(void* target, void* ref_con, IOService* new_service, IONotifier* notifier) {
    IORegistryEntry* parent = new_service->getParentEntry(gVoodooI2CPlane);
    new_service->detachFromParent(parent, gVoodooI2CPlane);
    // parent->release();

    return true;
}

bool VoodooI2CServices::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    gVoodooI2CPlane = IORegistryEntry::makePlane(kVoodooI2CPlane);

    attachToParent(getRegistryRoot(), gVoodooI2CPlane);

    OSDictionary*  property_match = OSDictionary::withCapacity(1);
    OSDictionary* service_match = OSDictionary::withCapacity(1);

    service_match->setObject("VoodooI2CServices Supported", kOSBooleanTrue);
    property_match->setObject(gIOPropertyMatchKey, service_match);

    device_matcher = addMatchingNotification(gIOMatchedNotification, property_match, VoodooI2CServices::attachDevice, this, NULL, 0);
    terminate_matcher = addMatchingNotification(gIOTerminatedNotification, property_match, VoodooI2CServices::detachDevice, this, NULL, 0);

    service_match->release();
    property_match->release();

    registerService();

    return true;
}

void VoodooI2CServices::stop(IOService* provider) {
    device_matcher->remove();

    detachFromParent(getRegistryRoot(), gVoodooI2CPlane);

    super::stop(provider);
}
