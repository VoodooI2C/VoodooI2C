//
//  VoodooI2CControllerDriver.hpp
//  VoodooI2C
//
//  Created by Alexandre on 03/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CControllerDriver_hpp
#define VoodooI2CControllerDriver_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>

#include "VoodooI2CControllerConstants.hpp"
#include "VoodooI2CControllerNub.hpp"
#include "../VoodooI2CDevice/VoodooI2CDeviceNub.hpp"
#include "../../../Dependencies/helpers.hpp"

typedef struct {
    UInt16 address;
    UInt8* buffer;
    UInt16 flags;
    UInt16 length;
} VoodooI2CControllerBusMessage;

typedef struct {
    UInt32 ss_hcnt = 0x01b0;
    UInt32 fs_hcnt = 0x48;
    UInt32 ss_lcnt = 0x01fb;
    UInt32 fs_lcnt = 0xa0;
    UInt32 sda_hold = 0x9;
} VoodooI2CControllerBusConfig;

typedef struct {
    UInt32 abort_source;
    VoodooI2CControllerBusConfig acpi_config;
    bool awake;
    UInt32 bus_config;
    int command_error;
    bool command_complete = false;
    UInt32 functionality;
    VoodooI2CControllerBusMessage* messages;
    int message_error;
    int message_number;
    int message_read_index;
    int message_write_index;
    const char* name;
    UInt32 receive_buffer_length;
    UInt8* receive_buffer;
    UInt receive_fifo_depth;
    int receive_outstanding;
    UInt status;
    UInt32 transaction_buffer_length;
    UInt8* transaction_buffer;
    UInt transaction_fifo_depth;
} VoodooI2CControllerBusDevice;

class VoodooI2CController;

/* Implements a driver for the Intel LPSS Designware I2C Controller which attaches to a <VoodooI2CControllerNub> object
 *
 * The members of this class are responsible for interfacing with the I2C bus and implementing the I2C protocol. The driver also
 * publishes nubs for each I2C slave device attached to the bus it drives.
 */
class EXPORT VoodooI2CControllerDriver : public IOService {
  OSDeclareDefaultStructors(VoodooI2CControllerDriver);

 public:
    VoodooI2CControllerBusDevice bus_device;
    OSArray* device_nubs;
    VoodooI2CControllerNub* nub;

    /* Frees <VoodooI2CControllerDriver> class
     *
     * This is the last function called during the unload routine and frees the memory
     * allocated in <init>.
     */

    void free() override;

    /* Handles an interrupt that has been asserted by the controller */

    void handleInterrupt(OSObject* target, void* refCon, IOService* nubDevice, int source);

    /* Initialises <VoodooI2CControllerDriver> class
     * @properties OSDictionary* representing the matched personality
     *
     * This is the first function called during the load routine and allocates the memory
     * needed for <VoodooI2CControllerBusDevice>.
     *
     * @return *true* on successful initialisation, *false* otherwise
     */

    bool init(OSDictionary* properties) override;

    /* Probes the controller to determine whether or not we can drive it
     * @provider The provider which we have matched against
     * @score    Probe score as specified in the matched personality
     *
     * This function probes the controller to determine if this driver is suitable to
     * drive it. This is done by quering the *DW_IC_COMP_TYPE* register. We consider the
     * probe successful if the value returned is *DW_IC_COMP_TYPE_VALUE*. Note that
     * Linux implements other accepted returned values (which involve modifying the output
     * of <VoodooI2CControllerNub::readRegister>) but we do not implement them as we have yet
     * to see such a controller in the wild (yet!).
     *
     * @return pointer to this instance of <VoodooI2CControllerDriver> on successful probe, *NULL*
     * otherwise
     */

    VoodooI2CControllerDriver* probe(IOService* provider, SInt32* score) override;

    /* Starts the bus
     * @provider The provider which we have matched against
     *
     * This function is called after <probe> and is responsible for allocating the resources
     * needed by the physical controller. This includes initialising system power management
     * and calling <initialiseBus> and <publishNubs>.
     *
     * @return *true* on successful start, *false* otherwise
     */

    bool start(IOService* provider) override;

    /* Stops the bus
     * @provider The provider which we have matched against
     *
     * This function is called before <free> and is responsible for deallocating the resources
     * that were allocated in <start>. This includes stopping system power management and
     * stopping the associated device nubs.
     */

    void stop(IOService* provider) override;

    /* Directs the command gate to add an I2C transfer routine to the work loop
     * @messages The messages to be transferred
     * @number   The number of messages
     *
     * @return *kIOReturnSuccess* on success, *kIOReturnError* otherwise
     */

    IOReturn transferI2C(VoodooI2CControllerBusMessage* messages, int number);

 private:
    IOCommandGate* command_gate;
    IOWorkLoop* work_loop;

