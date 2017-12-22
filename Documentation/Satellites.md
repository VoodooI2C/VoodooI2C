#<cldoc:Satellite Kexts>

&#8291;

## [VoodooI2CHID](https://github.com/alexandred/VoodooI2CHID)

VoodooI2CHID implements support for I2C HID devices as specified by [Microsoft's protocol](http://download.microsoft.com/download/7/d/d/7dd44bb7-2a7a-4505-ac1c-7227d3d96d5b/hid-over-i2c-protocol-spec-v1-0.docx). Most users of VoodooI2C will use this kext in conjunction with the core kext but it is possible that a different satellite kext will provide better support for certain I2C HID devices that also possess a propriety protocol.

To tell whether or not a device is supported by VoodooI2CHID, you must know its ACPI device ID (the GPIO pinning guide has more on this). Search for the ACPI device ID in IORegExplorer. The device is supported by VoodooI2CHID if the `Compatible` property is `PNP0C50`.

## [VoodooI2CElan](https://github.com/kprinssu/VoodooI2CELan)

VoodooI2CElan implements support for the propriety Elan protocol found on many Elan trackpads and touchscreens. Your Elan device may have better support with this kext than with VoodooI2CHID.