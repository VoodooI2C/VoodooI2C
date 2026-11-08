#<cldoc:Polling Mode>

&#8291;

## What is Polling Mode?

Due to incompatibilities with Apple's core kexts, it is often necessary to manually edit your DSDT to enable GPIO interrupts. This is only the case for systems that are Skylake or newer. The process of GPIO patching is fairly involved and because of this, certain satellite kexts (currently only VoodooI2CHID) support running in two different modes:

1. Polling
2. Interrupts (either APIC or GPIO)

You can think of polling mode as the "safe boot" mode of VoodooI2C. As such it is a suitable mode for use during the installation of macOS. Polling mode is also suitable for people who machines with buggy GPIO implementation (such as various ASUS laptops).

However, it is highly recommended that once your system is up and running, you should apply all the GPIO patches (except Haswell and Broadwell users) to ensure optimal performance. If you find that you cannot get your trackpad to work in interrupts mode or that interrupts mode leads to high CPU usage then it is likely you have a system with a buggy implementation of GPIO. In this case you will have to switch back to polling mode. In any case, you should still go through the troubleshooting process as outlined on the <Troubleshooting> page to make sure that you have not made a mistake.

## DSDT edits

Polling mode may sometimes require some minor modifications to your DSDT. This is usally only the case if you have previously followed the GPIO pinning guide. Vanilla DSDTs should not typically need to follow the following instructions.

Using the <GPIO Pinning> guide, determine your device's ACPI ID. In the example from that guide, the ACPI ID is `TPL0`.

Navigate to your device's entry in your DSDT. Find its `_CRS` method. You must ensure that all the `Return` statements are of the form `Return (ConcatenateResTemplate (SBFB, SBFI))` or `Return (SBFB)`. In other words, there should be no mention of `SBFG` in the `_CRS` method.

## Forcing Polling Mode via Boot Argument or Device Property

Starting from version 2.5.3, VoodooI2C allows you to force polling mode directly through boot arguments or device properties, without requiring manual DSDT modifications.

### 1. Boot Argument: `-vi2c-force-polling`

Add `-vi2c-force-polling` to your bootloader's boot arguments (`boot-args` in OpenCore under `NVRAM -> Add -> 7C436110-AB2A-4BBB-A880-FE41995C9F82`, or `Arguments` in Clover).

When this boot argument is present, VoodooI2C skips GPIO interrupt registration and forces matching satellite drivers (such as `VoodooI2CHID`) to run in polling mode:

```text
-vi2c-force-polling
```

This is especially helpful for modern platforms (such as Alder Lake and newer) or systems where GPIO interrupts are problematic.

### 2. Device Property: `force-polling`

If you have multiple I2C controllers and only wish to force polling mode on a specific controller, you can inject the `force-polling` property on that specific I2C PCI controller:

- **OpenCore:** Add under `DeviceProperties -> Add` targeting the I2C PCI controller path.
- **Clover:** Add under `Devices -> Properties`.

| Property | Type | Value |
| --- | --- | --- |
| `force-polling` | `DATA` / `BOOLEAN` | `01000000` / `1` |

When forced polling is active, `VoodooI2CDeviceNub` will log the following in the kernel log:

```text
VoodooI2CDeviceNub::Forced polling mode, skipping GPIO interrupts
```
 