//
//  VoodooCSGestureHIPointingWrapper.cpp
//  VoodooI2C
//
//  Created by CoolStar on 9/6/16.
//  Copyright © 2016 CoolStar. All rights reserved.
//

#include <IOKit/IOLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include "VoodooCSGestureHIPointingWrapper.h"
#include <libkern/version.h>


OSDefineMetaClassAndStructors(VoodooCSGestureHIPointingWrapper, IOHIPointing);

UInt32 VoodooCSGestureHIPointingWrapper::deviceType()
{
    return NX_EVS_DEVICE_TYPE_MOUSE;
}

UInt32 VoodooCSGestureHIPointingWrapper::interfaceID()
{
    return NX_EVS_DEVICE_INTERFACE_BUS_ACE;
}

IOItemCount VoodooCSGestureHIPointingWrapper::buttonCount(){
    return 2;
};

IOFixed VoodooCSGestureHIPointingWrapper::resolution(){
    return (300) << 16;
};

bool VoodooCSGestureHIPointingWrapper::init(){
    if (!super::init())
        return false;
    
    return true;
}

bool VoodooCSGestureHIPointingWrapper::start(IOService *provider){
    if (!super::start(provider))
        return false;
    
    if (version_major>=16) {
        
        setProperty("SupportsGestureScrolling", true);
        setProperty("TrackpadFourFingerGestures", false);
        setProperty("ApplePreferenceIdentifier", "com.apple.AppleMultitouchTrackpad");
        setProperty("MTHIDDevice", true);
        setProperty("MT Built-in", true);
        setProperty("ApplePreferenceCapability", true);
        setProperty("TrackpadEmbedded", true);
        setProperty("TrackpadThreeFingerDrag", false);
        
    }
    
    
    
    int enabledProperty = 1;
    setProperty("Clicking", enabledProperty,
                sizeof(enabledProperty) * 8);
    setProperty("TrackpadScroll", enabledProperty,
                sizeof(enabledProperty) * 8);
    setProperty("TrackpadHorizScroll", enabledProperty,
                sizeof(enabledProperty) * 8);
    
    
    
    //
    // Must add this property to let our superclass know that it should handle
    // trackpad acceleration settings from user space.  Without this, tracking
    // speed adjustments from the mouse prefs panel have no effect.
    //
    
    //int prodID = 0x223; //old settings pane (VoodooPS2)
    int prodID = 0x0262; //product id from newer Macbook
    setProperty("ProductID", prodID, sizeof(prodID) * 8);
    
    int vendorID = 0x05ac;
    setProperty("VendorID", vendorID, sizeof(vendorID) * 8);
    
    //Need to fake Vendor and Product ID to pull up the trackpad settings pane
    
    setProperty(kIOHIDPointerAccelerationTypeKey, kIOHIDTrackpadAccelerationType);
    setProperty(kIOHIDScrollAccelerationTypeKey, kIOHIDTrackpadScrollAccelerationKey);
    setProperty(kIOHIDScrollResolutionKey, 800 << 16, 32);
    
    setProperty("HIDScrollResolutionX", 800 << 16, 32);
    setProperty("HIDScrollResolutionY", 800 << 16, 32);
    
    return true;
}

void VoodooCSGestureHIPointingWrapper::stop(IOService *provider){
    super::stop(provider);
}

void VoodooCSGestureHIPointingWrapper::updateRelativeMouse(int dx, int dy, int buttons){
    // 0x1 = left button
    // 0x2 = right button
    // 0x4 = middle button
    
    uint64_t now_abs;
    clock_get_uptime(&now_abs);
    dispatchRelativePointerEvent(dx, dy, buttons, now_abs);
}

void VoodooCSGestureHIPointingWrapper::updateScroll(short dy, short dx, short dz){
    uint64_t now_abs;
    clock_get_uptime(&now_abs);
    if (!horizontalScroll)
        dx = 0;
    dispatchScrollWheelEvent(dy, dx, dz, now_abs);
}

IOReturn VoodooCSGestureHIPointingWrapper::setParamProperties(OSDictionary *dict)
{
    
    /*Known Keys:
     HIDDefaultParameters, HIDClickTime, HIDClickSpace, HIDKeyRepeat, HIDInitialKeyRepeat, HIDPointerAcceleration, HIDScrollAcceleration, HIDPointerButtonMode, HIDF12EjectDelay, EjectDelay, HIDSlowKeysDelay, HIDStickyKeysDisabled, HIDStickyKeysOn, HIDStickyKeysShiftToggles, HIDMouseKeysOptionToggles, HIDFKeyMode, HIDMouseKeysOn, HIDKeyboardModifierMappingPairs, MouseKeysStopsTrackpad, HIDScrollZoomModifierMask, HIDMouseAcceleration, HIDMouseScrollAcceleration, HIDTrackpadAcceleration, HIDTrackpadScrollAcceleration, TrackpadPinch, TrackpadFourFingerVertSwipeGesture, TrackpadRotate, TrackpadHorizScroll, TrackpadFourFingerPinchGesture, TrackpadTwoFingerDoubleTapGesture, TrackpadMomentumScroll, TrackpadThreeFingerTapGesture, TrackpadThreeFingerHorizSwipeGesture, Clicking, TrackpadScroll, DragLock, TrackpadFiveFingerPinchGesture, TrackpadThreeFingerVertSwipeGesture, TrackpadTwoFingerFromRightEdgeSwipeGesture, Dragging, TrackpadRightClick, TrackpadCornerSecondaryClick, TrackpadFourFingerHorizSwipeGesture, TrackpadThreeFingerDrag, JitterNoMove, JitterNoClick, PalmNoAction When Typing, PalmNoAction Permanent, TwofingerNoAction, OutsidezoneNoAction When Typing, Use Panther Settings for W, Trackpad Jitter Milliseconds, USBMouseStopsTrackpad, HIDWaitCursorFrameInterval*/
    
    if (version_major >= 16) {
        //Can't read trackpad preferences on macOS Sierra (Darwin 16), defaulting to true:
        gesturerec->softc->settings.tapToClickEnabled = true;
        
        gesturerec->softc->settings.tapDragEnabled = true;
    }
    else {
        OSNumber *clicking = OSDynamicCast(OSNumber, dict->getObject("Clicking"));
        if (clicking){
            gesturerec->softc->settings.tapToClickEnabled = clicking->unsigned32BitValue() & 0x1;
        }
        
        OSNumber *dragging = OSDynamicCast(OSNumber, dict->getObject("Dragging"));
        if (dragging){
            gesturerec->softc->settings.tapDragEnabled = dragging->unsigned32BitValue() & 0x1;
        }
    }
    
    gesturerec->softc->settings.multiFingerTap = true;
    
    OSNumber *hscroll  = OSDynamicCast(OSNumber, dict->getObject("TrackpadHorizScroll"));
    if (hscroll){
        horizontalScroll = hscroll->unsigned32BitValue() & 0x1;
    }
    return super::setParamProperties(dict);
}

