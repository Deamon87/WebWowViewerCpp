//
// Created by Deamon on 7/10/2017.
//

#ifndef WEBWOWVIEWERCPP_M2WMOOBJECT_H
#define WEBWOWVIEWERCPP_M2WMOOBJECT_H

#include "m2Object.h"

class M2WmoObject : public M2Object {
public:
    explicit M2WmoObject(IWoWInnerApi *api): M2Object::M2Object(api){
    }

    
};


#endif //WEBWOWVIEWERCPP_M2WMOOBJECT_H
