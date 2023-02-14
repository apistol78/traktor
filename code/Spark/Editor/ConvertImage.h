#pragma once

#include <string>
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

Ref< Movie > convertImage(const db::Instance* sourceInstance, IStream* sourceStream, const std::wstring& extension);

}
