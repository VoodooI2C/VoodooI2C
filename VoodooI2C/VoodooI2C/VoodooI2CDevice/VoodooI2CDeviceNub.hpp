//
//  VoodooI2CDeviceNub.hpp
//  VoodooI2C
//
//  Created by Alexandre on 07/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CDeviceNub_hpp
#define VoodooI2CDeviceNub_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "../../../Dependencies/VoodooGPIO/VoodooGPIO/VoodooGPIO.h"

class VoodooI2CControllerDriver;

/* Implements a device nub to which an instance of a device driver may attach. Examples include <VoodooI2CHIDDevice>
 *
 * The members of this class are responsible for low-level interfacing with an I2C slave device. The public
 * member functions that are not inherited from <IOService> collectively form the so-called 'device API'.
 * Device drivers access the API in order to perform low-level hardware operations such as receiving interrupts
 * and I2C protocol messaging.
 */

class VoodooI2CDeviceNub : public IOService {
  OSDeclareDefaultStructors(VoodooI2CDeviceNub);

 public:
    /* Attaches <VoodooI2CController> class
     * @provider The controller driving the slave device
     * @child The physical ACPI slave device
     *
     * This function is called by <VoodooI2CControllerDriver> in order for the nub to attach itself
     * and perform its initialisation routine.
     *
     * @return *true* if the successfully attached and initialised, *false* otherwise
     */

    bool attach(IOService* provider, IOService* child);

    /* Disables an interrupt source
     * @source The index of the interrupt source in the case of APIC interrupts
     *
     * This function disables the interrupt source via the GPIO controller or passing it onto the APCI device
     * depending on the configuration
     *
     * @return *kIOReturnSuccess* if the interrupt is sucessfully disabled, *kIOReturnNoInterrupt* if the interrupt source
     * is invalid
     */

    IOReturn disableInterrupt(int source) override;

    /* Enables an interrupt source
     * @source The index of the interrupt source in the case of APIC interrupts
     *
     * This function enables the interrupt source via the GPIO controller or passing it onto the APCI device
     * depending on the configuration
     *
     * @return *kIOReturnSuccess* if the interrupt is sucessfully enabled, *kIOReturnNoInterrupt* if the interrupt source
     * is invalid
     */

    IOReturn enableInterrupt(int source) override;

    /* Gets the type of an interrupt source
     * @source The index of the interrupt source in the case of APIC interrupts
     * @interruptType The interrupt type for the interrupt source will be stored here
     *
     * This function gets the interrupt type for the given interrupt source. It stores in *interruptType* either *kIOInterruptTypeEdge* for edge-trigggered sources or *kIOInterruptTypeLevel* for level-trigggered sources.
     *
     * @return *kIOReturnSuccess* if the interrupt type is successfully found, *kIOReturnNoInterrupt* if the interrupt source
     * is invalid
     */

    IOReturn getInterruptType(int source, int *interruptType) override;

    /* Gets an *IOWorkLoop* object
     *
     * This function either grabs the existing workloop or creates a new one if none is found. This workloop is intended to be used by
     * the nub itself along with any drivers that attach to it.
     * @return A pointer to an *IOWorkLoop* object, else *NULL*
     */
    IOWorkLoop* getWorkLoop();

    /* Transmits an I2C read request to the slave device
     * @values The buffer that the returned data is to be written into
     * @length The length of the message
     * 
     * This function instructs the controller to read data from an I2C slave device. WARNING: This function is not safe to call from an interrupt context.
     *
     * @return *kIOReturnSuccess* upon a successful read, *kIOReturnBusy* if the bus is busy, *kIOReturnTimeout* if the controller driver waits too long for the controller to assert its interrupt line, *kIOReturnError* otherwise
     */

    IOReturn readI2C(UInt8* values, UInt16 length);

    /* Registers a slave for interrupts
     * @source The index of the interrupt source in the case of APIC interrupts
     * @target The slave driver
     * @handler The interrupt handler callback
     * @refcon A reference constant
     *
     * This function is called during the slave driver's initialisation process in order to register it for interrupts.
     *
     * @return *kIOReturnSuccess* upon a successful registration, *kIOReturnNoInterrupt* if the interrupt source is invalid, *kIOReturnNoResources* if
     * the interrupt already has an installed handler
     */

    IOReturn registerInterrupt(int source, OSObject *target, IOInterruptAction handler, void *refcon) override;

    /* Starts the device nub
     * @provider The controller that drives this slave device
     *
     * This function is called by the controller to start the resources needed for the device nub.
     *
     * @return *true* on successful start, *false* otherwise
     */

    bool start(IOService* provider) override;

