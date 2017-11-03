//
//  VoodooI2CDigitiserStylus.cpp
//  VoodooI2C
//
//  Created by Alexandre on 13/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CDigitiserStylus.hpp"

#define super VoodooI2CDigitiserTransducer
OSDefineMetaClassAndStructors(VoodooI2CDigitiserStylus, VoodooI2CDigitiserTransducer);

VoodooI2CDigitiserStylus* VoodooI2CDigitiserStylus::stylus(DigitiserTransducuerType transducer_type, IOHIDElement* digitizer_collection) {
    VoodooI2CDigitiserStylus* transducer = NULL;
    
    transducer = new VoodooI2CDigitiserStylus;
    
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
