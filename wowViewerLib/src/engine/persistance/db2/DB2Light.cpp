//
// Created by deamon on 02.04.18.
//

#include "DB2Light.h"
const float ROUNDING_ERROR_f32 = 0.001f;
inline bool feq(const float a, const float b, const float tolerance = ROUNDING_ERROR_f32)
{
    return (a + tolerance >= b) && (a - tolerance <= b);
}


void DB2Light::fillRTree() {
    int recordCount = m_base->getRecordCount();
    for (int i = 0; i < recordCount; i++) {
        DBLightRecord dbLightRecord;
        int id = m_base->getIdForRecord(i);
        m_base->readRecord(id, 0, 4, [&dbLightRecord](int fieldNum, char *data, size_t length) -> void {
            if (fieldNum == 0) {
                memcpy(&dbLightRecord.gameCoords[0], data, length);
            } else if (fieldNum == 1) {
                memcpy(&dbLightRecord.gameFalloffStart, data, length);
            } else if (fieldNum == 2) {
                memcpy(&dbLightRecord.gameFalloffEnd, data, length);
            } else if (fieldNum == 3) {
                memcpy(&dbLightRecord.continentID, data, length);
            }
        });
        float min[4];
        float max[4];

        min[0] = 17066.666f - (dbLightRecord.gameCoords[2] / 36.0f);
        min[1] = 17066.666f - (dbLightRecord.gameCoords[0] / 36.0f);
        min[2] = dbLightRecord.gameCoords[1] / 36.0f;

        float diff = dbLightRecord.gameFalloffEnd / 36.0f;
        if (feq(diff, 0.0f)) {
            diff += 0.001;
        }

        max[0] = min[0] + diff;
        max[1] = min[1] + diff;
        max[2] = min[2] + diff;
        max[3] = dbLightRecord.continentID + 0.1f;

        min[0] = min[0] - diff;
        min[1] = min[1] - diff;
        min[2] = min[2] - diff;
        min[3] = dbLightRecord.continentID - 0.1f;

        lightRTree.Insert(min, max, id);
    }

    //test
//    float m[4];
//    m[0] = 17066.666f;
//    m[1] = 17066.666f;
//    m[2] = 0;
//    m[3] = 530;
//
//    lightRTree.SearchWithin(m, [](const int &a) -> bool {
//        std::cout << "found id = " << a << std::endl;
//        return true;
//    });
}

