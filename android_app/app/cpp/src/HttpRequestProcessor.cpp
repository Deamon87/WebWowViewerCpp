//
// Created by deamon on 24.10.17.
//
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <jni.h>
#include <android_native_app_glue.h>
#include "HttpRequestProcessor.h"
#include "../../../../wowViewerLib/src/include/string_utils.h"

struct UserDataForRequest {
    std::string fileName;
    std::string url;
    CacheHolderType holderType;
    HttpRequestProcessorAndroid *processor;
    std::shared_ptr<PersistentFile> s_file;
};

//void downloadProgress(emscripten_fetch_t *fetch) {
//    UserDataForRequest *userDataForRequest = (UserDataForRequest *) fetch->userData;
//    if (userDataForRequest->fileContent == nullptr) {
//        userDataForRequest->fileContent = std::make_shared<FileContent>(fetch->numBytes);
//    }
//
//
//}

void downloadSucceeded(JNIEnv *env, jobject, jlong userDataPtr, jbyteArray array) {
//    printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
//
//    try {
//        std::cout << "Creating FileContent with " << fetch->numBytes << " bytes" << std::endl;
        jbyte* bufferPtr = env->GetByteArrayElements(array, nullptr);
        jsize lengthOfArray = env->GetArrayLength(array);

        HFileContent fileContent = std::make_shared<FileContent>(lengthOfArray);
//        std::cout << "FileContent was created" << std::endl;
        std::copy(bufferPtr, bufferPtr + lengthOfArray, fileContent->begin());

        env->ReleaseByteArrayElements(array, bufferPtr, JNI_ABORT);


//    HFileContent fileContent(&fetch->data[0], &fetch->data[fetch->numBytes]);
        UserDataForRequest *userDataForRequest = (UserDataForRequest *) userDataPtr;

        if (userDataForRequest->holderType != CacheHolderType::CACHE_ANIM && fileContent->size() > 4 &&
            (*(uint32_t *) fileContent->data() == 0)) {
            std::cout << "Encountered encrypted file " << std::string(userDataForRequest->url) << std::endl;
            return;
        }

        userDataForRequest->s_file->process(fileContent, userDataForRequest->fileName);
        userDataForRequest->processor->toBeProcessed--;

        // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
        delete userDataForRequest;
        //    } catch (...) {
//        printf("Exception on Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
//    }

}

void downloadFailed(JNIEnv *, jobject, jlong userDataPtr) {
    UserDataForRequest * userDataForRequest = (UserDataForRequest *)userDataPtr;
    userDataForRequest->processor->toBeProcessed--;
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_deamon_wow_viewer_MainActivity_downloadSucceeded(JNIEnv *env,
                                                                                     jobject thiz,
                                                                             jlong userDataPtr,
                                                                             jbyteArray array) {
        downloadSucceeded(env, thiz, userDataPtr, array);
    }

    JNIEXPORT void JNICALL Java_com_deamon_wow_viewer_MainActivity_downloadFailed(JNIEnv *env, jobject thiz,
                                                                jlong userDataPtr) {
        downloadFailed(env, thiz, userDataPtr);
    }
}


void HttpRequestProcessorAndroid::processFileRequest(std::string &fileName, CacheHolderType holderType, std::weak_ptr<PersistentFile> s_file) {
    auto perstFile = s_file.lock();
    if (perstFile == nullptr){
        toBeProcessed--;
        return;
    }
//    std::cout << "processFileRequest : filename = " << fileName << std::endl;

    const std::string charsToEscape = " !*'();:@&=+$,/?#[]";

    std::string escapedFileName = fileName;
    for (int i = 0; i < charsToEscape.size(); i++) {
        char c = charsToEscape[i];
        escapedFileName = ReplaceAll(escapedFileName, std::string(1, c), url_char_to_escape(c));
    }

    std::string fullUrl;
    if (escapedFileName.find("File") == 0) {
        std::stringstream ss;
        std::string fileDataIdHex = escapedFileName.substr(4, escapedFileName.find(".")-4);
        uint32_t fileDataId;
        ss << std::hex << fileDataIdHex;
        ss >> fileDataId;

        if (fileDataId == 0) {
            toBeProcessed--;
            return;
        }

        fullUrl = m_urlBaseFileId + std::to_string(fileDataId);
    } else {
        fullUrl = m_urlBase + escapedFileName;
    }

//    fullUrl = urlEncode(fullUrl);

    UserDataForRequest * userDataForRequest = new UserDataForRequest();
    userDataForRequest->fileName = fileName;
    userDataForRequest->url = fullUrl;
    userDataForRequest->holderType = holderType;
    userDataForRequest->processor = this;
    userDataForRequest->s_file = perstFile;

    JNIEnv* java_env = NULL;

    JavaVM* java_vm = m_app->activity->vm;
    jint jni_return = java_vm->GetEnv((void**)&java_env, JNI_VERSION_1_6);
    if (jni_return == JNI_ERR)
        return ;

    jni_return = java_vm->AttachCurrentThread(&java_env, NULL);
    if (jni_return != JNI_OK)
        return ;

    jclass native_activity_clazz = java_env->GetObjectClass(m_app->activity->clazz);
    if (native_activity_clazz == NULL)
        return;

    jmethodID method_id = java_env->GetMethodID(native_activity_clazz, "downloadFile", "(Ljava/lang/String;J)V");
    if (method_id == NULL)
        return;

    jstring jurl = java_env->NewStringUTF(fullUrl.c_str());
    java_env->CallVoidMethod(m_app->activity->clazz, method_id, jurl, userDataForRequest);

    jni_return = java_vm->DetachCurrentThread();
    if (jni_return != JNI_OK)
        return;

}
