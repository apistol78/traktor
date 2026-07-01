/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"

namespace traktor
{

class IStream;
class Path;

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
