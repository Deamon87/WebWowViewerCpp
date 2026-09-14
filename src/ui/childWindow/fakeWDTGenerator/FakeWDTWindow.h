//
// Created by Deamon on 5/20/2025.
//

#ifndef FAKEWDTWINDOW_H
#define FAKEWDTWINDOW_H

#include <vector>
#include <array>
#include "../../../../wowViewerLib/src/engine/WowFilesCacheStorage.h"


class FakeWDTWindow {
public:
    FakeWDTWindow(HApiContainer &api);

    void doInitialization();

    bool draw();
private:
    HApiContainer m_api;
    std::shared_ptr<WdtFile> m_filePtr;

    bool m_initialized = false;

    //
    std::vector<std::array<int, 5>> m_files;
    int m_currentFile = 0;
    std::shared_ptr<AdtFile> m_adtFile = nullptr;
};



#endif //FAKEWDTWINDOW_H
