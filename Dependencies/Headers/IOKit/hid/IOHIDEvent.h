/*
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef _IOKIT_IOHIDEVENT_H
#define _IOKIT_IOHIDEVENT_H

#include <TargetConditionals.h>
#include <libkern/c++/OSObject.h>
#include <libkern/c++/OSArray.h>
#include <IOKit/IOTypes.h>
#include <IOKit/hid/IOHIDEventTypes.h>

#if !TARGET_OS_EMBEDDED
#include <IOKit/hidsystem/IOLLEvent.h>
#endif /*!TARGET_OS_EMBEDDED*/

#define ALIGNED_DATA_SIZE(data_size,align_size) ((((data_size - 1) / align_size) + 1) * align_size)

typedef struct IOHIDEventData IOHIDEventData;

class IOHIDEvent: public OSObject
{
    OSDeclareAbstractStructors( IOHIDEvent )
    
    IOHIDEventData *    _data;
    OSArray *           _children;
    IOHIDEvent *        _parent;
    size_t              _capacity;
    AbsoluteTime        _timeStamp;
    UInt64              _senderID;
    uint64_t            _typeMask;
    IOOptionBits        _options;
    UInt32              _eventCount;

    bool initWithCapacity(IOByteCount capacity);
    bool initWithType(IOHIDEventType type, IOByteCount additionalCapacity=0);
    bool initWithTypeTimeStamp(IOHIDEventType type, AbsoluteTime timeStamp, IOOptionBits options = 0, IOByteCount additionalCapacity=0);
    IOByteCount getLength(UInt32 * eventCount);
    IOByteCount appendBytes(UInt8 * bytes, IOByteCount withLength);
    
    static IOHIDEvent * _axisEvent (    IOHIDEventType          type,
                                        AbsoluteTime            timeStamp,
                                        IOFixed                 x,
                                        IOFixed                 y,
                                        IOFixed                 z,
                                        IOOptionBits            options = 0);

    static IOHIDEvent * _motionEvent (  IOHIDEventType          type,
                                        AbsoluteTime            timeStamp,
                                        IOFixed                 x,
                                        IOFixed                 y,
                                        IOFixed                 z,
                                        uint32_t                motionType = 0,
                                        uint32_t                motionSubType = 0,
                                        UInt32                  sequence = 0,
                                        IOOptionBits            options = 0);

public:
    static IOHIDEvent *     withBytes(  const void *            bytes,
                                        IOByteCount             size);

    static IOHIDEvent *     withType(   IOHIDEventType          type    = kIOHIDEventTypeNULL,
                                        IOOptionBits            options = 0);
                                        
    static IOHIDEvent *     keyboardEvent(  
                                        AbsoluteTime            timeStamp, 
                                        UInt32                  usagePage,
                                        UInt32                  usage,
                                        Boolean                 down,
                                        IOOptionBits            options = 0);
    
    static IOHIDEvent *     keyboardEvent(
                                        AbsoluteTime            timeStamp,
                                        UInt32                  usagePage,
                                        UInt32                  usage,
                                        Boolean                 down,
                                        UInt8                   pressCount,
                                        Boolean                 longPress,
                                        UInt8                   clickSpeed,
                                        IOOptionBits            options = 0);

    
    static IOHIDEvent *     translationEvent (
                                        AbsoluteTime            timeStamp,
                                        IOFixed                 x,
                                        IOFixed                 y,
                                        IOFixed                 z,
                                        IOOptionBits            options = 0);

    static IOHIDEvent *     scrollEventWithFixed (
                                        AbsoluteTime            timeStamp,
                                        IOFixed                 x,
                                        IOFixed                 y,
                                        IOFixed                 z,
                                        IOOptionBits            options = 0);
  
    static IOHIDEvent *     scrollEvent (
                                        AbsoluteTime            timeStamp,
                                        SInt32                  x,
                                        SInt32                  y,
                                        SInt32                  z,
                                        IOOptionBits            options = 0);
 
    static IOHIDEvent *     zoomEvent (
                                        AbsoluteTime            timeStamp,
                                        IOFixed                 x,
                                        IOFixed                 y,
                                        IOFixed                 z,
                                        IOOptionBits            options = 0);
                                       
    static IOHIDEvent *     accelerometerEvent (
                                        AbsoluteTime            timeStamp,
                                        IOFixed                 x,
                                        IOFixed                 y,
                                        IOFixed                 z,
                                        IOHIDMotionType         type = 0,
                                        IOHIDMotionPath         subType = 0,
                                        UInt32                  sequence = 0,
                                        IOOptionBits            options = 0);

