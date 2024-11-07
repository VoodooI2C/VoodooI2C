//
//  VoodooI2CNativeEngine.cpp
//  VoodooI2C
//
//  Created by Alexandre on 10/02/2018.
//  Copyright © 2018 Alexandre Daoud and Kishor Prins. All rights reserved.
//

#include "VoodooI2CNativeEngine.hpp"

#define super VoodooI2CMultitouchEngine
OSDefineMetaClassAndStructors(VoodooI2CNativeEngine, VoodooI2CMultitouchEngine);

MultitouchReturn VoodooI2CNativeEngine::handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp) {
    if (!voodooInputInstance) {
        return MultitouchReturnContinue;
    }

    message.timestamp = timestamp;
    message.contact_count = event.contact_count;
    memset(message.transducers, 0, VOODOO_INPUT_MAX_TRANSDUCERS * sizeof(VoodooInputTransducer));
    
    VoodooI2CDigitiserTransducer* firstTransducer = (VoodooI2CDigitiserTransducer*) event.transducers->getObject(0);

    if (!firstTransducer)
        return MultitouchReturnBreak;
    
    if (firstTransducer->type == kDigitiserTransducerStylus)
        stylus_check = 1;

    int valid_touch_count = 0;

    for (int i = 0; i < event.contact_count; i++) {
        VoodooI2CDigitiserTransducer* transducer = (VoodooI2CDigitiserTransducer*) event.transducers->getObject(i+stylus_check);
        VoodooInputTransducer* inputTransducer = &message.transducers[i];
        
        if (!transducer) {
            continue;
        }
        
        inputTransducer->fingerType = (MT2FingerType) (kMT2FingerTypeIndexFinger + (i % 4));
        inputTransducer->secondaryId = transducer->secondary_id;
        
        inputTransducer->type = (transducer->type == DigitiserTransducerType::kDigitiserTransducerFinger) ? VoodooInputTransducerType::FINGER : VoodooInputTransducerType::STYLUS;
        
        inputTransducer->isValid = transducer->is_valid;
        if (inputTransducer->isValid) {
            valid_touch_count++;
        }
        
        if (!transducer->confidence.value()) {
            inputTransducer->fingerType = kMT2FingerTypePalm;
        }
        
        inputTransducer->isTransducerActive = transducer->tip_switch.value();
        inputTransducer->isPhysicalButtonDown = !transducer->has_secondary_button && transducer->physical_button.value(); // if it has secondary button, then it will be passed as buttons on the "trackpoint" device
        
        inputTransducer->currentCoordinates.x = transducer->coordinates.x.value();
        inputTransducer->previousCoordinates.x = transducer->coordinates.x.last.value;
        
        inputTransducer->currentCoordinates.y = transducer->coordinates.y.value();
        inputTransducer->previousCoordinates.y = transducer->coordinates.y.last.value;
        inputTransducer->supportsPressure = false;
        inputTransducer->timestamp = timestamp;

        // TODO: does VoodooI2C know width(s)? how does it measure pressure?
        inputTransducer->currentCoordinates.width = transducer->tip_pressure.value() / 2;
        inputTransducer->previousCoordinates.width = transducer->tip_pressure.last.value / 2;

        inputTransducer->currentCoordinates.pressure = transducer->tip_pressure.value();
        inputTransducer->previousCoordinates.pressure = transducer->tip_pressure.last.value;

        // Force Touch emulation
        // The button state is saved in the first transducer
        // Disable Force Touch emulation for two-button trackpads
        if (!firstTransducer->has_secondary_button && firstTransducer->physical_button.value() && isForceClickEnabled()) {
            inputTransducer->supportsPressure = true;
            inputTransducer->isPhysicalButtonDown = false;
            inputTransducer->currentCoordinates.pressure = 0xff;
            inputTransducer->currentCoordinates.width = 10;
        }
    }
    
    // set the thumb to improve 4F pinch and spread gesture and cross-screen dragging
    if (valid_touch_count >= 4 || firstTransducer->physical_button.value()) {
        // simple thumb detection: to find the lowest finger touch in the vertical direction.
        UInt32 y_max = 0;
        int thumb_index = 0;
        for (int i = 0; i < event.contact_count; i++) {
            VoodooInputTransducer* inputTransducer = &message.transducers[i];
            if (inputTransducer->isValid && inputTransducer->currentCoordinates.y >= y_max) {
                y_max = inputTransducer->currentCoordinates.y;
                thumb_index = i;
            }
        }
        message.transducers[thumb_index].fingerType = kMT2FingerTypeThumb;
    }
        
    super::messageClient(kIOMessageVoodooInputMessage, voodooInputInstance, &message, sizeof(VoodooInputEvent));
    
    if (firstTransducer->has_secondary_button) {
        RelativePointerEvent event;
        event.dx = 0;
        event.dy = 0;
        event.buttons = firstTransducer->physical_button.value();
        event.timestamp = timestamp;
        super::messageClient(kIOMessageVoodooTrackpointRelativePointer, voodooInputInstance, &event, sizeof(event));
    }
    return MultitouchReturnBreak;
}

