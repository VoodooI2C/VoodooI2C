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
#include "VoodooI2CUPDDData.h"
#include "../VoodooI2CDigitiserStylus.hpp"
#include "../VoodooI2CMultitouchEngine.hpp"
#include "../VoodooI2CMultitouchInterface.hpp"
#include "VoodooI2CUPDDGestureSocket.hpp"


#define MAX_FINGERS 5

class VoodooI2CMultitouchInterface;

class VoodooI2CUPDDEngine : VoodooI2CMultitouchEngine {
    OSDeclareDefaultStructors(VoodooI2CUPDDEngine);
    
public:
    updd_data finger_data;
    bool data_sent = false;
    
    /* Data is only send to UPDD when there is positive interaction with the device.  A watchdog timer calls gestureRelease when
     * input is no longer being received.  gestureRelease then sends a packet to the updd client software letting it know
     * to release the last executed gesture.
     */
    
    void gestureRelease();
    
    /* Sets the UPDD Engine ranking.
     */
    
    UInt8 getScore();
    
    /* Compiles the available finger data and sends it to the user client socket.  If the socket isn't available (ie - the user
     * software isn't running, data_sent will be false and MultitouchReturnContinue is returned to hand the event to the next
     * available multitouch engine.  If the user client is availabe, MultitouchReturnBreak is returned to prevent other
     * multitouch engines from interferring.
     *
     * @event The current event
     *
     * @timestamp The timestamp of the current event being processed
     */
    
    MultitouchReturn handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp);
    
    /* Sets up the work loop and timer source for the watchdog timer.  Also attempts to initialise the gesture socket
     * when available.
     */
    
    bool start(IOService* service);
    
protected:
private:
    IOTimerEventSource *timer_source;
    IOWorkLoop *work_loop;
    
};

#endif /* VoodooI2CUPDDEngine_hpp */

