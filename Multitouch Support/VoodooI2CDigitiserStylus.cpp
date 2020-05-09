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

VoodooI2CDigitiserStylus* VoodooI2CDigitiserStylus::stylus(DigitiserTransducerType transducer_type, IOHIDElement* digitizer_collection) {
    VoodooI2CDigitiserStylus* transducer = NULL;
    
    transducer = OSTypeAlloc(VoodooI2CDigitiserStylus);
    
    if (!transducer || !transducer->init()) {
        OSSafeReleaseNULL(transducer);
        goto exit;
    }
    
    transducer->type        = transducer_type;
    transducer->collection  = digitizer_collection;
    transducer->in_range    = false;
    
exit:
    return transducer;
}
