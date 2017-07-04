//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_CCHUNKFILEREADER_H
#define WOWVIEWERLIB_CCHUNKFILEREADER_H

#include <functional>
#include <map>

class ChunkData {
public:
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
    int dataOffset;
    void *chunkData;
    void *fileData;

public:
    template<typename T> inline void readValue(T &value) {
        static_assert(!std::is_pointer<T>::value, "T is a pointer");
        value = *(T*)&(((unsigned char *)chunkData)[currentOffset]);
        currentOffset += sizeof(T);
    }
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
template <typename T>
class CChunkFileReader {
private:
    std::vector<unsigned char> &m_file;
    chunkDef<T> *m_sectionReaders;
public:
    CChunkFileReader(std::vector<unsigned char> &file, chunkDef<T> *sectionReaders) : m_file(file), m_sectionReaders(sectionReaders){
    }

    unsigned long getFileSize () {
        return m_file.size();
    }

    void processChunkAtOffsWithSize (int &offs, int size, T &resultObj, chunkDef<T> *parentSectionHandlerProc) {
        ChunkData chunk = this->loadChunkAtOffset(offs, size);

        chunkDef<T> *sectionHandlerProc;
        if (parentSectionHandlerProc != nullptr) {
            sectionHandlerProc = &parentSectionHandlerProc->subChunks[chunk.chunkIdent];
        } else {
            sectionHandlerProc = parentSectionHandlerProc;
        }
        this->processChunk(sectionHandlerProc, chunk, resultObj);
    }


    void processChunkAtOffs (int &offs, T &resultObj, chunkDef<T> *parentSectionHandlerProc) {
        ChunkData chunk = this->loadChunkAtOffset(offs, -1);
        chunkDef<T> * sectionHandlerProc;
        if (parentSectionHandlerProc != nullptr) {
            sectionHandlerProc = &parentSectionHandlerProc->subChunks[chunk.chunkIdent];
        } else {
            sectionHandlerProc = parentSectionHandlerProc;
        }

        this->processChunk(sectionHandlerProc, chunk, resultObj);
    }

    void processChunk(chunkDef<T> * &sectionHandlerProc, ChunkData &chunk, T &resultObj) {
        sectionHandlerProc->handler(resultObj, chunk);

        /* Iteration through subchunks */
        if (chunk.chunkLen > 0) {
            int chunkLoadOffset = chunk.dataOffset;
            int chunkEndOffset = chunk.dataOffset + chunk.chunkLen;
            ChunkData subChunk;
            while (chunkLoadOffset < chunkEndOffset){
                subChunk = this->loadChunkAtOffset(chunkLoadOffset, -1);
                if (subChunk.chunkIdent == 0) break;

                chunkDef<T> *subchunkHandler = &sectionHandlerProc->subChunks[subChunk.chunkIdent];
                this->processChunk(subchunkHandler, subChunk, resultObj);
            }
        }
    }
    void processFile(T &resultObj) {
        int offset = 0;
        ChunkData chunk = this->loadChunkAtOffset(offset, -1);

        while (chunk.chunkIdent != 0) {
            chunkDef<T> * sectionHandlerProc = &m_sectionReaders->subChunks[chunk.chunkIdent];
            this->processChunk(sectionHandlerProc, chunk, resultObj);
            chunk = this->loadChunkAtOffset(offset, -1);
        }
    }

    ChunkData loadChunkAtOffset(int &offset, int size) {
        int offs = offset;

        /* Read chunk header */
        ChunkData chunkData;
        chunkData.currentOffset = offset;
        chunkData.fileData = &m_file[0];

        // 8 is length of chunk header

        if (offs + 8 < m_file.size()) {
            chunkData.readValue(chunkData.chunkIdent);
            chunkData.readValue(chunkData.chunkLen);

            if (size != -1) {
                chunkData.chunkLen = size;
            }
            chunkData.dataOffset = chunkData.currentOffset;
        }

        return chunkData;
    }
};



#endif //WOWVIEWERLIB_CCHUNKFILEREADER_H
