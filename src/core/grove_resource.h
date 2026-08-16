#pragma once

enum class GroveOwner { NONE, CAN, PROXMARK };

bool acquireGrove(GroveOwner owner);
void releaseGrove(GroveOwner owner);
GroveOwner groveOwner();
const char *groveOwnerName();
