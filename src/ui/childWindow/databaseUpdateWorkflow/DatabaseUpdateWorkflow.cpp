//
// Created by Deamon on 8/27/2022.
//

#include "DatabaseUpdateWorkflow.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "../../../persistance/httpFile/httpFile.h"
#include "../../../../3rdparty/filesystem_impl/include/ghc/filesystem.hpp"
#include "../../../../3rdparty/DBImporter/fileReaders/DBD/DBDFile.h"
#include "../../../../3rdparty/DBImporter/fileReaders/DBD/DBDFileStorage.h"
#include "../../../../3rdparty/DBImporter/exporters/sqlite/CSQLLiteExporter.h"
#include "../../../../3rdparty/DBImporter/importers/WDC3/WDC3Importer.h"

struct RequiredTableStruct {
    int fileDataId;
    std::string tableName;
    bool notInClassic;
};

std::array<RequiredTableStruct, 13> requiredTables = {{
    {1353545, "AreaTable", false},
    {1375579, "Light", false},
    {1375580, "LightData", false},
    {1334669, "LightParams", false},
    {1308501, "LightSkybox", false},
    {1132538, "LiquidMaterial", false},
    {1308058, "LiquidObject", false},
    {1371380, "LiquidType", false},
    {2261065, "LiquidTypeXTexture", true},
    {1349477, "Map", false},
    {1355528, "WMOAreaTable", false},
    {1310253, "ZoneLight", false},
    {1310256, "ZoneLightPoint", false},
}};


static const std::string definitionsDir = "definitions";

static const std::string POPUP_UPDATE_DBD = "Update DBD files";
static const std::string POPUP_UPDATING_DBD = "Updating DBD files";
static const std::string POPUP_UPDATING_DATABASE_DB2 = "Updating database from DB2";
static const std::string POPUP_UPDATED_DATABASE_DB2 = "Database updated succesfully";

DatabaseUpdateWorkflow::DatabaseUpdateWorkflow(HApiContainer &api, bool isClassic) {
    m_api = api;
    m_isClassic = isClassic;
}

void DatabaseUpdateWorkflow::render() {
    if (m_showDBDPrompt) {
        ImGui::OpenPopup(POPUP_UPDATE_DBD.c_str());
    } else if(m_needToUpdateDBD) {
        ImGui::OpenPopup(POPUP_UPDATING_DBD.c_str());
    } else if (m_needToUpdateDatabase) {
        ImGui::OpenPopup(POPUP_UPDATING_DATABASE_DB2.c_str());
    } else if (m_showOkModal) {
        ImGui::OpenPopup(POPUP_UPDATED_DATABASE_DB2.c_str());
    }


    defineDialogs();
}