    /* Requests the nub to fetch bus configuration values from the ACPI tables
     *
     * This function evaluates the *SSCN* and *FMCN* methods in the ACPI tables via
     * <VoodooI2CControllerNub::getACPIParams>.
     *
     * @return *kIOReturnSuccess* if all desired values were obtained, *kIOReturnNotFound( if (some or all)
     * configuration values are missing
     */

    IOReturn getBusConfig();

    /* Prints an error message when the bus reports a transaction error */

    void handleAbortI2C();

    /* Initialises the bus by writing in configuration values
     *
     * @return *kIOReturnSuccess* on successful initialisation, *kIOReturnError* otherwise
     */

    IOReturn initialiseBus();

    /* Prepares the driver for an I2C transfer routine
     * @messages The messages to be transferred
     * @number   The number of messages
     *
     * This function prepares the driver for an I2C transfer routine by requesting the bus to start a transfer.
     * The driver then spins the current thread using *IOCommandGate::commandSleep* until either:
     *
     * - The bus informs us that a successful transfer took place after calling <transferMessageToBus> in
     *   <handleInterrupt>.<handleInterrupt>
     * - The driver times out waiting for the bus to become ready.
     * - The bus informs us that we cannot start a transfer due to an error.
     *
     * @return *kIOReturnSuccess* on successful preparation; *kIOReturnBusy* if the bus is busy;
     * *kIOReturnTimeout* if the driver timed out waiting for the bus to become ready; *kIOReturnDeviceBusy*
     * otherwise
     */

    IOReturn prepareTransferI2C(VoodooI2CControllerBusMessage* messages, int* number);

    /* Traverses the IOACPIPlane to find children and publishes `VoodooI2CDeviceNub` entries
     * into the IORegistry for matching
     *
     * @return *kIOReturnSuccess* on success, *kIOReturnError* otherwise
     */

    IOReturn publishNubs();

    /* Determines what type of interrupt has fired
     *
     * @return *DW_IC_INT* satus code
     */

    UInt32 readClearInterruptBits();

    /* Reads an I2C message from the bus
     *
     * This function is called by <handleInterrupt> when the bus informs us that there is an I2C message
     * waiting to be read.
     */

    void readFromBus();

    void releaseResources();

    /* Requests the bus to prepare for an I2C transfer routine
     *
     * This function informs the bus that the driver would like to commence an I2C transfer routine. At this point
     * we only inform the bus of the slave address and enable the bus.
     */

    void requestTransferI2C();

    /* Sets the power state of the bus
     * @whichState The power state the bus is expected to enter represented by either
     *  *kIOPMPowerOn* or *kIOPMPowerOff*
     * @whatDevice The power management policy maker
     *
     * This function is called by the operating system's power management services
     * to instruct the bus to enter a certain power state.
     *
     * @return *kIOPMAckImplied* on succesful state change, *kIOReturnError* otherwise
     */

    IOReturn setPowerState(unsigned long whichState, IOService* whatDevice) override;

    /* Toggle the bus's enabled state
     * @param enabled The power state the bus is expected to enter represented by either
     *  *kVoodooI2CStateOn* or *kVoodooI2CStateOff*
     
     @return *kIOReturnSuccess* on successful state toggle, *kIOReturnTimeout* otherwise
     */

    IOReturn toggleBusState(VoodooI2CState enabled);

    /* Toggle clock gating to save power
     * @enabled The state the clock gate is expected to enter represented by either
     *  *kVoodooI2CStateOn* or *kVoodooI2CStateOff*
     *
     * Clock gating is a method by which power consumption by a particular circuit is reduced. We need to disable the
     * I2C controller's clock gate in order to be able to receive messages from the bus. We re-enable it when we aren't
     * waiting for a message to save power.
     */

    inline void toggleClockGating(VoodooI2CState enabled);

    /* Toggle the interrupts' enabled state
     * @param enabled The state the interrupts are expected to enter represented by either
     *  *kVoodooI2CStateOn* or *kVoodooI2CStateOff*
     
     @return *kIOReturnSuccess* on successful state toggle, *kIOReturnTimeout* otherwise
     */

    void toggleInterrupts(VoodooI2CState enabled);

    /* Attempts an I2C transfer routine
     * @messages The messages to be transferred
     * @number   The number of messages
     
     @return returns kIOReturnSuccess on successful
     */

    IOReturn transferI2CGated(VoodooI2CControllerBusMessage* messages, int* number);

    /* Transfers an I2C message to the bus */

    void transferMessageToBus();

    /* Waits for the bus not to be busy
     *
     * This function spins the current thread by a hardocded amount using *IODelay* until the bus states that it is
     * ready.
     *
     * @return *kIOReturnSuccess* on success, *kIOReturnTimeout* otherwise
     */

    IOReturn waitBusNotBusyI2C();
};


#endif /* VoodooI2CControllerDriver_hpp */
