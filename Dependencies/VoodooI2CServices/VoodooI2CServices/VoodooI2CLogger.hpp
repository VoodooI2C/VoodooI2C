//
//  VoodooI2CLogger.hpp
//  VoodooI2CServices
//
//  Created by Alexandre on 09/10/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CLogger_hpp
#define VoodooI2CLogger_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

#define kVoodooI2CSilentLogging 0
#define kVoodooI2CNormalLogging 1
#define kVoodooI2CDebugLogging  2

class VoodooI2CLogger : public IOService {
  OSDeclareDefaultStructors(VoodooI2CLogger);

 public:
    bool start(IOService* provider);
    void stop(IOService* provider);

 protected:
 private:
    UInt8 logging_level = kVoodooI2CNormalLogging;
};


#endif /* VoodooI2CLogger_hpp */
