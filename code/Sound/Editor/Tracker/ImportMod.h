/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

namespace traktor
{

class Path;

}

namespace traktor::db
{

class Group;

}

namespace traktor::sound
{

class ImportMod : public Object
{
	T_RTTI_CLASS;

public:
	bool import(const Path& fileName, const Path& assetPath, const Path& samplePath, db::Group* group) const;
};

}