MultitouchReturn VoodooI2CNativeEngine::handleTrackpointReport(VoodooI2CTrackpointEvent event, AbsoluteTime timestamp) {
    TrackpointReport vt_event;
    
    vt_event.dx = event.dx;
    vt_event.dy = event.dy;
    vt_event.buttons = event.buttons;
    vt_event.timestamp = timestamp;
    super::messageClient(kIOMessageVoodooTrackpointMessage, voodooInputInstance, &event, sizeof(event));
    
    return MultitouchReturnBreak;
}

bool VoodooI2CNativeEngine::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    parentProvider = OSDynamicCast(VoodooI2CMultitouchInterface, provider);
    
    if (!parentProvider) {
        return false;
    }
    
    voodooInputInstance = nullptr;
    
    setProperty(VOODOO_INPUT_LOGICAL_MAX_X_KEY, parentProvider->logical_max_x, 32);
    setProperty(VOODOO_INPUT_LOGICAL_MAX_Y_KEY, parentProvider->logical_max_y, 32);
    
    setProperty(VOODOO_INPUT_PHYSICAL_MAX_X_KEY, parentProvider->physical_max_x, 32);
    setProperty(VOODOO_INPUT_PHYSICAL_MAX_Y_KEY, parentProvider->physical_max_y, 32);
    
    setProperty("VoodooInputSupported", kOSBooleanTrue);
    
    OSDictionary* dict = OSDictionary::withCapacity(0);
    OSNumber* tmp = OSNumber::withNumber(3, 32);
    dict->setObject(VOODOO_TRACKPOINT_BTN_CNT, tmp);
    setProperty(VOODOO_TRACKPOINT_KEY, dict);
    
    OSSafeReleaseNULL(tmp);
    OSSafeReleaseNULL(dict);
    
    stylus_check = 0;

    return true;
}

bool VoodooI2CNativeEngine::isForceClickEnabled() {
    AbsoluteTime now_abs;
    uint64_t diff_ns;
    clock_get_uptime(&now_abs);
    absolutetime_to_nanoseconds(now_abs - lastForceClickPropertyUpdateTime, &diff_ns);

    if (diff_ns < 1e9) {
        lastForceClickPropertyUpdateTime = now_abs;
        return lastIsForceClickEnabled;
    }
    
    // Blocking reading here takes about 10 microseconds
    OSDictionary* dict = OSDynamicCast(OSDictionary, this->getProperty("MultitouchPreferences", gIOServicePlane, kIORegistryIterateRecursively));
    if (!dict) {
        return lastIsForceClickEnabled;
    }
    if (OSCollectionIterator* i = OSCollectionIterator::withCollection(dict)) {
        while (OSSymbol* key = OSDynamicCast(OSSymbol, i->getNextObject())) {
            // If tap to click (property "Clicking") is disabled,
            // then we should forbid force clicking,
            // because it makes impossible to perform a non-force click
            if (key->isEqualTo("Clicking")) {
                OSBoolean *boolValue;
                OSNumber *numValue;
                if ((boolValue = OSDynamicCast(OSBoolean, dict->getObject(key))) != nullptr) {
                    if (!boolValue->getValue()) {
                        lastIsForceClickEnabled = false;
                        break;
                    }
                }
                // is it needed? in default properties it's number, not boolean
                else if ((numValue = OSDynamicCast(OSNumber, dict->getObject(key))) != nullptr && !numValue->unsigned64BitValue()) {
                    lastIsForceClickEnabled = false;
                    break;
                }
            }
            // System -> Preferences -> Trackpad -> Force Click and haptic feedback
            // ForceSuppressed
            if (key->isEqualTo("ForceSuppressed")) {
                OSBoolean* value = OSDynamicCast(OSBoolean, dict->getObject(key));

                if (value != nullptr) {
                    lastIsForceClickEnabled = !value->getValue();
                }
            }
        }

        i->release();
    }

    lastForceClickPropertyUpdateTime = now_abs;
    return lastIsForceClickEnabled;
}

void VoodooI2CNativeEngine::onPropertyChange() {
    if (voodooInputInstance) {
        super::messageClient(kIOMessageVoodooInputUpdatePropertiesNotification, voodooInputInstance);
    }
}

void VoodooI2CNativeEngine::stop(IOService* provider) {
    super::stop(provider);
}

bool VoodooI2CNativeEngine::handleOpen(IOService *forClient, IOOptionBits options, void *arg) {
    if (forClient && forClient->getProperty(VOODOO_INPUT_IDENTIFIER)) {
        voodooInputInstance = forClient;
        voodooInputInstance->retain();
        
        return true;
    }
    return false;
}

bool VoodooI2CNativeEngine::handleIsOpen(const IOService *forClient) const {
    return voodooInputInstance != nullptr && forClient == voodooInputInstance;
}

void VoodooI2CNativeEngine::handleClose(IOService *forClient, IOOptionBits options) {
    if (voodooInputInstance && forClient == voodooInputInstance) {
        OSSafeReleaseNULL(voodooInputInstance);
    }
}

bool VoodooI2CNativeEngine::willTerminate(IOService *provider, IOOptionBits options) {
    return super::willTerminate(provider, options);
}
