#pragma once

#include "Core/Config.h"

namespace traktor
{
    
void addNanoSecToTimeSpec(struct timespec* r, int64_t b);

void addMilliSecToTimeSpec(struct timespec* r, int64_t b);

}