    static IOHIDEvent *     gyroEvent ( AbsoluteTime            timeStamp,
                                        IOFixed                 x,
                                        IOFixed                 y,
                                        IOFixed                 z,
                                        IOHIDMotionType         type = 0,
                                        IOHIDMotionPath         subType = 0,
                                        UInt32                  sequence = 0,
                                        IOOptionBits            options = 0);

    static IOHIDEvent *     compassEvent (
                                        AbsoluteTime            timeStamp,
                                        IOFixed                 x,
                                        IOFixed                 y,
                                        IOFixed                 z,
                                        IOHIDMotionType         type = 0,
                                        IOHIDMotionPath         subType = 0,
                                        UInt32                  sequence = 0,
                                        IOOptionBits            options = 0);
    
    static IOHIDEvent *     buttonEvent (
                                        AbsoluteTime            timeStamp,
                                        UInt32                  mask,
                                        UInt8                   number,
                                        bool                    state,
                                        IOOptionBits            options = 0);
                                        
    static IOHIDEvent *     buttonEvent (
                                        AbsoluteTime            timeStamp,
                                        UInt32                  mask,
                                        UInt8                   number,
                                        IOFixed                 pressure,
                                        IOOptionBits            options = 0);    
                                        
    static IOHIDEvent *     ambientLightSensorEvent (
                                        AbsoluteTime            timeStamp,
                                        UInt32                  level,
                                        UInt32                  channel0    = 0,
                                        UInt32                  channel1    = 0,
                                        UInt32                  channel2    = 0,
                                        UInt32                  channel3    = 0,
                                        IOOptionBits            options     = 0);

    static IOHIDEvent *     ambientLightSensorEvent(
                                        AbsoluteTime            timeStamp,
                                        UInt32                  level,
                                        UInt8                   colorSpace,
                                        IOHIDDouble             colorComponent0,
                                        IOHIDDouble             colorComponent1,
                                        IOHIDDouble             colorComponent2,
                                        IOOptionBits            options);
  
    static IOHIDEvent *     proximityEvent (
                                        AbsoluteTime                timeStamp,
                                        IOHIDProximityDetectionMask    mask,
                                        UInt32                        level,
                                        IOOptionBits                options = 0);
    
    static IOHIDEvent *     temperatureEvent (
                                        AbsoluteTime            timeStamp,
                                        IOFixed                 temperature,
                                        IOOptionBits            options = 0);


    static IOHIDEvent *     relativePointerEventWithFixed(
                                        AbsoluteTime            timeStamp,
                                        IOFixed                 x,
                                        IOFixed                 y,
                                        IOFixed                 z,
                                        UInt32                  buttonState,
                                        UInt32                  oldButtonState = 0,
                                        IOOptionBits            options = 0);

    static IOHIDEvent *     relativePointerEvent(
                                        AbsoluteTime            timeStamp,
                                        SInt32                  x,
                                        SInt32                  y,
                                        SInt32                  z,
                                        UInt32                  buttonState,
                                        UInt32                  oldButtonState = 0,
                                        IOOptionBits            options = 0);

    static IOHIDEvent *     absolutePointerEvent(
                                        AbsoluteTime            timeStamp,
                                        IOFixed                 x,
                                        IOFixed                 y,
                                        IOFixed                 z,
                                        UInt32                  buttonState,
                                        UInt32                  oldButtonState = 0,
                                        IOOptionBits            options = 0);
    
    static IOHIDEvent *     multiAxisPointerEvent(
                                        AbsoluteTime            timeStamp,
                                        IOFixed                 x,
                                        IOFixed                 y,
                                        IOFixed                 z,
                                        IOFixed                 rX,
                                        IOFixed                 rY,
                                        IOFixed                 rZ,
                                        UInt32                  buttonState,
                                        UInt32                  oldButtonState = 0,
                                        IOOptionBits            options = 0);

    static IOHIDEvent *     digitizerEvent(
                                        AbsoluteTime                    timeStamp,
                                        UInt32                          transducerID,
                                        IOHIDDigitizerTransducerType    type,
                                        bool                            inRange,
                                        UInt32                          buttonState,
                                        IOFixed                         x,
                                        IOFixed                         y,
                                        IOFixed                         z           = 0,
                                        IOFixed                         tipPressure = 0,
                                        IOFixed                         auxPressure = 0,
                                        IOFixed                         twist       = 0,
                                        IOOptionBits                    options     = 0 );
    
