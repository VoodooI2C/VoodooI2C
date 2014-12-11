#include <IOKit/IOLib.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>

class VoodooRMI4Device : public IOService {
    
    
    OSDeclareDefaultStructors(VoodooRMI4Device);
    
    IOACPIPlatformDevice *fACPIDevice;
    
    bool start(IOService * provider);
    char* getMatchedName(IOService* provider);
    
};

