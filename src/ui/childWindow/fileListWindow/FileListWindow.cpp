//
// Created by Deamon on 10/28/2023.
//

#include "FileListWindow.h"
#include "../../../database/csvtest/csv.h"

FileListWindow::FileListWindow(const HApiContainer &api) : m_api(api), m_storage(FileListDB::makeStorage("fileList.db3")) {
    m_filesTotal = 0;
    m_showWindow = true;

    m_storage.pragma.synchronous(0);
    m_storage.pragma.journal_mode(sqlite_orm::journal_mode::MEMORY);

    m_storage.sync_schema();

    m_filesTotal = m_storage.count<FileListDB::FileRecord>();

    makeNewSelectStatement();
}

bool FileListWindow::draw() {
    ImGui::Begin("File list", &m_showWindow);
    {
        if (ImGui::Button("Import filelist.csv...")) {
            importCSV();
        }
        ImGui::SameLine();
        if (ImGui::Button("Scan repository...")) {

        }
        if (ImGui::InputText("Filter: ", filterText.data(), filterText.size()-1)) {
            filterText[filterText.size()-1] = 0;
            filterTextStr = filterText.data();
            filterTextStr = "%"+filterTextStr+"%";
            m_fileRecCache.clear();
            makeNewSelectStatement();
        }

        if (ImGui::BeginTable("FileListTable", 3, ImGuiTableFlags_Resizable |
                                              ImGuiTableFlags_Reorderable |
                                              ImGuiTableFlags_Sortable |
                                              ImGuiTableFlags_NoHostExtendX |
                                              ImGuiTableFlags_NoHostExtendY |
                                              ImGuiTableFlags_ScrollX, ImVec2(-1, -1))) {

            ImGui::TableSetupColumn("FileDataId", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("FileName", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("FileType", ImGuiTableColumnFlags_None);
            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
            ImGui::TableHeadersRow();

            FileRecordCache newCache = FileRecordCache();
            {
                ImGuiListClipper clipper;
                clipper.Begin(m_filesTotal);
                while (clipper.Step()) {
                    auto f_amount = clipper.DisplayEnd-clipper.DisplayStart;
                    auto f_offset = clipper.DisplayStart;

                    std::vector<FileListDB::FileRecord> fileRecords;

                    auto iter = m_fileRecCache.find({f_amount, f_offset});
                    if (iter != m_fileRecCache.end()) {
                        fileRecords = iter->second;
                    } else {
                        fileRecords = selectStatement->getFiles(f_offset, f_amount);
                    }
                    newCache[{f_amount, f_offset}] = fileRecords;

                    for (int i = 0; i < fileRecords.size(); i++) {
                        auto const &fileItem = fileRecords[i];
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", fileItem.fileDataId);
                        ImGui::TableNextColumn();
                        ImGui::Text(fileItem.fileName.c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(fileItem.fileType.c_str());
                    }
                }
            }
            m_fileRecCache = newCache;
            ImGui::EndTable();
        }
    }
    ImGui::End();

    return m_showWindow;
}

void FileListWindow::importCSV() {
    auto csv = new io::CSVReader<2, io::trim_chars<' '>, io::no_quote_escape<';'>>("listfile.csv");

    int currentFileDataId;
    std::string currentFileName;

    using namespace sqlite_orm;
    auto statement = m_storage.prepare(
        select(
            columns(&FileListDB::FileRecord::fileType), from<FileListDB::FileRecord>(),
            where(is_equal(&FileListDB::FileRecord::fileDataId, std::ref(currentFileDataId)))
        )
    );

    while (csv->read_row(currentFileDataId, currentFileName)) {
        auto typeFromDB = m_storage.execute(statement);

        std::string fileType = "";
        if (!typeFromDB.empty()) {
            fileType = get<0>(typeFromDB[0]);
        }

        m_storage.replace(FileListDB::FileRecord({currentFileDataId, currentFileName, fileType}));
    }

    m_filesTotal = m_storage.count<FileListDB::FileRecord>();

    delete csv;
}

static auto createStatement(decltype(FileListDB::makeStorage("")) &storage, const std::string &searchClause, int &recordsTotal) {
    using namespace sqlite_orm;

    recordsTotal = 0;

    auto whereClause = where(
        or_(
            like(&FileListDB::FileRecord::fileName, searchClause),
            like(&FileListDB::FileRecord::fileDataId, searchClause)
        )
    );

    recordsTotal = storage.count<FileListDB::FileRecord>(whereClause);

    auto statement = storage.prepare(
        get_all<FileListDB::FileRecord>(
            whereClause,
            order_by(&FileListDB::FileRecord::fileDataId),
            limit(2, offset(3))
        )
    );

    //std::cout << statement.sql() << std::endl;
//    std::cout << get<0>(statement) << std::endl;
//    std::cout << get<1>(statement) << std::endl;
//    std::cout << get<2>(statement) << std::endl;
//    std::cout << get<3>(statement) << std::endl;

    return statement;
}

class FileListLambdaInst : public FileListLamda {
public:
    FileListLambdaInst(decltype(FileListDB::makeStorage("")) &storage, std::string &searchClause, int &recordsTotal) : m_storage(storage),
        m_recordsTotal(recordsTotal), m_statement(createStatement(storage, searchClause, recordsTotal))
    {

    }

    std::vector<FileListDB::FileRecord> getFiles(int offset, int limit) override {
        using namespace sqlite_orm;

        get<2>(m_statement) = limit;
        get<3>(m_statement) = offset;

        return m_storage.execute(m_statement);
    };
private:
    int &m_recordsTotal;
    decltype(FileListDB::makeStorage("")) &m_storage;
    decltype(createStatement(m_storage, "", m_recordsTotal)) m_statement;
};

void FileListWindow::makeNewSelectStatement() {

    selectStatement = std::make_unique<FileListLambdaInst>(m_storage, filterTextStr, m_filesTotal);
}
