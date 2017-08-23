//
//  ___FILENAME___
//  ___PROJECTNAME___
//
//  Created by ___FULLUSERNAME___ on ___DATE___.
//___COPYRIGHT___
//

#ifndef ___FILEBASENAMEASIDENTIFIER____hpp
#define ___FILEBASENAMEASIDENTIFIER____hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

class ___FILEBASENAMEASIDENTIFIER___ : public ___VARIABLE_superClass___ {
  OSDeclareDefaultStructors(___FILEBASENAMEASIDENTIFIER___);

 public:
    bool attach(IOService* provider);
    void detach(IOService* provider);
    bool init(OSDictionary* properties);
    void free();
    bool start(IOService* provider);
    void stop(IOService* provider);

 protected:
 private:
};


#endif /* ___FILEBASENAMEASIDENTIFIER____hpp */
