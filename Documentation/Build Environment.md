#<cldoc:Build Environment>

&#8291;

VoodooI2C is a complex project with many individual parts. This makes setting up the development environment for this kext particularly daunting. This file is intended for developers who wish to contribute to VoodooI2C or users who wish to compile custom versions for VoodooI2C. **Please note that no support will be provided for custom built kexts**

## Prerequisites

Before attempting to build VoodooI2C, make sure that you and your development environment satisfy the following prerequisites:

    1. A machine running at least macOS 10.12 Sierra.
    2. The latest XCode available for your version of macOS.
    3. The development SDK for your target OS version installed on XCode. Note that XCode always comes with the latest SDK.
    4. Previous experience building with XCode or similar IDEs.
    5. Previous experience with C++.

## Cloning VoodooI2C

In a directory of your choosing, run the following command to clone VoodooI2C:

```
git clone --recursive https://github.com/alexandred/VoodooI2C
```
Note that this command will pull VoodooI2C, its dependencies and all the satellite kexts that depend on VoodooI2C. The dependencies and satellites all exist as so-called submodules of the main git repository. We shall discuss this further later on. If you happen to have already cloned VoodooI2C with the command `git clone https://github.com/alexandred/VoodooI2C`, then you will not have pulled the submodules. In this case you must run the following additional commands inside the newly cloned repo:
```
git submodule init
git submodule update
```
## Installing the build dependencies

VoodooI2C relies on certain packages to aid its build and deploy process. It is not strictly necessary to have these dependencies installed and the build phases that are dependent on them can be removed from the build phases for the main VoodooI2C project. Anyone who wishes to contribute to VoodooI2C, however, must have them installed and we will not accept PRs where the build phases have been deleted (as we depend on them for continuous integration).

### VoodooInput
VoodooI2C depends on VoodooInput for multitouch handling. You will need to retrieve and build the latest versions of VoodooInput by running `src=$(/usr/bin/curl -Lfs https://raw.githubusercontent.com/acidanthera/VoodooInput/master/VoodooInput/Scripts/bootstrap.sh) && eval "$src" && mv VoodooInput Dependencies`. Please run the command in the Git repository root, to ensure that the `VoodooInput` folder exists within the `Dependencies` folder.

### `cpplint`

We use `cpplint` to lint most of the repository's file in order to enforce a common programming paradigm across the project's many components. As such, we encourage all contributions to be linted against `cpplint` . To install `cpplint`, you must have the `pip` package manager installed on your system. If you do not, a quick Google search will tell you how to do so. Then run this command:
```
sudo pip install cpplint
```

### `cldoc`

