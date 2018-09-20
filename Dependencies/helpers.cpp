//
//  helpers.cpp
//  VoodooI2C
//
//  Created by Alexandre on 31/07/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "helpers.hpp"

const char* getMatchedName(IOService* provider) {
    OSData *data;
    data = OSDynamicCast(OSData, provider->getProperty("name"));
    return (const char *)(data->getBytesNoCopy());
}

UInt16 abs(SInt16 x){
    if (x < 0)
        return x * -1;
    return x;
}
