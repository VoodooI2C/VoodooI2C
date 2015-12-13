//
//  VoodooCyapaGen3Device.cpp
//  VoodooI2C
//
//  Created by CoolStar on 12/13/15.
//  Copyright © 2015 CoolStar. All rights reserved.
//  ported from crostrackpad 3.0 beta 3 for Windows
//

#include "VoodooCyapaGen3Device.h"
#include "VoodooI2C.h"
#include "VoodooHIDWrapper.h"

OSDefineMetaClassAndStructors(VoodooI2CHIDDevice, IOService);

#ifndef ABS32
#define ABS32
inline int32_t abs(int32_t num){
    if (num < 0){
        return num * -1;
    }
    return num;
}
#endif

typedef unsigned char BYTE;

#define MOUSE_BUTTON_1     0x01
#define MOUSE_BUTTON_2     0x02
#define MOUSE_BUTTON_3     0x04

#define KBD_LCONTROL_BIT     1
#define KBD_LGUI_BIT         8

#define KBD_KEY_CODES        6

int VoodooI2CCyapaGen3Device::distancesq(int delta_x, int delta_y){
    return (delta_x * delta_x) + (delta_y*delta_y);
}

void VoodooI2CCyapaGen3Device::ProcessMove(csgesture_softc *sc, int abovethreshold, int iToUse[3]) {
    if (abovethreshold == 1) {
        int i = iToUse[0];
        int delta_x = sc->x[i] - sc->lastx[i];
        int delta_y = sc->y[i] - sc->lasty[i];
        
        if (abs(delta_x) > 75 || abs(delta_y) > 75) {
            delta_x = 0;
            delta_y = 0;
        }
        
        sc->dx = delta_x;
        sc->dy = delta_y;
    }
}

void VoodooI2CCyapaGen3Device::ProcessScroll(csgesture_softc *sc, int abovethreshold, int iToUse[3]) {
    sc->scrollx = 0;
    sc->scrolly = 0;
    //CyapaPrint(DEBUG_LEVEL_INFO, DBG_IOCTL, "DBGPAD Threshold: %d\n", abovethreshold);
    if (abovethreshold == 2) {
        int i1 = iToUse[0];
        int delta_x1 = sc->x[i1] - sc->lastx[i1];
        int delta_y1 = sc->y[i1] - sc->lasty[i1];
        
        int i2 = iToUse[1];
        int delta_x2 = sc->x[i2] - sc->lastx[i2];
        int delta_y2 = sc->y[i2] - sc->lasty[i2];
        
        if ((abs(delta_y1) + abs(delta_y2)) > (abs(delta_x1) + abs(delta_x2))) {
            int avgy = (delta_y1 + delta_y2) / 2;
            sc->scrolly = avgy;
        }
        else {
            int avgx = (delta_x1 + delta_x2) / 2;
            sc->scrollx = avgx;
        }
        //CyapaPrint(DEBUG_LEVEL_INFO,DBG_IOCTL,"DBGPAD Scroll X: %d Y: %d\n", sc->scrollx, sc->scrolly);
        if (abs(sc->scrollx) > 75)
            sc->scrollx = 0;
        if (abs(sc->scrolly) > 75)
            sc->scrolly = 0;
        if (sc->scrolly > 5)
            sc->scrolly = 1;
        else if (sc->scrolly < -5)
            sc->scrolly = -1;
        else
            sc->scrolly = 0;
        
        if (sc->scrollx > 5)
            sc->scrollx = -1;
        else if (sc->scrollx < -5)
            sc->scrollx = 1;
        else
            sc->scrollx = 0;
    }
}