In order to streamline user experience, we use (a modified version of) `cldoc` to automatically generate documentation on two levels. `cldoc` will forst compile all the Markdown files in the `Documentation` directory. It will then crawl the project repository and compile documentation for each documented function, class, variable etc. These files are packaged together during continuous integration and are uploaded to the [documentation site](https://voodooi2c.github.io) that you are currently on. We will further discuss documentation later on.

To install `cldoc`, first clone the custom version of `cldoc` from [this repo](https://github.com/alexandred/cldoc) then follow the "Installing from source" instructions found [here](http://jessevdk.github.io/cldoc/gettingstarted.html).

## Building, loading, and installing VoodooI2C

You should now be ready to build VoodooI2C. With XCode, open the `VoodooI2C.xcworkspace` file and build it as usual. Once the build is complete, the products directory should be populated with the `VoodooI2C.kext` file along with many other satellite kexts.

### Manually loading VoodooI2C

Provided that VoodooI2C.kext is not already loaded on your machine, you can immediately manually load it. Navigate to the directory containing the build products and first run the following command:

```
sudo chown -R root:wheel VoodooI2C.kext XXX.KEXT
```
replacing `XXX` with the name of the satellite that you want to run. Then run the following command to load the kext:

```
sudo kextutil -d VoodooI2C.kext XXX.kext
```

### Installing VoodooI2C

The preferred install location for VoodooI2C for the purposes of testing is the Clover `kexts/Other` directory. Simply place VoodooI2C.kext and your desired satellite kext(s) there and restart.

## Contributing to VoodooI2C

We follow standard open source development practices in the VoodooI2C project. To contribute to the main repository or any of the dependency/satellite repositories, you should follow the following instructions:

    1. Fork the repository on Github
    1. Create a new feature branch locally and add commits
    2. Push your feature branch to your fork
    3. Submit a pull request upstream

### Creating and managing a satellite for VoodooI2C

VoodooI2C has been written with modularity in mind. It is thus relatively easy to extend the functionality of VoodooI2C by writing new satellite kexts. One may want to do this in order to add support for a particular properiety protocol; examples of this are VoodooI2CElan and VoodooI2CSynaptics. Satellites can also extend core multitouch functionality by adding new multitouch engines; examples of this are VoodooI2CUPDDEngine which replaces the standard multitouch engine with an API which connects to TouchBase's (paid) UPDD software.

I am of the opinion that the best way to learn how to write plugins is to consult the documentation and existing examples. It is thus recommended that you consult the documentation for `VoodooI2CDeviceNub` which acts as the interface between the I2C controller driver and the satellites and can therefore be regarded as the interface of the VoodooI2C API. It is then recommended that you view the source code of the various satellite kexts listed on the <Satellite Kexts> page. It is of course recommended that you read the [IOKit Fundmentals ](https://developer.apple.com/library/archive/documentation/DeviceDrivers/Conceptual/IOKitFundamentals/Introduction/Introduction.html) to understand how to write kernel land drivers for macOS.

At this stage, I will only outline the proper steps one should take in order to add a satellite to the VoodooI2C build environment. Satellites are added as git submodules and the way to do this is slightly convoluted. It is probably not the best way but XCode makes life difficult and we have not yet been able to find an alternative. In any case the steps you must take as as follows:

    1. Create a Github repository for your satellite and clone it locally in a folder separate to the VoodooI2C folder.
    2. Create a blank XCode `Generic Kernel Extension` project and save it in this git repository and push it back to Github.
    3. Inside the VoodooI2C directory, run the command `git submodule add <URL_TO_GITHUB_REPO_HERE> "VoodooI2C Satellites"/<NAME_OF_SATELLITE_HERE>`.
    4. Open the main VoodooI2C workspace in XCode and on the left hand side, locate the `VoodooI2C Satellites` folder. Right click this folder and click "Add files to VoodooI2C". Navigate to `/VoodooI2C Satellites/<NAME_OF_SATELLITE_HERE>/<NAME_OF_SATELLITE_HERE>.xcodeproj` and choose this file.
    5. Navigate to the build phases for the main VoodooI2C project and expand `Target Dependencies`. In the list you should find your satellite's build product (i.e <NAME_OF_SATELLITE_HERE>.kext), choose this file.

You can now use the main VoodooI2C workspace to edit your satellite kext and access the VoodooI2C API's headers. Everytime you build, your satellite will automatically be added to the build process.

Everytime you wish to update your satellite you will need to follow the following steps:

    1. Navigate to the submodule directory (i.e `/VoodooI2C Satellites/<NAME_OF_SATELLITE_HERE>`) in terminal and add and commit the changes as you usually would with git.
    2. Push these changes from the submodule itself to your github remote.
    3. Navigate back to the main VoodooI2C directory and update VoodooI2C to point to the new submodule commit by adding and commiting as usual.

If there have been changes to the submodule remotely and you wish to pull them, you must run the follow command in the main VoodooI2C repo:

```
    git submodule update --recursive --remote
```

## Debugging VoodooI2C panics

Writing kernel land drivers is hard and it is often the case that you will cause kernel panics with your code. Thankfully Apple provides detailed kernel panic logs and tools to decipher these logs which allow us to obtain the exact line of code that caused the panic. This is what the debug symbols you will find after building are actually for. When your system panics because of VoodooI2C, there are two things you must first do in order to obtain a proper log. Firstly, ensure that you have set the kernel boot flag `keepsyms=1` in your Clover's config.plist. Alternatively, you can simply select `Keep Debug Symbols` in the Clover boot options before you instigate the kernel panic. You must next ensure that you have NVRAM enabled on your system in order for the panic log to appear after rebooting. 

Let us now consider the following actual VoodooI2C-related kernel panic:

```
Backtrace (CPU 7), Frame : Return Address
0xffffff81f0423830 : 0xffffff80003aeafd mach_kernel : _handle_debugger_trap + 0x48d
0xffffff81f0423880 : 0xffffff80004e85a3 mach_kernel : _kdp_i386_trap + 0x153
0xffffff81f04238c0 : 0xffffff80004d9fca mach_kernel : _kernel_trap + 0x4fa
0xffffff81f0423930 : 0xffffff800035bca0 mach_kernel : _return_from_trap + 0xe0
0xffffff81f0423950 : 0xffffff80003ae517 mach_kernel : _panic_trap_to_debugger + 0x197
0xffffff81f0423a70 : 0xffffff80003ae363 mach_kernel : _panic + 0x63
0xffffff81f0423ae0 : 0xffffff80004da1ed mach_kernel : _kernel_trap + 0x71d
0xffffff81f0423c50 : 0xffffff800035bca0 mach_kernel : _return_from_trap + 0xe0
0xffffff81f0423c70 : 0xffffff7f8353155c com.alexandred.VoodooI2C : __ZN27VoodooI2CMT2SimulatorDevice24getMultitouchPreferencesEPvS0_P9IOServiceP10IONotifier + 0x6c
0xffffff81f0423d70 : 0xffffff8000a28a7a mach_kernel : __ZN9IOService14invokeNotifierEP18_IOServiceNotifier + 0xea
0xffffff81f0423dc0 : 0xffffff8000a345ab mach_kernel : __ZN9IOService23addMatchingNotificationEPK8OSSymbolP12OSDictionaryPFbPvS5_PS_P10IONotifierES5_S5_i + 0x4b
0xffffff81f0423df0 : 0xffffff7f83531773 com.alexandred.VoodooI2C : __ZN27VoodooI2CMT2SimulatorDevice5startEP9IOService + 0x1f5
0xffffff81f0423e20 : 0xffffff7f83532cab com.alexandred.VoodooI2C : __ZN21VoodooI2CNativeEngine5startEP9IOService + 0x9d
0xffffff81f0423e40 : 0xffffff8000a2c65b mach_kernel : __ZN9IOService14startCandidateEPS_ + 0x6b
0xffffff81f0423e80 : 0xffffff8000a2c3a1 mach_kernel : __ZN9IOService15probeCandidatesEP12OSOrderedSet + 0x911
0xffffff81f0423f00 : 0xffffff8000a2b8f7 mach_kernel : __ZN9IOService14doServiceMatchEj + 0x2c7
0xffffff81f0423f50 : 0xffffff8000a2d3c6 mach_kernel : __ZN15_IOConfigThread4mainEPvi + 0x1a6
0xffffff81f0423fa0 : 0xffffff800035b0ce mach_kernel : _call_continuation + 0x2e
Kernel Extensions in backtrace:
com.alexandred.VoodooI2C(2.1.4)[F3C676CB-CAAC-3748-A38E-E339E7C8426D]@0xffffff7f83528000->0xffffff7f83556fff
dependency: com.alexandred.VoodooI2CServices(1)[D30D6DC2-53F4-3DBE-8DB9-8423EFF1F350]@0xffffff7f834f3000
dependency: com.apple.iokit.IOACPIFamily(1.4)[99A8A054-9F64-3FB8-BB1D-5973F8AB04A1]@0xffffff7f80d10000
dependency: com.apple.iokit.IOHIDFamily(2.0.0)[2AEFB432-C333-3CFC-955E-24BBDDDE0F5E]@0xffffff7f81233000
dependency: com.apple.iokit.IOPCIFamily(2.9)[7EA30FDD-A2FB-390F-99DD-42BC19691BB4]@0xffffff7f80c95000
dependency: org.coolstar.VoodooGPIO(1.1)[4E12B69A-2ECF-3D57-8DFE-F1F9D21FAF03]@0xffffff7f83512000
```

By enabling debug symbols during this boot, we already know the culprit function: `VoodooI2CMT2SimulatorDevice::getMultitouchPreferences`. With a little more work, however, we can do much better. First identify which kext caused the kernel panic. In this example it is `VoodooI2C.kext`. We must next identify the load address of `VoodooI2C.kext`. To obtain this, look in the list after `Kernel Extensions in backtrace`. The load address is the number `0xffffff7f83528000` found after the `@` symbol. We next need the return address of the function the caused the panic. In this case, this is `0xffffff7f8353155c`.

With these numbers in mind, navigate in terminal to the directory containing the debug symbols and kexts **for the version listed in the panic log**. It is of no use to use debug symbols from a different version as there is a strong chance that the address of the functions have changed with the new version. At this stage we can now run the command
```
atos -arch x86_64 -o VoodooI2C.kext/Contents/MacOS/VoodooI2C -l 0xffffff7f83528000 0xffffff7f8353155c
```
which gives the output
```
VoodooI2CMT2SimulatorDevice::getMultitouchPreferences(void&, void&, IOService&, IONotifider&) (in VoodooI2C) (VoodooI2CMT2SimulatorDevice.cpp:265)
```
In particular, the culprit is line 265 in the file `VoodooI2CMT2SimulatorDevice.cpp`.
