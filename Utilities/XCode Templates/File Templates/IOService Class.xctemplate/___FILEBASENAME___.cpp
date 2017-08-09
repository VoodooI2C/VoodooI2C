//
//  ___FILENAME___
//  ___PROJECTNAME___
//
//  Created by ___FULLUSERNAME___ on ___DATE___.
//___COPYRIGHT___
//

#include "___FILEBASENAME___.hpp"

#define super ___VARIABLE_superClass___
OSDefineMetaClassAndStructors(___FILEBASENAMEASIDENTIFIER___, ___VARIABLE_superClass___);

/**
 Attaches an IOService object to `this`
 
 @param provider IOService* representing the provider
 
 @return returns true on succesful attach, else returns false
 */

bool ___FILEBASENAMEASIDENTIFIER___::attach(IOService* provider) {
    if (!super::attach(provider))
        return false;

    return true;
}

/**
 Deattaches an IOService object from `this`
 
 @param provider IOService* representing the provider
 */

void ___FILEBASENAMEASIDENTIFIER___::detach(IOService* provider) {
    super::detach(provider);
}

/**
 Initialises class

 @param properties OSDictionary* representing the matched personality

 @return returns true on successful initialisation, else returns false
 */

bool ___FILEBASENAMEASIDENTIFIER___::init(OSDictionary* properties) {
    if (!super::init(properties))
        return false;

    return true;
}

/**
 Frees class - releases objects instantiated in `init`
 */

void ___FILEBASENAMEASIDENTIFIER___::free() {
    super::free();
}

/**
 Starts the class
 
 @param provider IOService* representing the matched entry in the IORegistry
 
 @return returns true on succesful start, else returns false
 */

bool ___FILEBASENAMEASIDENTIFIER___::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    return true;
}

/**
 Stops the class and undoes the effects of `start` and `probe`

 @param provider IOService* representing the matched entry in the IORegistry
 */

void ___FILEBASENAMEASIDENTIFIER___::stop(IOService* provider) {
    super::stop(provider);
}