/*void ProcessThreeFingerSwipe(csgesture_softc *sc, int abovethreshold, int iToUse[3]) {
    if (abovethreshold == 3) {
        int i1 = iToUse[0];
        int delta_x1 = sc->x[i1] - sc->lastx[i1];
        int delta_y1 = sc->y[i1] - sc->lasty[i1];
        
        int i2 = iToUse[1];
        int delta_x2 = sc->x[i2] - sc->lastx[i2];
        int delta_y2 = sc->y[i2] - sc->lasty[i2];
        
        int i3 = iToUse[2];
        int delta_x3 = sc->x[i3] - sc->lastx[i3];
        int delta_y3 = sc->y[i3] - sc->lasty[i3];
        
        int avgx = (delta_x1 + delta_x2 + delta_x3) / 3;
        int avgy = (delta_y1 + delta_y2 + delta_y3) / 3;
        
        sc->multitaskingx += avgx;
        sc->multitaskingy += avgy;
        sc->multitaskinggesturetick++;
        
        if (sc->multitaskinggesturetick > 5 && !sc->multitaskingdone) {
            if ((abs(delta_y1) + abs(delta_y2) + abs(delta_y3)) > (abs(delta_x1) + abs(delta_x2) + abs(delta_x3))) {
                if (abs(sc->multitaskingy) > 50) {
                    BYTE shiftKeys = KBD_LGUI_BIT;
                    BYTE keyCodes[KBD_KEY_CODES] = { 0, 0, 0, 0, 0, 0 };
                    if (sc->multitaskingy < 0)
                        keyCodes[0] = 0x2B;
                    else
                        keyCodes[0] = 0x07;
                    update_keyboard(shiftKeys, keyCodes);
                    shiftKeys = 0;
                    keyCodes[0] = 0x0;
                    update_keyboard(shiftKeys, keyCodes);
                    sc->multitaskingx = 0;
                    sc->multitaskingy = 0;
                    sc->multitaskingdone = true;
                }
            }
            else {
                if (abs(sc->multitaskingx) > 50) {
                    BYTE shiftKeys = KBD_LGUI_BIT | KBD_LCONTROL_BIT;
                    BYTE keyCodes[KBD_KEY_CODES] = { 0, 0, 0, 0, 0, 0 };
                    if (sc->multitaskingx > 0)
                        keyCodes[0] = 0x50;
                    else
                        keyCodes[0] = 0x4F;
                    update_keyboard(shiftKeys, keyCodes);
                    shiftKeys = 0;
                    keyCodes[0] = 0x0;
                    update_keyboard(shiftKeys, keyCodes);
                    sc->multitaskingx = 0;
                    sc->multitaskingy = 0;
                    sc->multitaskingdone = true;
                }
            }
        }
        else if (sc->multitaskinggesturetick > 25) {
            sc->multitaskingx = 0;
            sc->multitaskingy = 0;
            sc->multitaskinggesturetick = 0;
            sc->multitaskingdone = false;
        }
    }
    else {
        sc->multitaskingx = 0;
        sc->multitaskingy = 0;
        sc->multitaskinggesturetick = 0;
        sc->multitaskingdone = false;
    }
}*/

void VoodooI2CCyapaGen3Device::TapToClick(csgesture_softc *sc, int button) {
    sc->tickssinceclick++;
    if (sc->mousedown) {
        sc->tickssinceclick = 0;
        return;
    }
    if (button == 0)
        return;
    int buttonmask = 0;
    
    switch (button) {
        case 1:
            buttonmask = MOUSE_BUTTON_1;
            break;
        case 2:
            buttonmask = MOUSE_BUTTON_2;
            break;
        case 3:
            buttonmask = MOUSE_BUTTON_3;
            break;
    }
    if (buttonmask != 0 && sc->tickssinceclick > 10) {
        update_relative_mouse(buttonmask, 0, 0, 0, 0);
        update_relative_mouse(0, 0, 0, 0, 0);
        sc->tickssinceclick = 0;
    }
}

