//
// Created by Deamon on 10/28/2023.
//

#include "FileListWindow.h"
#include "../../../database/csvtest/csv.h"
#include "../../../../wowViewerLib/src/include/string_utils.h"

namespace FileListDB {
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
class StatementHolderAbstract {
public:
    virtual ~StatementHolderAbstract() {};
    virtual std::vector<FileListDB::FileRecord> execute(decltype(FileListDB::makeStorage("")) &storage, int limit, int offset) = 0;
    virtual FileListDB::FileRecord getOrCreateFile(decltype(FileListDB::makeStorage("")) &storage, int fileDataId, const std::string &fileName) = 0;
    virtual void setFileType(decltype(FileListDB::makeStorage("")) &storage, int fileDataId, const std::string &filetype) = 0;
    virtual int getTotal() = 0;
};

decltype(FileListDB::makeStorage("")) *u_storage;

template<int order>
class StatementHolderA {
private:
    int refFileDataId = 0;
    std::string refFileName = "";
    std::string refFileType = "";
public:
    std::function<int()> calcTotal;

public:
    auto createStatement(decltype(FileListDB::makeStorage("")) &storage,
                                       const std::string &searchClause, bool sortAsc) {
        using namespace sqlite_orm;

        auto whereClause = where(
            or_(
                like(&FileListDB::FileRecord::fileName, searchClause),
                like(&FileListDB::FileRecord::fileDataId, searchClause)
            )
        );

        calcTotal = [whereClause, &storage]() {
            return storage.count<FileListDB::FileRecord>(whereClause);
        };

        calcTotal();

        if constexpr (order == 1) {
            return [statement =
            storage.prepare(get_all<FileListDB::FileRecord>(
                whereClause,
                !sortAsc ?
                order_by(&FileListDB::FileRecord::fileDataId).desc() :
                order_by(&FileListDB::FileRecord::fileDataId).asc(),
                limit(2, offset(3))
            ))](decltype(storage) &storage, int limit, int offset) mutable {
                using namespace sqlite_orm;

                get<2>(statement) = limit;
                get<3>(statement) = offset;

                return storage.execute(statement);
            };
        } else if constexpr (order == 2) {
            return [statement =
            storage.prepare(get_all<FileListDB::FileRecord>(
                whereClause,
                !sortAsc ?
                order_by(&FileListDB::FileRecord::fileName).desc() :
                order_by(&FileListDB::FileRecord::fileName).asc(),
                limit(2, offset(3))
            ))](decltype(storage) &storage, int limit, int offset) mutable {
                using namespace sqlite_orm;

                get<2>(statement) = limit;
                get<3>(statement) = offset;

                return storage.execute(statement);
            };
        } else if constexpr (order == 3) {
            return [&searchClause, statement =
            storage.prepare(get_all<FileListDB::FileRecord>(
                whereClause,
                !sortAsc ?
                order_by(&FileListDB::FileRecord::fileType).desc() :
                order_by(&FileListDB::FileRecord::fileType).asc(),
                limit(2, offset(3))
            ))](decltype(storage) &storage, int limit, int offset) mutable {
                using namespace sqlite_orm;

                get<2>(statement) = limit;
                get<3>(statement) = offset;

                return storage.execute(statement);
            };
        }

        //std::cout << statement.sql() << std::endl;
//    std::cout << get<0>(statement) << std::endl;
//    std::cout << get<1>(statement) << std::endl;
//    std::cout << get<2>(statement) << std::endl;
//    std::cout << get<3>(statement) << std::endl;
    }
    auto createCheckFileOrCreate(decltype(FileListDB::makeStorage("")) &storage) {
        using namespace sqlite_orm;

        return [&, statement = storage.prepare(select(
            columns(&FileListDB::FileRecord::fileDataId, &FileListDB::FileRecord::fileName, &FileListDB::FileRecord::fileType),
            where(
                (c(&FileListDB::FileRecord::fileDataId) == std::ref(refFileDataId)) ||
                (c(&FileListDB::FileRecord::fileName) == std::ref(refFileName))
            )
        ))
        ](decltype(storage) &storage, int fileDataId, const std::string &fileName) {
            this->refFileDataId = fileDataId;
            this->refFileName = fileName;
            auto records = storage.execute(statement);
            if (!records.empty()) {
                auto &record = records[0];
                return FileListDB::FileRecord({get<0>(record), get<1>(record), get<2>(record)});
            };

            FileListDB::FileRecord newFile;
            newFile.fileDataId = fileDataId;
            newFile.fileName = fileName;

            storage.replace(newFile);

            return newFile;
        };
    }

