#ifndef __LEGO_VOXELIZER_H__
#define __LEGO_VOXELIZER_H__

class GfTriangleMesh;
class LegoUniverse;
class MfVec3i;

class LegoVoxelizer {
  public:
    LegoVoxelizer(LegoUniverse *universe);

    void Voxelize(GfTriangleMesh *mesh,
                  int maxBrickUnits,
                  const MfVec3i &placementOrigin);

  private:
    LegoUniverse *_universe;
};

#endif // __LEGO_VOXELIZER_H__
