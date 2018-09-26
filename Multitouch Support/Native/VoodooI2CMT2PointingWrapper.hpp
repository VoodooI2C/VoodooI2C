//
//  VoodooI2CMT2PointingWrapper.hpp
//  VoodooI2C
//
//  Created by Alexandre on 26/09/2018.
//  Copyright © 2018 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CMT2PointingWrapper_hpp
#define VoodooI2CMT2PointingWrapper_hpp

#include <IOKit/hidsystem/IOHIPointing.h>

class VoodooI2CMT2PointingWrapper : public IOHIPointing {
    typedef IOHIPointing super;
    OSDeclareDefaultStructors(VoodooI2CMT2PointingWrapper);
    
protected:
    virtual IOItemCount buttonCount() override;
    virtual IOFixed resolution() override;
    
public:
    
    virtual bool init() override;
    
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    virtual UInt32 deviceType() override;
    virtual UInt32 interfaceID() override;
    
    void updateRelativeMouse(int dx, int dy, int buttons);
};

#endif /* VoodooI2CMT2PointingWrapper_hpp */
