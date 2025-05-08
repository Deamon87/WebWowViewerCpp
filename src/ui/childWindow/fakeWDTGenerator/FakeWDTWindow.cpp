//
// Created by Deamon on 5/20/2025.
//

#include "FakeWDTWindow.h"

#include <fstream>

#include "imgui.h"
#include "../../../../wowViewerLib/src/engine/algorithms/mathHelper.h"
#include "../../../wowViewerLib/src/include/string_utils.h"

FakeWDTWindow::FakeWDTWindow(HApiContainer &api) : m_api(api) {

    m_filePtr = std::make_shared<WdtFile>(775971);
    api->requestProcessor->requestFile(CacheHolderType::CACHE_WDT, m_filePtr);
}

void FakeWDTWindow::doInitialization() {
    if (m_filePtr->mphd) {
        m_filePtr->mphd->lgtFileDataID = 0;
        m_filePtr->mphd->occFileDataID = 0;
        m_filePtr->mphd->fogsFileDataID = 0;
        m_filePtr->mphd->mpvFileDataID = 0;
        m_filePtr->mphd->texFileDataID = 0;
        m_filePtr->mphd->wdlFileDataID = 0;
        m_filePtr->mphd->pd4FileDataID = 0;
    }
    if (m_filePtr->mapFileDataIDs) {
        auto &arr = m_filePtr->mapFileDataIDs->array;

        for (auto &it : arr) {
            it.rootADT        = 0;
            it.obj0ADT        = 0;
            it.obj1ADT        = 0;
            it.tex0ADT        = 0;
            it.lodADT         = 0;
            it.mapTexture     = 0;
            it.mapTextureN    = 0;
            it.minimapTexture = 0;
        }

        std::ifstream infile("data.txt");
        std::string line;

        int filesAdded = 0;
        std::array<int, 5> adtFileDataIds;
        while (std::getline(infile, line)) {
            std::vector<std::string> out;
            tokenize(line, ",", out);

            if (out.size() == 3) {
                if (out[2] == "adt") {
                    filesAdded = 0;
                    adtFileDataIds = {0};
                    adtFileDataIds[filesAdded++] = std::stoi(out[0]);
                } else if (out[2] == "adt_sec") {
                    adtFileDataIds[filesAdded++] = std::stoi(out[0]);
                }
                if (filesAdded >= 5) {
                    m_files.push_back(adtFileDataIds);
                }
            }
        }
    }
}

template<typename T>
std::string int_to_hex(T i) {
    std::stringstream stream;
    stream << "0x"
           << std::setfill ('0') << std::setw(sizeof(T)*2)
           << std::hex << i;
    return stream.str();
}

bool FakeWDTWindow::draw() {
    bool showWindow = true;

    if (ImGui::Begin("Loading donor", &showWindow)) {
        ImGui::Text("Processing");
    }
    ImGui::End();

    if (m_filePtr->getStatus() == FileStatus::FSLoaded) {
        if (!m_initialized) {
            doInitialization();
            m_initialized = true;
        }
        if (m_initialized) {
            if (m_currentFile >= m_files.size()) {
                //Save WDT and close
                std::ofstream outfile("test.wdt", std::ios::binary);
                auto file = m_filePtr->getFile();
                outfile.write((char *)file->data(), file->size());
                outfile.close();
                return false;
            }

            if (m_adtFile == nullptr) {
                m_adtFile = std::make_shared<AdtFile>(m_files[m_currentFile][0]);
                m_adtFile->setIsMain(true);

                m_api->requestProcessor->requestFile(CacheHolderType::CACHE_ADT, m_adtFile);
            }
            if (m_adtFile) {
                if (m_adtFile->getStatus() == FileStatus::FSRejected) {
                    m_currentFile++;
                    m_adtFile = nullptr;
                }
                if (m_adtFile && m_adtFile->getStatus() == FileStatus::FSLoaded) {
                    for (int i = 0; i <= m_adtFile->mcnkRead; i++) {
                        SMChunk &mcnkObj = m_adtFile->mapTile[i];

                        int adt_x = worldCoordinateToAdtIndex(mcnkObj.position.y);
                        int adt_y = worldCoordinateToAdtIndex(mcnkObj.position.x);

                        auto &adtRec = m_filePtr->mapFileDataIDs->array[adt_y * 64 + adt_x];

                        adtRec.rootADT = m_files[m_currentFile][0];
                        adtRec.obj0ADT = m_files[m_currentFile][1];
                        adtRec.obj1ADT = m_files[m_currentFile][2];
                        adtRec.tex0ADT = m_files[m_currentFile][3];
                        adtRec.lodADT  = m_files[m_currentFile][4];
                        adtRec.minimapTexture = 3185156;
                        break;
                    }

                    m_currentFile++;
                    m_adtFile = nullptr;
                }
            }
        }
    }
    if (m_filePtr->getStatus() == FileStatus::FSRejected) {
        return false;
    }

    return true;
}
