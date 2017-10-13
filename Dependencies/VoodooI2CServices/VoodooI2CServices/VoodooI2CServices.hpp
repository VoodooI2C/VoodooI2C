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

class VoodooI2CServices : public IOService {
  OSDeclareDefaultStructors(VoodooI2CServices);

 public:
    static const IORegistryPlane* gVoodooI2CPlane;

    bool start(IOService* provider);
    void stop(IOService* provider);

 protected:
 private:
    IONotifier* device_matcher;
    IONotifier* terminate_matcher;

    static bool attachDevice(void* target, void* ref_con, IOService* new_service, IONotifier* notifier);
    static bool detachDevice(void* target, void* ref_con, IOService* new_service, IONotifier* notifier);
};


#endif /* VoodooI2CServices_hpp */
