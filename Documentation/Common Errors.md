#<cldoc:Common Errors>

&#8291;

The following is a list of common errors that can occur when trying to use VoodooI2C. On macOS 10.11, they can be found in the `system.log` log in the `Console` application. On 10.12+, you can run the following command in Terminal to view the kernel log:

```
	log show --predicate 'process == "kernel"' --start "2017-01-01 00:00:00"
```

You can adjust the time after `--start` to make the log shorter (you could set it for the last 30 minutes, for example). Note that you might have to search for `VoodooI2C` and `VoodooGPIO` to find relevant logs.

## Common Errors

 - **Could not find GPIO controller** - The GPIO controller enable patch has not been applied.
 - **Pin cannot be used as IRQ** - The GPIO pin for your device in the DSDT is likely wrong.
 - **Could not get interrupt event source** - Could not find either APIC or GPIO interrupts in your device's DSDT entry.
 - **Unknown Synopsys component type: 0xffffffff** - The I2C controller patch has not been applied.