    auto createSetFileType(decltype(FileListDB::makeStorage("")) &storage) {
        using namespace sqlite_orm;

        return [&, statement = storage.prepare(update_all(
            set(assign(&FileListDB::FileRecord::fileType, std::ref(refFileType))),
            where(
                (c(&FileListDB::FileRecord::fileDataId) == std::ref(refFileDataId))
            )
        ))
        ](decltype(storage) &storage, int fileDataId, const std::string &fileType) {
            this->refFileDataId = fileDataId;
            this->refFileType = fileType;

            storage.execute(statement);
        };
    }
};

template<int order>
class StatementHolder: public StatementHolderAbstract, public StatementHolderA<order> {
private:
    decltype(((StatementHolderA<order> *)(nullptr))->createStatement(*u_storage,"", false)) m_statementLambda;
    decltype(((StatementHolderA<order> *)(nullptr))->createCheckFileOrCreate(*u_storage)) m_checkFileOrCreate;
    decltype(((StatementHolderA<order> *)(nullptr))->createSetFileType(*u_storage)) m_setFileType;
public:
    StatementHolder(decltype(FileListDB::makeStorage("")) &storage,const std::string &searchClause, bool sortAsc) :
        m_statementLambda(StatementHolderA<order>::createStatement(storage, searchClause, sortAsc)),
        m_checkFileOrCreate(StatementHolderA<order>::createCheckFileOrCreate(storage)),
        m_setFileType(StatementHolderA<order>::createSetFileType(storage)) {

    }

    std::vector<FileListDB::FileRecord> execute(decltype(FileListDB::makeStorage("")) &storage, int limit, int offset) override {
        return m_statementLambda(storage, limit, offset);
    }

    int getTotal() override {
        return StatementHolderA<order>::calcTotal();
    };
    FileListDB::FileRecord getOrCreateFile(decltype(FileListDB::makeStorage("")) &storage, int fileDataId, const std::string &fileName) override {
        return m_checkFileOrCreate(storage, fileDataId, fileName);
    }
    void setFileType(decltype(FileListDB::makeStorage("")) &storage, int fileDataId, const std::string &filetype) override {
        m_setFileType(storage, fileDataId, filetype);
    };
};


std::unique_ptr<StatementHolderAbstract>
statementFactory(decltype(FileListDB::makeStorage("")) &storage, const std::string &searchClause, int order) {
    switch(abs(order)) {
        case 2:
            return std::make_unique<StatementHolder<2>>(storage, searchClause, order > 0);
        case 3:
            return std::make_unique<StatementHolder<3>>(storage, searchClause, order > 0);
        case 1:
        default:
            return std::make_unique<StatementHolder<1>>(storage, searchClause, order > 0);
    }
}

enum class EnumParamsChanged {
    OFFSET_LIMIT = 0, SEARCH_STRING = 1, SORTING = 2, SCAN_REPOSITORY = 3, LOAD_CSV_FILE = 4
};

//namespace std {
//
//// partial specialization for reference_wrapper
//// is this really necessary?
//    template<typename EnumParamsChanged>
//    struct hash<std::reference_wrapper<EnumParamsChanged>> {
//    public:
//        std::size_t operator()(const std::reference_wrapper<EnumParamsChanged> &x) const { return std::hash<EnumParamsChanged>()(x.get()); }
//    };
//}


template <typename T>
class my_container {
    // important: this really needs to be a deque and only front
    // insertion/deletion is allowed to not get dangling references
    typedef std::deque<T> storage;
public:
    typedef typename storage::value_type value_type;
    typedef typename storage::reference reference;
    typedef typename storage::const_reference const_reference;
    typedef typename storage::size_type size_type;

    my_container<T>(){}

    // no move semantics
    void push_back(const T& t) {
        auto it = lookup_.find(t);
        if(it != end(lookup_)) {
            // is already inserted report error
            return;
        }
        store_.push_back(t);
        // this is important to not have dangling references
        lookup_.insert(store_.back());
    }

