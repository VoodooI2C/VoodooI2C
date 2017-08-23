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

bool ___FILEBASENAMEASIDENTIFIER___::attach(IOService* provider) {
    if (!super::attach(provider))
        return false;

    return true;
}

void ___FILEBASENAMEASIDENTIFIER___::detach(IOService* provider) {
    super::detach(provider);
}

bool ___FILEBASENAMEASIDENTIFIER___::init(OSDictionary* properties) {
    if (!super::init(properties))
        return false;

    return true;
}

void ___FILEBASENAMEASIDENTIFIER___::free() {
    super::free();
}

bool ___FILEBASENAMEASIDENTIFIER___::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    return true;
}

void ___FILEBASENAMEASIDENTIFIER___::stop(IOService* provider) {
    super::stop(provider);
}
