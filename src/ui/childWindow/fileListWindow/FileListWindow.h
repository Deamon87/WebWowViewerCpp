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

    struct FileRecord_FT5 {
        int fileDataId;
        std::string fileName;
        std::string fileType;
    };

    inline static auto makeStorage(const std::string &dataBaseFile) {
        using namespace sqlite_orm;
        return make_storage(dataBaseFile,
                            make_trigger("files_after_insert",
                                         after()
                                             .insert()
                                             .on<FileRecord>()
                                             .begin(
                                                 insert(into<FileRecord_FT5>(),
                                                        columns(&FileRecord_FT5::fileDataId,
                                                                &FileRecord_FT5::fileName),
                                                        values(std::make_tuple(new_(&FileRecord::fileDataId),
                                                                               new_(&FileRecord::fileName)
                                                                              )
                                                        )
                                                 )
                                             )
                                             .end()),
                            make_trigger("files_after_delete",
                                         after()
                                             .delete_()
                                             .on<FileRecord>()
                                             .begin(
                                                 remove_all<FileRecord_FT5>(where(is_equal(old(&FileRecord::fileDataId), &FileRecord_FT5::fileDataId)))
                                             )
                                             .end()),
                            make_trigger("files_after_update",
                                         after()
                                             .delete_()
                                             .on<FileRecord>()
                                             .begin(
                                                 update_all(set(assign(&FileRecord_FT5::fileName, &FileRecord::fileName)),
                                                            where(is_equal(old(&FileRecord::fileDataId), &FileRecord_FT5::fileDataId)))
                                             )
                                             .end()),
                            make_virtual_table("files_FT5", using_fts5(make_column("fileName", &FileRecord_FT5::fileName), make_column("fileDataId", &FileRecord_FT5::fileDataId))),
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

class FileListLamda {
public:
    virtual ~FileListLamda() {}
    virtual std::vector<FileListDB::FileRecord> getFiles(int offset, int limit) = 0;
};

class FileListWindow {
public:
    FileListWindow(const HApiContainer &api);

    bool draw();
private:
    HApiContainer m_api;

    bool m_showWindow;
    decltype(FileListDB::makeStorage("")) m_storage;


    struct FileRecordCacheKey {
        int offset;
        int amount;
        bool operator==(const FileRecordCacheKey &other) const {
            return offset == other.offset && amount == other.amount;
        }
    };
    struct FileRecordCacheKeyHasher {
        std::size_t operator()(const FileRecordCacheKey& k) const {
            using std::hash;
            return hash<int>{}(k.offset) ^ (hash<int>{}(k.amount) << 12);
        };
    };

    typedef std::unordered_map<FileRecordCacheKey, std::vector<FileListDB::FileRecord>, FileRecordCacheKeyHasher> FileRecordCache;
    FileRecordCache m_fileRecCache;

    int m_filesTotal;
    std::unique_ptr<FileListLamda> selectStatement;
    std::array<char, 128> filterText = {0};
    std::string filterTextStr = "";
private:
    void makeNewSelectStatement();
    void importCSV();
};


#endif //AWEBWOWVIEWERCPP_FILELISTWINDOW_H