    // trivial functions

    bool empty() const { return store_.empty(); }
    const T& front() const { return store_.front(); }
    T& front() { return store_.front(); }

    void pop_front() { lookup_.erase(store_.front()); store_.pop_front();  }
private:
    // look-up mechanism
    std::unordered_set<T> lookup_ = std::unordered_set<T>();
    // underlying storage
    storage store_;
};





class FileListLambdaInst : public FileListLamda {
public:
    FileListLambdaInst(const HApiContainer &api, std::string &searchClause, int &recordsTotal) : m_recordsTotal(recordsTotal), m_api(api)
    {
        dbThread = std::thread([&]() {
            decltype(FileListDB::makeStorage("")) storage = FileListDB::makeStorage(":memory:");

            storage.pragma.synchronous(0);
            storage.pragma.journal_mode(sqlite_orm::journal_mode::MEMORY);

            const std::string fileListDB = "fileList.db3";
            storage.backup_from(fileListDB);

            storage.sync_schema();

            std::unique_ptr<StatementHolderAbstract> statement = statementFactory(storage, searchClause, 1);
            m_recordsTotal = statement->getTotal();

            //stateChangeAwaiter.pushInput(EnumParamsChanged::OFFSET_LIMIT);

            while (!this->m_isTerminating) {
                stateChangeAwaiter.waitAndProcess([&](auto & stateWasChanged) {
                    switch (stateWasChanged) {
                        case EnumParamsChanged::OFFSET_LIMIT:
                            break;

                        case EnumParamsChanged::SEARCH_STRING:
                            statement = statementFactory(storage, searchClause, m_order);
                            m_recordsTotal = statement->getTotal();
                            break;

                        case EnumParamsChanged::SORTING:
                            statement = statementFactory(storage, searchClause, m_order);
                            break;

                        case EnumParamsChanged::SCAN_REPOSITORY: {
                            std::mutex scanMutex;

                            {
                                auto lock = std::unique_lock<std::mutex>(scanMutex);
                                auto r_unq = std::make_unique<IterateFilesRequest>(
                                    lock,
                                    [&statement, &storage](int fileDataId, const std::string &fileName) -> bool {
                                        auto fileRecord = statement->getOrCreateFile(storage, fileDataId, fileName);

                                        return fileRecord.fileType.empty();
                                    },
                                    [&statement, &storage](int fileDataId, const HFileContent &fileData) -> void {
                                        statement->setFileType(storage, fileDataId, "unk");
                                    }
                                );

                                m_api->requestProcessor->iterateAllFiles(r_unq);
                            }
                            std::unique_lock <std::mutex> waitLock(scanMutex);

                            std::cout << "code after wait lock " << std::endl;

                            break;
                        }


                        case EnumParamsChanged::LOAD_CSV_FILE:
                            importCSVInternal(storage);
                            break;
                    }

                    std::vector<DBResults> results;
                    {
                        std::vector<DbRequest> l_requests = {};
                        {
                            std::unique_lock lock(paramsChange);
                            l_requests = m_requests; //make local copy
                        }

                        for (auto const & request : l_requests) {
                            auto &result1 = results.emplace_back();
                            result1.offset = request.offset;
                            result1.records = statement->execute(storage, request.limit, request.offset);
                        }
                    }
                    setResults(results);
                });
            }

            storage.backup_to(fileListDB);
        });
    }
    ~FileListLambdaInst() {
        m_isTerminating = true;
        stateChangeAwaiter.pushInput(EnumParamsChanged::OFFSET_LIMIT);

        dbThread.join();
    }
    const std::vector<DBResults> getResults() override{
        std::unique_lock lock(resultsChange);

        return m_results;
    }
    void makeRequest(const std::vector<DbRequest> &newRequest) override {
        {
            std::unique_lock lock(paramsChange);
            m_requests = newRequest;
            for (auto &request : m_requests) {
                if (request.limit > 1) {
                    request.offset = std::max<int>(request.offset - request.limit, 0);
                    request.limit = request.limit * 3;
                }
            }
        }
        stateChangeAwaiter.pushInput(EnumParamsChanged::OFFSET_LIMIT);
    };
    void searchChanged() override {
        std::unique_lock lock(resultsChange);

        m_results = {};

        stateChangeAwaiter.pushInput(EnumParamsChanged::SEARCH_STRING);
    }
    void setOrder(int order) override {
        m_order = order;
        stateChangeAwaiter.pushInput(EnumParamsChanged::SORTING);
    }
    void scanFiles() override {
        stateChangeAwaiter.pushInput(EnumParamsChanged::SCAN_REPOSITORY);
    }
    void importCSV() override {
        stateChangeAwaiter.pushInput(EnumParamsChanged::LOAD_CSV_FILE);
    }

