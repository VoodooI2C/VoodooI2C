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
const IORegistryPlane* gVoodooI2CPlane;

class VoodooI2CServices : public IOService {
  OSDeclareDefaultStructors(VoodooI2CServices);

 public:
    bool start(IOService* provider);
    void stop(IOService* provider);

 protected:
 private:
    IONotifier* device_matcher;

    bool attachDevice(void* target, void* ref_con, IOService* new_service, IONotifier* notifier);
};


#endif /* VoodooI2CServices_hpp */
