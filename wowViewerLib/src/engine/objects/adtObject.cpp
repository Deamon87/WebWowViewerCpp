//
// Created by deamon on 10.07.17.
//

#include "adtObject.h"


chunkDef<AdtObject> AdtObject::adtObjectTable = {

        handler : [](AdtObject& object, ChunkData& chunkData){},
        subChunks : {
                {
                        'MVER',
                        {
                                handler: [](AdtObject& object, ChunkData& chunkData){
                                    std::cout<<"Entered MVER"<<std::endl;
                                }
                        }
                },
                {
                        'MHDR',
                        {
                                handler: [](AdtObject& object, ChunkData& chunkData){
                                    std::cout<<"Entered MHDR"<<std::endl;
                                    chunkData.readValue(object.mhdr);
                                }
                        }
                },
                {
                        'MCIN',
                        {
                                handler: [](AdtObject& object, ChunkData& chunkData){
                                    std::cout<<"Entered MCIN"<<std::endl;
                                    chunkData.readValues(object.mcins, 16*16);
                                }
                        }
                },
                {
                        'MTEX',
                        {
                                handler: [](AdtObject& object, ChunkData& chunkData){
                                    std::cout<<"Entered MTEX"<<std::endl;
                                    object.textureNamesFieldLen = chunkData.chunkLen;
                                    chunkData.readValues(object.textureNamesField, object.textureNamesFieldLen);
                                }
                        }
                },
                {
                        'MMDX',
                        {
                                handler: [](AdtObject& object, ChunkData& chunkData){
                                    std::cout<<"Entered MMDX"<<std::endl;
                                    object.doodadNamesFieldLen = chunkData.chunkLen;
                                    chunkData.readValues(object.doodadNamesField, object.doodadNamesFieldLen);
                                }
                        }
                },
                {
                        'MMID',
                        {
                                handler: [](AdtObject& object, ChunkData& chunkData){
                                    std::cout<<"Entered MMID"<<std::endl;
                                }
                        }
                },
                {
                        'MWMO',
                        {
                                handler: [](AdtObject& object, ChunkData& chunkData){
                                    std::cout<<"Entered MWMO"<<std::endl;
                                    object.wmoNamesFieldLen = chunkData.chunkLen;
                                    chunkData.readValues(object.wmoNamesField, object.wmoNamesFieldLen);
                                }
                        }
                },
                {
                        'MWID',
                        {
                                handler: [](AdtObject& object, ChunkData& chunkData){
                                    std::cout<<"Entered MWID"<<std::endl;
                                }
                        }
                },
                {
                        'MDDF',
                        {
                                handler: [](AdtObject& object, ChunkData& chunkData){
                                    std::cout<<"Entered MDDF"<<std::endl;
                                }
                        }
                },
                {
                        'MODF',
                        {
                                handler: [](AdtObject& object, ChunkData& chunkData){
                                    std::cout<<"Entered MODF"<<std::endl;
                                }
                        }
                },
                {
                        'MCNK',
                        {
                                handler: [](AdtObject& object, ChunkData& chunkData){
                                    std::cout<<"Entered MCNK"<<std::endl;
                                },
                                subChunks : {
                                        {
                                                'MCVT',
                                                {
                                                        handler: [](AdtObject& object, ChunkData& chunkData){
                                                            std::cout<<"Entered MCVT"<<std::endl;
                                                        }
                                                }
                                        },
                                        {
                                                'MCLV',
                                                {
                                                        handler: [](AdtObject& object, ChunkData& chunkData){
                                                            std::cout<<"Entered MCLV"<<std::endl;
                                                        }
                                                }
                                        },
                                        {
                                                'MCCV',
                                                {
                                                        handler: [](AdtObject& object, ChunkData& chunkData){
                                                            std::cout<<"Entered MCCV"<<std::endl;
                                                        }
                                                }
                                        },
                                        {
                                                'MCNR',
                                                {
                                                        handler: [](AdtObject& object, ChunkData& chunkData){
                                                            std::cout<<"Entered MCNR"<<std::endl;
                                                        }
                                                }
                                        },
                                        {
                                                'MCLY',
                                                {
                                                        handler: [](AdtObject& object, ChunkData& chunkData){
                                                            std::cout<<"Entered MCLY"<<std::endl;
                                                        }
                                                }
                                        },
                                        {
                                                'MCRF',
                                                {
                                                        handler: [](AdtObject& object, ChunkData& chunkData){
                                                            std::cout<<"Entered MCRF"<<std::endl;
                                                        }
                                                }
                                        },
                                        {
                                                'MCAL',
                                                {
                                                        handler: [](AdtObject& object, ChunkData& chunkData){
                                                            std::cout<<"Entered MCAL"<<std::endl;
                                                        }
                                                }
                                        }
                                }
                        }
                },
        }

};

void AdtObject::process(std::vector<unsigned char> &adtFile) {
    CChunkFileReader<AdtObject> reader(adtFile, &AdtObject::adtObjectTable);
    reader.processFile(*this);

//    createVBO();
//    createIndexVBO();
}
