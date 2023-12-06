//
// Created by Deamon on 10/28/2023.
//

#ifndef AWEBWOWVIEWERCPP_FILELISTWINDOW_H
#define AWEBWOWVIEWERCPP_FILELISTWINDOW_H

#include "../../renderer/uiScene/FrontendUIRenderer.h"
#include "../../../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../../3rdparty/sqlite_orm/sqlite_orm.h"

namespace FileListDB {
    struct FileRecord {
        int fileDataId;
        std::string fileName;
        std::string fileType;
    };


}

struct DbRequest {
    int offset;
    int limit;
};
struct DBResults {
    int offset;
    std::vector<FileListDB::FileRecord> records;
};


class FileListLamda {
public:
    virtual ~FileListLamda() {}
    virtual void makeRequest(const std::vector<DbRequest> &paramsChange) = 0;
    virtual void searchChanged() = 0;
    virtual void setOrder(int order) = 0;
    virtual void scanFiles() = 0;
    virtual const std::vector<DBResults> getResults() = 0;
};

class FileListWindow {
public:
    FileListWindow(const HApiContainer &api, const std::function<void(int fileId, const std::string &fileType)> &fileOpenCallback);

    bool draw();
private:
    bool m_showWindow;

    std::function<void(int fileId, const std::string &fileType)> m_fileOpenCallback;

    int m_filesTotal;
    const HApiContainer m_api;
    std::unique_ptr<FileListLamda> selectStatement;
    std::array<char, 128> filterText = {0};
    std::string filterTextStr = "%%";
private:
    void importCSV();
};


#endif //AWEBWOWVIEWERCPP_FILELISTWINDOW_H
