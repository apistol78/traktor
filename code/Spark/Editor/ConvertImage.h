/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
