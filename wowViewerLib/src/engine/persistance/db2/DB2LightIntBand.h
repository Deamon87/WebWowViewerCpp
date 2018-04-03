//
// Created by Deamon on 4/2/2018.
//

#ifndef WEBWOWVIEWERCPP_DB2LIGHTINTBAND_H
#define WEBWOWVIEWERCPP_DB2LIGHTINTBAND_H


#include <cstdint>
#include <cstring>
#include "base/DB2Base.h"

struct DB2LightIntBandRecord {
    uint32_t num;
    uint32_t time[16];
    uint32_t data[16];
};

class DB2LightIntBand {
public:
    static const int DIFFUSE_INDEX = 0;
    static const int AMBIENT_INDEX = 1;
    static const int FOG_INDEX = 7;

    explicit DB2LightIntBand(DB2Base *base) : m_base(base) {
    };

    bool getIsLoaded() {
        return m_base->getIsLoaded();
    }



    DB2LightIntBandRecord getRecord(int id) {
        DB2LightIntBandRecord dbLightIntBandRecord;

        m_base->readRecord(id, 0, -1, [&dbLightIntBandRecord](int fieldNum, char *data, size_t length) -> void {
            if (fieldNum == 0) {
                memcpy(&dbLightIntBandRecord.num, data, length);
            } else if (fieldNum == 1) {
                memcpy(&dbLightIntBandRecord.time[0], data, length);
            }  else if (fieldNum == 2) {
                memcpy(&dbLightIntBandRecord.data[0], data, length);
            }
        });

        return dbLightIntBandRecord;
    };


private:
    DB2Base *m_base;


};


#endif //WEBWOWVIEWERCPP_DB2LIGHTINTBAND_H
