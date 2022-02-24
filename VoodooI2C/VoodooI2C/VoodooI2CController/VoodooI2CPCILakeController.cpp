//
//  VoodooI2CPCILakeController.cpp
//  VoodooI2C
//
//  Created by Zhen on 10/3/20.
//  Copyright © 2020 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CPCILakeController.hpp"

OSDefineMetaClassAndStructors(VoodooI2CPCILakeController, VoodooI2CPCIController);

void VoodooI2CPCILakeController::configurePCI() {
    IOLog("%s::%s Set PCI power state D0\n", getName(), physical_device.name);
    auto pci_device = physical_device.pci_device;
    pci_device->enablePCIPowerManagement(kPCIPMCSPowerStateD0);

    /* Apply Forcing D0 patch which modify 0x80 below to your findings.
       Credit to @startpenghubingzhou #246 and @phu54321 for original solution. */

    IOLog("%s::%s Current CPU is Comet Lake or Ice Lake, patching...\n",
          getName(), physical_device.name);
    uint16_t oldPowerStateWord = pci_device->configRead16(0x80 + 0x4);
    uint16_t newPowerStateWord = (oldPowerStateWord & (~0x3)) | 0x0;
    pci_device->configWrite16(0x80 + 0x4, newPowerStateWord);

    pci_device->setBusMasterEnable(true);
    pci_device->setMemoryEnable(true);
}
