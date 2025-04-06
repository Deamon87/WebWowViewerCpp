//
// Created by Deamon on 7/25/2023.
//

#ifndef AWEBWOWVIEWERCPP_IBUFFERVERSIONED_H
#define AWEBWOWVIEWERCPP_IBUFFERVERSIONED_H
template<typename T>
class IBufferChunkVersioned {
public:
    using structureType = T;

    virtual ~IBufferChunkVersioned() = default;
    virtual T &getObject(int version) = 0;
    virtual void saveVersion(int version) = 0;
};
#endif //AWEBWOWVIEWERCPP_IBUFFERVERSIONED_H
