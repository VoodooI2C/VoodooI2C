//
//  VoodooI2CMultitouchInterface.hpp
//  VoodooI2C
//
//  Created by Alexandre on 22/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CMultitouchInterface_hpp
#define VoodooI2CMultitouchInterface_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

#include "MultitouchHelpers.hpp"
#include "VoodooI2CDigitiserStylus.hpp"

#define kIOFBTransformKey               "IOFBTransform"

enum {
    // transforms
    kIOFBRotateFlags                    = 0x0000000f,

    kIOFBSwapAxes                       = 0x00000001,
    kIOFBInvertX                        = 0x00000002,
    kIOFBInvertY                        = 0x00000004,

    kIOFBRotate0                        = 0x00000000,
    kIOFBRotate90                       = kIOFBSwapAxes | kIOFBInvertX,
    kIOFBRotate180                      = kIOFBInvertX  | kIOFBInvertY,
    kIOFBRotate270                      = kIOFBSwapAxes | kIOFBInvertY
};

class VoodooI2CMultitouchEngine;

/* Acts as a middleman between multitouch capable device drivers such as <VoodooI2CHIDMultitouchEventDriver> and a <VoodooI2CMultitouchEngine>
 *
 * This class is designed to allow multiple <VoodooI2CMultitouchEngines> (or inherited classes thereof) to attach and receive interrupt reports.
 * This allows for situations in which different multitouch engines handle different gestures.
 */

class EXPORT VoodooI2CMultitouchInterface : public IOService {
  OSDeclareDefaultStructors(VoodooI2CMultitouchInterface);

 public:
    UInt32 logical_max_x = 0;
    UInt32 logical_max_y = 0;
    UInt32 physical_max_x = 0;
    UInt32 physical_max_y = 0;

    /* Forwards a multitouch event to the attached multitouch engines
     * @event The event to forward
     * @timestamp The timestamp of the event
     *
     * Multitouch engines with a higher <VoodooI2CMultitouchEngine::getScore` are given higher priority.
     */

    void handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp);

    /* Controls the open behavior of <VoodooI2CMultitouchInterface>
     * @forClient An instance of <VoodooI2CMultitouchEngine> that wishes to be a client
     * @options Options avaliable for the open
     *
     * @return *true* upon successful opening, *false* otherwise
     */

    bool handleOpen(IOService* forClient, IOOptionBits options, void* arg) override;

    /* Controls the close behavior of <VoodooI2CMultitouchInterface>
     * @forClient An instance of <VoodooI2CMultitouchEngine> that wishes to close the connection
     * @options Options avaliable for the close
     */

    void handleClose(IOService* client, IOOptionBits options) override;
    
    
    /* Provides a check if the calling IOService instance has called open
     * @forClient An instance of <VoodooI2CMultitouchEngine> that wishes to check if it has called open
     */
    bool handleIsOpen(const IOService *forClient ) const override;

    using IORegistryEntry::setProperty;
    bool setProperty(const OSSymbol* key, OSObject* object) override;

    /* Orders engines according to <VoodooI2CMultitouchEngine::getScore>
     * @a The first engine in the comparison
     * @b The second engine in the comparison
     *
     * @return 1 if <a> is to have higher priority than <b>, -1 if <b> is to have higher priority than <a>, 0 if they have the same priority
     */

    static SInt8 orderEngines(VoodooI2CMultitouchEngine* a, VoodooI2CMultitouchEngine* b);

    /* Sets up the multitouch interface
     * @provider The driver which has created us
     *
     * @return *true* on successful start, *false* otherwise
     */

    bool start(IOService* provider) override;

    /* Stops up the multitouch interface
     * @provider The driver which has created us
     */

    void stop(IOService* provider) override;

 private:
    OSOrderedSet* engines;
};


#endif /* VoodooI2CMultitouchInterface_hpp */
