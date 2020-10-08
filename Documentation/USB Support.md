#<cldoc:USB Support>

&#8291;

VoodooI2C has experimental support for USB multitouch devices such as touchscreens and trackpads. There are no instructions for these kinds of devices, simply inject `VoodooI2C.kext` and its plugins as well as the satellite `VoodooI2CHID.kext` using Clover or OpenCore and you should be good to go! The former two are only needed to satisfy the dependency-requirements of VoodooI2C and don't do any real work or harm in USB-only setups.
