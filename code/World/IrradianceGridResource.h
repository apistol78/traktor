/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

}

namespace traktor::render
{

class IRenderSystem;

}

namespace traktor::world
{

class IrradianceGrid;

class T_DLLCLASS IrradianceGridResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< IrradianceGrid > createInstance(IStream* stream, render::IRenderSystem* renderSystem) const;

	virtual void serialize(ISerializer& s) override final;
};

}
