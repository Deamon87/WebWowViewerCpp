//
// Created by Deamon on 2/16/2021.
//

#ifndef AWEBWOWVIEWERCPP_DATAEXPORTERCLASS_H
#define AWEBWOWVIEWERCPP_DATAEXPORTERCLASS_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <mathfu/glsl_mappings.h>
#include "../../../wowViewerLib/src/engine/objects/m2/m2Object.h"
#include "../../database/csvtest/csv.h"
#include <fstream>

class DataExporterClass {
public:
    DataExporterClass(HApiContainer apiContainer);
    void process();
    bool isDone() {
        return finished;
    }
private:
    HApiContainer m_apiContainer;

    int currentFileDataId;
    std::string currentFileName;

    HM2Geom m_m2Geom = nullptr;
    io::CSVReader<2, io::trim_chars<' '>, io::no_quote_escape<';'>> *csv ;

    bool finished = false;
    int processedFiles = 0;

    std::ofstream outputLog;
};


#endif //AWEBWOWVIEWERCPP_DATAEXPORTERCLASS_H
