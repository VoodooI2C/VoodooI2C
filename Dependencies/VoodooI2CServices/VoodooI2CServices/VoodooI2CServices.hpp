//
//  VoodooI2CServices.hpp
//  VoodooI2CServices
//
//  Created by Alexandre on 30/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CServices_hpp
#define VoodooI2CServices_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

#define kVoodooI2CPlane "VoodooI2C"

/* Provides various services to all VoodooI2C related classes. */
class VoodooI2CServices : public IOService {
  OSDeclareDefaultStructors(VoodooI2CServices);

 public:
    static const IORegistryPlane* gVoodooI2CPlane;

    /* Starts the services kext
     * @provider IOResources
     *
     * This function is responsible for initialising the VoodooI2C plane and the logger.
     * @return *true* on successful start, *false* otherwise
     */

    bool start(IOService* provider);

    /* Stops the services kext
     * @provider IOResources
     */

    void stop(IOService* provider);

 protected:
 private:
    IONotifier* device_matcher;
    IONotifier* terminate_matcher;

    /* Called to attach a VoodooI2C related service into the VoodooI2C plane
     * @target The VoodooI2CServices instance
     * @ref_con A reference constant
     * @new_service The service to be attached
     * @notifier The IONotifier listening for matching services
     *
     * <new_service> will always be a VoodooI2C class with the "VoodooI2CServices Supported" property set to *true*
     *
     * @return *true* on successful attach, *false* otherwise
     */

    static bool attachDevice(void* target, void* ref_con, IOService* new_service, IONotifier* notifier);
    
    /* Called to detach a VoodooI2C related service from the VoodooI2C plane
     * @target The VoodooI2CServices instance
     * @ref_con A reference constant
     * @new_service The service to be detached
     * @notifier The IONotifier listening for terminating services
     *
     * <new_service> will always be a VoodooI2C class with the "VoodooI2CServices Supported" property set to *true*
     *
     * @return *true* on successful detach, *false* otherwise
     */
    static bool detachDevice(void* target, void* ref_con, IOService* new_service, IONotifier* notifier);
};


#endif /* VoodooI2CServices_hpp */
