# VoodooI2C

VoodooI2C is a project to bring support for I2C input devices to OS X. This repository contains code for both I2C controllers and I2C devices.

# Current Status

Currently the Intel Lynxpoint I2C controllers with device IDs `INT33C2` and `INT33C3` are almost fully supported.

Support for i2c-hid touchscreens, trackpads and sensor hubs (which control accelerometer devices) is underway. Please note that this means the drivers are **WORK IN PROGRESS**, your devices will not work if you build and install this.

# Supported Devices
The following devices are confirmed to work with the current iteration of VoodooI2C

* Synaptics 7500 Clearpad touchscreen
* Atmel 1000 touchscreen

Support for the Surface Pro 3 touchscreen is more than likely guaranteed (we have not yet been able to test it)

# Helping out and testing

We are always looking for people with computers that have I2C controllers to help ensure these drivers will work for them.

The first step to setting up debugging is to open up IORegExplorer (v2.1) and search for `INT33C2` and `INT33C3`. If you find something, great! Contact @alexandred at http://gitter.im/alexandred/VoodooI2C and we'll see what kind of support your device will have.

If not (and you still suspect that you have I2C devices), you may need to patch your DSDT to ensure that OS X can properly enumerate the devices. Again, you can visit the gitter chatroom to get more help with this.

# License

This program is protected by the GPL license. Please refer to the `LICENSE.txt` file for more information

# Contributing

We are looking for competent C++, OS X kernel, Linux kernel, I2C, HID etc developers to help bring this project into a usable state! Here are the guidelines for contributing:

* Fork this repository and clone to your local machine
* Create a new feature branch and add commits
* Push your feature branch to your fork
* Submit a pull request upstream