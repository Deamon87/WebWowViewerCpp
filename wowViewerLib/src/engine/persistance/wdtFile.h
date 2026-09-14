#ifndef WEBWOWVIEWERCPP_WDTFILE_H_H
#define WEBWOWVIEWERCPP_WDTFILE_H_H

#include "header/wdtFileHeader.h"
#include "helper/ChunkFileReader.h"
#include "header/adtFileHeader.h"
#include "../../include/sharedFile.h"
#include "PersistentFile.h"


class WdtFile : public PersistentFile {
public:
    WdtFile(const std::string &fileName) : PersistentFile(fileName){};
    WdtFile(int fileDataId) : PersistentFile(fileDataId){};
	~WdtFile() {
//		std::cout << "destructor for WdtFile was called" << std::endl;
	}

    void process(HFileContent wdtFile) override;
public:
    MPHD *mphd = nullptr;
    struct {
        MAIN mainInfo[64][64];
    } *mapTileTable;

    PACK(
    struct MapFileDataIDs
    {
        uint32_t rootADT; // reference to fdid of mapname_xx_yy.adt
        uint32_t obj0ADT; // reference to fdid of mapname_xx_yy_obj0.adt
        uint32_t obj1ADT; // reference to fdid of mapname_xx_yy_obj1.adt
        uint32_t tex0ADT; // reference to fdid of mapname_xx_yy_tex0.adt
        uint32_t lodADT;  // reference to fdid of mapname_xx_yy_lod.adt
        uint32_t mapTexture; // reference to fdid of mapname_xx_yy.blp
        uint32_t mapTextureN; // reference to fdid of mapname_xx_yy_n.blp
        uint32_t minimapTexture; // reference to fdid of mapxx_yy.blp
    });
    struct {
        MapFileDataIDs array[64*64];
    } *mapFileDataIDs;

    std::string wmoFileName;
    SMMapObjDef *wmoDef = nullptr;

    HFileContent getFile() {return m_wdtFile;}

private:

    HFileContent m_wdtFile;
    static chunkDef<WdtFile> wdtFileTable;
};

#endif //WEBWOWVIEWERCPP_WDTFILE_H_H
