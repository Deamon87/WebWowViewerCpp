//
// Created by Deamon on 11/22/2017.
//

#include <algorithm>
#include <sstream>
#include <iterator>
#include "CascRequestProcessor.h"
#include "../database/buildInfoParser/buildDefinition.h"
#include "../../3rdparty/TactCpp/TactCppLib/Error.h"
#include "../../3rdparty/TactCpp/TactCppLib/BuildInstance.h"
#include "../../3rdparty/TactCpp/TactCppLib/utils/KeyService.h"
#include "../../3rdparty/filesystem_impl/include/ghc/filesystem.hpp"


static inline RootWoW::LocaleFlags GetLocaleFlag(const std::string &szTag) {
    // Build a little‑endian 32‑bit integer from the four chars
    uint32_t lang = 0;
    if (szTag.size() >= 4) {
        lang |= (static_cast<uint8_t>(szTag[0]) << 24);
        lang |= (static_cast<uint8_t>(szTag[1]) << 16);
        lang |= (static_cast<uint8_t>(szTag[2]) << 8);
        lang |= (static_cast<uint8_t>(szTag[3]));
    }

    // Map the integer to a flag
    switch (lang) {
        case 0x656e5553: return RootWoW::LocaleFlags::enUS; // "enus"
        case 0x656e4742: return RootWoW::LocaleFlags::enGB; // "engb"
        case 0x656e434e: return RootWoW::LocaleFlags::enCN; // "encn"
        case 0x656e5457: return RootWoW::LocaleFlags::enTW; // "entw"
        case 0x65734553: return RootWoW::LocaleFlags::esES; // "eses"
        case 0x65734d58: return RootWoW::LocaleFlags::esMX; // "esmX"
        case 0x70744252: return RootWoW::LocaleFlags::ptBR; // "ptbr"
        case 0x70745054: return RootWoW::LocaleFlags::ptPT; // "ptpt"
        case 0x7a68434e: return RootWoW::LocaleFlags::zhCN; // "zhnC"
        case 0x7a685457: return RootWoW::LocaleFlags::zhTW; // "zhTw"
        case 0x6b6f4b52: return RootWoW::LocaleFlags::koKR; // "kokr"
        case 0x66724652: return RootWoW::LocaleFlags::frFR; // "frfr"
        case 0x64654445: return RootWoW::LocaleFlags::deDE; // "dede"
        case 0x72755255: return RootWoW::LocaleFlags::ruRU; // "ruru"
        case 0x69744954: return RootWoW::LocaleFlags::itIT; // "itti"

        default:
            return RootWoW::LocaleFlags::enUS; // Default language
    }
}

CascRequestProcessor::CascRequestProcessor(const std::string &path, const BuildDefinition &buildDef,
                                           StorageErrorCallback errorCallback) : m_errorCallback(
    std::move(errorCallback)) {
    Settings settings;
    if (path.size() != 0)
        settings.BaseDir = path;

    settings.BuildConfigPathOrHash = buildDef.buildConfigHash;
    settings.CDNConfigPathOrHash = buildDef.cdnConfigHash;
    settings.CDNPath = buildDef.cdnPath;
    settings.CDNServersOverride = buildDef.cdnServersOverride;
    settings.Product = buildDef.productName;
    settings.Locale = GetLocaleFlag(buildDef.installedLanguage);
    settings.ArmadilloKey = buildDef.armadilloKey;
    settings.Region = buildDef.region;

    settings.allowOnlineDownload = buildDef.allowOnlineDownload;
    settings.preferLowViolence = buildDef.preferLowViolence;
    settings.preferHiResTextures = buildDef.preferHiResTextures;

    settings.useTactLocal = buildDef.useTactLocal;
    if (buildDef.useTactLocal && !buildDef.localTactPath.empty()) {
        settings.LocalTactPath = std::filesystem::path(buildDef.localTactPath);
    }

    if (settings.Product.empty())
        settings.Product = "wow";

    std::shared_ptr<BuildInstance> buildInstance;
    try {
        buildInstance = std::make_shared<BuildInstance>(settings);
        buildInstance->Load();
    } catch (const std::exception &error) {
        if (!m_errorCallback) throw;
        m_errorCallback({StorageErrorCategory::StorageOpen, std::string("Opening storage: ") + error.what()});
        return;
    } catch (...) {
        if (!m_errorCallback) throw;
        m_errorCallback({StorageErrorCategory::StorageOpen, "Opening storage: unknown error"});
        return;
    }
    m_isOpen = true;

    readFileContentLambda = [buildInstance](const std::string &fileName, uint32_t fileDataId) {
        auto data = fileDataId > 0
                        ? buildInstance->OpenFileByFDID(fileDataId)
                        : buildInstance->OpenFileByName(fileName);
        return data.empty() ? HFileContent{} : std::make_shared<FileContent>(std::move(data));
    };

    iterateFilesLambda = [buildInstance, this](
        std::function<bool (int fileDataId, const std::string &fileName)> &process,
        std::function<void (int fileDataId, const HFileContent &fileData)> &callback
    ) {
        auto FDIDs = buildInstance->GetRoot()->GetAvailableFDIDs();
        for (auto fdid: FDIDs) {
            std::string fileName = "File" + std::to_string(fdid) + ".unk";

            if (!process(fdid, fileName)) continue;

            auto fileContent = this->readFileContent(fileName, fdid);
            if (fileContent) {
                callback(fdid, fileContent);
            }
        }
    };
}

