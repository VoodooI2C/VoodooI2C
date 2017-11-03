//
//  VoodooI2CUPDDEngine.cpp
//  VoodooI2C
//
//  Created by blankmac on 10/6/17.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CUPDDEngine.hpp"

#define super VoodooI2CMultitouchEngine
OSDefineMetaClassAndStructors(VoodooI2CUPDDEngine, VoodooI2CMultitouchEngine)

void VoodooI2CUPDDEngine::gestureRelease() {
    
    finger_data.finger_lift = true;
    
    sendInput(&finger_data);
    
    finger_data.finger_lift = false;
    
}

UInt8 VoodooI2CUPDDEngine::getScore() {
    
    return 0x2;
    
}

MultitouchReturn VoodooI2CUPDDEngine::handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp) {
    int i;
    
    for (int i = 0;i < event.transducers->getCount(); i++) {
        finger_data.current_x[i] = -1;
        finger_data.current_y[i] = -1;
    }
    
    for (i=0; i < event.contact_count; i++) {
        VoodooI2CDigitiserTransducer* transducer = OSDynamicCast(VoodooI2CDigitiserTransducer, event.transducers->getObject(i));
        if (!transducer)
            continue;
        finger_data.logical_x = transducer->logical_max_x;
        finger_data.logical_y = transducer->logical_max_y;
        
        if (transducer->tip_switch) {
            finger_data.current_x[i] = transducer->coordinates.x.value();
            finger_data.current_y[i] = transducer->coordinates.y.value();
            
        }
    }

    data_sent = sendInput(&finger_data);
    
    if (!data_sent) {
        return MultitouchReturnContinue;
    }
    this->timer_source->setTimeoutMS(14);
    
    return MultitouchReturnBreak;
    
}

bool VoodooI2CUPDDEngine::start(IOService *service) {
    if (!super::start(service))
        return false;
    
    this->work_loop = getWorkLoop();
    if (!this->work_loop){
        IOLog("%s::Unable to get workloop\n", getName());
        return false;
    }
    
    this->work_loop->retain();
    this->timer_source = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CUPDDEngine::gestureRelease));
    this->work_loop->addEventSource(this->timer_source);
    
    kern_return_t initialise_status = initialiseGestureSocket();
    if(initialise_status == KERN_SUCCESS) {
        IOLog("%s::GestureSocket: Initialised the gesture socket!\n", getName());
    }
    
    return true;
}



