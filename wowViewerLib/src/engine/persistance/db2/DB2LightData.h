//
// Created by Deamon on 4/2/2018.
//

#ifndef WEBWOWVIEWERCPP_DB2LIGHTINTBAND_H
#define WEBWOWVIEWERCPP_DB2LIGHTINTBAND_H


#include <cstdint>
#include <cstring>
#include "base/DB2Base.h"

struct DB2LightDataRecord {
    int32_t directColor;                //1
    int32_t ambientColor;               //2
    int32_t skyTopColor;                //3
    int32_t skyMiddleColor;             //4
    int32_t skyBand1Color;              //5
    int32_t skyBand2Color;              //6
    int32_t skySmogColor;               //7
    int32_t skyFogColor;                //8
    int32_t sunColor;                   //9
    int32_t cloudSunColor;              //10
    int32_t cloudEmissiveColor;         //11
    int32_t cloudLayer1AmbientColor;    //12
    int32_t cloudLayer2AmbientColor;    //13
    int32_t oceanCloseColor;            //14
    int32_t oceanFarColor;              //15
    int32_t riverCloseColor;            //16
    int32_t riverFarColor;              //17
    int32_t shadowOpacity;              //18
    float fogEnd;                       //19
    float fogScaler;                    //20
    float cloudDensity;                 //21
    float fogDensity;                   //22
    float fogHeight;                    //23
    float fogHeightScaler;              //24
    float fogHeightDensity;             //25
    float sunFogAngle;                  //26
    float endFogColorDistance;          //27
    int32_t sunFogColor;                //28
    int32_t endFogColor;                //29
    int32_t fogHeightColor;             //30
    int32_t colorGradingFileDataID;     //31
    int32_t horizonAmbientColor;        //32
    int32_t groundAmbientColor;         //33
    int16_t lightParamID;               //34
    int16_t time;                       //35
};

class DB2LightData {
public:
    static const int DIFFUSE_INDEX = 0;
    static const int AMBIENT_INDEX = 1;
    static const int FOG_INDEX = 7;

    explicit DB2LightData(DB2Base *base) : m_base(base) {
    };

    bool getIsLoaded() {
        return m_base->getIsLoaded();
    }



    DB2LightDataRecord getRecord(int id) {
        DB2LightDataRecord dbLightIntBandRecord;

        m_base->readRecord(id, true, 0, -1, [&dbLightIntBandRecord](int fieldNum, char *data, size_t length) -> void {
            if (fieldNum == 0) {
                memcpy(&dbLightIntBandRecord.directColor, data, length);
            } else if (fieldNum == 1) {
                memcpy(&dbLightIntBandRecord.ambientColor, data, length);
            }else if (fieldNum == 2) {
                memcpy(&dbLightIntBandRecord.skyTopColor, data, length);
            }else if (fieldNum == 3) {
                memcpy(&dbLightIntBandRecord.skyMiddleColor, data, length);
            }else if (fieldNum == 4) {
                memcpy(&dbLightIntBandRecord.skyBand1Color, data, length);
            }else if (fieldNum == 5) {
                memcpy(&dbLightIntBandRecord.skyBand2Color, data, length);
            }else if (fieldNum == 6) {
                memcpy(&dbLightIntBandRecord.skySmogColor, data, length);
            }else if (fieldNum == 7) {
                memcpy(&dbLightIntBandRecord.skyFogColor, data, length);
            }else if (fieldNum == 8) {
                memcpy(&dbLightIntBandRecord.sunColor, data, length);
            }else if (fieldNum == 9) {
                memcpy(&dbLightIntBandRecord.cloudSunColor, data, length);
            }else if (fieldNum == 10) {
                memcpy(&dbLightIntBandRecord.cloudEmissiveColor, data, length);
            }else if (fieldNum == 11) {
                memcpy(&dbLightIntBandRecord.cloudLayer1AmbientColor, data, length);
            }else if (fieldNum == 12) {
                memcpy(&dbLightIntBandRecord.cloudLayer2AmbientColor, data, length);
            }else if (fieldNum == 13) {
                memcpy(&dbLightIntBandRecord.oceanCloseColor, data, length);
            }else if (fieldNum == 14) {
                memcpy(&dbLightIntBandRecord.oceanFarColor, data, length);
            }else if (fieldNum == 15) {
                memcpy(&dbLightIntBandRecord.riverCloseColor, data, length);
            }else if (fieldNum == 16) {
                memcpy(&dbLightIntBandRecord.riverFarColor, data, length);
            }else if (fieldNum == 17) {
                memcpy(&dbLightIntBandRecord.shadowOpacity, data, length);
            }else if (fieldNum == 18) {
                memcpy(&dbLightIntBandRecord.fogEnd, data, length);
            }else if (fieldNum == 19) {
                memcpy(&dbLightIntBandRecord.fogScaler, data, length);
            }else if (fieldNum == 20) {
                memcpy(&dbLightIntBandRecord.cloudDensity, data, length);
            }else if (fieldNum == 21) {
                memcpy(&dbLightIntBandRecord.fogDensity, data, length);
            } else if (fieldNum == 22) {
                memcpy(&dbLightIntBandRecord.fogHeight, data, length);
            } else if (fieldNum == 23) {
                memcpy(&dbLightIntBandRecord.fogHeightScaler, data, length);
            } else if (fieldNum == 24) {
                memcpy(&dbLightIntBandRecord.fogHeightDensity, data, length);
            } else if (fieldNum == 25) {
                memcpy(&dbLightIntBandRecord.sunFogAngle, data, length);
            } else if (fieldNum == 26) {
                memcpy(&dbLightIntBandRecord.endFogColorDistance, data, length);
            } else if (fieldNum == 27) {
                memcpy(&dbLightIntBandRecord.sunFogColor, data, length);
            } else if (fieldNum == 28) {
                memcpy(&dbLightIntBandRecord.endFogColor, data, length);
            } else if (fieldNum == 29) {
                memcpy(&dbLightIntBandRecord.fogHeightColor, data, length);
            } else if (fieldNum == 30) {
                memcpy(&dbLightIntBandRecord.colorGradingFileDataID, data, length);
            } else if (fieldNum == 31) {
                memcpy(&dbLightIntBandRecord.horizonAmbientColor, data, length);
            } else if (fieldNum == 32) {
                memcpy(&dbLightIntBandRecord.groundAmbientColor, data, length);
            } else if (fieldNum == 33) {
                memcpy(&dbLightIntBandRecord.lightParamID, data, length);
            } else if (fieldNum == 34) {
                memcpy(&dbLightIntBandRecord.time, data, length);
            }
        });

        return dbLightIntBandRecord;
    };


private:
    DB2Base *m_base;


};


#endif //WEBWOWVIEWERCPP_DB2LIGHTINTBAND_H
