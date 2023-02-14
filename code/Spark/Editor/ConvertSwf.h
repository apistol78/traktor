#pragma once

#include "Core/Ref.h"

namespace traktor
{

class IStream;

}

namespace traktor::db
{

class Instance;

}

namespace traktor::spark
{

class Movie;

Ref< Movie > convertSwf(const db::Instance* sourceInstance, IStream* sourceStream);

}
