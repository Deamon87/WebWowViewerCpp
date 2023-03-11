//
// Created by Deamon on 11/22/2017.
//

#include <algorithm>
#include <sstream>
#include <iterator>
#include "CascRequestProcessor.h"
#include "../../3rdparty/casclib/src/CascLib.h"
#include "../../3rdparty/filesystem_impl/include/ghc/filesystem.hpp"


static DWORD GetLocaleValue_Copy(const std::string &szTag)
{
    DWORD Language = 0;

    // Convert the string language to integer
    Language = (Language << 0x08) | szTag[0];
    Language = (Language << 0x08) | szTag[1];
    Language = (Language << 0x08) | szTag[2];
    Language = (Language << 0x08) | szTag[3];

    // Language-specific action
    switch(Language)
    {
        case 0x656e5553: return CASC_LOCALE_ENUS;
        case 0x656e4742: return CASC_LOCALE_ENGB;
        case 0x656e434e: return CASC_LOCALE_ENCN;
        case 0x656e5457: return CASC_LOCALE_ENTW;
        case 0x65734553: return CASC_LOCALE_ESES;
        case 0x65734d58: return CASC_LOCALE_ESMX;
        case 0x70744252: return CASC_LOCALE_PTBR;
        case 0x70745054: return CASC_LOCALE_PTPT;
        case 0x7a68434e: return CASC_LOCALE_ZHCN;
        case 0x7a685457: return CASC_LOCALE_ZHTW;
        case 0x6b6f4b52: return CASC_LOCALE_KOKR;
        case 0x66724652: return CASC_LOCALE_FRFR;
        case 0x64654445: return CASC_LOCALE_DEDE;
        case 0x72755255: return CASC_LOCALE_RURU;
        case 0x69744954: return CASC_LOCALE_ITIT;
    }

    return 0;
}

CascRequestProcessor::CascRequestProcessor(std::string &path, BuildDefinition &buildDef) {

    m_cascDir = path;
    if (!buildDef.productName.empty()) {
        path = path + "*"+buildDef.productName+"**"+buildDef.region;
    }

    uint32_t localMask = 0xFFFFFFFF;
    if (!buildDef.installedLanguage.empty()) {
        auto const &lan = buildDef.installedLanguage;
        if (lan.length() >= 4)
            localMask = GetLocaleValue_Copy(lan);
    }

    bool openResult = false;
    bool openOnlineResult = false;
    openResult = CascOpenStorage(path.c_str(), localMask, &this->m_storage);
    if (openResult) {
        std::cout << "Opened local Casc Storage at "<< path << std::endl;
    } else {
        std::cout << "Could not open local Casc Storage at "<< path << std::endl;
    }

    //Disable online feature for now
    /*
    if (!buildDef.buildConfig.empty()) {
        auto cacheDir = (ghc::filesystem::current_path() / "casc_cache");
        if (!ghc::filesystem::is_directory(cacheDir) || !ghc::filesystem::exists(cacheDir)) {
            ghc::filesystem::create_directory(cacheDir); // create cache folder
        }
        std::string cacheDirStr = cacheDir;
        if (!buildDef.productName.empty())
            cacheDirStr = cacheDir / (":"+buildDef.productName+":"+buildDef.region+":"+buildDef.buildConfig);

        CASC_OPEN_STORAGE_ARGS openArgs;
        openArgs.Size = sizeof(CASC_OPEN_STORAGE_ARGS);
        openArgs.szLocalPath = nullptr;
        openArgs.szCodeName = nullptr; //buildDef.productName.c_str();
        openArgs.szRegion = nullptr; //buildDef.region.c_str();
        openArgs.PfnProgressCallback = nullptr;
        openArgs.PtrProgressParam = nullptr;
        openArgs.PfnProductCallback = nullptr;
        openArgs.PtrProductParam = nullptr;
        openArgs.dwLocaleMask = 0;
        openArgs.dwFlags = 0;
        openArgs.szBuildKey = nullptr;//buildDef.buildConfig.c_str();

        openOnlineResult = CascOpenStorageEx(cacheDirStr.c_str(), &openArgs, true, &this->m_storageOnline);
        if (openOnlineResult) {
            std::cout << "Opened ONLINE CascStorage at "<< cacheDirStr << std::endl;
        } else {
            std::cout << "Could not open ONLINE CascStorage at "<< cacheDirStr << std::endl;
        }
    }
    */

    if (!openResult && !openOnlineResult) {
        this->m_storage = nullptr;
        this->m_storageOnline = nullptr;
        throw "Could not open CascStorage";
    }
}