    static IOHIDEvent *     digitizerEventWithTiltOrientation(
                                        AbsoluteTime                    timeStamp,
                                        UInt32                          transducerID,
                                        IOHIDDigitizerTransducerType    type,
                                        bool                            inRange,
                                        UInt32                          buttonState,
                                        IOFixed                         x,
                                        IOFixed                         y,
                                        IOFixed                         z               = 0,
                                        IOFixed                         tipPressure     = 0,
                                        IOFixed                         auxPressure     = 0,
                                        IOFixed                         twist           = 0,
                                        IOFixed                         xTilt           = 0,
                                        IOFixed                         yTilt           = 0,
                                        IOOptionBits                    options         = 0 );

    static IOHIDEvent *     digitizerEventWithPolarOrientation(
                                        AbsoluteTime                    timeStamp,
                                        UInt32                          transducerID,
                                        IOHIDDigitizerTransducerType    type,
                                        bool                            inRange,
                                        UInt32                          buttonState,
                                        IOFixed                         x,
                                        IOFixed                         y,
                                        IOFixed                         z               = 0,
                                        IOFixed                         tipPressure     = 0,
                                        IOFixed                         auxPressure     = 0,
                                        IOFixed                         twist           = 0,
                                        IOFixed                         altitude        = 0,
                                        IOFixed                         azimuth         = 0,
                                        IOOptionBits                    options         = 0 );
    
    static IOHIDEvent *     digitizerEventWithPolarOrientation(
                                        AbsoluteTime                    timeStamp,
                                        UInt32                          transducerID,
                                        IOHIDDigitizerTransducerType    type,
                                        bool                            inRange,
                                        UInt32                          buttonState,
                                        IOFixed                         x,
                                        IOFixed                         y,
                                        IOFixed                         z               = 0,
                                        IOFixed                         tipPressure     = 0,
                                        IOFixed                         auxPressure     = 0,
                                        IOFixed                         twist           = 0,
                                        IOFixed                         altitude        = 0,
                                        IOFixed                         azimuth         = 0,
                                        IOFixed                         quality         = 0,
                                        IOFixed                         density         = 0,
                                        IOOptionBits                    options         = 0 );

    static IOHIDEvent *     digitizerEventWithPolarOrientation(
                                        AbsoluteTime                    timeStamp,
                                        UInt32                          transducerID,
                                        IOHIDDigitizerTransducerType    type,
                                        bool                            inRange,
                                        UInt32                          buttonState,
                                        IOFixed                         x,
                                        IOFixed                         y,
                                        IOFixed                         z               = 0,
                                        IOFixed                         tipPressure     = 0,
                                        IOFixed                         auxPressure     = 0,
                                        IOFixed                         twist           = 0,
                                        IOFixed                         altitude        = 0,
                                        IOFixed                         azimuth         = 0,
                                        IOFixed                         quality         = 0,
                                        IOFixed                         density         = 0,
                                        IOFixed                         majorRadius     = 6<<16,
                                        IOFixed                         minorRadius     = 6<<16,
                                        IOOptionBits                    options         = 0 );

    static IOHIDEvent *     digitizerEventWithQualityOrientation(
                                        AbsoluteTime                    timeStamp,
                                        UInt32                          transducerID,
                                        IOHIDDigitizerTransducerType    type,
                                        bool                            inRange,
                                        UInt32                          buttonState,
                                        IOFixed                         x,
                                        IOFixed                         y,
                                        IOFixed                         z               = 0,
                                        IOFixed                         tipPressure     = 0,
                                        IOFixed                         auxPressure     = 0,
                                        IOFixed                         twist           = 0,
                                        IOFixed                         quality         = 0,
                                        IOFixed                         density         = 0,
                                        IOFixed                         irregularity    = 0,
                                        IOFixed                         majorRadius     = 6<<16,
                                        IOFixed                         minorRadius     = 6<<16,
                                        IOOptionBits                    options         = 0 );

    static IOHIDEvent *     powerEvent(
                                        AbsoluteTime            timeStamp,
                                        int64_t                 measurement,
                                        IOHIDPowerType          powerType,
                                        IOHIDPowerSubType       powerSubType    = 0,
                                        IOOptionBits            options         = 0);

    static IOHIDEvent *     vendorDefinedEvent(
                                        AbsoluteTime            timeStamp,
                                        UInt32                  usagePage,
                                        UInt32                  usage,
                                        UInt32                  version,
                                        UInt8 *                 data,
                                        UInt32                  length,
                                        IOOptionBits            options = 0);

