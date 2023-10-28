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

            {
                ImGuiListClipper clipper;
                clipper.Begin(m_filesTotal);
                while (clipper.Step()) {
                    auto f_amount = clipper.DisplayEnd-clipper.DisplayStart;
                    auto f_offset = clipper.DisplayStart;

                    using namespace sqlite_orm;
                    auto items = m_storage.get_all<FileListDB::FileRecord>(
                        order_by(&FileListDB::FileRecord::fileDataId), limit(f_offset, f_amount));

                    for (int i = 0; i < items.size(); i++) {
                        auto const &fileItem = items[i];
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
