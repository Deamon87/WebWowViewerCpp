//
// Created by Deamon on 12/6/2023.
//

#ifndef AWEBWOWVIEWERCPP_SCENEOBJECTWITHID_H
#define AWEBWOWVIEWERCPP_SCENEOBJECTWITHID_H

class SceneObjectWithId {
public:
    SceneObjectWithId(int id) : m_id(id) {

    }

    int getObjectId() { return m_id;}
private:
    int m_id;
};

#endif //AWEBWOWVIEWERCPP_SCENEOBJECTWITHID_H
