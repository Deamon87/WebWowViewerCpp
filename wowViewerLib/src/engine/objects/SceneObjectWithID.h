//
// Created by Deamon on 12/6/2023.
//

#ifndef AWEBWOWVIEWERCPP_SCENEOBJECTWITHID_H
#define AWEBWOWVIEWERCPP_SCENEOBJECTWITHID_H

template<typename ObjIdType>
class ObjectWithId {
    template<uint16_t MemoryBlockSize, typename T, typename MainClass, typename... Types> friend class EntityFactory;
public:
    ObjectWithId() {

    }
    virtual ~ObjectWithId() = default;

    ObjIdType getObjectId() { return m_id;}
private:
    void setId(ObjIdType id) {m_id = id;};

    ObjIdType m_id = ObjIdType(-1);
};

#endif //AWEBWOWVIEWERCPP_SCENEOBJECTWITHID_H