    int getCurrentScanningProgress() { return m_currentScanningProgress;}
    int getAmountOfFilesInRep() { return m_filesInRepository;}
private:
    void setResults(const std::vector<DBResults> &results) {
        std::unique_lock lock(resultsChange);

        m_results = results;
    }
    void importCSVInternal(decltype(FileListDB::makeStorage("")) &storage) {
        auto csv = new io::CSVReader<2, io::trim_chars<' '>, io::no_quote_escape<';'>>("listfile.csv");

        int currentFileDataId;
        std::string currentFileName;

        using namespace sqlite_orm;
        auto statement = storage.prepare(
            select(
                columns(&FileListDB::FileRecord::fileType), from<FileListDB::FileRecord>(),
                where(is_equal(&FileListDB::FileRecord::fileDataId, std::ref(currentFileDataId)))
            )
        );

        while (csv->read_row(currentFileDataId, currentFileName)) {
            auto typeFromDB = storage.execute(statement);

            std::string fileType = "";
            if (!typeFromDB.empty()) {
                fileType = get<0>(typeFromDB[0]);
            }

            storage.replace(FileListDB::FileRecord({currentFileDataId, currentFileName, fileType}));
        }

        delete csv;
    }

private:
    std::mutex paramsChange;
    std::mutex resultsChange;

    std::thread dbThread;
    bool m_isTerminating = false;

    const HApiContainer m_api;

    int &m_recordsTotal;
    int m_order = 1;

    int m_currentScanningProgress = -1;
    int m_filesInRepository = -1;

    std::vector<DBResults> m_results;
    std::vector<DbRequest> m_requests;