    /* Stops the device nub
     * @provider The controller that drives this slave device
     */

    void stop(IOService* provider) override;

    /* Unregisters a slave for interrupts
     * @source he index of the interrupt source in the case of APIC interrupts
     *
     * This function is called during the slave driver's exit process in order to unregister it for interrupts.
     *
     * @return *kIOReturnSuccess* upon a successful unregistration, *kIOReturnNoInterrupt* if the interrupt source is invalid
     */

    IOReturn unregisterInterrupt(int source) override;

    /* Transmits an I2C write request to the slave device
     * @values A buffer containing the message to be written
     * @length The length of the message
     *
     * This function instructs the controller to write data to an I2C slave device. WARNING: This function is not safe to call from an interrupt context.
     *
     * @return *kIOReturnSuccess* upon a successful read, *kIOReturnBusy* if the bus is busy, *kIOReturnTimeout* if the controller driver waits too long for the controller to assert its interrupt line, *kIOReturnError* otherwise
     */

    IOReturn writeI2C(UInt8* values, UInt16 length);

    /* Transmits an I2C write-read request to the slave device
     * @write_buffer A buffer containing the message to be written
     * @write_length The length of the write message
     * @read_buffer The buffer that the returned data is to be written into
     * @read_length The length of the read message
     *
     * This function instructs the controller to write data to an I2C slave device and then requests a read from the I2C slave device. WARNING: This function is not safe to call from an interrupt context.
     *
     * @return *kIOReturnSuccess* upon a successful read, *kIOReturnBusy* if the bus is busy, *kIOReturnTimeout* if the controller driver waits too long for the controller to assert its interrupt line, *kIOReturnError* otherwise
     */


    IOReturn writeReadI2C(UInt8* write_buffer, UInt16 write_length, UInt8* read_buffer, UInt16 read_length);

 private:
    IOACPIPlatformDevice* acpi_device;
    IOCommandGate* command_gate;
    VoodooI2CControllerDriver* controller;
    const char* controller_name;
    VoodooGPIO* gpio_controller;
    int gpio_irq;
    UInt16 gpio_pin;
    UInt8 i2c_address;
    bool has_gpio_interrupts;
    bool use_10bit_addressing;
    IOWorkLoop* work_loop;

    /* Instantiates a <VoodooI2CACPICRSParser> object to grab I2C slave properties as well as potential GPIO interrupt properties.
     *
     * @return *kIOReturnSuccess* upon a successfull *_CRS* parse, *kIOReturnNotFound* if no I2C slave properties were found.
     */

    IOReturn getDeviceResources();

    /* Searches the IOService plane to find a <VoodooGPIO> controller object.
     */

    VoodooGPIO* getGPIOController();

    /* Transmits an I2C read request to the slave device
     * @values The buffer that the returned data is to be written into
     * @length The length of the message
     *
     * This function is the gated version of <readI2C>.
     *
     * @return *kIOReturnSuccess* upon a successful read, *kIOReturnBusy* if the bus is busy, *kIOReturnTimeout* if the controller driver waits too long for the controller to assert its interrupt line, *kIOReturnError* otherwise
     */

    IOReturn readI2CGated(UInt8* values, UInt16* length);

    /* Releases resources allocated in <start>
     *
     * This function is called during a graceful exit from <start> and during
     * execution of <stop> in order to release resources retained by <start>.
     */

    void releaseResources();

    /* Transmits an I2C write request to the slave device
     * @values A buffer containing the message to be written
     * @length The length of the message
     *
     * This function is the gated version of <writeI2C>.
     *
     * @return *kIOReturnSuccess* upon a successful read, *kIOReturnBusy* if the bus is busy, *kIOReturnTimeout* if the controller driver waits too long for the controller to assert its interrupt line, *kIOReturnError* otherwise
     */

    IOReturn writeI2CGated(UInt8* values, UInt16* length);

    /* Transmits an I2C write-read request to the slave device
     * @write_buffer A buffer containing the message to be written
     * @write_length The length of the write message
     * @read_buffer The buffer that the returned data is to be written into
     * @read_length The length of the read message
     *
     * This function is the gated version of <writeReadI2C>.
     *
     * @return *kIOReturnSuccess* upon a successful read, *kIOReturnBusy* if the bus is busy, *kIOReturnTimeout* if the controller driver waits too long for the controller to assert its interrupt line, *kIOReturnError* otherwise
     */

    IOReturn writeReadI2CGated(UInt8* write_buffer, UInt16* write_length, UInt8* read_buffer, UInt16* read_length);
};


#endif /* VoodooI2CDeviceNub_hpp */
