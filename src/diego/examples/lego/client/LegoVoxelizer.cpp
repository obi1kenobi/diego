#include "LegoVoxelizer.h"

#include "LegoUniverse.h"
#include "TriangleMesh.h"
#include "BBox3d.h"
#include "Ray3d.h"
#include "Vec3d.h"
#include "Vec3i.h"

#include <iostream>
#include <list>
#include <vector>

LegoVoxelizer::LegoVoxelizer(LegoUniverse *universe) :
    _universe(universe)
{
}

struct _Voxel {
    _Voxel() : inside(false) {}

    MfBBox3d bbox;
    typedef std::vector<uint32_t> TriangleList;
    TriangleList triangles;
    bool inside;
};

void
LegoVoxelizer::Voxelize(GfTriangleMesh *mesh,
                        int maxBrickUnits,
                        const MfVec3i &placementOrigin)
{
    maxBrickUnits -= 2;

    const std::vector<MfVec3d> &vertices = mesh->GetVertices();
    const std::vector<GfTriangle> &triangles = mesh->GetTriangles();
    MfBBox3d bbox = mesh->CalculateBBox();
    MfVec3d bboxSize = bbox.GetSize();

    MfVec3i numBricks(0);
    if (bboxSize[0] >= bboxSize[1] && bboxSize[0] >= bboxSize[2]) {
        numBricks[0] = maxBrickUnits;
        numBricks[1] = bboxSize[1] / bboxSize[0] * maxBrickUnits;
        numBricks[2] = bboxSize[2] / bboxSize[0] * maxBrickUnits;
    } else if (bboxSize[1] >= bboxSize[0] && bboxSize[1] >= bboxSize[2]) {
        numBricks[1] = maxBrickUnits;
        numBricks[0] = bboxSize[0] / bboxSize[1] * maxBrickUnits;
        numBricks[2] = bboxSize[2] / bboxSize[1] * maxBrickUnits;
    } else if (bboxSize[2] >= bboxSize[0] && bboxSize[2] >= bboxSize[1]) {
        numBricks[2] = maxBrickUnits;
        numBricks[0] = bboxSize[0] / bboxSize[2] * maxBrickUnits;
        numBricks[1] = bboxSize[1] / bboxSize[2] * maxBrickUnits;
    }

    // X, Y are double sized
    numBricks[0] /= 2;
    numBricks[1] /= 2;

    MfVec3d voxelSize;
    for (int i = 0; i < 3; ++i) {
        voxelSize[i] = bboxSize[i] / numBricks[i];
    }

#if 0
    voxelSize[2] = bboxSize[2] / double(_zBricks);
    voxelSize[0] = voxelSize[2] / 0.721519;
    voxelSize[1] = voxelSize[0];
#endif

    MfVec3i gridSize(bboxSize[0] / voxelSize[0] + 2,
                     bboxSize[1] / voxelSize[1] + 2,
                     bboxSize[2] / voxelSize[2] + 2);
    int gridVoxels = gridSize[0] * gridSize[1] * gridSize[2];

    std::cerr << "Mesh bbox: " << bbox << std::endl;
    std::cerr << "Grid size: " << gridSize[0] << "x" << gridSize[1] << "x"
        << gridSize[2] << std::endl;
    std::cerr << "Voxel size: " << voxelSize << std::endl;

    std::cerr << "Initializing." << std::endl;

    std::vector< std::vector< std::vector<_Voxel> > > voxels;
    voxels.resize(gridVoxels);
    for (int i = 0; i < gridSize[0]; ++i) {
        voxels[i].resize(gridSize[1]);
        for (int j = 0; j < gridSize[1]; ++j) {
            voxels[i][j].resize(gridSize[2]);
        }
    }

    for (int x = 0; x < gridSize[0]; ++x) {
        for (int y = 0; y < gridSize[1]; ++y) {
            for (int z = 0; z < gridSize[2]; ++z) {
                MfVec3d voxelMin = 
                    bbox.GetMin() + MfVec3d(x * voxelSize[0], 
                                       y * voxelSize[1], 
                                       z * voxelSize[2]);
                MfVec3d voxelMax = voxelMin + voxelSize;
                voxels[x][y][z].bbox = MfBBox3d(voxelMin, voxelMax);
            }
        }
    }

    std::cerr << "Building voxel grid." << std::endl;

    for (uint32_t t = 0; t < triangles.size(); ++t) {
        const GfTriangle &tri = triangles[t];
        MfBBox3d triBBox;
        for (int i = 0; i < 3; ++i) {
            triBBox.ExtendBy(vertices[tri.indices[i]]);
        }
        MfVec3d minPoint = triBBox.GetMin() - bbox.GetMin();
        MfVec3d maxPoint = triBBox.GetMax() - bbox.GetMin();
        int xStart = int(minPoint[0] / voxelSize[0]);
        int yStart = int(minPoint[1] / voxelSize[1]);
        int zStart = int(minPoint[2] / voxelSize[2]);
        int xEnd = int(maxPoint[0] / voxelSize[0]);
        int yEnd = int(maxPoint[1] / voxelSize[1]);
        int zEnd = int(maxPoint[2] / voxelSize[2]);
        if (xEnd == gridSize[0]) --xEnd;
        if (yEnd == gridSize[1]) --yEnd;
        if (zEnd == gridSize[2]) --zEnd;
        if (gridVoxels < 4) {
            std::cerr << "minPoint = " << minPoint << std::endl;
            std::cerr << "maxPoint = " << maxPoint << std::endl;
            std::cerr << "voxelSize = " << voxelSize << std::endl;
            std::cerr << "xStart = " << xStart << ", xEnd = " << xEnd << std::endl;
            std::cerr << "yStart = " << yStart << ", yEnd = " << yEnd << std::endl;
            std::cerr << "zStart = " << zStart << ", zEnd = " << zEnd << std::endl;
        }
        for (int x = xStart; x <= xEnd; ++x) {
            for (int y = yStart; y <= yEnd; ++y) {
                for (int z = zStart; z <= zEnd; ++z) {
                    if (voxels[x][y][z].bbox.Intersects(triBBox)) {
                        voxels[x][y][z].triangles.push_back(t);
                    }
                }
            }
        }
    }

    // We will keep track of which triangles we've already seen while ray
    // casting in this list of triangle specific flags.
    std::vector<int> haveSeen(triangles.size(), -1);

    std::cerr << "Solid voxelization." << std::endl;

    MfVec3d dir(voxelSize[0], 0.0f, 0.0f);
    for (int y = 0; y < gridSize[1]; ++y) {
        for (int z = 0; z < gridSize[2]; ++z) {
            bool inside = false;
            MfVec3d center = voxels[0][y][z].bbox.GetCenter();
            const int yzMark = y << 16 | z;
            for (int x = 0; x < gridSize[0]; ++x) {
                MfVec3d orig(voxels[x][y][z].bbox.GetMin()[0], center[1], center[2]);
                MfRay3d ray(orig, dir);
                if (!voxels[x][y][z].triangles.empty()) {
                    auto t = voxels[x][y][z].triangles.begin();
                    auto tEnd = voxels[x][y][z].triangles.end();
                    int beforeCenter = 0, afterCenter = 0;
                    for (; t != tEnd; ++t) {
                        const uint32_t triIndex = *t;
                        const GfTriangle &tri = triangles[triIndex];
                        double depth;
                        const double EPSILON = 0.00001;
                        if (ray.GetTriangleIntersection(&depth,
                                                        vertices[tri.indices[0]],
                                                        vertices[tri.indices[1]],
                                                        vertices[tri.indices[2]],
                                                        EPSILON) && 
                            depth >= 0.0f && depth < 1.0f) {
                            // Have we seen this triangle before along this x
                            // column of voxels?
                            if (haveSeen[triIndex] == yzMark) {
                                // Skip triangle; already processed along this column
                                continue;
                            } else {
                                haveSeen[triIndex] = yzMark;
                            }
                            if (depth < 0.5f) {
                                ++beforeCenter;
                            } else {
                                ++afterCenter;
                            }
                        }
                    }
                    if (beforeCenter % 2 == 1)
                        inside = !inside;
                    voxels[x][y][z].inside = true; // inside;
                    if (afterCenter % 2 == 1)
                        inside = !inside;
                } else {
                    voxels[x][y][z].inside = inside;
                }
                if (x == gridSize[0] - 1 && inside) {
                    std::cerr << "Runaway voxels for (y, z): " << y << ", " << z << std::endl;
                }
            }
        }
    }

    std::cerr << "Building voxelization mesh." << std::endl;

    static MfVec3f colors[] = {
        MfVec3f(1, 0, 0),
        MfVec3f(0, 1, 0),
        MfVec3f(0, 0, 1),
        MfVec3f(1, 1, 0),
        MfVec3f(1, 1, 1),
    };
    int numBrickColors = sizeof(colors) / sizeof(colors[0]);

    _universe->GetTransactionMgr()->OpenTransaction();
    bool brickMap[gridSize[0]][gridSize[1]][gridSize[2]];
    memset(brickMap, 0, sizeof(brickMap));
    for (int x = 0; x < gridSize[0]; ++x) {
        for (int y = 0; y < gridSize[1]; ++y) {
            for (int z = 0; z < gridSize[2]; ++z) {
                if (voxels[x][y][z].inside) {
#if 0
                    bool taken = false;
                    for (int bx = 0; bx < 2; ++bx) {
                        for (int by = 0; by < 2; ++by) {
                            for (int bz = 0; bz < 1; ++bz) {
                                if (brickMap[x + bx][y + by][z + bz]) {
                                    taken = true;
                                }
                            }
                        }
                    }
                    if (taken) {
                        continue;
                    }
#endif

                    int whichColor = drand48() * numBrickColors;
                    _universe->CreateBrick(placementOrigin + MfVec3i(2 * x, 2 * y, z),
                                           MfVec3i(2, 2, 1),
                                           LegoBrick::EAST,
                                           colors[whichColor]);
                    std::cerr << "CreateBrick " << 2 * x << ", " << 2 * y << ", " << z << std::endl;

#if 0
                    for (int bx = 0; bx < 2; ++bx) {
                        for (int by = 0; by < 2; ++by) {
                            for (int bz = 0; bz < 1; ++bz) {
                                brickMap[x + bx][y + by][z + bz] = true;
                            }
                        }
                    }
#endif
                }
            }
        }
    }
    _universe->GetTransactionMgr()->CloseTransaction();
}