    ProdConsumerIOConnector<EnumParamsChanged, my_container<EnumParamsChanged>> stateChangeAwaiter = {m_isTerminating};
};

//--------------------------------------
// FileListWindow
// -------------------------------------

FileListWindow::FileListWindow(const HApiContainer &api, const std::function<void(int fileId, const std::string &fileType)> &fileOpenCallback) : m_api(api), m_fileOpenCallback(fileOpenCallback) {
    m_filesTotal = 0;
    m_showWindow = true;

    flInterface = std::make_unique<FileListLambdaInst>(api, filterTextStr, m_filesTotal);
}

bool FileListWindow::draw() {
    ImGui::Begin("File list", &m_showWindow);
    {
        if (ImGui::Button("Import filelist.csv...")) {
            importCSV();
        }
        ImGui::SameLine();
        if (ImGui::Button("Scan repository...")) {
            flInterface->scanFiles();
        }
        if (ImGui::InputText("Filter: ", filterText.data(), filterText.size()-1)) {
            filterText[filterText.size()-1] = 0;
            filterTextStr = filterText.data();
            filterTextStr = "%"+filterTextStr+"%";
            flInterface->searchChanged();
        }

        if (ImGui::BeginTable("FileListTable", 3, ImGuiTableFlags_Resizable |
                                              ImGuiTableFlags_Reorderable |
                                              ImGuiTableFlags_Sortable |
                                              ImGuiTableFlags_ScrollY |
                                              ImGuiTableFlags_NoHostExtendX |
                                              ImGuiTableFlags_NoHostExtendY |
                                              ImGuiTableFlags_ScrollX, ImVec2(-1, -1))) {
            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible

            ImGui::TableSetupColumn("FileDataId", ImGuiTableColumnFlags_None,0,1);
            ImGui::TableSetupColumn("FileName", ImGuiTableColumnFlags_None,0,2);
            ImGui::TableSetupColumn("FileType", ImGuiTableColumnFlags_None,0,3);
            ImGui::TableHeadersRow();

            ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs();
            if (sorts_specs && sorts_specs->SpecsDirty && flInterface) {
                int order = 1;
                for (int i = 0; i < sorts_specs->SpecsCount; i++) {
                    auto const &sort_spec = sorts_specs->Specs[i];
                    if (sort_spec.SortDirection != ImGuiSortDirection_None ) {
                        order = sort_spec.ColumnUserID * (
                            sort_spec.SortDirection == ImGuiSortDirection_Descending ? -1 : 1
                        );
                        break;
                    }
                }

                flInterface->setOrder(order);
                sorts_specs->SpecsDirty = false;
            }


            auto dbResults = flInterface->getResults();
            bool needRequest = false;
            std::vector<DbRequest> newRequests = {};
            {
                ImGuiListClipper clipper;
                clipper.Begin(m_filesTotal);
                while (clipper.Step()) {
                    auto f_amount = clipper.DisplayEnd-clipper.DisplayStart;
                    auto f_offset = clipper.DisplayStart;

                    auto& request = newRequests.emplace_back();
                    request.offset = f_offset;
                    request.limit = f_amount;

                    int maxIndex = -1;
                    int maxRecordsOverlapped = 0;
                    for (int i = 0; i < dbResults.size(); i++) {
                        auto const &dbResult = dbResults[i];

                        int db_start = std::max<int>(f_offset-dbResult.offset, 0);
                        int recordsOverlapped = std::min<int>(dbResult.records.size() - db_start, f_amount);

                        if (recordsOverlapped > maxRecordsOverlapped) {
                            maxRecordsOverlapped = recordsOverlapped;
                            maxIndex = i;
                        }
                    }

                    int post = f_amount;
                    if ( maxIndex >= 0 ) {
                        auto const &dbResult = dbResults[maxIndex];
                        int pre = std::max<int>(dbResult.offset - f_offset, 0);
                        int db_start = std::max<int>(f_offset-dbResult.offset, 0);
                        int db_end = db_start + std::min<int>(dbResult.records.size() - db_start, f_amount);
                        post = std::max<int>(f_amount - (db_end-db_start), 0);

                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5,0.5,0.5,1.0));
                        for (int i = 0; i < pre; i++) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text("");
                            ImGui::TableNextColumn();
                            ImGui::Text("LOADING...");
                            ImGui::TableNextColumn();
                            ImGui::Text("");
                        }
                        ImGui::PopStyleColor();

                        for (int i = db_start; i < db_end; i++) {
                            auto const &fileItem = dbResult.records[i];
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            if (ImGui::Selectable(std::to_string(fileItem.fileDataId).c_str(), lastSelectedFiledataId == fileItem.fileDataId,
                                                  ImGuiSelectableFlags_SpanAllColumns |
                                                  ImGuiSelectableFlags_AllowItemOverlap)) {
                                lastSelectedFiledataId = fileItem.fileDataId;
                                if (m_fileOpenCallback) {
                                    if (endsWith(fileItem.fileName, ".blp")) {
                                        m_fileOpenCallback(fileItem.fileDataId, "blp");
                                    }
                                }
                            }
                            ImGui::TableNextColumn();
                            ImGui::Text(fileItem.fileName.c_str());
                            ImGui::TableNextColumn();
                            ImGui::Text(fileItem.fileType.c_str());
                        }

                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5,0.5,0.5,1.0));
                        for (int i = 0; i < post; i++) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text("");
                            ImGui::TableNextColumn();
                            ImGui::Text("LOADING...");
                            ImGui::TableNextColumn();
                            ImGui::Text("");
                        }
                        ImGui::PopStyleColor();

                        if ((pre > 0) || (post > 0))
                            needRequest = true;

                        post = 0;
                    }

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5,0.5,0.5,1.0));
                    for (int i = 0; i < post; i++) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("");
                        ImGui::TableNextColumn();
                        ImGui::Text("LOADING...");
                        ImGui::TableNextColumn();
                        ImGui::Text("");
                    }
                    ImGui::PopStyleColor();

                    needRequest = needRequest || post > 0;
                }
            }
            if (needRequest)
                flInterface->makeRequest(newRequests);

            ImGui::EndTable();
        }
    }
    ImGui::End();

    return m_showWindow;
}

void FileListWindow::importCSV() {
    flInterface->importCSV();
}

