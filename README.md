# VoodooI2C

VoodooI2C is a project to bring support for I2C input devices to OS X. This repository contains code for both I2C controllers and I2C devices.

# Current Status

Currently the following Intel Lynxpoint I2C controllers are almost fully supported:
* `INT33C2` and `INT33C3` - Haswell era
* `INT3432` and `INT3433` - Broadwell era

Most i2c-hid touchscreens and trackpads work with minor modifications to the drivers. Note that this is only very basic mouse functionality (navigating, left/right click, scrolling). Multitouch is a whole other beast.

# Supported Devices
The following devices are confirmed to work with the current iteration of VoodooI2C

* Synaptics 7500 Clearpad touchscreen (Dell Venue Pro 11)
* Synaptics 7501 Clearpad touchscreen (Acer Switch 12)
* Atmel 1000 touchscreen (Dell XPS 12)
* NTRG 0001 touchscreen (Surface Pro 3)
* FTSC1000 touchscreen (Cube i7)
* CYAP0000 trackpad (Haswell Chromebooks)
 * This uses the propriety interface `VoodooCyapaGen3Device`.
* ATML0001 touchscreen (Acer C720P Chromebook/Chromebook Pixel 2)
  * This uses the propriety interface `VoodooI2CAtmelMxtScreenDevice`.

# Helping out and testing

We are always looking for people with computers that have I2C controllers to help ensure these drivers will work for them.

The first step to setting up debugging is to open up IORegExplorer (v2.1) and search for `INT33C2` and `INT33C3` or `INT3432` and `INT3433`. If you find something, great! Contact @alexandred at http://gitter.im/alexandred/VoodooI2C and we'll see what kind of support your device will have.

If not (and you still suspect that you have I2C devices), you may need to patch your DSDT to ensure that OS X can properly enumerate the devices. You can see the wiki for instructions on how to do this.

# License

This program is protected by the GPL license. Please refer to the `LICENSE.txt` file for more information

# Contributing

We are looking for competent C++, OS X kernel, Linux kernel, I2C, HID etc developers to help bring this project into a usable state! Here are the guidelines for contributing:

* Fork this repository and clone to your local machine
* Create a new feature branch and add commits
* Push your feature branch to your fork
* Submit a pull request upstream
