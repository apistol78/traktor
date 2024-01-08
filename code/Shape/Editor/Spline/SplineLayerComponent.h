/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class TransformPath;

}

namespace traktor::model
{

class Model;

}

namespace traktor::shape
{

class T_DLLCLASS SplineLayerComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	virtual Ref< model::Model > createModel(const TransformPath& path, bool closed, bool preview) const = 0;
};

}
