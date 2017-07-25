//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_CCHUNKFILEREADER_H
#define WOWVIEWERLIB_CCHUNKFILEREADER_H

#include <functional>
#include <map>
#include <iostream>

class ChunkData {
public:
    ChunkData (){
        chunkIdent = 0;
        chunkLen = 0;
        currentOffset = 0;
        bytesRead = 0;

        chunkData = nullptr;
        fileData = nullptr;
    }
    unsigned int chunkIdent;
    int chunkLen;

    int currentOffset;
    int bytesRead;
    int dataOffset;
    void *chunkData;
    void *fileData;

public:
    template<typename T> inline void readValue(T &value) {
        static_assert(!std::is_pointer<T>::value, "T is a pointer");
        value = *(T*)&(((unsigned char *)fileData)[currentOffset]);
        currentOffset += sizeof(T);
        bytesRead += sizeof(T);
    }
    template<typename T> inline void readValue(T* &value) {
        static_assert(!std::is_pointer<T>::value, "T is a pointer");
        value = (T*)&(((unsigned char *)fileData)[currentOffset]);
        currentOffset += sizeof(T);
        bytesRead += sizeof(T);
    }
    template<typename T> inline void readValues(T* &value, int count) {
        static_assert(!std::is_pointer<T>::value, "T is a pointer");
        value = (T*)&(((unsigned char *)fileData)[currentOffset]);
        currentOffset += count*sizeof(T);
        bytesRead += count*sizeof(T);
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

    void loopOverSubChunks(chunkDef<T> * &sectionHandlerProc, int chunkLoadOffset, int chunkEndOffset, T &resultObj){
        ChunkData subChunk;
        while (chunkLoadOffset < chunkEndOffset) {
            subChunk = this->loadChunkAtOffset(chunkLoadOffset, -1);
            if (subChunk.chunkIdent == 0) break;

            if (sectionHandlerProc->subChunks.count(subChunk.chunkIdent) > 0) {
                chunkDef<T> *subchunkHandler = &sectionHandlerProc->subChunks[subChunk.chunkIdent];
                this->processChunk(subchunkHandler, subChunk, resultObj);
            } else {
                char *indentPtr = (char *) &subChunk.chunkIdent;
                char indent[5] = { indentPtr[3], indentPtr[2], indentPtr[1], indentPtr[0], 0x0};
                std::cout << "Handler for "<< indent << " was not found in "<< __PRETTY_FUNCTION__ << std::endl;
            }
        }
    }

    void processChunk(chunkDef<T> * &sectionHandlerProc, ChunkData &chunk, T &resultObj) {
        sectionHandlerProc->handler(resultObj, chunk);

        /* Iteration through subchunks */
        if (chunk.bytesRead < chunk.chunkLen) {
            if (sectionHandlerProc->subChunks.size() == 0) {
                char *indentPtr = (char *) &chunk.chunkIdent;
                char indent[5] = { indentPtr[3], indentPtr[2], indentPtr[1], indentPtr[0], 0x0};
                std::cout << "Not all data was read from "<< indent << " chunk, parsed from "<< __PRETTY_FUNCTION__ << std::endl;
            } else {
                int chunkLoadOffset = chunk.dataOffset+chunk.bytesRead;
                int chunkEndOffset = chunk.dataOffset + chunk.chunkLen;

                loopOverSubChunks(sectionHandlerProc, chunkLoadOffset, chunkEndOffset, resultObj);
            }
        }
    }
    void processFile(T &resultObj) {
        loopOverSubChunks(m_sectionReaders, 0, m_file.size(), resultObj);
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
            chunkData.bytesRead = 0;

            if (size != -1) {
                chunkData.chunkLen = size;
            }
            chunkData.dataOffset = chunkData.currentOffset;
            offset = chunkData.dataOffset + chunkData.chunkLen;
        }


        return chunkData;
    }
};



#endif //WOWVIEWERLIB_CCHUNKFILEREADER_H
