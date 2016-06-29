// CSGesture Multitouch Touchpad Library
// © 2016, CoolStar. All Rights Reserved.

#ifndef csgesturestruct_h
#define csgesturestruct_h

#define MAX_FINGERS 15

struct csgesture_softc {
    //hardware input
    int x[15];
    int y[15];
    int p[15];
    
    bool buttondown;
    
    //hardware info
    bool infoSetup;
    
    char product_id[16];
    char firmware_version[4];
    
    int resx;
    int resy;
    int phyx;
    int phyy;
    
    //system output
    int dx;
    int dy;
    
    int scrollx;
    int scrolly;
    
    int buttonmask;
    
    //used internally in driver
    int panningActive;
    int idForPanning;
    
    int scrollingActive;
    int idsForScrolling[2];
    int ticksSinceScrolling;
    
    int blacklistedids[15];
    
    bool mouseDownDueToTap;
    int idForMouseDown;
    bool mousedown;
    int mousebutton;
    
    int lastx[15];
    int lasty[15];
    int lastp[15];
    
    int xhistory[15][10];
    int yhistory[15][10];
    
    int flextotalx[15];
    int flextotaly[15];
    
    int totalx[15];
    int totaly[15];
    int totalp[15];
    
    int multitaskingx;
    int multitaskingy;
    int multitaskinggesturetick;
    bool multitaskingdone;
    
    int tick[15];
    int truetick[15];
    int ticksincelastrelease;
    int tickssinceclick;
};

#endif