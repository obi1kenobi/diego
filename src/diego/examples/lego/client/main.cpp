#include "LegoUniverse.h"

int
main()
{
    LegoUniverse universe(MfVec3i(100, 100, 100));
    LegoBrick *brick = 
        universe.CreateBrick(MfVec3i(0, 0, 0), 
                             MfVec3i(2, 2, 1),
                             LegoBrick::NORTH,
                             MfVec3f(1, 0, 0));
    brick->SetPosition(MfVec3i(1, 0, 0));
    return 0;
}
