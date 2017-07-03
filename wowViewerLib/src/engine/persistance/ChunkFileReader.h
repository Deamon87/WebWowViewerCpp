//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_CCHUNKFILEREADER_H
#define WOWVIEWERLIB_CCHUNKFILEREADER_H

struct chunkData_t {
    chunkData_t (){
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
};



template<typename T>
void readValue(chunkData_t chunkData, T &value) {
    value = chunkData.chunkData[chunkData.currentOffset];
    chunkData.currentOffset += sizeof(T);
}
chunkData_t getChunkAtOffset(int offset, void* fileData, int size) {
    chunkData_t chunkData;
    chunkData.currentOffset = offset;
    readValue(chunkData, chunkData.chunkIdent);
    readValue(chunkData, chunkData.chunkLen);
    if (chunkData.chunkLen == 0) {
        chunkData.chunkLen = size;
    }
    chunkData.fileData = fileData;
    chunkData.chunkData = fileData + chunkData.currentOffset;
    return chunkData;
}
chunkData_t getNextSubChunk(chunkData_t &chunkData, int size) {
    chunkData_t subChunkData;
    subChunkData.currentOffset = chunkData.currentOffset;
    readValue(subChunkData, subChunkData.chunkIdent);
    readValue(subChunkData, subChunkData.chunkLen);
    if (subChunkData.chunkLen == 0) {
        subChunkData.chunkLen = size;
    }
    subChunkData.chunkData = subChunkData.fileData + subChunkData.currentOffset;
}


#endif //WOWVIEWERLIB_CCHUNKFILEREADER_H
