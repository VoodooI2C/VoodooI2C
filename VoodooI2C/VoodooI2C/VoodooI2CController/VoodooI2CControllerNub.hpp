//
//  VoodooI2CControllerNub.hpp
//  VoodooI2C
//
//  Created by Alexandre on 03/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CControllerNub_hpp
#define VoodooI2CControllerNub_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOCommandGate.h>

#ifndef EXPORT
#define EXPORT __attribute__((visibility("default")))
#endif

class VoodooI2CController;

/* Implements a controller nub to which an instance of <VoodooI2CControllerDriver> may attach
 *
 * The members of this class are responsible for acting as a middle-man between the physical 
 * device and the I2C bus iself.
 */
class EXPORT VoodooI2CControllerNub : public IOService {
    OSDeclareDefaultStructors(VoodooI2CControllerNub);

 public:
    VoodooI2CController* controller;
    const char* name;

    /* Attaches the nub to the physical controller
     * @provider The physical controller
     *
     * This function attaches the nub to the physical controller so that it appears as a child entry
     * in the IOService plane.
     *
     * @return *true* on succesful attach, *false* otherwise
     */

    bool attach(IOService* provider) override;

    /* Detaches the nub from the physical controller
     * @provider The physical controller
     *
     * This function detaches the nub from the physical controller so that it is removed as a child
     * entry in the IOService plane.
     */

    void detach(IOService* provider) override;

    IOReturn disableInterrupt(int source) override;

    IOReturn enableInterrupt(int source) override;

    /* Evaluates ACPI methods pertaining to the controller's ACPI device in the ACPI tables
     * @method   The name of the method to be evaluated
     * @hcnt     Pointer to the *UInt32* where we store the high count
     * @lcnt     Pointer to the *UInt32* where we store the low count
     * @sda_hold Pointer to the *UInt32* where we store the SDA hold time
     *
     * @return *kIOReturnSuccess* on successful retrieval of all desired values, *kIOReturnNotFound*
     * otherwise
     */

    IOReturn getACPIParams(const char* method, UInt32* hcnt, UInt32* lcnt, UInt32* sda_hold);

    /* Passes to <VoodooI2CController::readRegister>
     * @offset The offset of the register relative to the controller's base address
     *
     * @return The value of the register
     */

    IOReturn getInterruptType(int source, int *interruptType) override;

    UInt32 readRegister(int offset);

    /* Starts the controller nub
     * @provider The physical controller
     *
     * This function starts the controller nub and is responsible for allocating the resources needed by the nub and
     * its driver. This includes instantiating <work_loop>, <interrupt_source> and <command_gate>.
     *
     * @return *true* on successful start, *false* otherwise
     */

    IOReturn registerInterrupt(int source, OSObject *target, IOInterruptAction handler, void *refcon) override;

    bool start(IOService* provider) override;

    /* Stops the controller nub
     * @provider The physical controller
     *
     * This function stops the controller nub and is responsible for deallocating the nub's resources by calling
     * <releaseResources>.
     *
     * @return *true* on successful start, *false* otherwise
     */

    void stop(IOService* provider) override;

    /* Passes to <VoodooI2CController::writeRegister>
     * @value  The *UInt32* value to be written
     * @offset The offset of the register relative to the controller's base address
     *
     * @return The value of the register
     */

    void writeRegister(UInt32 value, int offset);

    IOReturn unregisterInterrupt(int source) override;

 private:
    /* Handles an interrupt when the controller asserts its interrupt line
     * @owner    The owner of this interrupt
     * @src      The interrupt event source
     * @intCount   The index of the interrupt in the provider
     *
     * This function is called by the operating system when the controller asserts its interrupt line. 
     * It's only purpose is to delegate the interrupt to the attached driver.
     */

    void interruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount);

    /* Releases resources allocated in <start>
     *
     * This function is called during a graceful exit from <start> and during
     * execution of <stop> in order to release resources retained by <start>.
     */

    void releaseResources();
};

#endif /* VoodooI2CControllerNub_hpp */
