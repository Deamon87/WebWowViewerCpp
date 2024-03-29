//
// Created by Deamon on 2/16/2021.
//

#include "DataExporterClass.h"
#include "../../../wowViewerLib/src/include/string_utils.h"

DataExporterClass::DataExporterClass(HApiContainer apiContainer) {
    m_apiContainer = apiContainer;
    processedFiles = 0;
    outputLog.open ("m2Log.txt");
    csv = new io::CSVReader<2, io::trim_chars<' '>, io::no_quote_escape<';'>>("listfile.csv");
}

void DataExporterClass::process() {
    if (finished)
        return;

    if (m_m2Geom == nullptr) {

        while (csv->read_row(currentFileDataId, currentFileName)) {
            if (endsWith(currentFileName, ".m2"))
                break;
        }
        if (!endsWith(currentFileName, ".m2")) {
            finished = true;
            return;
        }
        m_m2Geom = m_apiContainer->cacheStorage->getM2GeomCache()->getFileId(currentFileDataId);
    }

    if (m_m2Geom != nullptr && m_m2Geom->getStatus() == FileStatus::FSRejected)
        m_m2Geom = nullptr;

    if (m_m2Geom != nullptr && m_m2Geom->getStatus() == FileStatus::FSLoaded) {
        if (m_m2Geom->m_m2Data->particle_emitters.size > 0 || m_m2Geom->m_m2Data->ribbon_emitters.size > 0) {

            outputLog << currentFileName << " ";
            for (int i = 0; i < m_m2Geom->m_m2Data->particle_emitters.size; i++) {
                auto pe = m_m2Geom->m_m2Data->particle_emitters[i];
                bool isRecursive = (i < m_m2Geom->recursiveFileDataIDs.size()) && (m_m2Geom->recursiveFileDataIDs[i] > 0);
                bool isModelEmitter = (i < m_m2Geom->particleModelFileDataIDs.size()) && (m_m2Geom->particleModelFileDataIDs[i] > 0);

                std::string emitterGenerator = "";
                if (pe->old.emitterType == 1) {
                    emitterGenerator = "PLANE";
                } else if (pe->old.emitterType == 2) {
                    emitterGenerator = "SPHERE";
                } else if (pe->old.emitterType == 3) {
                    emitterGenerator = "SPLINE";
                } else if (pe->old.emitterType == 4) {
                    emitterGenerator = "BONE";
                } else  {
                    emitterGenerator = "UNK_GENERATOR_"+std::to_string(pe->old.emitterType);
                }

                std::string txacVal = "";
                if (i < m_m2Geom->txacMParticle.size() && (m_m2Geom->txacMParticle[i].value != 0)) {
                    txacVal = "TXAC = ("
                        + std::to_string((int)m_m2Geom->txacMParticle[i].perByte[0])
                        + ", "
                        + std::to_string((int)m_m2Geom->txacMParticle[i].perByte[0])
                        + ") ";
                }

                outputLog << "( "
                    << (isRecursive ? ("RECURSIVE " + std::to_string(m_m2Geom->recursiveFileDataIDs[i]) + " ") : "")
                    << (isModelEmitter ? ("MODEL " + std::to_string(m_m2Geom->particleModelFileDataIDs[i]) + " ") : "")
                    << txacVal
                    << emitterGenerator
                    << " ) ";
            }

            for (int i = 0; i < m_m2Geom->m_m2Data->ribbon_emitters.size; i++) {
                auto ribbon = m_m2Geom->m_m2Data->ribbon_emitters[i];
                bool isRibbonSubstitution = ribbon->ribbonColorIndex > 0;
                bool isTextureTransform = (m_m2Geom->m_m2Data->global_flags.flag_unk_0x20000 > 0);

                if (isRibbonSubstitution || isTextureTransform) {
                    outputLog << "( ribbon " << i << " "
                        << (isRibbonSubstitution ? ("ribbonColorIndex = " + std::to_string(ribbon->ribbonColorIndex) + " " ) : "")
                        << (isTextureTransform ? ("textureTransformLookupIndex = " + std::to_string(ribbon->textureTransformLookupIndex) + " ") : "")
                        << " ) ";
                }
            }
            processedFiles++;
            outputLog << std::endl;
            if (processedFiles % 1000 == 0) {
                outputLog << std::flush;
            }

        }

        m_m2Geom = nullptr;
    }
}
