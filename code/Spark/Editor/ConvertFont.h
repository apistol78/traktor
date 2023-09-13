#pragma once

#include "Core/Ref.h"
#include "Spark/Editor/MovieAsset.h"

namespace traktor::spark
{

class Movie;

uint16_t convertFont(const traktor::Path& assetPath, const MovieAsset::Font& font, Movie* movie);

}
