//
// Created by Deamon on 7/18/2023.
//

#include "KeysUpdateWorkflow.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "../../../../3rdparty/filesystem_impl/include/ghc/filesystem.hpp"
#include "../../../persistance/httpFile/httpFile.h"
#include "../../../persistance/CascRequestProcessor.h"

static const std::string POPUP_DOWNLOADING_KEYS = "Downloading key from git";
static const std::string POPUP_DOWNLOADING_FAILED = "Updating keys failed";
static const std::string POPUP_DOWNLOADING_SUCCESS = "Updating keys success";

KeysUpdateWorkflow::KeysUpdateWorkflow(const std::shared_ptr<CascRequestProcessor> &cascRequestProcessor) : m_cascRequestProcessor(cascRequestProcessor) {

}

bool KeysUpdateWorkflow::render() {
    if (m_showDownloading) {
        ImGui::OpenPopup(POPUP_DOWNLOADING_KEYS.c_str());
    } else if(m_showDownloadingSuccess) {
        ImGui::OpenPopup(POPUP_DOWNLOADING_SUCCESS.c_str());
    } else if (m_showDownloadingFailed) {
        ImGui::OpenPopup(POPUP_DOWNLOADING_FAILED.c_str());
    }

    return defineDialogs();
}

bool KeysUpdateWorkflow::defineDialogs() {
    bool return_value = true;
    if (ImGui::BeginPopupModal(POPUP_DOWNLOADING_KEYS.c_str()))
    {
        keyThreadDownloadLogic();
        if (m_failedToDownloadFile) {
            m_showDownloading = false;
            m_showDownloadingFailed = true;
        } else if (m_httpDownloadThreadFinished && !m_failedToDownloadFile) {
            m_showDownloading = false;
            m_showDownloadingSuccess = true;
            if (m_cascRequestProcessor)
                m_cascRequestProcessor->updateKeys();
        }

        ImGui::Text("Downloading keys from git");
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal(POPUP_DOWNLOADING_SUCCESS.c_str())) {
        ImGui::Text("Keys were updated succesfully");
        if (ImGui::Button("Ok", ImVec2(-1, 23))) {
            return_value = false;
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal(POPUP_DOWNLOADING_FAILED.c_str())) {
        ImGui::Text("Failed to download keys");
        if (ImGui::Button("Ok", ImVec2(-1, 23))) {
            return_value = false;
        }
        ImGui::EndPopup();
    }

    return return_value;
}

void KeysUpdateWorkflow::keyThreadDownloadLogic() {
    if (httpDownloadThread == nullptr) {
        auto &l_httpDownloadThreadFinished = m_httpDownloadThreadFinished;
        auto &l_failedToDownloadFile = m_failedToDownloadFile;
        httpDownloadThread = std::make_shared<std::thread>([&l_httpDownloadThreadFinished, &l_failedToDownloadFile]() -> void {
            std::string fileToDownload = "https://raw.githubusercontent.com/wowdev/TACTKeys/master/WoW.txt";
            HttpFile httpFile = HttpFile(fileToDownload);
            httpFile.setCallback([](HFileContent fileContent) -> void {
                std::string outputFileName = CASC_KEYS_FILE;

                std::ofstream FILE(outputFileName, std::ios::out | std::ios::binary);
                std::copy(fileContent->begin(), fileContent->end(), std::ostreambuf_iterator<char>(FILE));
                FILE.flush();
                FILE.close();
            });

            httpFile.setFailCallback([fileToDownload, &l_failedToDownloadFile](HFileContent fileContent) -> void {
                l_failedToDownloadFile = true;
            });
            httpFile.startDownloading();
            l_httpDownloadThreadFinished = true;
        });
    }
    if (m_httpDownloadThreadFinished || m_failedToDownloadFile) {
        httpDownloadThread->detach();
        httpDownloadThread = nullptr;
    }
}
