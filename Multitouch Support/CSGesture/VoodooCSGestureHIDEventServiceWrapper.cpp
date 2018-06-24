//
//  VoodooCSGestureHIDEventServiceWrapper.cpp
//  VoodooI2C
//
//  Created by CoolStar on 9/6/16.
//  Copyright © 2016 CoolStar. All rights reserved.
//

#include <IOKit/IOLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include "VoodooCSGestureHIDEventServiceWrapper.hpp"
#include "VoodooI2CCSGestureEngine.hpp"
#include <libkern/version.h>


OSDefineMetaClassAndStructors(VoodooCSGestureHIDEventServiceWrapper, IOHIDEventService);

bool VoodooCSGestureHIDEventServiceWrapper::init(){
    if (!super::init())
        return false;
    
    return true;
}

bool VoodooCSGestureHIDEventServiceWrapper::start(IOService *provider){
    if (!super::start(provider))
        return false;

    int enabledProperty = 1;
    
    if (version_major>=16) {
        
        setProperty("SupportsGestureScrolling", true);
        setProperty("TrackpadFourFingerGestures", false);
        setProperty("ApplePreferenceIdentifier", "com.apple.AppleMultitouchTrackpad");
        setProperty("MTHIDDevice", true);
        setProperty("MT Built-in", true);
        setProperty("ApplePreferenceCapability", true);
        setProperty("TrackpadEmbedded", true);
        setProperty("TrackpadThreeFingerDrag", false);
    } else {
        setProperty("TrackpadCornerSecondaryClick", enabledProperty,
                    sizeof(enabledProperty) * 8);
    }

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

void VoodooCSGestureHIDEventServiceWrapper::stop(IOService *provider){
    super::stop(provider);
}

void VoodooCSGestureHIDEventServiceWrapper::updateRelativeMouse(int dx, int dy, int buttons){
    // 0x1 = left button
    // 0x2 = right button
    // 0x4 = middle button
    
    uint64_t now_abs;
    clock_get_uptime(&now_abs);
    dispatchRelativePointerEvent(now_abs, dx, dy, buttons);
}

void VoodooCSGestureHIDEventServiceWrapper::updateAbsoluteMouse(SInt16 x, SInt16 y, int buttons){
    // 0x1 = left button
    // 0x2 = right button
    // 0x4 = middle button
    
    uint64_t now_abs;
    clock_get_uptime(&now_abs);
    
    IOLog("%s::Absolute input (%d x %d) max: (%d x %d)\n", getName(), x, y, gesturerec->softc.resx, gesturerec->softc.resy);
    
    if (x != -1 && y != -1){
        last_x = x;
        last_y = y;
    } else {
        x = last_x;
        y = last_y;
    }
    
    IOFixed xFixed = ((x * 1.0f)/gesturerec->softc.resx) * 65535;
    IOFixed yFixed = ((y * 1.0f)/gesturerec->softc.resy) * 65535;
    
    dispatchDigitizerEvent(now_abs, 1, IOHIDEventService::kDigitizerTransducerTypeFinger, 1, buttons, xFixed, yFixed);
}

void VoodooCSGestureHIDEventServiceWrapper::updateScroll(short dy, short dx, short dz){
    uint64_t now_abs;
    clock_get_uptime(&now_abs);
    if (!horizontalScroll)
        dx = 0;
    
    IOFixed deltaAxis1 = dy << 12;
    IOFixed deltaAxis2 = dx << 12;
    IOFixed deltaAxis3 = dz << 12;
    
    dispatchScrollWheelEventWithFixed(now_abs, deltaAxis1, deltaAxis2, deltaAxis3);
}

IOReturn VoodooCSGestureHIDEventServiceWrapper::setSystemProperties(OSDictionary *dict)
{
    
    /*Known Keys:
     HIDDefaultParameters, HIDClickTime, HIDClickSpace, HIDKeyRepeat, HIDInitialKeyRepeat, HIDPointerAcceleration, HIDScrollAcceleration, HIDPointerButtonMode, HIDF12EjectDelay, EjectDelay, HIDSlowKeysDelay, HIDStickyKeysDisabled, HIDStickyKeysOn, HIDStickyKeysShiftToggles, HIDMouseKeysOptionToggles, HIDFKeyMode, HIDMouseKeysOn, HIDKeyboardModifierMappingPairs, MouseKeysStopsTrackpad, HIDScrollZoomModifierMask, HIDMouseAcceleration, HIDMouseScrollAcceleration, HIDTrackpadAcceleration, HIDTrackpadScrollAcceleration, TrackpadPinch, TrackpadFourFingerVertSwipeGesture, TrackpadRotate, TrackpadHorizScroll, TrackpadFourFingerPinchGesture, TrackpadTwoFingerDoubleTapGesture, TrackpadMomentumScroll, TrackpadThreeFingerTapGesture, TrackpadThreeFingerHorizSwipeGesture, Clicking, TrackpadScroll, DragLock, TrackpadFiveFingerPinchGesture, TrackpadThreeFingerVertSwipeGesture, TrackpadTwoFingerFromRightEdgeSwipeGesture, Dragging, TrackpadRightClick, TrackpadCornerSecondaryClick, TrackpadFourFingerHorizSwipeGesture, TrackpadThreeFingerDrag, JitterNoMove, JitterNoClick, PalmNoAction When Typing, PalmNoAction Permanent, TwofingerNoAction, OutsidezoneNoAction When Typing, Use Panther Settings for W, Trackpad Jitter Milliseconds, USBMouseStopsTrackpad, HIDWaitCursorFrameInterval*/
    
    /*
     macOS 10.12 (and above)
     Sierra fix requires the cast to be changed to OSBoolean
     Still need the OSNumber checks as a last resort, DO NOT clean up the check statements below by checking os version
     */
    
    OSBoolean *clicking = OSDynamicCast(OSBoolean, dict->getObject("Clicking"));
    if(clicking) {
        gesturerec->softc.settings.tapToClickEnabled = clicking->isTrue();
    } else {
        OSNumber *clickingNum = OSDynamicCast(OSNumber, dict->getObject("Clicking"));
        if (clickingNum ){
            gesturerec->softc.settings.tapToClickEnabled = clickingNum->unsigned32BitValue() & 0x1;
        }
    }
    
    OSBoolean *dragging = OSDynamicCast(OSBoolean, dict->getObject("Dragging"));
    if (dragging){
        gesturerec->softc.settings.tapDragEnabled = dragging->isTrue();
    } else {
        OSNumber *draggingNum = OSDynamicCast(OSNumber, dict->getObject("Dragging"));
        if (draggingNum) {
            gesturerec->softc.settings.tapDragEnabled = draggingNum->unsigned32BitValue() & 0x1;
        }
        
    }
    
    OSBoolean *hscroll  = OSDynamicCast(OSBoolean, dict->getObject("TrackpadHorizScroll"));
    if (hscroll){
        horizontalScroll = hscroll->isTrue();
    } else {
        
        OSNumber *hscrollNum = OSDynamicCast(OSNumber, dict->getObject("TrackpadHorizScroll"));
        if(hscrollNum) {
            horizontalScroll = hscrollNum->unsigned32BitValue() & 0x1;
        }
    }
    
    OSBoolean* right_click = OSDynamicCast(OSBoolean, dict->getObject("TrackpadCornerSecondaryClick"));
    if (right_click){
        gesturerec->softc.settings.literal_right_click = right_click->isTrue();
    } else {
        
        OSNumber *right_click_num = OSDynamicCast(OSNumber, dict->getObject("TrackpadCornerSecondaryClick"));
        if(right_click_num) {
            gesturerec->softc.settings.literal_right_click = right_click_num->unsigned32BitValue() & 0x1;
        }
    }

    
    gesturerec->softc.settings.multiFingerTap = true;
    
    return super::setSystemProperties(dict);
}
