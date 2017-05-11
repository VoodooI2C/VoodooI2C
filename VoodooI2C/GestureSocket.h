//
//  GestureSocket.h
//  VoodooI2C
//
//  Copyright © 2017 Kishor Prins. All rights reserved.
//

#ifndef GestureClient_h
#define GestureClient_h

#include <libkern/libkern.h>
#include <sys/kern_control.h>
#include <sys/sysctl.h>

#include "csgesture-softc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 This header and implementation files will allow a user mode client 
 to communicate with the VoodooI2C driver.
 */

#define GESTURE_CTL_NAME "com.alexandred.VoodooI2C.GestureSocket"

enum gesture_socket_cmd_type {
    GESTURE_DATA,
    GESTURE_QUIT
};
    
struct gesture_socket_cmd {
    enum gesture_socket_cmd_type type;
    struct csgesture_softc gesture;
};

kern_return_t initialise_gesture_socket();
kern_return_t destroy_gesture_socket();
void send_quit();
bool send_input(struct csgesture_softc* sc);
    
#ifdef __cplusplus
}
#endif

#endif /* GestureClient_h */
