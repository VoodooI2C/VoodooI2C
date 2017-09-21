//
//  VoodooI2CDigitiserTransducer.cpp
//  VoodooI2C
//
//  Created by Alexandre on 13/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CDigitiserTransducer.hpp"

#define super OSObject
OSDefineMetaClassAndStructors(VoodooI2CDigitiserTransducer, OSObject);

void VoodooI2CDigitiserTransducer::free() {
    OSSafeReleaseNULL(collection);
    OSSafeReleaseNULL(elements);
    super::free();
}

bool VoodooI2CDigitiserTransducer::serialize(OSSerialize* serializer) {
    OSDictionary* temp_dictionary = OSDictionary::withCapacity(2);

    bool result = false;

    if (temp_dictionary) {
        temp_dictionary->setObject(kIOHIDElementParentCollectionKey, collection);
        temp_dictionary->setObject(kIOHIDElementKey, elements);
        temp_dictionary->serialize(serializer);
        temp_dictionary->release();

        result = true;
    }
    
    return result;
}

VoodooI2CDigitiserTransducer* VoodooI2CDigitiserTransducer::transducer(DigitiserTransducuerType transducer_type, IOHIDElement* digitizer_collection) {
    VoodooI2CDigitiserTransducer* transducer = NULL;
    
    transducer = new VoodooI2CDigitiserTransducer;
    
    if (!transducer)
        goto exit;
    
    if (!transducer->init()) {
        transducer = NULL;
        goto exit;
    }
    
    transducer->type        = transducer_type;
    transducer->collection  = digitizer_collection;
    transducer->in_range    = false;
    
    if (transducer->collection)
        transducer->collection->retain();
    
    transducer->elements = OSArray::withCapacity(4);

    if (!transducer->elements) {
        if (transducer->collection)
            OSSafeReleaseNULL(transducer->collection);
        transducer = NULL;
        goto exit;
    } else {
        transducer->elements->retain();
    }

exit:
    return transducer;
}