    static IOHIDEvent *     biometricEvent(AbsoluteTime timeStamp, IOFixed level, IOHIDBiometricEventType eventType, IOOptionBits options=0);
    
    static IOHIDEvent *     biometricEvent(AbsoluteTime timeStamp, IOFixed level, IOHIDBiometricEventType eventType, UInt32 usagePage, UInt32 usage, UInt8 tapCount, IOOptionBits options=0);
    
    static IOHIDEvent *     atmosphericPressureEvent(AbsoluteTime timeStamp, IOFixed level, UInt32 sequence=0, IOOptionBits options=0);

    static IOHIDEvent *     unicodeEvent(AbsoluteTime timeStamp, UInt8 * payload, UInt32 length, IOHIDUnicodeEncodingType encoding, IOFixed quality, IOOptionBits options);

    static IOHIDEvent *     standardGameControllerEvent(
                                        AbsoluteTime                    timeStamp,
                                        IOFixed                         dpadUp,
                                        IOFixed                         dpadDown,
                                        IOFixed                         dpadLeft,
                                        IOFixed                         dpadRight,
                                        IOFixed                         faceX,
                                        IOFixed                         faceY,
                                        IOFixed                         faceA,
                                        IOFixed                         faceB,
                                        IOFixed                         shoulderL,
                                        IOFixed                         shoulderR,
                                        IOOptionBits                    options = 0);

    static IOHIDEvent *     extendedGameControllerEvent(
                                        AbsoluteTime                    timeStamp,
                                        IOFixed                         dpadUp,
                                        IOFixed                         dpadDown,
                                        IOFixed                         dpadLeft,
                                        IOFixed                         dpadRight,
                                        IOFixed                         faceX,
                                        IOFixed                         faceY,
                                        IOFixed                         faceA,
                                        IOFixed                         faceB,
                                        IOFixed                         shoulderL1,
                                        IOFixed                         shoulderR1,
                                        IOFixed                         shoulderL2,
                                        IOFixed                         shoulderR2,
                                        IOFixed                         joystickX,
                                        IOFixed                         joystickY,
                                        IOFixed                         joystickZ,
                                        IOFixed                         joystickRz,
                                        IOOptionBits                    options = 0);

    static IOHIDEvent *     humidityEvent(AbsoluteTime timeStamp, IOFixed rh, UInt32 sequence=0, IOOptionBits options=0);

    static IOHIDEvent *     brightnessEvent(AbsoluteTime timeStamp, IOFixed currentBrightness, IOFixed targetBrightness, UInt64 transitionTime, IOOptionBits options = 0);

    virtual void            appendChild(IOHIDEvent *childEvent);

    virtual AbsoluteTime    getTimeStamp();
    virtual void            setTimeStamp(AbsoluteTime timeStamp);
    
    virtual IOHIDEventType  getType();
    virtual void            setType(IOHIDEventType type);
    
    virtual IOHIDEventPhaseBits  getPhase();
    virtual void            setPhase(IOHIDEventPhaseBits phase);
    
    virtual IOHIDEvent *    getEvent(IOHIDEventType type, IOOptionBits options = 0);
    
    virtual SInt32          getIntegerValue(
                                        IOHIDEventField key, IOOptionBits options = 0);
    virtual void            setIntegerValue(    
                                        IOHIDEventField key, SInt32 value, IOOptionBits options = 0);
                                        
    virtual IOFixed         getFixedValue(IOHIDEventField key, IOOptionBits options = 0);
    virtual void            setFixedValue(IOHIDEventField key, IOFixed value, IOOptionBits options = 0);
    
    virtual UInt8 *         getDataValue(IOHIDEventField key, IOOptionBits options = 0);

    virtual void            free();
    
    virtual size_t          getLength(); 
    virtual IOByteCount     readBytes(void *bytes, IOByteCount withLength);
    
    virtual void            setSenderID(uint64_t senderID);
    
    virtual uint64_t        getLatency(uint32_t scaleFactor);

    virtual IOHIDDouble     getDoubleValue( IOHIDEventField  key,  IOOptionBits  options);
    virtual void            setDoubleValue( IOHIDEventField  key, IOHIDDouble value, IOOptionBits  options);
    
    inline  IOOptionBits    getOptions() { return _options; };

};

#endif /* _IOKIT_IOHIDEVENT_H */