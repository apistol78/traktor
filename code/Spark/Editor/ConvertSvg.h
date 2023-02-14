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

Ref< Movie > convertSvg(const db::Instance* sourceInstance, IStream* sourceStream);

}
