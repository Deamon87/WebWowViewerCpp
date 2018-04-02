//
// Created by deamon on 02.04.18.
//

#ifndef WEBWOWVIEWERCPP_DB2LIGHT_H
#define WEBWOWVIEWERCPP_DB2LIGHT_H

#include <cstdint>
#include "base/DB2Base.h"

struct DBLightRecord {
    float gameCoords[3];
    float gameFalloffStart;
    float gameFalloffEnd;
    int32_t continentID;
};

class DB2Light {
public:
    DB2Light(DB2Base *base) : m_base(base) {

    };

    bool getIsLoaded() {
        return m_base->getIsLoaded();
    }

    DBLightRecord &getRecord(int id) {
        DBLightRecord dbLightRecord;

        m_base->readRecord(id, [&dbLightRecord](int fieldNum, char *data, int length) -> {

        });

        return dbLightRecord;
    };


private:
    DB2Base *m_base;
};


#endif //WEBWOWVIEWERCPP_DB2LIGHT_H
