//
//  VoodooI2CUPDDGestureSocket.cpp
//  VoodooI2C
//
//  Created by blankmac on 10/31/17.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CUPDDGestureSocket.hpp"

errno_t controlConnect(kern_ctl_ref ctlref, struct sockaddr_ctl* sac, void** unitinfo) {
    lck_mtx_lock(updd_lock);
    
    current_connection = ctlref;
    current_unit = sac->sc_unit;
    
    lck_mtx_unlock(updd_lock);
    
    return KERN_SUCCESS;
}

errno_t controlDisconnect(kern_ctl_ref ctlref, UInt32 unit, void* unitinfo) {
    lck_mtx_lock(updd_lock);
    
    if(unit != current_unit) {
        lck_mtx_unlock(updd_lock);
        return KERN_SUCCESS;
    }
    
    current_connection = NULL;
    current_unit = -1;
    
    lck_mtx_unlock(updd_lock);
    
    return KERN_SUCCESS;
}

kern_return_t destroyGestureSocket() {
    // check if control structure was registered
    if(!control_reference) {
        return KERN_SUCCESS;
    }
    
    errno_t return_status = ctl_deregister(control_reference);
    
    if(return_status != 0) {
        control_reference = NULL;
    } else {
        // disconnect any remaining clients on the socket
        sendQuit();
    }
    
    // release memory
    if(updd_lock) {
        lck_mtx_free(updd_lock, updd_lock_group);
        updd_lock = NULL;
    }
    
    if(updd_lock_group) {
        lck_grp_free(updd_lock_group);
        updd_lock_group = NULL;
    }
    
    if (updd_malloc_tag) {
        OSMalloc_Tagfree(updd_malloc_tag);
        updd_malloc_tag = NULL;
    }
    
    return KERN_SUCCESS;
}

kern_return_t initialiseGestureSocket() {
    if(control_reference) return KERN_FAILURE;

    // allocate memory for the controls and set up lock
    updd_malloc_tag = OSMalloc_Tagalloc(GESTURE_CTL_NAME, OSMT_DEFAULT);
    
    if(!updd_malloc_tag) {
        return KERN_NO_SPACE;
    } else {
        updd_lock_group = lck_grp_alloc_init(GESTURE_CTL_NAME, LCK_GRP_ATTR_NULL);
    }
    
    if(updd_lock_group) {
        updd_lock = lck_mtx_alloc_init(updd_lock_group, LCK_ATTR_NULL);
    } else {
        // free memory for malloc_tag
        OSMalloc_Tagfree(updd_malloc_tag);
        updd_malloc_tag = NULL;
        return KERN_NO_SPACE;
    }
    
    // free allocated memory for malloc_tag and lock_group
    if(!updd_lock) {
        
        lck_grp_free(updd_lock_group);
        updd_lock_group = NULL;
        
        OSMalloc_Tagfree(updd_malloc_tag);
        updd_malloc_tag = NULL;
        
        return KERN_NO_SPACE;
    }
    
    bzero(&control_register, sizeof(struct kern_ctl_reg));
    control_register.ctl_id    = 0;
    control_register.ctl_unit  = 0;
    strncpy( control_register.ctl_name, GESTURE_CTL_NAME, strlen(GESTURE_CTL_NAME));
    control_register.ctl_flags         = CTL_FLAG_PRIVILEGED & CTL_FLAG_REG_ID_UNIT;
    control_register.ctl_connect       = controlConnect;
    control_register.ctl_disconnect    = controlDisconnect;
    
    errno_t return_status = ctl_register(&control_register, &control_reference);
    
    if(return_status != KERN_SUCCESS) {
        return KERN_FAILURE; // Could not register the control structure
    }
    
    return KERN_SUCCESS;
}

bool sendInput(struct updd_data* finger_data) {
    if(!finger_data) return false;
    
    lck_mtx_lock(updd_lock);
    
    // check connection and unit id
    if(!current_connection || current_unit == -1) {
        lck_mtx_unlock(updd_lock);
        return false;
    }
    
    struct gesture_socket_cmd gesture_cmd;
    gesture_cmd.type = GESTURE_DATA;
    memcpy(&gesture_cmd.gesture, finger_data, sizeof(struct updd_data));
    
    errno_t result = ctl_enqueuedata(current_connection, current_unit, &gesture_cmd, sizeof(struct gesture_socket_cmd), 0);
    
    // check for socket buffer overflow
    if (result != KERN_SUCCESS) {
        current_connection = NULL;
        current_unit = -1;
        
        lck_mtx_unlock(updd_lock);
        return false;
    }
    
    lck_mtx_unlock(updd_lock);
    return true;
}

void sendQuit() {
    lck_mtx_lock(updd_lock);
    
    // check connection and unit id
    if(!current_connection || current_unit == -1) {
        lck_mtx_unlock(updd_lock);
        return;
    }
    
    struct gesture_socket_cmd gesture_cmd;
    gesture_cmd.type = GESTURE_QUIT;
    errno_t result = ctl_enqueuedata(current_connection, current_unit, &gesture_cmd, sizeof(struct gesture_socket_cmd), 0);
    
    // check for socket buffer overflow
    if (result != KERN_SUCCESS) {
        current_connection = NULL;
        current_unit = -1;
    }
    
    lck_mtx_unlock(updd_lock);
}

