#include "grove_resource.h"

namespace { GroveOwner owner = GroveOwner::NONE; }

bool acquireGrove(GroveOwner requested) {
    if (owner != GroveOwner::NONE && owner != requested) return false;
    owner = requested; return true;
}
void releaseGrove(GroveOwner requested) { if (owner == requested) owner = GroveOwner::NONE; }
GroveOwner groveOwner() { return owner; }
const char *groveOwnerName() {
    switch (owner) { case GroveOwner::CAN: return "CAN"; case GroveOwner::PROXMARK: return "PROXMARK"; default: return "LIBRE"; }
}
