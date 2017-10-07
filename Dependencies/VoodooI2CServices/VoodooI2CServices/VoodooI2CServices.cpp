//
//  VoodooI2CServices.cpp
//  VoodooI2CServices
//
//  Created by Alexandre on 30/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CServices.hpp"

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CServices, IOService);

bool VoodooI2CServices::attachDevice(void* target, void* ref_con, IOService* new_service, IONotifier* notifier) {
    IOService* target_service = OSDynamicCast(IOService, target);
    IOLog("class: %s\n", target_service->getMetaClass()->getClassName());

    return true;
}

bool VoodooI2CServices::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    gVoodooI2CPlane = IORegistryEntry::makePlane(kVoodooI2CPlane);

    attachToParent(getRegistryRoot(), gVoodooI2CPlane);

    OSDictionary*  property_match = OSDictionary::withCapacity(1);

    device_matcher = addMatchingNotification(gIOMatchedNotification, property_match, OSMemberFunctionCast(IOServiceMatchingNotificationHandler, this, &VoodooI2CServices::attachDevice), this);
    device_matcher->retain();
    property_match->release();

    return true;
}

void VoodooI2CServices::stop(IOService* provider) {
    device_matcher->disable();
    OSSafeReleaseNULL(device_matcher);

    super::stop(provider);
}