HFileContent CascRequestProcessor::tryGetFile(void *cascStorage, void *fileNameToPass, uint32_t openFlags){
    HANDLE fileHandle;
    HFileContent fileContent = nullptr;
    bool fileOpened = false;
    if (CascOpenFile(cascStorage, fileNameToPass, 0xFFFFFFFF, openFlags, &fileHandle)) {
        DWORD fileSize1 = CascGetFileSize(fileHandle, nullptr);
        if (fileSize1 != CASC_INVALID_SIZE) {
            fileOpened = true;
            fileContent = std::make_shared<FileContent>(FileContent(fileSize1 + 1));
            auto dataPtr = fileContent->data();

            DWORD totalBytesRead = 0;
            while (totalBytesRead < fileSize1) {
                DWORD dwBytesRead = 0;

                if (!CascReadFile(fileHandle, &dataPtr[totalBytesRead], fileSize1 - totalBytesRead, &dwBytesRead)) {
                    return nullptr;
                }

                totalBytesRead += dwBytesRead;
            }
        }
    }

    return fileContent;
}

static const std::string cacheDirectory = "./file_edits/";
HFileContent CascRequestProcessor::tryGetFileFromOverrides(int fileDataId) {

    std::string inputFileName = cacheDirectory + std::to_string(fileDataId);
    std::ifstream cache_file(inputFileName, std::ios::in |std::ios::binary);
    if (cache_file.good()) {
        cache_file.unsetf(std::ios::skipws);

        // get its size:
        std::streampos fileSize;

        cache_file.seekg(0, std::ios::end);
        fileSize = cache_file.tellg();
        cache_file.seekg(0, std::ios::beg);


        HFileContent vec = std::make_shared<FileContent>(fileSize);
        cache_file.read((char *) vec->data(), fileSize);

        // read the data:
        std::copy(std::istream_iterator<unsigned char>(cache_file),
                  std::istream_iterator<unsigned char>(),
                  std::back_inserter(*vec.get()));

        std::cout << "Loaded fdid = " << fileDataId << " from " << inputFileName << std::endl;

        return vec;
    }
    return nullptr;
}


void CascRequestProcessor::processFileRequest(std::string &fileName, CacheHolderType holderType, std::weak_ptr<PersistentFile> s_file) {
    auto perstFile = s_file.lock();
    uint32_t fileDataId = 0;
    if (fileName.find("File") == 0) {
        std::stringstream ss;
        std::string fileDataIdHex = fileName.substr(4, fileName.find(".") - 4);

        ss << std::hex << fileDataIdHex;
        ss >> fileDataId;
    }

    if (perstFile == nullptr) {
        std::cout << "perstFile for " << fileName << "(fileDataId = "<<fileDataId<<" ) is expired" << std::endl;
        toBeProcessed--;
        return;
    }

    std::string fileNameFixed = fileName;
    std::replace( fileNameFixed.begin(), fileNameFixed.end(), '/', '\\');

    void *fileNameToPass = (void *) fileNameFixed.c_str();
    DWORD openFlags = CASC_OPEN_BY_NAME;
    if (fileNameFixed.find("File") == 0) {
        if (fileDataId == 0) {
            return;
        }

        fileNameToPass = reinterpret_cast<void *>(fileDataId);
        openFlags = CASC_OPEN_BY_FILEID;
    }

    openFlags |= CASC_OVERCOME_ENCRYPTED;

    HFileContent fileContent;

    fileContent = this->tryGetFileFromOverrides(fileDataId);
    if (this->m_storage != nullptr && fileContent == nullptr) {
        fileContent = this->tryGetFile(this->m_storage, fileNameToPass, openFlags);
        if (fileContent == nullptr) {
            if (fileDataId > 0) {
                std::cout << "Could read fileDataId " << fileDataId << " from local " << std::endl << std::flush;
            } else {
                std::cout << "Could read file " << fileName << " from local " << std::endl << std::flush;
            }
        }
    }
    if (this->m_storageOnline != nullptr && fileContent == nullptr) {
        fileContent = this->tryGetFile(this->m_storageOnline, fileNameToPass, openFlags);
        if (fileContent == nullptr) {
            if (fileDataId > 0) {
                std::cout << "Could read fileDataId " << fileDataId << " from online " << std::endl << std::flush;
            } else {
                std::cout << "Could read file " << fileName << " from online " << std::endl << std::flush;
            }
        }
    }

    if (!(fileContent == nullptr)) {
        toBeProcessed--;
        processResult(perstFile, fileContent, fileName);
    } else {
        if (fileDataId > 0) {
            std::cout << "Could read fileDataId " << fileDataId << std::endl << std::flush;
        } else {
            std::cout << "Could read file " << fileName << std::endl << std::flush;
        }
        toBeProcessed--;
        this->m_fileRequester->rejectFile(holderType, fileName.c_str());
    }
}

CascRequestProcessor::~CascRequestProcessor() {
    if (m_storage != nullptr)
        CascCloseStorage(m_storage);
    if (m_storageOnline != nullptr)
        CascCloseStorage(m_storageOnline);
}