void VoodooI2CCyapaGen3Device::ProcessGesture(csgesture_softc *sc) {
#pragma mark reset inputs
    sc->dx = 0;
    sc->dy = 0;
    
#pragma mark process touch thresholds
    int avgx[15];
    int avgy[15];
    
    int abovethreshold = 0;
    int recentlyadded = 0;
    int iToUse[3] = { 0,0,0 };
    int a = 0;
    
    int nfingers = 0;
    for (int i = 0;i < 15;i++) {
        if (sc->x[i] != -1)
            nfingers++;
    }
    
    for (int i = 0;i < 15;i++) {
        if (sc->truetick[i] < 30 && sc->truetick[i] != 0)
            recentlyadded++;
        if (sc->tick[i] == 0)
            continue;
        avgx[i] = sc->totalx[i] / sc->tick[i];
        avgy[i] = sc->totaly[i] / sc->tick[i];
        if (distancesq(avgx[i], avgy[i]) > 2) {
            abovethreshold++;
            iToUse[a] = i;
            a++;
        }
        /*else if (nfingers == 1 && sc->x[i] != -1 && sc->truetick[i] > 50) {
         abovethreshold = 1;
         iToUse[a] = i;
         a++;
         }*/
    }
    
#pragma mark process different gestures
    ProcessMove(sc, abovethreshold, iToUse);
    ProcessScroll(sc, abovethreshold, iToUse);
    //ProcessThreeFingerSwipe(sc, abovethreshold, iToUse);
    
#pragma mark process clickpad press state
    int buttonmask = 0;
    
    sc->mousebutton = recentlyadded;
    if (sc->mousebutton == 0)
        sc->mousebutton = abovethreshold;
    
    if (sc->mousebutton == 0) {
        sc->mousebutton = nfingers;
        if (sc->mousebutton == 0)
            sc->mousebutton = 1;
    }
    if (sc->mousebutton > 3)
        sc->mousebutton = 3;
    
    if (sc->mouseDownDueToTap) {
        sc->mousedown = true;
        sc->mousebutton = 1;
        buttonmask = MOUSE_BUTTON_1;
        sc->buttonmask = buttonmask;
    } else {
        if (sc->buttondown && !sc->mousedown) {
            sc->mousedown = true;
            sc->tickssinceclick = 0;
            
            switch (sc->mousebutton) {
                case 1:
                    buttonmask = MOUSE_BUTTON_1;
                    break;
                case 2:
                    buttonmask = MOUSE_BUTTON_2;
                    break;
                case 3:
                    buttonmask = MOUSE_BUTTON_3;
                    break;
            }
            sc->buttonmask = buttonmask;
        }
        else if (sc->mousedown && !sc->buttondown) {
            sc->mousedown = false;
            sc->mousebutton = 0;
            sc->buttonmask = 0;
        }
    }
    
#pragma mark shift to last
    int releasedfingers = 0;
    
    for (int i = 0;i < 15;i++) {
        if (sc->x[i] != -1) {
            /*if (sc->ticksincelastrelease < 25 && !sc->mouseDownDueToTap) {
             sc->mouseDownDueToTap = true;
             sc->idForMouseDown = i;
             }*/
            sc->truetick[i]++;
            if (sc->tick[i] < 10) {
                if (sc->lastx[i] != -1) {
                    sc->totalx[i] += abs(sc->x[i] - sc->lastx[i]);
                    sc->totaly[i] += abs(sc->y[i] - sc->lasty[i]);
                    sc->totalp[i] += sc->p[i];
                    
                    sc->flextotalx[i] = sc->totalx[i];
                    sc->flextotaly[i] = sc->flextotaly[i];
                    
                    int j = sc->tick[i];
                    sc->xhistory[i][j] = abs(sc->x[i] - sc->lastx[i]);
                    sc->yhistory[i][j] = abs(sc->y[i] - sc->lasty[i]);
                }
                sc->tick[i]++;
            }
            else if (sc->lastx[i] != -1) {
                int absx = abs(sc->x[i] - sc->lastx[i]);
                int absy = abs(sc->y[i] - sc->lasty[i]);
                
                int newtotalx = sc->flextotalx[i] - sc->xhistory[i][0] + absx;
                int newtotaly = sc->flextotaly[i] - sc->yhistory[i][0] + absy;
                
                bool oldsatisfies = distancesq(avgx[i], avgy[i]) > 2;
                bool newsatisfies = distancesq(newtotalx / 10, newtotaly / 10) > 2;
                
                bool isvalid = true;
                if (!oldsatisfies)
                    isvalid = true;
                if (oldsatisfies && !newsatisfies)
                    isvalid = false;
                
                if (isvalid) { //don't allow a threshold to drop. Only allow increasing.
                    sc->flextotalx[i] -= sc->xhistory[i][0];
                    sc->flextotaly[i] -= sc->yhistory[i][0];
                    for (int j = 1;j < 10;j++) {
                        sc->xhistory[i][j - 1] = sc->xhistory[i][j];
                        sc->yhistory[i][j - 1] = sc->yhistory[i][j];
                    }
                    sc->flextotalx[i] += abs(sc->x[i] - sc->lastx[i]);
                    sc->flextotaly[i] += abs(sc->y[i] - sc->lasty[i]);
                    
                    int j = 9;
                    sc->xhistory[i][j] = abs(sc->x[i] - sc->lastx[i]);
                    sc->yhistory[i][j] = abs(sc->y[i] - sc->lasty[i]);
                }
            }
        }
        if (sc->x[i] == -1) {
            if (i == sc->idForMouseDown) {
                sc->mouseDownDueToTap = false;
                sc->idForMouseDown = -1;
            }
            if (sc->lastx[i] != -1)
                sc->ticksincelastrelease = -1;
            for (int j = 0;j < 10;j++) {
                sc->xhistory[i][j] = 0;
                sc->yhistory[i][j] = 0;
            }
            if (sc->tick[i] < 10 && sc->tick[i] != 0) {
                int avgp = sc->totalp[i] / sc->tick[i];
                if (avgp > 7)
                    releasedfingers++;
            }
            sc->totalx[i] = 0;
            sc->totaly[i] = 0;
            sc->totalp[i] = 0;
            sc->tick[i] = 0;
            sc->truetick[i] = 0;
        }
        sc->lastx[i] = sc->x[i];
        sc->lasty[i] = sc->y[i];
        sc->lastp[i] = sc->p[i];
    }
    sc->ticksincelastrelease++;
    
#pragma mark process tap to click
    TapToClick(sc, releasedfingers);
    
#pragma mark send to system
    update_relative_mouse(sc->buttonmask, sc->dx, sc->dy, sc->scrolly, sc->scrollx);
}