static const std::string cacheDirectory = "./file_edits/";

HFileContent CascRequestProcessor::tryGetFileFromOverrides(int fileDataId) {
    std::string inputFileName = cacheDirectory + std::to_string(fileDataId);
    std::ifstream cache_file(inputFileName, std::ios::in | std::ios::binary);
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

void CascRequestProcessor::processFileRequest(CacheHolderType holderType, const std::weak_ptr<PersistentFile> &s_file) {
    auto perstFile = s_file.lock();

    if (perstFile == nullptr) {
        //        std::cout << "perstFile is expired" << std::endl;
        toBeProcessed--;
        return;
    }

    uint32_t fileDataId = perstFile->getFileDataId();
    std::string fileName = perstFile->getFileName();

    HFileContent fileContent = readFileContent(fileName, fileDataId);

    if (!(fileContent == nullptr)) {
        toBeProcessed--;
        processResult(perstFile, fileContent);
    } else {
        if (fileDataId > 0) {
            std::cout << "Could read fileDataId " << fileDataId << std::endl << std::flush;
        } else {
            std::cout << "Could read file " << fileName << std::endl << std::flush;
        }
        toBeProcessed--;
        perstFile->setRejected();
        //        this->m_fileRequester->rejectFile(holderType, fileName.c_str());
    }
}

HFileContent CascRequestProcessor::readFileContent(const std::string &fileName, uint32_t fileDataId) {
    std::string fileNameFixed = fileName;
    std::replace(fileNameFixed.begin(), fileNameFixed.end(), '/', '\\');

    HFileContent fileContent = nullptr;
    try {
        fileContent = tryGetFileFromOverrides(fileDataId);
        if (fileContent != nullptr) return fileContent;
        if (readFileContentLambda)
            fileContent = readFileContentLambda(fileName, fileDataId);
    } catch (const std::exception &e) {
        auto category = StorageErrorCategory::FileRead;
        if (auto tactError = dynamic_cast<const TACT::Exception *>(&e)) {
            switch (tactError->category) {
                case TACT::ErrorCategory::FileNotFound:
                    category = StorageErrorCategory::FileNotFound;
                    break;
                case TACT::ErrorCategory::MissingKey:
                    category = StorageErrorCategory::MissingKey;
                    break;
                case TACT::ErrorCategory::FileRead: break;
            }
        }
        const auto context = fileDataId > 0 ? "FileDataId " + std::to_string(fileDataId) : fileName;
        if (m_errorCallback)
            m_errorCallback({category, context + ": " + e.what()});

        std::cerr << "Failed to load file: " << e.what() << std::endl << std::flush;
    } catch (...) {
        const auto context = fileDataId > 0 ? "FileDataId " + std::to_string(fileDataId) : fileName;
        if (m_errorCallback) m_errorCallback({StorageErrorCategory::FileRead, context + ": unknown error"});
        std::cerr << "Failed to load file." << std::endl << std::flush;
    }

    if (fileContent != nullptr)
        return fileContent;

    if (fileDataId > 0) {
        std::cout << "Could not read fileDataId " << fileDataId << " from local " << std::endl << std::flush;
    } else {
        std::cout << "Could not read file " << fileName << " from local " << std::endl << std::flush;
    }

    return nullptr;
}

void CascRequestProcessor::iterateFilesInternal(
    std::function<bool (int fileDataId, const std::string &fileName)> &process,
    std::function<void (int fileDataId, const HFileContent &fileData)> &callback) {
    if (iterateFilesLambda) iterateFilesLambda(process, callback);
}

CascRequestProcessor::~CascRequestProcessor() {
}

void CascRequestProcessor::updateKeys() {
    KeyService::LoadKeys();
}
