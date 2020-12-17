[![release](https://img.shields.io/github/release/alexandred/VoodooI2C.svg)](https://github.com/VoodooI2C/VoodooI2C/releases)
[![circleci](https://circleci.com/gh/VoodooI2C/VoodooI2C.svg?style=shield&circle-token=:circle-token)](https://github.com/VoodooI2C/VoodooI2C/releases)
[![Gitter chat](https://img.shields.io/gitter/room/nwjs/nw.js.svg?colorB=ed1965)](https://gitter.im/alexandred/VoodooI2C)

## What is VoodooI2C?

VoodooI2C is a project consisting of macOS kernel extensions that add support for I2C bus devices. The project is split into two main components: the **core** extension and various other **satellite** extensions.

### The Core

The core is the `VoodooI2C.kext` kernel extension. This kext is intended to be installed by anyone whose computer requires some form of I2C support. It consists of I2C controller drivers and is responsible for publishing device nubs to the IOService plane.

### The Satellites

The satellites are a collection of various kernel extensions that implement support for a specific type of I2C device. An example of a satellite kext is `VoodooI2CHID.kext` which adds support for I2C-HID devices. Usually a user will install one satellite kext per class of I2C device.

## Current Status

The following Intel I2C controllers are fully supported:

1. `INT33C2` and `INT33C3` - Haswell era
2. `INT3432` and `INT3433` - Broadwell era
3. `pci8086,9d60`, `pci8086,9d61`, `pci8086,9d62` and `pci8086,9d63` - Skylake era
4. `pci8086,a160`, `pci8086,a161`, `pci8086,a162` and `pci8086,a163` - Kaby Lake era
5. `pci8086,9de8`, `pci8086,9de9`, `pci8086,9dea` and `pci8086,9deb` - Cannon Lake/Whiskey Lake era
6. `pci8086,a368`, `pci8086,a369`, `pci8086,a36a` and `pci8086,a36b` - Coffee Lake era
7. `pci8086,2e8`, `pci8086,2e9`, `pci8086,2ea`, `pci8086,2eb`, `pci8086,6e8`, `pci8086,6e9`, `pci8086,6ea` and `pci8086,6eb`- Comet Lake era
8. `pci8086,34e8`, `pci8086,34e9`, `pci8086,34ea` and `pci8086,34eb` - Ice Lake era

The following device classes are fully supported:

1. I2C-HID devices
2. ELAN devices
3. FTE devices

Note that there is sometimes an overlap between device classes. For example, some ELAN devices may also be I2C-HID devices.

## Releases

The latest version is ![release](https://img.shields.io/github/release/alexandred/VoodooI2C.svg) and can be downloaded on the [release page](https://github.com/alexandred/VoodooI2C/releases).

## Compatibility

Please check the [compatibility page](https://github.com/alexandred/VoodooI2C/wiki/Compatibility) on the VoodooI2C wiki to find out if your device is compatible. If it is not on the list but you still suspect VoodooI2C may work for you, contact us on our [Gitter page](http://gitter.im/alexandred/VoodooI2C).

## Documentation and Troubleshooting

Please visit the [documentation site](https://voodooi2c.github.io/) for further information how to install and troubleshoot VoodooI2C. 

## License

This program is protected by the GPL license. Please refer to the `LICENSE.txt` file for more information

## Contributing

We are looking for competent C++, OS X kernel, Linux kernel, I2C, HID etc developers to help improve this project! Here are the guidelines for contributing:

* Fork this repository and clone to your local machine
* Create a new feature branch and add commits
* Push your feature branch to your fork
* Submit a pull request upstream
