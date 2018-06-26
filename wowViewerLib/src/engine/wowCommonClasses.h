//
// Created by deamon on 26.06.18.
//

#ifndef WEBWOWVIEWERCPP_WOWCOMMONCLASSES_H
#define WEBWOWVIEWERCPP_WOWCOMMONCLASSES_H
#include <memory>

//Persistance classes
class AdtFile;
class WdtFile;
class WdlFile;
class M2Geom;
class SkinGeom;
class BlpTexture;
class WmoMainGeom;
class WmoGroupGeom;
class DB2Base;

//Shared pointers for persistance classes
typedef std::shared_ptr<AdtFile> HAdtFile;
typedef std::shared_ptr<WdtFile> HWdtFile;
typedef std::shared_ptr<WdlFile> HWdlFile;
typedef std::shared_ptr<M2Geom> HM2Geom;
typedef std::shared_ptr<SkinGeom> HSkinGeom;
typedef std::shared_ptr<BlpTexture> HBlpTexture;
typedef std::shared_ptr<WmoMainGeom> HWmoMainGeom;
typedef std::shared_ptr<WmoGroupGeom> HWmoGroupGeom;
typedef std::shared_ptr<DB2Base> HDB2Base;;

#endif //WEBWOWVIEWERCPP_WOWCOMMONCLASSES_H
