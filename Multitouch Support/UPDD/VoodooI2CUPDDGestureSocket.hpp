//
//  VoodooI2CUPDDGestureSocket.hpp
//  VoodooI2C
//
//  Created by blankmac on 10/31/17.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//
//  Socket code based on kAuthOrama in Apple's Xcode samples and
//  total-finder located here -https://github.com/binaryage/totalfinder-kext/blob/master/echelon.c
//

#ifndef VoodooI2CUPDDGestureSocket_hpp
#define VoodooI2CUPDDGestureSocket_hpp

#include <libkern/libkern.h>
#include <sys/kern_control.h>
#include <sys/sysctl.h>
#include <IOKit/IOLib.h>
#include <libkern/OSMalloc.h>
#include "VoodooI2CUPDDData.h"

#define GESTURE_CTL_NAME "com.alexandred.VoodooI2C.GestureSocket"
    
    enum gesture_socket_cmd_type {
        GESTURE_DATA,
        GESTURE_QUIT
    };
    
    struct gesture_socket_cmd {
        enum gesture_socket_cmd_type type;
        struct updd_data gesture;
    };

 /* Socket control structure variables
  */
static struct kern_ctl_reg control_register;
static kern_ctl_ref control_reference = NULL; // reference to control structure
static OSMallocTag updd_malloc_tag = NULL; // malloc tag
static lck_grp_t* updd_lock_group = NULL; // lock group
static lck_mtx_t* updd_lock = NULL; // concruency management
static kern_ctl_ref current_connection = NULL; // refernce to the currently connected client
static UInt32 current_unit = -1; // unit number for the currently connected client

 /*Socket functions
  */
kern_return_t destroyGestureSocket();
kern_return_t initialiseGestureSocket();
void sendQuit();
bool sendInput(struct updd_data* finger_data);

#endif /* VoodooI2CUPDDGestureSocket_hpp */
