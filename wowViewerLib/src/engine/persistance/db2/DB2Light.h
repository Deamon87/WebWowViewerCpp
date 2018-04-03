//
// Created by deamon on 02.04.18.
//

#ifndef WEBWOWVIEWERCPP_DB2LIGHT_H
#define WEBWOWVIEWERCPP_DB2LIGHT_H

#include <cstdint>
#include <iostream>
#include <cstring>
#include "base/DB2Base.h"
//#include "../../../../3rdparty/RTree/RTree.h"


struct DBLightRecord {
    float gameCoords[3];
    float gameFalloffStart;
    float gameFalloffEnd;
    int16_t continentID;
    int16_t lightParamsID[8];
};

class DB2Light {
public:
    explicit DB2Light(DB2Base *base) : m_base(base) {

    };

    int getRecordCount() {
        return m_base->getRecordCount();
    }

    bool getIsLoaded() {
        if (m_base->getIsLoaded() && !rTreeFilled) {
            fillRTree();
            rTreeFilled = true;
        }
        return m_base->getIsLoaded();
    }

    DBLightRecord findRecord(int mapId, mathfu::vec3 &pos) ;
    DBLightRecord getRecord(int id) {
        DBLightRecord dbLightRecord;

        m_base->readRecord(id, 0, -1, [&dbLightRecord](int fieldNum, char *data, size_t length) -> void {
            if (fieldNum == 0) {
                memcpy(&dbLightRecord.gameCoords[0], data, length);
            } else if (fieldNum == 1) {
                memcpy(&dbLightRecord.gameFalloffStart, data, length);
            }  else if (fieldNum == 2) {
                memcpy(&dbLightRecord.gameFalloffEnd, data, length);
            } else if (fieldNum == 3) {
                memcpy(&dbLightRecord.continentID, data, length);
            } else if (fieldNum == 4) {
                memcpy(&dbLightRecord.lightParamsID[0], data, length);
            }
        });

        return dbLightRecord;
    };



    void fillRTree();

private:
    DB2Base *m_base;

    bool rTreeFilled = false;
//    RTree<int, float, 4, float> lightRTree;
};


#endif //WEBWOWVIEWERCPP_DB2LIGHT_H
