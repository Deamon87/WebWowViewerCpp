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

    inline static auto makeStorage(const std::string &dataBaseFile) {
        using namespace sqlite_orm;
        return make_storage(dataBaseFile,
                            make_index("idx_files_name", &FileRecord::fileName),
                            make_index("idx_files_type", &FileRecord::fileType),
                            make_table("files",
                                       make_column("fileDataId", &FileRecord::fileDataId, primary_key()),
                                       make_column("fileName", &FileRecord::fileName),
                                       make_column("file_type", &FileRecord::fileType)
                            )
        );
    };
}

class FileListWindow {
public:
    FileListWindow(const HApiContainer &api);

    bool draw();
private:
    HApiContainer m_api;

    bool m_showWindow;
    int m_filesTotal;
    decltype(FileListDB::makeStorage("")) m_storage;

private:
    void importCSV();
};


#endif //AWEBWOWVIEWERCPP_FILELISTWINDOW_H