void VoodooI2CCyapaGen3Device::TrackpadRawInput(struct csgesture_softc *sc, struct cyapa_regs *regs, int tickinc){
    int nfingers;
    int afingers;	/* actual fingers after culling */
    int i;
    
    if ((regs->stat & CYAPA_STAT_RUNNING) == 0) {
        regs->fngr = 0;
    }
    
    nfingers = CYAPA_FNGR_NUMFINGERS(regs->fngr);
    
    for (int i = 0;i < 15;i++) {
        sc->x[i] = -1;
        sc->y[i] = -1;
        sc->p[i] = -1;
    }
    for (int i = 0;i < nfingers;i++) {
        int a = regs->touch[i].id;
        int x = CYAPA_TOUCH_X(regs, i);
        int y = CYAPA_TOUCH_Y(regs, i);
        int p = CYAPA_TOUCH_P(regs, i);
        sc->x[a] = x;
        sc->y[a] = y;
        sc->p[a] = p;
    }
    
    sc->buttondown = (regs->fngr & CYAPA_FNGR_LEFT);
    
    ProcessGesture(sc);
}

bool VoodooI2CCyapaGen3Device::attach(IOService * provider, IOService* child)
{
    if (!super::attach(provider))
        return false;
    
    assert(_controller == 0);
    _controller = (VoodooI2C*)provider;
    _controller->retain();
    
    
    child->attach(this);
    if (!probe(child))
        return false;
    
    return true;
}

