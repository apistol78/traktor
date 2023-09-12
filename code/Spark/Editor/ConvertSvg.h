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
class MovieAsset;

Ref< Movie > convertSvg(const traktor::Path& assetPath, const MovieAsset* movieAsset, const db::Instance* sourceInstance, IStream* sourceStream);

}
