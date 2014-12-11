//
//  VoodooACPII2CNub.cpp
//  VoodooI2C
//
//  Created by Alexandre on 10/12/2014.
//  Copyright (c) 2014 Alexandre Daoud. All rights reserved.
//

#include "VoodooACPII2CNub.h"

OSDefineMetaClassAndStructors(VoodooACPII2CNub, IOService);

bool VoodooACPII2CNub::start(IOService* provider) {
    if (!super::start(provider))
        return false;
    
    return true;
}

void VoodooACPII2CNub::stop(IOService* provider) {
    
}