bool VoodooI2CCyapaGen3Device::probe(IOService* device) {
    
    
    hid_device = (I2CDevice *)IOMalloc(sizeof(I2CDevice));
    
    //hid_device->_dev = _controller->_dev;
    
    if (!super::start(device))
        return false;
    
    
    hid_device->provider = OSDynamicCast(IOACPIPlatformDevice, device);
    hid_device->provider->retain();
    
    int ret = i2c_get_slave_address(hid_device);
    if (ret < 0){
        IOLog("%s::%s::Failed to get a slave address for an I2C device, aborting.\n", getName(), _controller->_dev->name);
        IOFree(hid_device, sizeof(I2CDevice));
        return false;
    }
    
    
    IOLog("%s::%s::HID Probe called for i2c 0x%02x\n", getName(), _controller->_dev->name, hid_device->addr);
    
    initHIDDevice(hid_device);
    
    //super::stop(device);
    return 0;
}

void VoodooI2CHIDDevice::stop(IOService* device) {
    
    IOLog("I2C HID Device is stopping\n");
    
    destroy_wrapper();
    
    if (hid_device->timerSource){
        hid_device->timerSource->cancelTimeout();
        hid_device->timerSource->release();
        hid_device->timerSource = NULL;
    }
    
    //hid_device->workLoop->removeEventSource(hid_device->interruptSource);
    //hid_device->interruptSource->disable();
    hid_device->interruptSource = NULL;
    
    hid_device->workLoop->release();
    hid_device->workLoop = NULL;
    
    
    
    
    i2c_hid_free_buffers(ihid, HID_MIN_BUFFER_SIZE);
    IOFree(ihid, sizeof(i2c_hid));
    
    IOFree(hid_device, sizeof(I2CDevice));
    
    //hid_device->provider->close(this);
    
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void VoodooI2CHIDDevice::detach( IOService * provider )
{
    assert(_controller == provider);
    _controller->release();
    _controller = 0;
    
    super::detach(provider);
}

int VoodooI2CHIDDevice::initHIDDevice(I2CDevice *hid_device) {
    int ret;
    UInt16 hidRegister;
    
    ihid = (i2c_hid*)IOMalloc(sizeof(i2c_hid));
    
    ihid->client = hid_device;
    
    ret = i2c_hid_acpi_pdata(ihid);
    
    ihid->client = hid_device;
    
    
    hidRegister = ihid->pdata.hid_descriptor_address;
    
    ihid->wHIDDescRegister = (__le16)hidRegister;
    
    ret = i2c_hid_alloc_buffers(ihid, HID_MIN_BUFFER_SIZE);
    if (ret < 0)
        goto err;
    
    //ret = i2c_hid_set_power(ihid, I2C_HID_PWR_ON);
    //if(ret<0)
    //   goto err;
    
    
    ret = i2c_hid_fetch_hid_descriptor(ihid);
    if (ret < 0)
        goto err;
    
    
    hid_device->workLoop = (IOWorkLoop*)getWorkLoop();
    if(!hid_device->workLoop) {
        IOLog("%s::%s::Failed to get workloop\n", getName(), _controller->_dev->name);
        stop(this);
        return -1;
    }
    
    hid_device->workLoop->retain();
    
    /*
     hid_device->interruptSource = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2CHIDDevice::InterruptOccured), hid_device->provider);
     
     if (hid_device->workLoop->addEventSource(hid_device->interruptSource) != kIOReturnSuccess) {
     IOLog("%s::%s::Could not add interrupt source to workloop\n", getName(), _controller->_dev->name);
     stop(this);
     return -1;
     }
     
     hid_device->interruptSource->enable();
     */
    
    hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CHIDDevice::i2c_hid_get_input));
    if (!hid_device->timerSource){
        goto err;
    }
    
    hid_device->workLoop->addEventSource(hid_device->timerSource);
    hid_device->timerSource->setTimeoutMS(10);
    /*
     
     hid_device->commandGate = IOCommandGate::commandGate(this);
     
     if (!hid_device->commandGate || (_dev->workLoop->addEventSource(hid_device->commandGate) != kIOReturnSuccess)) {
     IOLog("%s::%s::Failed to open HID command gate\n", getName(), _dev->name);
     return -1;
     }
     */
    
    i2c_hid_get_report_descriptor(ihid);
    
    initialize_wrapper();
    registerService();
    
    return 0;
    
