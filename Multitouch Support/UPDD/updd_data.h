// UPDD Interface Data
// © 2017, blankmac. All Rights Reserved.

#ifndef updd_data_h
#define updd_data_h


struct updd_data {
    //  finger data
    int current_x[5];
    int current_y[5];
    
    //  device data
    int logical_x;
    int logical_y;
    
    //  tells updd client to send 'touching false' packet to release the current gesture
    bool finger_lift;
    
};

#endif /* updd_data */
