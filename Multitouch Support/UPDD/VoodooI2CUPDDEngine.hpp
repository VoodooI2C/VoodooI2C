//
//  VoodooI2CUPDDEngine.hpp
//  VoodooI2C
//
//  Created by blankmac on 10/6/17.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CUPDDEngine_hpp
#define VoodooI2CUPDDEngine_hpp

#include <libkern/libkern/OSBase.h>

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOWorkLoop.h>

#include "../MultitouchHelpers.hpp"
#include "updd_data.h"
#include "../VoodooI2CDigitiserStylus.hpp"
#include "../VoodooI2CMultitouchEngine.hpp"
#include "../VoodooI2CMultitouchInterface.hpp"
#include "VoodooI2CUPDDGestureSocket.hpp"


#define MAX_FINGERS 5

class VoodooI2CMultitouchInterface;

class VoodooI2CUPDDEngine : VoodooI2CMultitouchEngine {
    OSDeclareDefaultStructors(VoodooI2CUPDDEngine);
    
    
private:
    IOTimerEventSource *timerSource;
    IOWorkLoop *workLoop;
    
protected:
public:
    updd_data ud;
    bool data_sent = false;

    void gestureRelease();
    UInt8 getScore();
    MultitouchReturn handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp);
    bool start(IOService* service);

};

#endif /* VoodooI2CUPDDEngine_hpp */