err:
    i2c_hid_free_buffers(ihid, HID_MIN_BUFFER_SIZE);
    IOFree(ihid, sizeof(i2c_hid));
    return ret;
}

void VoodooI2CHIDDevice::initialize_wrapper(void) {
    destroy_wrapper();
    
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    _wrapper = new VoodooHIDWrapper;
    if (_wrapper->init()) {
        IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
        _wrapper->attach(this);
        _wrapper->start(this);
    }
    else {
        IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
        _wrapper->release();
        _wrapper = NULL;
    }
}

void VoodooI2CHIDDevice::destroy_wrapper(void) {
    if (_wrapper != NULL) {
        _wrapper->terminate(kIOServiceRequired | kIOServiceSynchronous);
        _wrapper->release();
        _wrapper = NULL;
    }
}

int VoodooI2CHIDDevice::i2c_hid_acpi_pdata(i2c_hid *ihid) {
    
    UInt32 guid_1 = 0x3CDFF6F7;
    UInt32 guid_2 = 0x45554267;
    UInt32 guid_3 = 0x0AB305AD;
    UInt32 guid_4 = 0xDE38893D;
    
    
    OSObject *result = NULL;
    OSObject *params[3];
    char buffer[16];
    
    memcpy(buffer, &guid_1, 4);
    memcpy(buffer + 4, &guid_2, 4);
    memcpy(buffer + 8, &guid_3, 4);
    memcpy(buffer + 12, &guid_4, 4);
    
    
    params[0] = OSData::withBytes(buffer, 16);
    params[1] = OSNumber::withNumber(0x1, 8);
    params[2] = OSNumber::withNumber(0x1, 8);
    
    ihid->client->provider->evaluateObject("_DSM", &result, params, 3);
    
    OSNumber* number = OSDynamicCast(OSNumber, result);
    
    ihid->pdata.hid_descriptor_address = number->unsigned32BitValue();
    
    number->release();
    params[0]->release();
    params[1]->release();
    params[2]->release();
    
    return 0;
}

int VoodooI2CHIDDevice::i2c_get_slave_address(I2CDevice* hid_device){
    OSObject* result = NULL;
    
    hid_device->provider->evaluateObject("_CRS", &result);
    
    OSData* data = OSDynamicCast(OSData, result);
    
    hid_device->addr = *(int*)data->getBytesNoCopy(16,1) & 0xFF;
    
    data->release();
    
    return 0;
    
}

int VoodooI2CHIDDevice::i2c_hid_alloc_buffers(i2c_hid *ihid, UInt report_size) {
    int args_len = sizeof(UInt8) + sizeof(UInt16) + sizeof(UInt16) + report_size;
    
    ihid->inbuf = (char *)IOMalloc(report_size);
    ihid->argsbuf = (char *)IOMalloc(report_size);
    ihid->cmdbuf = (char *)IOMalloc(sizeof(union command) + args_len);
    
    if(!ihid->inbuf || !ihid->argsbuf || !ihid->cmdbuf) {
        i2c_hid_free_buffers(ihid, report_size);
        return -1;
    }
    
    ihid->bufsize = report_size;
    
    return 0;
}

void VoodooI2CHIDDevice::i2c_hid_free_buffers(i2c_hid *ihid, UInt report_size) {
    IOFree(ihid->inbuf, report_size);
    IOFree(ihid->argsbuf, report_size);
    IOFree(ihid->cmdbuf, sizeof(UInt8) + sizeof(UInt16) + sizeof(UInt16) + report_size);
    ihid->inbuf = NULL;
    ihid->cmdbuf = NULL;
    ihid->argsbuf = NULL;
    ihid->bufsize = 0;
}

