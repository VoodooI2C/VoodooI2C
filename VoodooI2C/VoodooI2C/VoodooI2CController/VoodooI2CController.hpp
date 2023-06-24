//
//  VoodooI2CController.hpp
//
//  Created by Alexandre on 31/07/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CController_hpp
#define VoodooI2CController_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/pci/IOPCIDevice.h>

#include "../../../Dependencies/helpers.hpp"

#ifndef kACPIDevicePathKey
#define kACPIDevicePathKey "acpi-path"
#endif

typedef struct {
    IOACPIPlatformDevice* acpi_device;
    bool awake = true;
    const char* name;
    IOPCIDevice* pci_device;
    IOMemoryMap* mmap;
    IOService* provider;
    bool access_intr_mask_workaround = false;
} VoodooI2CControllerPhysicalDevice;

class VoodooI2CControllerNub;

/* Implements a Synopsys DesignWare I2C Controller
 *
 * This is the base class from which all implementations of a physical
 * Synopsys DesignWare I2C Controller should inherit from. The members of this class
 * are responsible for low-level interfacing with the physical hardware. For the driver implementing
 * the proprietary Synopsys DesignWare I2C controller interface, see <VoodooI2CControllerDriver>.
 */
class EXPORT VoodooI2CController : public IOService {
  OSDeclareDefaultStructors(VoodooI2CController);

 public:
    /* Implemented to beat Apple's own LPSS kexts
     * @provider The provider which we have matched against
     * @score    Probe score as specified in the matched personality
     *
     * Apple has their own Intel LPSS controller kernel extensions which (as far
     * as we can tell) are not compatible with the ones found on PCs. We implement
     * a probe function in order to attach ourselves to the I2C controller before
     * Apple's kext does.
     *
     * @return A pointer to this instance of VoodooI2CController
     */

    VoodooI2CController* probe(IOService* provider, SInt32* score) override;

    /* Reads a controller register
     * @offset The offset of the register relative to the controller's base address
     *
     * @return The value of the register
     */

    UInt32 readRegister(int offset);

    /* Starts the physical controller
     * @provider The provider which we have matched against
     *
     * This function is called after <probe> and is responsible for allocating the resources
     * needed by the physical controller. This includes initialising system power management
     * and calling <publishNub>.
     *
     * @return *true* on successful start, *false* otherwise
     */

    bool start(IOService* provider) override;

    /* Stops the physical controller
     * @provider The provider which we have matched against
     *
     * This function is called before <free> and is responsible for deallocating the resources
     * that were allocated in <start>. This includes stopping system power management and
     * stopping the associated controller nub.
     */

    void stop(IOService* provider) override;

    /* Writes a specified value into a controller register
     * @value The *UInt32* value to be written
     * @offset The offset of the register relative to the controller's base address
     */

    void writeRegister(UInt32 value, int offset);

    VoodooI2CControllerNub* nub;
    VoodooI2CControllerPhysicalDevice physical_device;

 protected:
    /* Maps the controller's memory to a virtual address
     *
     * @return *KIOReturnSuccess* on successful mapping, *kIOReturnDeviceError*
     *  otherwise
     */

    IOReturn mapMemory();

    /* Releases the controller's mapped memory
      *
      * @return *KIOReturnSuccess* on successful releasing, *kIOReturnDeviceError*
      *  otherwise
      */

    IOReturn unmapMemory();

    /* Publishes a <VoodooI2CControllerNub> entry into the IORegistry for matching
     *
     * This function instantiates a new <VoodooI2CControllerNub> object and attaches it
     * to the current VoodooI2CController object. It then starts the nub and calls
     * VoodooI2CControllerNub::registerService to publish the nub and begin the matching
     * process.
     *
     * @return *kIOReturnSuccess* on successful publish, *kIOReturnError* otherwise
     */

    IOReturn publishNub();

 private:
    bool debug_logging = true;

    /* Frees <VoodooI2CController> class
     *
     * This is the last function called during the unload routine and
     * frees the memory allocated in <init>.
     */

    void free() override;

    /* Initialises <VoodooI2CController> class
     * @properties Contains the properties of the matched provider
     *
     * This is the first function called during the load routine and
     * allocates the memory needed for <VoodooI2CControllerPhysicalDevice>.
     *
     * @return *true* if the successfully initialised, *false* otherwise
     */

    bool init(OSDictionary* properties) override;

    /* Releases resources allocated in <start>
     *
     * This function is called during a graceful exit from <start> and during
     * execution of <stop> in order to release resources retained by <start>.
     */

    void releaseResources();

    /* Sets the physical power state of the controller
     * @whichState The power state the device is expected to enter represented by either
     *  *kIOPMPowerOn* or *kIOPMPowerOff*
     * @whatDevice The power management policy maker
     *
     * This function is called by the operating system's power management services
     * to instruct the controller to enter a certain power state.
     *
     * @return *kIOPMAckImplied* on successful state change, *kIOReturnError* otherwise
     */

    IOReturn setPowerState(unsigned long whichState, IOService* whatDevice) override;
};

#endif /* VoodooI2CController_hpp */
