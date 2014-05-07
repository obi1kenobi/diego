#include "LegoUniverse.h"

int
main()
{
    LegoUniverse universe(MfVec3i(100, 100, 100));
    universe.CreateBrick(MfVec3i(0, 0, 0), 
                         MfVec3i(2, 2, 1),
                         LegoBrick::NORTH,
                         MfVec3f(1, 0, 0));
    LegoBrick *brick = universe.GetBrickAt(MfVec3i(0, 0, 0));
    assert(brick);
    brick->SetPosition(MfVec3i(1, 0, 0));
    delete brick;
    return 0;
}