int VoodooI2CHIDDevice::i2c_hid_fetch_hid_descriptor(i2c_hid *ihid) {
    struct i2c_hid_desc *hdesc = &ihid->hdesc;
    UInt dsize;
    int ret;
    
    ret = i2c_hid_command(ihid, &hid_descr_cmd, ihid->hdesc_buffer, sizeof(struct i2c_hid_desc));
    
    if (ret)
    {
        IOLog("%s::%s::hid_descr_cmd failed\n", getName(), hid_device->name);
        return -1;
    }
    
    if((UInt16)(hdesc->bcdVersion) != 0x0100) {
        IOLog("%s::%s::Unexpected HID descriptor bcdVersion %x\n", getName(), hid_device->name, (UInt16)(hdesc->bcdVersion));
        return -1;
    }
    
    //dsize = (UInt16)(hdesc->wHIDDescLength);
    
    //if (dsize != sizeof(struct i2c_hid_desc)) {
    //    IOLog("%s::%s::weird size of HID descriptor\n", getName(), _dev->name);
    //    return -1;
    //}
    
    return 0;
}

int VoodooI2CHIDDevice::i2c_hid_command(i2c_hid *ihid, struct i2c_hid_cmd *command, unsigned char *buf_recv, int data_len) {
    return __i2c_hid_command(ihid, command, 0, 0, NULL, 0, buf_recv, data_len);
}

int VoodooI2CHIDDevice::__i2c_hid_command(i2c_hid *ihid, struct i2c_hid_cmd *command, UInt8 reportID, UInt8 reportType, UInt8 *args, int args_len, unsigned char *buf_recv, int data_len) {
    union command *cmd = (union command *)ihid->cmdbuf;
    int ret;
    struct i2c_msg msg[2];
    int msg_num = 1;
    
    int length = command->length;
    bool wait = command->wait;
    UInt registerIndex = command->registerIndex;
    
    if (command == &hid_descr_cmd) {
        cmd->c.reg = ihid->wHIDDescRegister;
    } else {
        cmd->data[0] = ihid->hdesc_buffer[registerIndex];
        cmd->data[1] = ihid->hdesc_buffer[registerIndex + 1];
    }
    
    if (length > 2) {
        cmd->c.opcode = command->opcode;
        cmd->c.reportTypeID = reportID | reportType << 4;
    }
    
    memcpy(cmd->data + length, args, args_len);
    length += args_len;
    
    
    msg[0].addr = ihid->client->addr;
    msg[0].flags = 0; //ihid->client->flags & I2C_M_TEN;
    msg[0].len = length;
    msg[0].buf = cmd->data;
    
    if (data_len > 0) {
        msg[1].addr = ihid->client->addr;
        msg[1].flags = I2C_M_RD;
        msg[1].len = data_len;
        msg[1].buf = buf_recv;
        msg_num = 2;
        hid_device->reading = true;
    }
    
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)msg, msg_num);
    
    if (data_len > 0)
        hid_device->reading = false;
    
    if (ret != msg_num)
        return ret < 0 ? ret : -1;
    
    ret = 0;
    
    return ret;
}

int VoodooI2CHIDDevice::i2c_hid_set_power(i2c_hid *ihid, int power_state) {
    int ret;
    
    ret = __i2c_hid_command(ihid, &hid_set_power_cmd, power_state, NULL, 0, NULL, 0, NULL);
    if (ret)
        IOLog("failed to change power settings \n");
    
    return ret;
}


void VoodooI2CHIDDevice::InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    IOLog("interrupt\n");
    if (hid_device->reading)
        return;
    
    //i2c_hid_get_input(ihid);
    
}