void DatabaseUpdateWorkflow::defineDialogs() {
    if (ImGui::BeginPopupModal(POPUP_UPDATE_DBD.c_str()))
    {
        ImGui::Text("Do you want to download DBD from github?");
        ImGui::Text("Warning: current files will be overwritten");

        if (ImGui::Button("Yes", ImVec2(-1, 23))) {
            m_showDBDPrompt = false;
            m_needToUpdateDBD = true;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("No", ImVec2(-1, 23))) {
            m_showDBDPrompt = false;
            m_needToUpdateDBD = false;
            m_needToUpdateDatabase = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal(POPUP_UPDATING_DBD.c_str())) {
        dbdThreadDownloadLogic();

        if (!m_failedToDownloadFile.empty()) {
            ImGui::Text("Failed to download file: %s", m_failedToDownloadFile.c_str());
            if (ImGui::Button("Ok", ImVec2(-1, 23))) {
                m_showDBDPrompt = false;
                m_needToUpdateDBD = false;
                ImGui::CloseCurrentPopup();
            }
        } else {
            if (m_currentDBDFile >= requiredTables.size()) {
                m_needToUpdateDatabase = true;
                m_needToUpdateDBD = false;

                if (httpDownloadThread != nullptr) {
                    httpDownloadThread->detach();
                    httpDownloadThread = nullptr;
                }
                ImGui::CloseCurrentPopup();
            } else {
                ImGui::Text("Downloading DBD file: %s", requiredTables[m_currentDBDFile].tableName.c_str());
            }
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal(POPUP_UPDATING_DATABASE_DB2.c_str())) {
        db2UpdateLogic();

        if (m_db2FailedMessage.empty()) {
            if (m_currentDB2File < requiredTables.size()) {
                ImGui::Text("Processing db2: %s", requiredTables[m_currentDB2File].tableName.c_str());
            } else {
                m_needToUpdateDatabase = false;
                m_showOkModal = true;
                ImGui::CloseCurrentPopup();
            }
        } else {
            ImGui::Text("%s", m_db2FailedMessage.c_str());
            if (ImGui::Button("Ok", ImVec2(-1, 23))) {
                m_needToUpdateDatabase = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal(POPUP_UPDATED_DATABASE_DB2.c_str())) {
        ImGui::Text("Database Updated succesfully");
        if (ImGui::Button("Ok", ImVec2(-1, 23))) {
            checkDB2Lambda = nullptr;
            addTableLambda = nullptr;
            m_showOkModal = false;
            m_databaseUpdated = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void DatabaseUpdateWorkflow::dbdThreadDownloadLogic() {
    if (httpDownloadThread == nullptr && m_httpDownloadThreadFinished && m_failedToDownloadFile.empty() && m_currentDBDFile < requiredTables.size()) {
        auto &l_currentDBDFile = m_currentDBDFile;
        auto &l_httpDownloadThreadFinished = m_httpDownloadThreadFinished;
        auto &l_failedToDownloadFile = m_failedToDownloadFile;
        m_httpDownloadThreadFinished = false;
        httpDownloadThread = std::make_shared<std::thread>([&l_currentDBDFile, &l_httpDownloadThreadFinished, &l_failedToDownloadFile]() -> void {

            std::string fileToDownload = "https://raw.githubusercontent.com/wowdev/WoWDBDefs/master/definitions/"+
                                         requiredTables[l_currentDBDFile].tableName +
                                         ".dbd";
            HttpFile httpFile = HttpFile(fileToDownload);
            httpFile.setCallback([&l_currentDBDFile](HFileContent fileContent) -> void {

                    if (!ghc::filesystem::is_directory(definitionsDir) || !ghc::filesystem::exists(definitionsDir)) {
                        ghc::filesystem::create_directory(definitionsDir); // create export folder
                    }

                    std::string outputFileName =
                        definitionsDir +
                        "/" +
                        requiredTables[l_currentDBDFile].tableName +
                        ".dbd";

                    std::ofstream FILE(outputFileName, std::ios::out | std::ios::binary);
                    std::copy(fileContent->begin(), fileContent->end(), std::ostreambuf_iterator<char>(FILE));
                    FILE.flush();
                    FILE.close();
                    l_currentDBDFile++;
                }
            );
            httpFile.setFailCallback([fileToDownload, &l_failedToDownloadFile](HFileContent fileContent) -> void {
                l_failedToDownloadFile = fileToDownload;
            });
            httpFile.startDownloading();
            l_httpDownloadThreadFinished = true;
        });
    }
    if (m_httpDownloadThreadFinished && httpDownloadThread != nullptr) {
        httpDownloadThread->detach();
        httpDownloadThread = nullptr;
    }
}

void DatabaseUpdateWorkflow::db2UpdateLogic() {
    if (addTableLambda == nullptr) {
        std::shared_ptr<DBDFileStorage> fileDBDStorage = std::make_shared<DBDFileStorage>(definitionsDir);
        std::shared_ptr<CSQLLiteExporter> csqlLiteExporter = std::make_shared<CSQLLiteExporter>("export.db3");

        addTableLambda = [fileDBDStorage, csqlLiteExporter](std::string tableName, std::shared_ptr<Db2File> db2File) -> bool {
            std::shared_ptr<WDC3::DB2Base> db2Base = std::make_shared<WDC3::DB2Base>();
            db2Base->process(db2File->getContent(), "");

            DBDFile::BuildConfig *buildConfig = nullptr;
            auto dbdFile = fileDBDStorage->getDBDFile(tableName);
            if (dbdFile != nullptr) {
                std::string tableNameFromDBD = fileDBDStorage->getTableName(db2Base->getWDCHeader()->table_hash);
                if (tableNameFromDBD != "") {
                    tableName = tableNameFromDBD;
                }

                bool configFound = dbdFile->findBuildConfigByLayout(db2Base->getLayoutHash(), buildConfig);
                if (!configFound) {
                    std::cout << "Could not find proper build config for table " << tableName <<
                              " for layout hash " << db2Base->getLayoutHash() << std::endl;

                    return false;
                }
            } else {
                std::cout << "Could not find DBD file for table " << tableName << std::endl;
            }


            if (db2Base->getWDCHeader()->field_storage_info_size == 0) {
                if (buildConfig == nullptr) {
                    std::cout << "DB2 " << tableName
                              << " do not have field info. Unable to parse without build config in DBD file"
                              << std::endl;
                    return false;
                }
            }

            WDC3Importer::processWDC3(tableName, db2Base, dbdFile, csqlLiteExporter.get(), buildConfig);
            return true;
        };
    }

    if (checkDB2Lambda == nullptr && m_db2ThreadFinished && m_db2FailedMessage.empty() && m_currentDB2File < requiredTables.size()) {
        auto &l_currentDB2File = m_currentDB2File;
        auto &l_db2ThreadFinished = m_db2ThreadFinished;
        auto &l_db2FailedMessage = m_db2FailedMessage;
        auto &l_addTableLambda = addTableLambda;
        auto &l_checkDB2Lambda = checkDB2Lambda;

        auto dbFile = m_api->cacheStorage->getDb2Cache()->getFileId(requiredTables[m_currentDB2File].fileDataId);
        checkDB2Lambda = [dbFile, &l_addTableLambda, &l_currentDB2File, &l_db2FailedMessage, &l_checkDB2Lambda]() -> bool {
            if (dbFile->getStatus() == FileStatus::FSNotLoaded)
                return false;
            if (dbFile->getStatus() == FileStatus::FSRejected) {
                l_db2FailedMessage = "Could not read db2 " + requiredTables[l_currentDB2File].tableName;
                return false;
            }

            if (dbFile->getStatus() == FileStatus::FSLoaded) {
                if (!l_addTableLambda(requiredTables[l_currentDB2File].tableName, dbFile)) {
                    l_db2FailedMessage = "Failed to import db2 " + requiredTables[l_currentDB2File].tableName;
                }
                l_checkDB2Lambda = nullptr;
                return true;
            }

            return false;
        };
    }

    if (m_db2FailedMessage.empty() && checkDB2Lambda()) {
        m_currentDB2File++;
        if (m_isClassic) {
            while (m_currentDB2File < requiredTables.size() && requiredTables[m_currentDB2File].notInClassic) {
                m_currentDB2File++;
            }
        }

    }
}
