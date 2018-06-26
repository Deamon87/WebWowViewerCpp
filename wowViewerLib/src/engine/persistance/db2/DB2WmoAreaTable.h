//
// Created by deamon on 06.04.18.
//

#ifndef WEBWOWVIEWERCPP_DB2WMOAREATABLE_H
#define WEBWOWVIEWERCPP_DB2WMOAREATABLE_H

#include <cstring>
#include <iostream>
#include "base/DB2Base.h"
#include "../../wowCommonClasses.h"

struct DBWmoAreaTableRecord {
    std::string AreaName;
    int AreaName_lang;
    int WMOGroupID;
    uint16_t WMOID;
    uint16_t AmbienceID;
    uint16_t ZoneMusic;
    uint16_t IntroSound;
    uint16_t AreaTableID;
    uint16_t uwIntroSound;
    uint16_t uwAmbience;
    uint8_t NameSetID;
    uint8_t SoundProviderPref;
    uint8_t SoundProviderPrefUnderwater;
    uint8_t flags;
    uint32_t ID;
    uint32_t uwZoneMusic;
};

class DB2WmoAreaTable {
public:
    explicit DB2WmoAreaTable(HDB2Base base) : m_base(base) {

    };

    int getRecordCount() {
        return m_base->getRecordCount();
    }

    bool getIsLoaded() {
        return m_base->getIsLoaded();
    }

    bool findRecord(uint16_t WMOID, int NameSetID, int WMOGroupID, DBWmoAreaTableRecord &areaTableRecord) {
        int defaultRecordIndex = -1;
        bool defaultRecordFound = false;
        DBWmoAreaTableRecord defaultRecord;

        DBWmoAreaTableRecord record;


        const std::function<void(int fieldNum, int stringOffset, char *data, size_t length)> partialCallback = [&record](int fieldNum, int stringOffset, char *data, size_t length) -> void {
            if (fieldNum == 1) {
                memcpy(&record.WMOGroupID, data, length);
            }  else if (fieldNum == 2) {
                memcpy(&record.WMOID, data, length);
            } else if (fieldNum == 9) {
                memcpy(&record.NameSetID, data, length);
            } else if (fieldNum == 13) {
                memcpy(&record.ID, data, length);
            };
        };

        const std::function<void(int fieldNum, int stringOffset, char *data, size_t length)> fullCallback = [&record](int fieldNum, int stringOffset, char *data, size_t length) -> void {
            if (fieldNum == 0) {
                memcpy(&record.AreaName_lang, data, length);

                record.AreaName_lang += stringOffset;
            } else if (fieldNum == 1) {
                memcpy(&record.WMOGroupID, data, length);
            }  else if (fieldNum == 2) {
                memcpy(&record.WMOID, data, length);
            } else if (fieldNum == 3) {
                memcpy(&record.AmbienceID, data, length);
            } else if (fieldNum == 4) {
                memcpy(&record.ZoneMusic, data, length);
            } else if (fieldNum == 5) {
                memcpy(&record.IntroSound, data, length);
            } else if (fieldNum == 6) {
                memcpy(&record.AreaTableID, data, length);
            } else if (fieldNum == 7) {
                memcpy(&record.uwIntroSound, data, length);
            } else if (fieldNum == 8) {
                memcpy(&record.uwAmbience, data, length);
            } else if (fieldNum == 9) {
                memcpy(&record.NameSetID, data, length);
            } else if (fieldNum == 10) {
                memcpy(&record.SoundProviderPref, data, length);
            } else if (fieldNum == 11) {
                memcpy(&record.SoundProviderPrefUnderwater, data, length);
            } else if (fieldNum == 12) {
                memcpy(&record.flags, data, length);
            } else if (fieldNum == 13) {
                memcpy(&record.ID, data, length);
            } else if (fieldNum == 14) {
                memcpy(&record.uwZoneMusic, data, length);
            }
        };

        int foundIndex = 0;
//        if ((foundIndex = m_base->readRecord(WMOID, true, 0, -1, partialCallback)) < 0) {
//            return false;
//        }

        while (foundIndex < getRecordCount() - 1) {
            if (record.WMOID == WMOID && record.NameSetID == NameSetID && record.WMOGroupID == -1) {
//                defaultRecord = record;
                defaultRecordFound = true;
                defaultRecordIndex = foundIndex;
            } else if (record.WMOID == WMOID && record.NameSetID == NameSetID && record.WMOGroupID == WMOGroupID) {
                m_base->readRecordByIndex(foundIndex, 0, -1, fullCallback);

                record.AreaName = m_base->readString(record.AreaName_lang );
                areaTableRecord = record;
                return true;
            }

            m_base->readRecordByIndex(++foundIndex, 0, -1, partialCallback);
        }

        if (defaultRecordFound) {
            m_base->readRecordByIndex(defaultRecordIndex, 0, -1, fullCallback);
            record.AreaName = m_base->readString(record.AreaName_lang );
            areaTableRecord = record;
            return true;
        }

        return false;
    }

    DBWmoAreaTableRecord getRecord(int id) {
        DBWmoAreaTableRecord dbWmoAreaTableRecord;

        m_base->readRecord(id, false, 0, -1, [&dbWmoAreaTableRecord](int fieldNum, int stringOffset, char *data, size_t length) -> void {
            if (fieldNum == 0) {
                memcpy(&dbWmoAreaTableRecord.AreaName_lang + stringOffset, data, length);
            } else if (fieldNum == 1) {
                memcpy(&dbWmoAreaTableRecord.WMOGroupID, data, length);
            }  else if (fieldNum == 2) {
                memcpy(&dbWmoAreaTableRecord.WMOID, data, length);
            } else if (fieldNum == 3) {
                memcpy(&dbWmoAreaTableRecord.AmbienceID, data, length);
            } else if (fieldNum == 4) {
                memcpy(&dbWmoAreaTableRecord.ZoneMusic, data, length);
            } else if (fieldNum == 5) {
                memcpy(&dbWmoAreaTableRecord.IntroSound, data, length);
            } else if (fieldNum == 6) {
                memcpy(&dbWmoAreaTableRecord.AreaTableID, data, length);
            } else if (fieldNum == 7) {
                memcpy(&dbWmoAreaTableRecord.uwIntroSound, data, length);
            } else if (fieldNum == 8) {
                memcpy(&dbWmoAreaTableRecord.uwAmbience, data, length);
            } else if (fieldNum == 9) {
                memcpy(&dbWmoAreaTableRecord.NameSetID, data, length);
            } else if (fieldNum == 10) {
                memcpy(&dbWmoAreaTableRecord.SoundProviderPref, data, length);
            } else if (fieldNum == 11) {
                memcpy(&dbWmoAreaTableRecord.SoundProviderPrefUnderwater, data, length);
            } else if (fieldNum == 12) {
                memcpy(&dbWmoAreaTableRecord.flags, data, length);
            } else if (fieldNum == 13) {
                memcpy(&dbWmoAreaTableRecord.ID, data, length);
            } else if (fieldNum == 14) {
                memcpy(&dbWmoAreaTableRecord.uwZoneMusic, data, length);
            }
        });

        return dbWmoAreaTableRecord;
    };
private:
    HDB2Base m_base = nullptr;
};


#endif //WEBWOWVIEWERCPP_DB2WMOAREATABLE_H