void VoodooI2CHIDDevice::i2c_hid_get_input(OSObject* owner, IOTimerEventSource* sender) {
    //    IOLog("getting input\n");
    UInt rsize;
    int ret;
    
    rsize = UInt16(ihid->hdesc.wMaxInputLength);
    
    unsigned char* rdesc = (unsigned char *)IOMalloc(rsize);
    
    ret = i2c_hid_command(ihid, &hid_input_cmd, rdesc, rsize);
    
    //    IOLog("===Input (%d)===\n", rsize);
    //    for (int i = 0; i < rsize; i++)
    //        IOLog("0x%02x ", (UInt8) rdesc[i]);
    //    IOLog("\n");
    
    int return_size = rdesc[0] | rdesc[1] << 8;
    if (return_size == 0) {
        /* host or device initiated RESET completed */
        // test/clear bit?
        hid_device->timerSource->setTimeoutMS(10);
        return;
    }
    
    if (return_size > rsize) {
        IOLog("%s: Incomplete report %d/%d\n", __func__, rsize, return_size);
    }
    
    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, return_size);
    buffer->writeBytes(0, rdesc + 2, return_size - 2);
    
    IOReturn err = _wrapper->handleReport(buffer, kIOHIDReportTypeInput);
    if (err != kIOReturnSuccess)
        IOLog("Error handling report: 0x%.8x\n", err);
    
    buffer->release();
    
    IOFree(rdesc, rsize);
    
    hid_device->timerSource->setTimeoutMS(10);
}

bool VoodooI2CHIDDevice::i2c_hid_get_report_descriptor(i2c_hid *ihid){
    UInt rsize;
    int ret;
    
    IOLog("reg: 0x%x\n",ihid->hdesc.wReportDescRegister);
    
    rsize = UInt16(ihid->hdesc.wReportDescLength);
    
    unsigned char* rdesc = (unsigned char *)IOMalloc(rsize);
    
    i2c_hid_hwreset(ihid);
    
    if (!rdesc){
        return -1;
    }
    /*
     if (!rsize || rsize > HID_MAX_DESCRIPTOR_SIZE){
     IOLog("%s::%s::Weird size of report descriptor (%u)\n", getName(), hid_device->name, rsize);ƒ
     return 1;
     }
     */
    
    ret = i2c_hid_command(ihid, &hid_report_desc_cmd, rdesc, rsize);
    
    if (!ret){
        IOLog("it worked!\n");
    }
    
    
    
    //_controller->registerDump(_controller->_dev);
    
    /*
     if (ret) {
     IOLog("%s::%s::Reading report descriptor failed", getName(), hid_device->name);
     return -1;
     }
     
     
     IOLog("%s::%s::Report descriptor: %s\n", getName(), hid_device->name, rdesc);
     
     */
    
    IOLog("===Report Descriptor===\n");
    for (int i = 0; i < UInt16(ihid->hdesc.wReportDescLength); i++)
        IOLog("0x%02x\n", (UInt8) rdesc[i]);
    IOLog("===Report Descriptor===\n");
    
    IOFree(rdesc, rsize);
    
    return 0;
    
};

void VoodooI2CHIDDevice::write_report_descriptor_to_buffer(IOBufferMemoryDescriptor *buffer){
    UInt rsize;
    int ret;
    
    IOLog("Report descriptor register: 0x%x\n",ihid->hdesc.wReportDescRegister);
    
    rsize = UInt16(ihid->hdesc.wReportDescLength);
    
    unsigned char* rdesc = (unsigned char *)IOMalloc(rsize);
    
    i2c_hid_hwreset(ihid);
    
    ret = i2c_hid_command(ihid, &hid_report_desc_cmd, rdesc, rsize);
    
    if (!ret)
        IOLog("Report descriptor was fetched\n");
    
    buffer->writeBytes(0, rdesc, rsize);
    IOLog("Report Descriptor written to buffer (%d)\n", rsize);
    
    IOFree(rdesc, rsize);
}

bool VoodooI2CHIDDevice::i2c_hid_hwreset(i2c_hid *ihid) {
    int ret;
    
    ret = i2c_hid_set_power(ihid, I2C_HID_PWR_ON);
    
    if (ret)
        return ret;
    
    ret = i2c_hid_command(ihid, &hid_reset_cmd, NULL, 0);
    if (ret) {
        i2c_hid_set_power(ihid, I2C_HID_PWR_SLEEP);
        return ret;
    }
    
    return 0;
};
