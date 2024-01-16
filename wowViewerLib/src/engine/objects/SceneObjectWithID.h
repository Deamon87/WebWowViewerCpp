//
// Created by Deamon on 12/6/2023.
//

#ifndef AWEBWOWVIEWERCPP_SCENEOBJECTWITHID_H
#define AWEBWOWVIEWERCPP_SCENEOBJECTWITHID_H

class ObjectWithId {
    template<class T> friend class EntityFactory;
public:
    ObjectWithId() {

    }
    virtual ~ObjectWithId() = default;

    int getObjectId() { return m_id;}
private:
    void setId(int id) {m_id = id;};

    int m_id = -1;
};

#endif //AWEBWOWVIEWERCPP_SCENEOBJECTWITHID_H
