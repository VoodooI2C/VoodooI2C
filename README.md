# VoodooI2C

VoodooI2C is a project to bring support for I2C input devices to OS X. This repository contains code for both I2C controllers and I2C devices.

# Current Status

Currently the following Intel Lynxpoint I2C controllers are almost fully supported:
* `INT33C2` and `INT33C3` - Haswell era
* `INT3432` and `INT3433` - Broadwell era
* `pci8086,9d60`, `pci8086,9d61`, `pci8086,a160` and `pci8086,a161` - Skylake era

Most i2c-hid touchscreens and trackpads work with minor modifications to the drivers. Note that most device have only very basic mouse functionality (navigating, left/right click, scrolling).

# Releases

The latest version is **v1.0.2** and can be downloaded on the [release page](https://github.com/alexandred/VoodooI2C/releases).

# Compatibility

Please check the [compatibility page](https://github.com/alexandred/VoodooI2C/wiki/Compatibility) on the VoodooI2C wiki to find out if your device is compatible. If it is not on the list but you still suspect VoodooI2C may work for you, contact us on our [Gitter page](http://gitter.im/alexandred/VoodooI2C).

# Helping out and testing

We are always looking for people with computers that have I2C controllers to help ensure these drivers will work for them.

The first step to setting up debugging is to open up IORegExplorer (v2.1) and search for controller ID relevant to your platform (see the **Current Status** section above). If you find something, great! Contact us on our [Gitter page](http://gitter.im/alexandred/VoodooI2C) and we'll see what kind of support your device will have.

If not (and you still suspect that you have I2C devices), you may need to patch your DSDT to ensure that OS X can properly enumerate the devices. You can see the wiki for instructions on how to do this.

# License

This program is protected by the GPL license. Please refer to the `LICENSE.txt` file for more information

# Contributing

We are looking for competent C++, OS X kernel, Linux kernel, I2C, HID etc developers to help improve this project! Here are the guidelines for contributing:

* Fork this repository and clone to your local machine
* Create a new feature branch and add commits
* Push your feature branch to your fork
* Submit a pull request upstream
