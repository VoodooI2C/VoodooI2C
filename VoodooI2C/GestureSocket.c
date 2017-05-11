//
//  GestureSocket.c
//  VoodooI2C
//
//  Copyright © 2017 Kishor Prins. All rights reserved.
//

#include <libkern/OSMalloc.h>
#include "GestureSocket.h"

// handler for incoming connections from user space clients
static errno_t ctl_connect(kern_ctl_ref ctl_ref, struct sockaddr_ctl* sac, void** unitinfo);
// handler for disconnecting user space clients
static errno_t ctl_disconnect(kern_ctl_ref ctl_ref, u_int32_t unit, void* unitinfo);

// control structure needed
static struct kern_ctl_reg ctl_reg;

static kern_ctl_ref ctl_ref = NULL; // reference to control structure
static OSMallocTag malloc_tag = NULL; // malloc tag
static lck_grp_t* lock_group = NULL; // lock group
static lck_mtx_t* lock = NULL; // concruency management

static kern_ctl_ref current_connection = NULL; // refernce to the currently connected client
static u_int32_t current_unit = -1; // unit number for the currently connected client

errno_t ctl_connect(kern_ctl_ref ctlref, struct sockaddr_ctl* sac, void** unitinfo) {
    lck_mtx_lock(lock);
    
    current_connection = ctlref;
    current_unit = sac->sc_unit;
    
    lck_mtx_unlock(lock);
    
    return KERN_SUCCESS;
}

errno_t ctl_disconnect(kern_ctl_ref ctlref, u_int32_t unit, void* unitinfo) {
    lck_mtx_lock(lock);
    
    if(unit != current_unit) {
        // invalid unit number (different client disconnecting?)
        lck_mtx_unlock(lock);
        return KERN_SUCCESS;
    }
    
    current_connection = NULL;
    current_unit = -1;
    
    lck_mtx_unlock(lock);
    
    return KERN_SUCCESS;
}

errno_t ctl_setopt(kern_ctl_ref ctlref, unsigned int unit, void *userdata, int opt, void *data, size_t len) {
    return KERN_SUCCESS;
}

kern_return_t initialise_gesture_socket() {
    if(ctl_ref != NULL) return KERN_FAILURE;
    
    // allocate memory for the locks
    malloc_tag = OSMalloc_Tagalloc(GESTURE_CTL_NAME, OSMT_DEFAULT);
    
    // sanity check
    if(malloc_tag == NULL) {
        return KERN_NO_SPACE;
    } else {
         lock_group = lck_grp_alloc_init(GESTURE_CTL_NAME, LCK_GRP_ATTR_NULL);
    }
    
    if(lock_group != NULL) {
        lock = lck_mtx_alloc_init(lock_group, LCK_ATTR_NULL);
    } else {
        // free memory for malloc_tag
        OSMalloc_Tagfree(malloc_tag);
        malloc_tag = NULL;
        return KERN_NO_SPACE;
    }
    
    // free allocated memory for malloc_tag and lock_group
    if(lock == NULL) {
        lck_grp_free(lock_group);
        lock_group = NULL;
        
        OSMalloc_Tagfree(malloc_tag);
        malloc_tag = NULL;
        
        return KERN_NO_SPACE;
    }
    
    bzero(&ctl_reg, sizeof(struct kern_ctl_reg));
    ctl_reg.ctl_id    = 0;
    ctl_reg.ctl_unit  = 0;
    strncpy( ctl_reg.ctl_name, GESTURE_CTL_NAME, strlen(GESTURE_CTL_NAME));
    ctl_reg.ctl_flags         = CTL_FLAG_PRIVILEGED & CTL_FLAG_REG_ID_UNIT;
    ctl_reg.ctl_send          = 0;
    ctl_reg.ctl_getopt        = 0;
    ctl_reg.ctl_setopt        = ctl_setopt;
    ctl_reg.ctl_connect       = ctl_connect;
    ctl_reg.ctl_disconnect    = ctl_disconnect;
    
    errno_t return_status = ctl_register(&ctl_reg, &ctl_ref);

    if(return_status != KERN_SUCCESS) {
        // failed to register the control structure
        return KERN_FAILURE;
    }

    return KERN_SUCCESS;
}

kern_return_t destroy_gesture_socket() {
    // never registered the control structure
    if(ctl_ref == NULL) {
        return KERN_SUCCESS;
    }
    
    errno_t return_status = ctl_deregister(ctl_ref);
    
    if(return_status != 0) {
        ctl_ref = NULL;
    } else {
        // clients are still connected on the socket
        // if we do not allow clients to close connection properly then KP will occur
        return KERN_FAILURE;
    }
    
    // memory clean up
    if(lock) {
        lck_mtx_free(lock, lock_group);
        lock = NULL;
    }
    
    if(lock_group) {
        lck_grp_free(lock_group);
        lock_group = NULL;
    }
    
    if (malloc_tag) {
        OSMalloc_Tagfree(malloc_tag);
        malloc_tag = NULL;
    }
    
    return KERN_SUCCESS;
}

void send_input(struct csgesture_softc* sc) {
    lck_mtx_lock(lock);
    
    // we do not have a current connection to send data to
    // OR
    // we have a invalid unit id (corrupted somehow?)
    if(current_connection == NULL || current_unit == -1) {
        lck_mtx_unlock(lock);
        return;
    }
    
    errno_t result = ctl_enqueuedata(current_connection, current_unit, sc, sizeof(struct csgesture_softc), 0);
    
    // ran out of socket buffer space?
    if (result != KERN_SUCCESS) {
        current_connection = NULL;
        current_unit = -1;
    }
    
    lck_mtx_unlock(lock);
    return;
}
