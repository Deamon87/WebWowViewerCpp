//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_CCHUNKFILEREADER_H
#define WOWVIEWERLIB_CCHUNKFILEREADER_H

#include <functional>
#include <map>

class ChunkData {
    ChunkData (){
        chunkIdent = 0;
        chunkLen = 0;
        currentOffset = 0;

        chunkData = nullptr;
        fileData = nullptr;
    }
    unsigned int chunkIdent;
    int chunkLen;

    int currentOffset;
    void *chunkData;
    void *fileData;

public:
    template<typename T> inline void readValue(T* &value) {
        static_assert(!std::is_pointer<T>::value, "T is a pointer");
        value = (T*)&(((unsigned char *)chunkData)[currentOffset]);
        currentOffset += sizeof(T);
    }
    template<typename T> inline void readValues(T* &value, int count) {
            static_assert(!std::is_pointer<T>::value, "T is a pointer");
            value = (T*)&(((unsigned char *)chunkData)[currentOffset]);
            currentOffset += count*sizeof(T);
    }
};

template <typename T>
struct chunkDef {
    std::function<void (T&, ChunkData&)> handler;
    std::map<unsigned int,chunkDef> subChunks;
};


//

//chunkData_t getChunkAtOffset(int offset, void* fileData, int size) {
//    chunkData_t chunkData;
//    chunkData.currentOffset = offset;
//    readValue(chunkData, chunkData.chunkIdent);
//    readValue(chunkData, chunkData.chunkLen);
//    if (chunkData.chunkLen == 0) {
//        chunkData.chunkLen = size;
//    }
//    chunkData.fileData = fileData;
//    chunkData.chunkData = fileData + chunkData.currentOffset;
//    return chunkData;
//}
//chunkData_t getNextSubChunk(chunkData_t &chunkData, int size) {
//    chunkData_t subChunkData;
//    subChunkData.currentOffset = chunkData.currentOffset;
//    readValue(subChunkData, subChunkData.chunkIdent);
//    readValue(subChunkData, subChunkData.chunkLen);
//    if (subChunkData.chunkLen == 0) {
//        subChunkData.chunkLen = size;
//    }
//    subChunkData.chunkData = subChunkData.fileData + subChunkData.currentOffset;
//}


#endif //WOWVIEWERLIB_CCHUNKFILEREADER_H
