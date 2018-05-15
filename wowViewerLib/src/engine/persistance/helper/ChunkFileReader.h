//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_CCHUNKFILEREADER_H
#define WOWVIEWERLIB_CCHUNKFILEREADER_H

#include <functional>
#include <map>
#include <vector>
#include <iostream>

//#define debuglog(x) std::cout<< x <<std::endl;
#define debuglog(x)

class CChunkFileReader;
typedef CChunkFileReader ChunkData;


template <typename T>
struct chunkDef {
    std::function<void (T&, ChunkData&)> handler;
    std::map<unsigned int,chunkDef> subChunks;
};

class CChunkFileReader {
protected:
    void *fileData;
    int regionSizeToProcess;

public:
    unsigned int chunkIdent;
    int chunkLen = -1;

    int chunkOffset = -1;
    int currentOffset = -1;
    int bytesRead = -1;
    int dataOffset = -1;
    void *chunkData;

public:
    CChunkFileReader() {
      }
    CChunkFileReader(std::vector<unsigned char> &file){
        regionSizeToProcess = file.size();
        fileData = &file[0];
    }

    template <typename T>
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

    template <typename T>
    void processChunkAtOffs (int offs, T &resultObj, chunkDef<T> *parentSectionHandlerProc) {
        ChunkData chunk = this->loadChunkAtOffset(offs, -1);
        chunkDef<T> * sectionHandlerProc;
        if (parentSectionHandlerProc != nullptr) {
            sectionHandlerProc = &parentSectionHandlerProc->subChunks[chunk.chunkIdent];
        } else {
            sectionHandlerProc = parentSectionHandlerProc;
        }

        this->processChunk(sectionHandlerProc, chunk, resultObj);
    }

    template <typename T>
    void processFile(T &resultObj, chunkDef<T> *sectionReaders) {
        loopOverSubChunks(sectionReaders, 0, regionSizeToProcess, resultObj);
    }

    ChunkData loadChunkAtOffset(int &offset, int size) {
        int offs = offset;

        /* Read chunk header */
        ChunkData chunkData;
        chunkData.chunkOffset = offset;
        chunkData.currentOffset = offset;
        chunkData.chunkIdent = 0;
        chunkData.fileData = fileData ;

        // 8 is length of chunk header

        if ((offs + 8 - dataOffset)  < regionSizeToProcess) {
            chunkData.readValue(chunkData.chunkIdent);
            chunkData.readValue(chunkData.chunkLen);
            chunkData.regionSizeToProcess = chunkData.chunkLen;

            chunkData.bytesRead = 0;

            if (size != -1) {
                chunkData.chunkLen = size;
            }
            chunkData.dataOffset = chunkData.currentOffset;
            offset = chunkData.dataOffset + chunkData.chunkLen;
        }

        return chunkData;
    }
private:

    template <typename T>
    void loopOverSubChunks(chunkDef<T> * &sectionHandlerProc, int chunkLoadOffset, int chunkEndOffset, T &resultObj){
        ChunkData subChunk;
        while ((chunkLoadOffset < chunkEndOffset)) {
            subChunk = this->loadChunkAtOffset(chunkLoadOffset, -1);
            if (subChunk.chunkIdent == 0) break;

            if (sectionHandlerProc->subChunks.count(subChunk.chunkIdent) > 0) {
                chunkDef<T> *subchunkHandler = &sectionHandlerProc->subChunks[subChunk.chunkIdent];
                this->processChunk(subchunkHandler, subChunk, resultObj);
            } else {
                char *indentPtr = (char *) &subChunk.chunkIdent;
                char indent[5] = { indentPtr[3], indentPtr[2], indentPtr[1], indentPtr[0], 0x0};
                debuglog("Handler for "<< indent << " was not found in "<< __PRETTY_FUNCTION__);
            }

            //TODO: HACK!
            if (chunkLoadOffset < 0) break;
        }
    }

    template <typename T>
    void processChunk(chunkDef<T> * &sectionHandlerProc, ChunkData &chunk, T &resultObj) {
        sectionHandlerProc->handler(resultObj, chunk);

        /* Iteration through subchunks */
        if (chunk.bytesRead < chunk.chunkLen) {
            if (sectionHandlerProc->subChunks.size() == 0) {
                char *indentPtr = (char *) &chunk.chunkIdent;
                char indent[5] = { indentPtr[3], indentPtr[2], indentPtr[1], indentPtr[0], 0x0};
                debuglog("Not all data was read from "<< indent << " chunk, parsed from "<< __PRETTY_FUNCTION__);
            } else {
                int chunkLoadOffset = chunk.dataOffset+chunk.bytesRead;
                int chunkEndOffset = chunk.dataOffset + chunk.chunkLen;

                loopOverSubChunks(sectionHandlerProc, chunkLoadOffset, chunkEndOffset, resultObj);
            }
        }
    }



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





#endif //WOWVIEWERLIB_CCHUNKFILEREADER_H
